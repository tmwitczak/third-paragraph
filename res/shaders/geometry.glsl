#version 430 core
#define PI 3.1415926535897932384626433832795

layout (points) in;
layout (triangle_strip, max_vertices = 170) out;

in vec2 texCoordVG[];
out vec2 texCoordGF;

uniform mat4 transform;
uniform int subdivisionLevelHorizontal;
uniform int subdivisionLevelVertical;

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
    float x = subdivisionLevelVertical;
    float y = subdivisionLevelHorizontal;
    float theta;
    float phi;

    for (int point = 0; point < gl_in.length(); point++) {
        vec4 origin = gl_in[point].gl_Position;
        // number of vertices: x(x-1) + 2
        vec4 sphere[15][15];

        // Generate vertices
        for (int i = 0; i <= x; i++) {
            theta = (PI / 2.0) - (PI * (i / x));
            for (int j = 0; j <= y; j++) {
                phi = (2.0 * PI) * (j / y);
                sphere[i][j] = origin + transform * (vec4(sphericalToCartesian(vec3(r, theta, phi)), 0.0) - vec4(0.0, r / 2.0, 0.0, 0.0));
            }
        }

        // Draw the sphere
        for (int i = 0; i <= (x - 1); i++) {
            for (int j = 0; j <= y; j++) {
                gl_Position = sphere[i][j % int(y)];
                texCoordGF = vec2(float(i) / float(x),
                                  float(j) / float(y));
                EmitVertex();

                gl_Position = sphere[i + 1][j % int(y)];
                texCoordGF = vec2(float(i + 1) / float(x),
                                  float(j) / float(y));
                EmitVertex();
            }
            EndPrimitive();
        }
    }
}
