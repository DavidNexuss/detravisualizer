#version 330 core

layout(location = 0) in vec3  aPosition;
layout(location = 1) in float aDegree;

uniform mat4 uViewProj;
uniform vec3 uCameraPos;

out float fDegree;

void main()
{
    gl_Position = uViewProj * vec4(aPosition, 1.0);
    gl_PointSize = 6.0;
    fDegree = aDegree;
}
