#version 330 core

layout(location = 0) in vec3  aPosition;
layout(location = 1) in float aDegree;
layout(location = 2) in vec3  aColor;

uniform mat4 uViewProj;
uniform vec3 uCameraPos;

out float fDegree;
out vec3 fColor;
uniform bool uColorMode;

void main()
{
    gl_Position = uViewProj * vec4(aPosition, 1.0);
    gl_PointSize = 6.0;
    fDegree = aDegree;

    if(uColorMode)
      fColor = aColor;
}
