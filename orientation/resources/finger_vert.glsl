uniform vec3 eyePos;
uniform mat4 mvpMatrix;
uniform mat4 mMatrix;

varying vec3 vModelPos;
varying vec4 vVertex;
varying vec3 vNormal;
varying vec3 vEyeDir;

void main()
{
	vModelPos		= gl_Vertex.xyz;
	vVertex			= mMatrix * gl_Vertex;
	vNormal			= normalize( vec3( mMatrix * vec4( gl_Normal, 0.0 ) ) );
//	vNormal			= normalize( vec3( mMatrix * vec4( normalize(gl_NormalMatrix * gl_Normal), 0.0 ) ) );
	
	vEyeDir			= normalize( eyePos - vVertex.xyz );
	

	gl_Position		= mvpMatrix * vVertex;
	gl_TexCoord[0]	= gl_MultiTexCoord0;
}