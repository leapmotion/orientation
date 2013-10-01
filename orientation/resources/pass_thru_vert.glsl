uniform sampler2D tex0;
uniform sampler2D tex1;
uniform sampler2D tex2;
uniform sampler2D tex3;

varying vec4 texCoord;

void main()
{

      texCoord = gl_MultiTexCoord0;
      gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex;

}
