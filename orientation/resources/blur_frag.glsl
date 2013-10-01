#version 110

uniform sampler2D	tex0;
uniform vec2 sampleOffset;
uniform float glowContrast;

float weights[11];

void main() {
  weights[0] = 0.0181588;
  weights[1] = 0.0408404;
  weights[2] = 0.0767134;
  weights[3] = 0.120345;
  weights[4] = 0.157675;
  weights[5] = 0.172534;
  weights[6] = 0.157675;
  weights[7] = 0.120345;
  weights[8] = 0.0767134;
  weights[9] = 0.0408404;
  weights[10] = 0.0181588;

  vec3 sum = vec3(0.0, 0.0, 0.0);
  vec2 offset = vec2(0.0, 0.0);
  vec2 baseOffset = -5.0 * sampleOffset;

  for (int s = 0; s < 11; ++s) {
    sum += texture2D(tex0, gl_TexCoord[0].st + baseOffset + offset).rgb * weights[s];
    offset += sampleOffset;
  }

  sum = clamp(glowContrast*sum, 0.0, 1.0);

  gl_FragColor.rgb = sum;
  gl_FragColor.a = 1.0;
}
