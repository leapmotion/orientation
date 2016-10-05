uniform sampler2D tex0;
uniform sampler2D tex1;
uniform sampler2D noiseTex;

varying vec4 texCoord;
uniform float postMult;
uniform float t;
uniform float blurMult;

void main(void)
{

  // Sum colors from both textures
  vec3 color = blurMult*texture2D(tex0, texCoord.st).rgb + (1.0-blurMult)*texture2D(tex1, texCoord.st).rgb;

  // calculate some noise
  vec3 n = texture2D(noiseTex, texCoord.st).rgb;
  n = mod(n+vec3(t), 1.0) - 0.5;

  // Set average
  gl_FragColor.rgb = postMult * (color + (0.05*blurMult)*n);
  gl_FragColor.a = 1.0;

}
