

uniform vec3 color;
uniform mat4 wMatrix;
uniform mat4 vMatrix;
uniform mat4 pMatrix;

#ifdef VS

in vec3 mPosition;
out vec3 vertexColor;
void main()
{
	vertexColor = (mPosition + 1.0) / 2 + color / 16;
	gl_Position = 
		pMatrix *
		vMatrix *
		wMatrix *
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


	
