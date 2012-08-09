

uniform vec3 color;
uniform mat4 modelMatrix;
uniform mat4 viewMatrix;
uniform mat4 projectionMatrix;

#ifdef VS

in vec3 mPosition;
out vec3 vertexColor;
void main()
{
	vertexColor = ((modelMatrix * vec4(mPosition, 1.0)).xyz + 1.0) / 20 + color / 16;
	gl_Position = 
		projectionMatrix *
		viewMatrix *
		modelMatrix *
		vec4(mPosition, 1.0);
}

#endif


#ifdef FS

in vec3 vertexColor;
layout(location = 0) out vec4 finalColor;

void main()
{
	finalColor = vec4(vertexColor, 0.8);
}

#endif


	
