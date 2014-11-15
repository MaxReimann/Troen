#version 130

uniform vec3 fenceColor;
uniform int modelID;
uniform float glowIntensity;
uniform float fadeOutFactor;

in float v_relWidth;

void main()
{
	float mixFactor = (0.5 - abs(v_relWidth - 0.5)) / 0.5;

	vec4 color = mix(
		vec4(fenceColor, 1.0),
		vec4(fenceColor, 0.10),
		mixFactor
	);

	color.w *= fadeOutFactor;

	gl_FragData[0] = color;

	//8bit int, 2 channels: select_group, attribute (f.e glowintensity for glow group)
	gl_FragData[1] = vec4(modelID, glowIntensity, 0, 0);
	return;
}