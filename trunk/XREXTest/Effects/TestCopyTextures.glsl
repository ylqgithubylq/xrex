


#ifdef VS

in vec2 position;
out vec2 textureCoordinate;

void main()
{
	textureCoordinate = (position + vec2(1, 1)) / 2;
	gl_Position = vec4(position, 0, 1);
}

#endif

uniform sampler2D color;
uniform sampler2D normal;
uniform sampler2D depthInColor;
uniform sampler2D depth;

#ifdef FS

in vec2 textureCoordinate;
out vec4 XREX_DefaultFrameBufferOutput;

//#define ONE_OUTPUT

void main()
{
	vec4 outputColor = vec4(0, 0, 0, 1);
#ifndef ONE_OUTPUT
	if (textureCoordinate.x < 0.5)
	{
		if (textureCoordinate.y < 0.5)
		{
			outputColor = texture(color, textureCoordinate * 2) + vec4(textureCoordinate * 2, 0, 0);
		}
		else
		{
			outputColor.xyz = texture(normal, (textureCoordinate - vec2(0, 0.5)) * 2).xyz;
		}
	}
	else
	{
		if (textureCoordinate.y < 0.5)
		{
			outputColor.xyz = (texture(depthInColor, (textureCoordinate - vec2(0.5, 0)) * 2).xxx) * gl_FragCoord.w;
		}
		else
		{
			outputColor.xyz = (vec3(1, 1, 1) - texture(depth, (textureCoordinate - vec2(0.5, 0.5)) * 2).xxx) * gl_FragCoord.w * 20;
		}
	}
#else
	outputColor = texture(normal, textureCoordinate);
#endif
	XREX_DefaultFrameBufferOutput = outputColor;
}

#endif


	
