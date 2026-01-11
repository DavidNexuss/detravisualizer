#version 330 core

out vec4 FragColor;

uniform vec3 uColor;

void main()
{
    FragColor = vec4(uColor == vec3(0.0) ? vec3(0.2, 0.6, 1.0) : uColor, 1.0);
}
