#version 430 core
#define PI 3.1415926535897932384626433832795

layout (points) in;
layout (triangle_strip, max_vertices = 204) out;

uniform mat4 transform;

// spherical - r, theta, phi
vec3 sphericalToCartesian(vec3 spherical) {
    float r = spherical.x,
                theta = spherical.y,
                phi = spherical.z;
    return r * vec3(cos(theta) * cos(phi),
                    cos(theta) * sin(phi),
                    sin(theta));
}

void main() {
    float r = 0.5;
    float x = 3;

    vec4 origin = gl_in[0].gl_Position;

    vec4 verts[16];

    for (int i = 0; i <= x; i++) {
        
        float theta = (PI / 2.0) - (PI * (i / x));

        for (int j = 0; j <= x; j++) {

            float phi = (2.0 * PI) * (j / x);

            verts[i * 4 + j] = origin + transform * (vec4(sphericalToCartesian(vec3(r, theta, phi)), 0.0) - vec4(0,r/2,0,0));

            gl_Position = verts[i * 4 + j];
            EmitVertex();
        }
    }

    // for (int i = 0; i <= x; i++) {
    //     for (int j = 0; j <= x; j++) {
    //         verts[i * 4 + j];
    //     }
    // }



    // gl_Position = gl_in[0].gl_Position;
    // EmitVertex();
    // gl_Position = gl_in[1].gl_Position;
    // EmitVertex();
    // gl_Position = gl_in[2].gl_Position;
    // EmitVertex();
    EndPrimitive();
}