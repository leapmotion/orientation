uniform vec3 col;
uniform float time;
uniform float alpha;

varying vec3 vModelPos;
varying vec4 vVertex;
varying vec3 vNormal;
varying vec3 vEyeDir;

void main()
{
	vec3 red		= vec3( 1.0, 0.3, 0.15 );
	vec3 orange		= vec3( 1.0, 0.5, 0.0 );
	vec3 blue		= vec3( 0.4, 0.7, 1.0 );
	
	float ppEyeDiff		= max( dot( vNormal, vEyeDir ), 0.05 );
	float ppEyeFres		= pow( 1.0 - ppEyeDiff, 1.0 );
	float ppEyeSpec		= pow( ppEyeDiff, 50.0 );
	
	
	float val1			= max( sin( vModelPos.y * 2.0 + time * 95.0 ) * 0.5 - 0.15, 0.0 );
	float val2			= max( sin( vModelPos.y * 1.1 + time * 55.0 ) * 0.5 - 0.15, 0.0 );
	float edgeGlow		= pow( ( 1.0 - sin( gl_TexCoord[0].t * 3.14159 ) ), 5.0 );
	edgeGlow += pow( edgeGlow, 5.0 );
	
	gl_FragColor.rgb	= ( vec3( ppEyeDiff * col * val1 + ppEyeFres * 0.1 ) + ppEyeSpec + val2 * ppEyeDiff * blue + edgeGlow * blue * 0.5 );
	gl_FragColor.a		= alpha;//val1 + val2;
}

