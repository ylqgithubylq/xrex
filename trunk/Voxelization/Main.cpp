#include <XREXAll.hpp>

#include "VoxelTest.h"

using namespace XREX;




#include <iostream>

void OutputList(std::vector<floatV4>& l, uint32 head)
{
	std::cout << "========" << std::endl;
	uint32 node = head;
	for (floatV4 value = l[head]; true; value = l[node])
	{
		std::cout << "[" << node << "]: " << "(" << value.X() << ", " << value.Y() << ", " << value.Z() << ", " << value.W() << ")" << std::endl;
		if (value.X() == 0)
		{
			break;
		}
		node = value.X();
	}
	std::cout << "========" << std::endl;
}

bool Compare(floatV4 left, floatV4 right)
{
	return left.Z() < right.Z();
}

struct Iterator
{
	floatV4 value;
	Iterator(floatV4 value)
		: value(value)
	{
	}
};

void WriteHead(std::vector<uint32>& heads, floatV2 coordinate, uint32 newHead)
{
	// write head
	std::cout << "write head: {" << coordinate.X() << "} [" << newHead << "]: " << std::endl;
	heads[coordinate.X()] = newHead;
}

void WriteNode(std::vector<floatV4>& nodePool, floatV4 value, uint32 index)
{
	std::cout << "write: [" << index << "]: " << "(" << value.X() << ", " << value.Y() << ", " << value.Z() << ", " << value.W() << ")" << std::endl;
	nodePool[index] = value;
//	assert(index != 0);
}

Iterator First(std::vector<uint32>& heads, floatV2 coordinate)
{
	return Iterator(floatV4(heads[coordinate.X()], 0, 0, 0));
}

bool HasNext(Iterator current)
{
	return current.value.X() != 0;
}

Iterator Next(std::vector<floatV4>& nodePool, Iterator current)
{
//	assert(current.value.X() != 0);
	return Iterator(nodePool[current.value.X()]);
}

Iterator Current(std::vector<floatV4>& nodePool, uint32 current)
{
	assert(current != 0);
	return Iterator(nodePool[current]);
}

uint32 NextAddress(Iterator current)
{
	return current.value.X();
}

struct LinkedListMergeSortter
{
	static const uint32 InsertionSortThresholdCount = 4;

	LinkedListMergeSortter(std::vector<uint32>& heads, std::vector<floatV4>& nodePool, floatV2 coordinate)
	{
		uint32 InsertionSortReducedPassCount = std::ceil(std::log(InsertionSortThresholdCount) / std::log(2));

		uint32 totalCount = InsertionSortPass(heads, nodePool, coordinate);

		uint32 passNeeded = std::ceil(std::log(totalCount) / std::log(2));
		
		for (uint32 pass = InsertionSortReducedPassCount; pass < passNeeded; ++pass)
		{
			std::cout << "before merge sort, pass: " << pass << std::endl;
			OutputList(nodePool, heads[coordinate.X()]);
			MergeSortPass(heads, nodePool, coordinate, pass);
		}

	}

	struct Cache
	{
		uint32 memoryIndex;
		uint32 cacheIndex;
		Cache()
		{
		}
		Cache(uint32 memoryIndex, uint32 cacheIndex)
			: memoryIndex(memoryIndex), cacheIndex(cacheIndex)
		{
		}
	};

