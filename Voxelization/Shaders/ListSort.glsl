
layout (r32ui) uniform uimage2D heads; // edge size twice larges than intermediateVolume
layout (rgba32ui) uniform uimageBuffer nodePool;

#ifdef VS

in vec2 position;

void main()
{
	gl_Position = vec4(position, 0, 1);
}

#endif

#ifdef FS

const uint InsertionSortThresholdCount = 32;


struct Iterator
{
	uvec4 value;
};


float GetDepth(Iterator i)
{
	return uintBitsToFloat(i.value.y);
}


bool Compare(uvec4 left, uvec4 right)
{
	return GetDepth(Iterator(left)) < GetDepth(Iterator(right));
}


void WriteHead(layout (r32ui) uimage2D heads, ivec2 coordinate, uint newHead)
{
	imageStore(heads, coordinate, uvec4(newHead, 0 ,0 ,0));
}

void WriteNode(layout (rgba32ui) uimageBuffer nodePool, uvec4 value, uint index)
{
	imageStore(nodePool, int(index), value);
}

Iterator First(layout (r32ui) uimage2D heads, ivec2 coordinate)
{
	return Iterator(imageLoad(heads, coordinate));
}

bool HasNext(Iterator current)
{
	return current.value.x != 0;
}

Iterator Next(layout (rgba32ui) uimageBuffer nodePool, Iterator current)
{
	return Iterator(imageLoad(nodePool, int(current.value.x)));
}

Iterator Current(layout (rgba32ui) uimageBuffer nodePool, uint current)
{
	return Iterator(imageLoad(nodePool, int(current)));
}

uint NextAddress(Iterator current)
{
	return current.value.x;
}

struct Cache
{
	uint memoryIndex;
	uint cacheIndex;
};


void InsertionSort(inout Cache indices[InsertionSortThresholdCount], inout uvec4 toSort[InsertionSortThresholdCount], uint start, uint end)
{
	for (uint j = start + 1; j < end; ++j)
	{
		uvec4 value = toSort[j];
		Cache index = indices[j];
		int i = int(j - 1);
		while (i >= 0 && Compare(value, toSort[indices[i].cacheIndex]))
		{
			indices[i + 1] = indices[i];
			i -= 1;
		}
		indices[i + 1] = index;
	}
}

uint InsertionSortPass(layout (r32ui) uimage2D heads, layout (rgba32ui) uimageBuffer nodePool, ivec2 coordinate)
{
	uvec4 nodeCache[InsertionSortThresholdCount];
	Cache indexCache[InsertionSortThresholdCount];
	uint totalCount = 0;

	bool firstWrite = true;

	Cache lastIndex = Cache(0, 0);
	uvec4 lastNode = uvec4(0, 0, 0, 0);

	Iterator current = First(heads, coordinate);

	while (HasNext(current) == true)
	{
		uint countInPass = 0;
		for (countInPass = 0; HasNext(current) && countInPass < InsertionSortThresholdCount; ++countInPass)
		{
			indexCache[countInPass] = Cache(NextAddress(current), countInPass);
			current = Next(nodePool, current);
			nodeCache[countInPass] = current.value;
			totalCount += 1;
		}
		InsertionSort(indexCache, nodeCache, 0, countInPass);

		// here can do a sort on indexCache by memory index

		lastNode.x = indexCache[0].memoryIndex;

		for (uint j = 0; j < countInPass - 1; ++j)
		{
			uint index = indexCache[j].cacheIndex;
			nodeCache[index].x = indexCache[j + 1].memoryIndex;
		}

		if (firstWrite == true)
		{
			WriteHead(heads, coordinate, indexCache[0].memoryIndex);
			firstWrite = false;
		}
		else
		{
			WriteNode(nodePool, lastNode, lastIndex.memoryIndex);
		}

		for (uint j = 0; j < countInPass - 1; ++j)
		{ // write all node back except last one
			WriteNode(nodePool, nodeCache[indexCache[j].cacheIndex], indexCache[j].memoryIndex);
		}

		lastNode = nodeCache[indexCache[countInPass - 1].cacheIndex];
		lastIndex = indexCache[countInPass - 1];
	}

	if (lastIndex.memoryIndex != 0)
	{ // write the last node
		lastNode.x = 0;
		WriteNode(nodePool, lastNode, lastIndex.memoryIndex);
	}

	return totalCount;
}


