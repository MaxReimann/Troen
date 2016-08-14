// dummy transform when bendedviews are not present
void mainDeform(vec4 vertex) {

	gl_Position = gl_ModelViewProjectionMatrix  * gl_Vertex;	
}
