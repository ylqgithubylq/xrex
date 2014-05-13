#version 430 core

layout (local_size_x = 1) in;

// (position.x, position.y, position.z, time)
layout (rgba32f) uniform readonly image1D PositionAndTime;
// (velocity.x, velocity.y, velocity.z, delta)
layout (rgba32f) uniform readonly image1D VelocityAndDelta;

// (position.x, position.y, position.z, time)
layout (rgba32f) uniform writeonly image1D NewPositionAndTime;

void main()
{
	vec4 positionAndTime = imageLoad(PositionAndTime, int(gl_GlobalInvocationID.x));
	vec4 velocityAndDelta = imageLoad(VelocityAndDelta, int(gl_GlobalInvocationID.x));

	vec4 newPositionAndTime = positionAndTime + vec4(velocityAndDelta.xyz * velocityAndDelta.w, velocityAndDelta.w);
	imageStore(NewPositionAndTime, int(gl_GlobalInvocationID.x), newPositionAndTime);
}

