// //////////////////////////////////////////////////////// GLSL version //
#version 430 core

// ////////////////////////////////////////////////////////// Primitives //
layout (triangles) in;
layout (triangle_strip, max_vertices = 3) out;

// ////////////////////////////////////////////////////////////// Inputs //
in vec2 texCoordG[3];

// ///////////////////////////////////////////////////////////// Outputs //
out vec2 texCoordF;

// //////////////////////////////////////////////////////////////// Main //
void main() {
    for (int i = 0; i < gl_in.length(); ++i) {
        gl_Position = gl_in[i].gl_Position;
        texCoordF = texCoordG[i];
        EmitVertex();
    }
    EndPrimitive();
}

// ///////////////////////////////////////////////////////////////////// //