	uint32 InsertionSortPass(std::vector<uint32>& heads, std::vector<floatV4>& nodePool, floatV2 coordinate)
	{
		std::array<floatV4, InsertionSortThresholdCount> nodeCache;
		std::array<Cache, InsertionSortThresholdCount> indexCache;
		uint32 totalCount = 0;

		bool firstWrite = true;

		Cache lastIndex = Cache(0, 0);
		floatV4 lastNode = floatV4::Zero;

		Iterator current = First(heads, coordinate);

		while (HasNext(current) == true)
		{
			uint32 countInPass = 0;
			for (; HasNext(current) && countInPass < InsertionSortThresholdCount; ++countInPass)
			{
				indexCache[countInPass] = Cache(NextAddress(current), countInPass);
				current = Next(nodePool, current);
				nodeCache[countInPass] = current.value;
				totalCount += 1;
			}
			assert(countInPass > 0);
			assert(countInPass <= InsertionSortThresholdCount);
			InsertionSort(indexCache, nodeCache, 0, countInPass);

			// here can do a sort on indexCache by memory index

			lastNode = floatV4(indexCache[0].memoryIndex, lastNode.Y(), lastNode.Z(), lastNode.W()); // lastNode.x = indexCache[0].memoryIndex

			for (uint32 j = 0; j < countInPass - 1; ++j)
			{
				uint32 index = indexCache[j].cacheIndex;
				nodeCache[index] = floatV4(indexCache[j + 1].memoryIndex, nodeCache[index].Y(), nodeCache[index].Z(), nodeCache[index].W()); // nodeCache[index].x = indexCache[j + 1].memoryIndex
			}

			if (firstWrite)
			{
				WriteHead(heads, coordinate, indexCache[0].memoryIndex);
				firstWrite = false;
			}
			else
			{
				WriteNode(nodePool, lastNode, lastIndex.memoryIndex);
			}

			for (uint32 j = 0; j < countInPass - 1; ++j)
			{ // write all node back except last one
				WriteNode(nodePool, nodeCache[indexCache[j].cacheIndex], indexCache[j].memoryIndex);
			}

			lastNode = nodeCache[indexCache[countInPass - 1].cacheIndex];
			lastIndex = indexCache[countInPass - 1];
		}

		if (lastIndex.memoryIndex != 0)
		{ // write the last node
			lastNode = floatV4(0, lastNode.Y(), lastNode.Z(), lastNode.W()); // lastNode.x = 0
			WriteNode(nodePool, lastNode, lastIndex.memoryIndex);
		}

		return totalCount;
	}

	void InsertionSort(std::array<Cache, InsertionSortThresholdCount>& indices, std::array<floatV4, InsertionSortThresholdCount>& toSort, uint32 start, uint32 end)
	{
		for (uint32 j = start + 1; j < end; ++j)
		{
			floatV4 value = toSort[j];
			Cache index = indices[j];
			int32 i = j - 1;
			while (i >= 0 && Compare(value, toSort[indices[i].cacheIndex]))
			{
				indices[i + 1] = indices[i];
				i -= 1;
			}
			indices[i + 1] = index;
		}
	}

	/*
	 *	One pass of MergeSort. sort every pair of (1 << pass) nodes in to a 2 * (1 << pass) sorted list.
	 *	@pass: should start from 0.
	 */
	void MergeSortPass(std::vector<uint32>& heads, std::vector<floatV4>& nodePool, floatV2 coordinate, uint32 pass)
	{
		Iterator head = First(heads, coordinate);
		if (HasNext(head) == false)
		{
			return;
		}
		uint32 countPerPass = 1 << pass;

		uint32 pToWrite = 0;
		Iterator toWrite(floatV4::Zero);

		bool firstNodeWrite = true;

		uint32 start = NextAddress(head);

		while (true)
		{
			uint32 pLeft = start;
			uint32 leftSize = 0;

			uint32 pRight = pLeft;
			uint32 rightSize = countPerPass; // at most

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
					toWrite.value = floatV4(start, toWrite.value.Y(), toWrite.value.Z(), toWrite.value.W()); // toWrite.value.x = pLeft
					WriteNode(nodePool, toWrite.value, pToWrite); // write the last cached node
				}
				break;
			}

			Iterator left = Current(nodePool, pLeft);
			Iterator right = Current(nodePool, pRight);

			while (leftSize > 0 && rightSize > 0 && pRight != 0)
			{
				Iterator small(floatV4::Zero);
				uint32 pSmall = 0;
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
				toWrite.value = floatV4(pSmall, toWrite.value.Y(), toWrite.value.Z(), toWrite.value.W()); // toWrite.value.x = pSmall
				if (firstNodeWrite)
				{ // first node pointer is in heads
					WriteHead(heads, coordinate, toWrite.value.X());
					firstNodeWrite = false;
				}
				else
				{
					WriteNode(nodePool, toWrite.value, pToWrite);
					assert(nodePool[pToWrite] == toWrite.value);
				}
				// after write in to memory, move toWrite to the small
				toWrite = small;
				pToWrite = pSmall;
			}

