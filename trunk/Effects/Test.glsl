

uniform vec3 color;
uniform vec3 centerPosition;
uniform mat4 modelMatrix;
uniform mat4 viewMatrix;
uniform mat4 projectionMatrix;

#ifdef VS

in vec3 mPosition;
out vec3 vertexColor;
void main()
{
	vertexColor = ((viewMatrix * modelMatrix * vec4(mPosition - centerPosition, 1.0)).xyz / 400 + 1.0) + color / 16;
	vertexColor = (mPosition - centerPosition) + 1.0 + color / 16;
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


	
