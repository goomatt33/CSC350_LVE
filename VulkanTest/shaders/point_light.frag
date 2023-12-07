#version 450

layout (location = 0) in vec2 fragOffset;
layout (location = 0) out vec4 outColor;

struct PointLight {
    vec4 position; // ignore w
    vec4 color; // w is intensity
};

layout (set = 0, binding = 0) uniform GlobalUbo {
    mat4 projection;
    mat4 view;
    mat4 invView;
    vec4 ambientLightColor;
    PointLight pointLights[10];
    int numLights;
} ubo;

layout(push_constant) uniform Push {
    vec4 position;
    vec4 color;
    float radius;
} push;

const float PI = 3.1415926535;

void main() {
    float dist = sqrt(dot(fragOffset, fragOffset));
    if (dist > 1.0) {
        discard;
    }
    outColor = vec4(push.color.xyz, 0.5 * (cos(dist * PI) + 1.0));
}