#version 330 core

out vec4 FragColor;
in vec3 fColor;
in float fDegree;

uniform float uAlphaAmount;
uniform bool uColorMode;

vec3 heatmap(float t) {
    t = clamp(t, 0.0, 1.0);return clamp(vec3(1.5 - abs(4.0*t - 3.0),1.5 - abs(4.0*t - 2.0),1.5 - abs(4.0*t - 1.0)), 0.0, 1.0);
}

void main()
{
  if(uColorMode)
    FragColor = vec4(fColor, uAlphaAmount);
  else
    FragColor = vec4(heatmap(fDegree), 1.0);
}
