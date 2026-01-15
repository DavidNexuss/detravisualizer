#version 330 core

layout(location = 0) in vec3  aPosition;
layout(location = 1) in float aDegree;
layout(location = 2) in vec3  aColor;

uniform mat4 uViewProj;

uniform bool  uColorMode;
uniform bool  uPerspectivePointSize;
uniform float uBasePointSize;

out float fDegree;
out vec3  fColor;

void main()
{
    vec4 clipPos = uViewProj * vec4(aPosition, 1.0);
    gl_Position = clipPos;

    if (uPerspectivePointSize)
    {
        gl_PointSize = uBasePointSize / clipPos.w;
    }
    else
    {
        gl_PointSize = uBasePointSize;
    }

    fDegree = aDegree;

    if (uColorMode)
        fColor = aColor;
}
