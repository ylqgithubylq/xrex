#version 430 core

#define OBJECT_COUNT 1024
//#define OBJECT_COUNT 512
const uint ObjectCount = OBJECT_COUNT;
layout (local_size_x = OBJECT_COUNT) in;



uniform TimeBuffer
{
	float delta;
} Time;

readonly buffer PositionBuffer
{
	vec3[ObjectCount] position;
} Position;

readonly buffer VelocityBuffer
{
	vec3[ObjectCount] velocity;
} Velocity;

writeonly buffer NewPositionBuffer
{
	vec3[ObjectCount] position;
} NewPosition;

writeonly buffer NewVelocityBuffer
{
	vec3[ObjectCount] velocity;
} NewVelocity;

shared vec3[ObjectCount] Positions;

const float GxMScale = 6.6732e-11 * 1000 * 1000 * 10;


void main()
{
	int id = int(gl_GlobalInvocationID.x);

	Positions[id] = Position.position[id];
	// make sure all the operation are finished for threads in this work group
	barrier();

	vec3 position = Position.position[id];
	vec3 velocity = Velocity.velocity[id];

	vec3 acceleration = vec3(0, 0, 0);
	// accumulate acceleration
	for (uint i = 0; i < ObjectCount; ++i)
	{
		vec3 distance = Positions[i] - position;
		// a = m1 * direction / distance ^ 2
		acceleration += GxMScale * distance / (length(distance) + 1); // add 1 to avoid large acceleration
	}

	vec3 velocityDelta = acceleration * Time.delta * 0.999;
	vec3 newVelocity = velocity + velocityDelta;

	vec3 newPosition = newVelocity * Time.delta + position;

	NewPosition.position[id] = newPosition;
	NewVelocity.velocity[id] = newVelocity;
}

