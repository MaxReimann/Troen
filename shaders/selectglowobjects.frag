#version 130

uniform sampler2D sceneLayer;
uniform sampler2D idLayer;

void main(void)
{
	vec2 st = gl_TexCoord[0].st;
	float objectID = texture2D(idLayer, st).x;

	if (objectID != 1.f){ //todo comment back in
		gl_FragColor = vec4(0.f, 0.f, 0.f, 1.f);
		return;
	}

	vec4 sceneColor = texture2D(sceneLayer, st);
	gl_FragColor  =  vec4(1,1,1,1);
	// gl_FragData[1] = vec4(1, 1, 0, 0);


	return;
}