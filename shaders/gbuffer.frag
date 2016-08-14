#version 130

uniform sampler2D colorTex;
uniform float objectID;

void main(void)
{
	vec4 texColor = vec4(0.0);

	const bool hasTexture = false;
	if (hasTexture)
	{
		texColor = texture2D(colorTex, gl_TexCoord[0].st);
	}
	gl_FragData[0] = vec4(1,1,1,1);//texColor; // sceneColor

	//8bit int, 2 channels: select_group, attribute (f.e glowintensity for glow group)
	gl_FragData[1] = vec4(objectID, 0, 0, 0);

	return;
}