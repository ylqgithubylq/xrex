

uniform vec3 centerPosition;
uniform mat4 modelMatrix;
uniform mat4 viewMatrix;
uniform mat4 projectionMatrix;


#ifdef VS

in vec3 position;
out vec4 color;

void main()
{
	gl_Position = 
		projectionMatrix *
		viewMatrix *
		modelMatrix *
		vec4(position, 1.0);
	color = vec4(((modelMatrix * vec4(position, 1)).xyz) / length(centerPosition) / 2, 1);
}

#endif


#ifdef FS

layout(location = 0) out vec4 finalColor;

in vec4 color;

void main()
{
	finalColor = color;
}

#endif


	
