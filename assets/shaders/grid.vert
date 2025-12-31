#version 330 core

layout(location = 0) in vec3 aPosition; // quad vertices

out vec2 vUV;

uniform mat4 u_ViewProj;
uniform mat4 u_Transform;

void main()
{
    // Transform quad position
    vec4 worldPos = u_Transform * vec4(aPosition, 1.0);
    gl_Position = u_ViewProj * worldPos;

    // Compute UV in 0..1 range from XZ plane
    vUV = worldPos.xz; // or scale to some range if desired
}
