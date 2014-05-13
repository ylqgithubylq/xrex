#version 430 core


in vec3 position;


out vec3 color;

const float NearPlanePointSize = 16;


void main()
{
	color = vec3(1, 0.7, 0.9);
	gl_Position = vec4(position, 1);
	float pointDepth = 1 - (position.z + 1) / 2;
	gl_PointSize = NearPlanePointSize / (pointDepth + 1);
}

