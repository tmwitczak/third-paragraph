// //////////////////////////////////////////////////////////// Includes //
#include "mesh.hpp"

#include "opengl-headers.hpp"

using std::vector;

// ///////////////////////////////////////////////////////// Class: Mesh //
Mesh::Mesh(vector<Vertex> const &vertices,
           vector<unsigned int> const &indices,
           vector<Texture> const &textures)
        : vertices(vertices),
          indices(indices),
          textures(textures) {
    setupMesh();
}

void Mesh::render(Shader &shader) {
    glActiveTexture(GL_TEXTURE0);

    //todo
    shader.uniform1i("texture0", 0);
    glBindTexture(GL_TEXTURE_2D, textures[0].id);
    
    glBindVertexArray(vao); {
        glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0);
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

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), &indices[0], GL_STATIC_DRAW);

        glEnableVertexAttribArray(0);	
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);

        glEnableVertexAttribArray(1);	
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)(sizeof(glm::vec3)));

        glEnableVertexAttribArray(2);	
        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, TexCoords));
    }
    glBindVertexArray(0);
}