			if (leftSize > 0)
			{ // link left list to the last node
				toWrite.value = floatV4(pLeft, toWrite.value.Y(), toWrite.value.Z(), toWrite.value.W()); // toWrite.value.x = pLeft
				start = pRight; // pRight points to the remaining list
			}
			else
			{ // link right list to the last node
				assert(rightSize > 0);
				assert(pRight != 0);
				toWrite.value = floatV4(pRight, toWrite.value.Y(), toWrite.value.Z(), toWrite.value.W()); // toWrite.value.x = pRight
			}
			assert(!firstNodeWrite);
			WriteNode(nodePool, toWrite.value, pToWrite);

			while (leftSize > 0)
			{ // let toWrite follow the rest of left list to the end
				assert(rightSize == 0 || pRight == 0);
				toWrite = left;
				pToWrite = pLeft;

				pLeft = NextAddress(left);
				left = Next(nodePool, left);
				leftSize -= 1;

			}
			while (rightSize > 0 && pRight != 0)
			{ // let toWrite follow the rest of right list to the end
				assert(pRight != 0);
				assert(leftSize == 0);
				toWrite = right;
				pToWrite = pRight;

				pRight = NextAddress(right);
				right = Next(nodePool, right);
				rightSize -= 1;

				start = pRight; // pRight points to the remaining list
			}

		}

	}

};

struct Pairinger
{
	static const int ObjectUnpairedMaxCount = 16;

	uint32 SetFrontFacing(uint32 value, bool frontFacing)
	{
		return value | (frontFacing ? (1 << 31) : 0);
	}
	bool GetFrontFacing(Iterator i)
	{
		return bool(int(i.value.Z()) & (1 << 31)); // frontFacing at highest bit
	}
	bool GetFrontFacing(uint32 value)
	{
		return bool(value & (1 << 31)); // frontFacing at highest bit
	}

	int GetObjectID(Iterator i)
	{
		return int(int(i.value.Z()) & (0x7fffffff)); // first bit is frontFacing
	}
	int GetObjectID(uint32 value)
	{
		return int(value & (0x7fffffff)); // first bit is frontFacing
	}

	struct Unpaired
	{
		uint32 objectIDAndFrontFacing;
		float depth;
		Unpaired()
		{
		}
		Unpaired(uint32 objectIDAndFrontFacing, float depth)
			: objectIDAndFrontFacing(objectIDAndFrontFacing), depth(depth)
		{
		}
	};

	/*
	 *	@return: -1 indicates unpaired.
	 */
	float Pairing(std::array<Unpaired, ObjectUnpairedMaxCount>& objectUnpairedBuffer, int& size, uint32 objectIDAndFrontFacing, float depth)
	{
		float pairedDepth = -1;

		int objectID = GetObjectID(objectIDAndFrontFacing);
		bool frontFacing = GetFrontFacing(objectIDAndFrontFacing);
		// find
		int pairedIndex = -1;
		for (int i = size - 1; i >= 0; --i)
		{
			if (GetObjectID(objectUnpairedBuffer[i].objectIDAndFrontFacing) == objectID)
			{ // found
				pairedIndex = i;
				break;
			}
		}

		if (pairedIndex != -1 && GetFrontFacing(objectUnpairedBuffer[pairedIndex].objectIDAndFrontFacing) != frontFacing)
		{ // found, remove paired one if frontFacing is not equal
			pairedDepth = objectUnpairedBuffer[pairedIndex].depth;
			for (int i = pairedIndex; i < size - 1; ++i)
			{
				objectUnpairedBuffer[i] = objectUnpairedBuffer[i + 1];
			}
			size -= 1;
		}
		else if (size < ObjectUnpairedMaxCount)
		{ // insert
			objectUnpairedBuffer[size] = Unpaired(objectIDAndFrontFacing, depth);
			size += 1;
		} // else simply discarded

		return pairedDepth;
	}

