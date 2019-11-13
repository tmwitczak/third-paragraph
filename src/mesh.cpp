// //////////////////////////////////////////////////////////// Includes //
#include "mesh.hpp"

#include "opengl-headers.hpp"

using std::vector;
using std::shared_ptr;

#include <iostream>
// ///////////////////////////////////////////////////////// Class: Mesh //
Mesh::Mesh(vector<Vertex> const &vertices,
           vector<unsigned int> const &indices,
           vector<Texture> const &textures)
        : vertices(vertices),
          indices(indices),
          textures(textures) {
}
void Mesh::render(shared_ptr<Shader> shader) const {
    glActiveTexture(GL_TEXTURE0);

    //todo
    shader->use();
    shader->uniform1i("texture0", 0);
    glBindTexture(GL_TEXTURE_2D, textures[0].id);
    
    glBindVertexArray(vao); {
        std::cout << indices.size();
        glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, nullptr);
    }
    glBindVertexArray(0);

    glActiveTexture(GL_TEXTURE0);
}

void Mesh::setupMesh() {
    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vbo);
    glGenBuffers(1, &ebo);

    glBindVertexArray(vao); {
        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), &vertices[0], GL_STATIC_DRAW);
        std::cout << "siema";

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), &indices[0], GL_STATIC_DRAW);

        glEnableVertexAttribArray(0);	
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);

        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)(sizeof(glm::vec3)));
    }
    glBindVertexArray(0);
}