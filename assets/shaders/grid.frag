#version 330 core

in vec2 vUV;
out vec4 FragColor;

uniform mat4 invViewProj;
uniform vec3 cameraPos;

uniform float gridSpacing;
uniform float lineRadius;

uniform vec3 gridColor;
uniform vec3 backgroundColor;

vec3 reconstructRay(vec2 uv) {
    vec4 ndc = vec4(uv * 2.0 - 1.0, 1.0, 1.0);
    vec4 world = invViewProj * ndc;
    world /= world.w;
    return normalize(world.xyz - cameraPos);
}

float rayLineDistance( vec3 ro, vec3 rd,vec3 p0, vec3 dir) {
    vec3 w0 = ro - p0;
    float a = dot(rd, rd);
    float b = dot(rd, dir);
    float c = dot(dir, dir);
    float d = dot(rd, w0);
    float e = dot(dir, w0);

    float denom = a * c - b * b;
    float sc = (b * e - c * d) / denom;

    vec3 closestRay = ro + sc * rd;
    vec3 closestLine = p0 + dir * dot(closestRay - p0, dir);
    return length(closestRay - closestLine);
}

void main() {
    vec3 ro = cameraPos;
    vec3 rd = reconstructRay(vUV);

    if (abs(rd.y) < 1e-5) discard;

    float t = -ro.y / rd.y;
    if (t <= 0.0) discard;

    vec3 hit = ro + rd * t;

    float gx = round(hit.x / gridSpacing) * gridSpacing;
    float gz = round(hit.z / gridSpacing) * gridSpacing;

    float distX = rayLineDistance(
        ro, rd,
        vec3(gx, 0.0, 0.0),
        vec3(0.0, 0.0, 1.0)
    );

    float distZ = rayLineDistance(
        ro, rd,
        vec3(0.0, 0.0, gz),
        vec3(1.0, 0.0, 0.0)
    );

    float d = min(distX, distZ);

    float aa = fwidth(d);
    float alpha = smoothstep(lineRadius + aa, lineRadius - aa, d);

    vec3 color = mix(backgroundColor, gridColor, alpha);
    FragColor = vec4(color, 1.0);
}
