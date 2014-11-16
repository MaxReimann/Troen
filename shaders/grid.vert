#version 130

uniform int levelSize;
uniform float objectID;
uniform vec2 nearFar;
uniform bool bendingActivated;

void mainDeform(vec4);

out vec2 uv;
out vec3 vertex_objCoords;
out vec4 bendedVertex;

void main()
{

	uv = gl_Vertex.xy / levelSize + vec2(0.5, 0.5);

	vertex_objCoords = gl_Vertex.xyz;

	gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex ;

	// todo bended
	// theNormal = normalize(gl_NormalMatrix * gl_Normal);
	// linearDepth = (-(gl_ModelViewMatrix * gl_Vertex).z-nearFar.x)/(nearFar.y-nearFar.x);
	// gl_TexCoord[1] = vec4(objectID);
	if (bendingActivated)
		mainDeform(gl_Vertex);

	bendedVertex = gl_Position;

	return;
}