	void Test()
	{
		std::array<Unpaired, ObjectUnpairedMaxCount> objectUnpairedBuffer;
		int size = 0;
		float result00 = Pairing(objectUnpairedBuffer, size, SetFrontFacing(0, true), 0.5f);
		float result01 = Pairing(objectUnpairedBuffer, size, SetFrontFacing(0, false), 0.51f);

		float result10 = Pairing(objectUnpairedBuffer, size, SetFrontFacing(0, true), 0.5f);
		float result11 = Pairing(objectUnpairedBuffer, size, SetFrontFacing(1, true), 0.52f);
		float result12 = Pairing(objectUnpairedBuffer, size, SetFrontFacing(1, false), 0.53f);
		float result13 = Pairing(objectUnpairedBuffer, size, SetFrontFacing(0, false), 0.54f);

		float result20 = Pairing(objectUnpairedBuffer, size, SetFrontFacing(0, true), 0.5f);
		float result21 = Pairing(objectUnpairedBuffer, size, SetFrontFacing(1, true), 0.52f);
		float result22 = Pairing(objectUnpairedBuffer, size, SetFrontFacing(0, false), 0.53f);
		float result23 = Pairing(objectUnpairedBuffer, size, SetFrontFacing(1, false), 0.54f);

		float result30 = Pairing(objectUnpairedBuffer, size, SetFrontFacing(0, true), 0.5f);
		float result31 = Pairing(objectUnpairedBuffer, size, SetFrontFacing(1, true), 0.52f);
		float result32 = Pairing(objectUnpairedBuffer, size, SetFrontFacing(0, true), 0.53f);
		float result34 = Pairing(objectUnpairedBuffer, size, SetFrontFacing(1, true), 0.54f);
		float result35 = Pairing(objectUnpairedBuffer, size, SetFrontFacing(1, false), 0.55f);
		float result36 = Pairing(objectUnpairedBuffer, size, SetFrontFacing(1, false), 0.56f);
		float result37 = Pairing(objectUnpairedBuffer, size, SetFrontFacing(0, false), 0.57f);
		float result38 = Pairing(objectUnpairedBuffer, size, SetFrontFacing(0, false), 0.58f);
	}
};


void test()
{
	std::vector<uint32> h;

	std::vector<floatV4> v;
	uint32 head = 9;
	uint32 head2 = 10;
	h.push_back(head);
	h.push_back(head2);

	v.push_back(floatV4(0, 0, 0, 0)); // as null

	//	v.push_back(floatV4(0, 0, 2, 0)); // 1

	v.push_back(floatV4(0, 0, 7, 0)); // 1
	v.push_back(floatV4(0, 0, 2, 0)); // 2
	v.push_back(floatV4(1, 0, 5, 0)); // 3
	v.push_back(floatV4(2, 0, 7, 0)); // 4
	v.push_back(floatV4(3, 0, 8, 0)); // 5
	v.push_back(floatV4(4, 0, 1, 0)); // 6
	v.push_back(floatV4(5, 0, 5, 0)); // 7
	v.push_back(floatV4(6, 0, 3, 0)); // 8
	v.push_back(floatV4(7, 0, 6, 0)); // 9
	v.push_back(floatV4(8, 0, 3, 0)); // 10

	OutputList(v, h[0]);
	OutputList(v, h[1]);
	//InsertionSortter s(v);
	LinkedListMergeSortter ls2(h, v, floatV2(1, 0));
	LinkedListMergeSortter ls1(h, v, floatV2(0, 0));
	OutputList(v, h[0]);
	OutputList(v, h[1]);
	return;
}

int main()
{
	// test();
	// Pairinger p;
	// p.Test();
	VoxelTest v;
	return 0;
}

#define MEMORY_LEAK_CHECK
#ifdef MEMORY_LEAK_CHECK
#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>
#include <iostream>

struct DML
{
	DML()
	{
		_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
	}
// 	~DML()
// 	{
// 		if (_CrtDumpMemoryLeaks())
// 		{
// 			std::cout << "memory leaks." << std::endl;
// 		}
// 		else
// 		{
// 			std::cout << "no memory leaks." << std::endl;
// 		}
// 		std::cin.get();
// 	}
} _dml;

#endif // MEMORY_LEAK_CHECK