/*
 *	One pass of MergeSort. sort every pair of (1 << pass) nodes in to a 2 * (1 << pass) sorted list.
 *	@pass: should start from 0.
 */
void MergeSortPass(layout (r32ui) uimage2D heads, layout (rgba32ui) uimageBuffer nodePool, ivec2 coordinate, uint pass)
{
	Iterator head = First(heads, coordinate);
	if (HasNext(head) == false)
	{
		return;
	}
	uint countPerPass = 1 << pass;

	uint pToWrite = 0;
	Iterator toWrite = Iterator(uvec4(0, 0, 0, 0));

	bool firstNodeWrite = true;

	uint start = NextAddress(head);

	while (true != false)
	{
		uint pLeft = start;
		uint leftSize = 0;

		uint pRight = pLeft;
		uint rightSize = countPerPass; // at most

		while (leftSize < countPerPass && pRight != 0)
		{ // move pRight countPerPass steps, so pLeft can have a list of size countPerPass
			Iterator right = Current(nodePool, pRight);
			pRight = NextAddress(right);
			leftSize += 1;
		}

		if (pRight == 0)
		{ // right list is empty, so already sorted
			// append left list
			if (pToWrite != 0)
			{
				toWrite.value.x = pLeft;
				WriteNode(nodePool, toWrite.value, pToWrite); // write the last cached node
			}
			break;
		}

		Iterator left = Current(nodePool, pLeft);
		Iterator right = Current(nodePool, pRight);

		while (leftSize > 0 && rightSize > 0 && pRight != 0)
		{
			Iterator small = Iterator(uvec4(0, 0, 0, 0));
			uint pSmall = 0;
			if (Compare(left.value, right.value))
			{ // use left as small, move forward left
				small = left;
				pSmall = pLeft;
				pLeft = NextAddress(left);
				left = Next(nodePool, left);
				leftSize -= 1;
			}
			else
			{ // use right as small, move forward right
				small = right;
				pSmall = pRight;
				pRight = NextAddress(right);
				right = Next(nodePool, right);
				rightSize -= 1;
			}
			toWrite.value.x = pSmall;
			if (firstNodeWrite == true)
			{ // first node pointer is in heads
				WriteHead(heads, coordinate, toWrite.value.x);
				firstNodeWrite = false;
			}
			else
			{
				WriteNode(nodePool, toWrite.value, pToWrite);
			}
			// after write in to memory, move toWrite to the small
			toWrite = small;
			pToWrite = pSmall;
		}

		if (leftSize > 0)
		{ // link left list to the last node
			toWrite.value.x = pLeft;
			start = pRight; // pRight points to the remaining list
		}
		else
		{ // link right list to the last node
			toWrite.value.x = pRight;
		}
		WriteNode(nodePool, toWrite.value, pToWrite);

		while (leftSize > 0)
		{ // let toWrite follow the rest of left list to the end
			toWrite = left;
			pToWrite = pLeft;

			pLeft = NextAddress(left);
			left = Next(nodePool, left);
			leftSize -= 1;

		}
		while (rightSize > 0 && pRight != 0)
		{ // let toWrite follow the rest of right list to the end
			toWrite = right;
			pToWrite = pRight;

			pRight = NextAddress(right);
			right = Next(nodePool, right);
			rightSize -= 1;

			start = pRight; // pRight points to the remaining list
		}

	}

}



void LinkedListHybridMergeSort(layout (r32ui) uimage2D heads, layout (rgba32ui) uimageBuffer nodePool, ivec2 coordinate)
{
	const uint InsertionSortReducedPassCount = uint(ceil(log(InsertionSortThresholdCount) / log(2)));

	uint totalCount = InsertionSortPass(heads, nodePool, coordinate);

	uint passNeeded = uint(max(ceil(log(totalCount) / log(2)), 0));
		
	for (uint pass = InsertionSortReducedPassCount; pass < passNeeded; ++pass)
	{
		MergeSortPass(heads, nodePool, coordinate, pass);
	}

}


void main()
{
	ivec2 coordinate = ivec2(gl_FragCoord.xy);
	// LinkedListHybridMergeSort(heads, nodePool, coordinate);
}

#endif

