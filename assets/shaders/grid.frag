#version 330 core
in vec2 vUV; 
out vec4 FragColor;

uniform mat4 invViewProj;
uniform vec3 cameraPos;

uniform float gridSpacing;
uniform float lineRadius;

uniform vec3 gridColor;
uniform vec3 backgroundColor;

vec3 reconstructRay(vec2 uv)
{
    vec4 ndc = vec4(uv * 2.0 - 1.0, 1.0, 1.0); 
    vec4 world = invViewProj * ndc;
    world /= world.w;
    return normalize(world.xyz - cameraPos);
}

void main()
{
    vec3 ro = cameraPos;
    vec3 rd = reconstructRay(vUV);

    if (abs(rd.y) < 1e-5) discard;

    float t = -ro.y / rd.y;
    if (t <= 0.0) discard;

    vec3 hit = ro + rd * t;

    float fx = mod(hit.x, gridSpacing);
    float fz = mod(hit.z, gridSpacing);

    fx = min(fx, gridSpacing - fx);
    fz = min(fz, gridSpacing - fz);

    float dist = min(fx, fz);

    FragColor = vec4(vec3(dist), 1.0f);

    float aa = fwidth(dist);
    float alpha = smoothstep(lineRadius + aa, lineRadius - aa, dist);

    vec3 color = mix(backgroundColor, gridColor, alpha);
    FragColor = vec4(color, 1.0);
}
