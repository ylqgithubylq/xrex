#version 430 core



in vec3 color;

out vec4 finalColor;


void main()
{
//	float depth = gl_FragCoord.z; // [0.0, 1.0]
	vec2 pointCoordinate = gl_PointCoord; // [0.0, 1.0]
	vec2 distanceToCenter = 2 * (pointCoordinate - vec2(0.5, 0.5)); // [-1.0, 1.0]
	float distance = length(distanceToCenter);
	//vec3 colorOutput = smoothstep(vec3(0, 0, 0), color, vec3(distance, distance, distance));
	vec3 colorOutput = mix(color, vec3(0, 0, 0), min(distance, 1));
	finalColor = vec4(colorOutput, 1);
	//finalColor =  vec4(distance, distance, 1, 1);
}

