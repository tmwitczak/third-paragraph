// //////////////////////////////////////////////////////////// Includes //
#include "opengl-headers.hpp"
#include "shader.hpp"
#include "vertex.hpp"

#include <array>
#include <cmath>
#include <exception>
#include <fstream>
#include <iostream>
#include <memory>
#include <sstream>
#include <tuple>
#include <vector>

// ////////////////////////////////////////////////////////////// Usings //
using glm::mat4;
using glm::perspective;
using glm::radians;
using glm::rotate;
using glm::scale;
using glm::value_ptr;
using glm::vec3;

using std::array;
using std::begin;
using std::cerr;
using std::end;
using std::endl;
using std::exception;
using std::make_unique;
using std::string;
using std::stringstream;
using std::unique_ptr;
using std::vector;

// /////////////////////////////////////////////////////////// Constants //
int const WINDOW_WIDTH = 982;
int const WINDOW_HEIGHT = 982;
char const *WINDOW_TITLE =
    "Tomasz Witczak 216920 - Zadanie 2"
    " (Piramida Sierpińskiego)";

int const RECURSION_DEPTH_LEVEL_MIN = 0;
int const RECURSION_DEPTH_LEVEL_MAX = 8;

vector<Vertex> const BASE_PYRAMID = {
{0.0f, 0.0f, 0.1f, 0.0f, 0.0f}};
//     // Front triangle
//    {-1.0f, 0.0f, -1.0f / sqrt(2.0f), 0.0f, 0.0f},
//    {1.0f, 0.0f, -1.0f / sqrt(2.0f), 1.0f, 0.0f},
//    {0.0f, -1.0f, 1.0f / sqrt(2.0f), 0.5f, 1.0f},
//
//    // Left triangle
//    {0.0f, 1.0f, 1.0f / sqrt(2.0f), 0.0f, 0.0f},
//    {-1.0f, 0.0f, -1.0f / sqrt(2.0f), 1.0f, 0.0f},
//    {0.0f, -1.0f, 1.0f / sqrt(2.0f), 0.5f, 1.0f},
//
//    // Right triangle
//    {1.0f, 0.0f, -1.0f / sqrt(2.0f), 0.0f, 0.0f},
//    {0.0f, 1.0f, 1.0f / sqrt(2.0f), 1.0f, 0.0f},
//    {0.0f, -1.0f, 1.0f / sqrt(2.0f), 0.5f, 1.0f},
//
//    // Bottom triangle
//    {1.0f, 0.0f, -1.0f / sqrt(2.0f), 0.0f, 0.0f},
//    {-1.0f, 0.0f, -1.0f / sqrt(2.0f), 1.0f, 0.0f},
//    {0.0f, 1.0f, 1.0f / sqrt(2.0f), 0.5f, 1.0f}};

// /////////////////////////////////////////////////////////// Variables //
// ----------------------------------------------------------- Window -- //
GLFWwindow *window = nullptr;

// ---------------------------------------------------------- Shaders -- //
unique_ptr<Shader> shader;

// ---------------------------------------------------- VAOs and VBOs -- //
GLuint vertexArrayObject,
    vertexBufferObject;

// --------------------------------------------------------- Textures -- //
unsigned int texture;

// -------------------------------------- Pyramid rotation parameters -- //
float pyramidRotationAngleX = 0.0f,
      pyramidRotationAngleY = 0.0f;

// ----------------------------------------------------------- Colors -- //
ImVec4 fractalColor = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);

// ----------------------------------------------------- Pyramid data -- //
vector<Vertex> sierpinskiPyramid;

int currentRecursionDepthLevel = 4,
    previousRecursionDepthLevel = -1;

// --------------------------------------------------- Rendering mode -- //
bool wireframeMode = false;

// ////////////////////////////////////////////////// Sierpinski pyramid //
vector<Vertex> generateSierpinskiPyramidVertices(
    vector<Vertex> vertices,
    int const recursionDepth) {
    constexpr int NUMBER_OF_VERTICES_IN_PYRAMID = 12,
                  NUMBER_OF_VERTICES_IN_TRIANGLE = 3;

    // ----------------------------------------- Return basic pyramid -- //
    if (recursionDepth == 0) {
        // Set base UV coordinates for each pyramid
        for (int i = 0; i < NUMBER_OF_VERTICES_IN_PYRAMID; ++i) {
            vertices[i].u = BASE_PYRAMID[i].u;
            vertices[i].v = BASE_PYRAMID[i].v;
        }
        return vertices;
    }

    // -------------------------------------- Calculate center points -- //
    vector<Vertex> centerPoints;
    centerPoints.reserve(NUMBER_OF_VERTICES_IN_PYRAMID);
    for (int i = 0; i < NUMBER_OF_VERTICES_IN_PYRAMID; ++i) {
        Vertex const firstVertex = vertices[(i + 0) % NUMBER_OF_VERTICES_IN_TRIANGLE + (i / NUMBER_OF_VERTICES_IN_TRIANGLE) * NUMBER_OF_VERTICES_IN_TRIANGLE];
        Vertex const secondVertex = vertices[(i + 1) % NUMBER_OF_VERTICES_IN_TRIANGLE + (i / NUMBER_OF_VERTICES_IN_TRIANGLE) * NUMBER_OF_VERTICES_IN_TRIANGLE];

        centerPoints.push_back((firstVertex + secondVertex) / 2.0f);
    }

    // ----------- Recursively calculate vertices of smaller pyramids -- //
    array<vector<Vertex>, 4> smallerPyramids;
    smallerPyramids[0] = generateSierpinskiPyramidVertices({vertices[0], centerPoints[0], centerPoints[2],
                                                            centerPoints[3], vertices[4], centerPoints[4],
                                                            centerPoints[0], centerPoints[3], centerPoints[2],
                                                            vertices[0], centerPoints[3], centerPoints[0]},
                                                           recursionDepth - 1);
    smallerPyramids[1] = generateSierpinskiPyramidVertices({vertices[3], centerPoints[3], centerPoints[5],
                                                            centerPoints[6], vertices[7], centerPoints[7],
                                                            centerPoints[3], centerPoints[6], centerPoints[5],
                                                            vertices[3], centerPoints[6], centerPoints[3]},
                                                           recursionDepth - 1);
    smallerPyramids[2] = generateSierpinskiPyramidVertices({centerPoints[0], vertices[1], centerPoints[1],
                                                            centerPoints[6], centerPoints[0], centerPoints[1],
                                                            vertices[1], centerPoints[6], centerPoints[8],
                                                            vertices[1], centerPoints[0], centerPoints[6]},
                                                           recursionDepth - 1);
    smallerPyramids[3] = generateSierpinskiPyramidVertices({centerPoints[2], centerPoints[1], vertices[2],
                                                            centerPoints[5], centerPoints[4], vertices[5],
                                                            centerPoints[8], centerPoints[7], vertices[8],
                                                            centerPoints[2], centerPoints[5], centerPoints[1]},
                                                           recursionDepth - 1);

    // --------------------------------- Sum up the resulting pyramid -- //
    vector<Vertex> result;
    for (auto const &smallerPyramid : smallerPyramids) {
        result.insert(end(result),
                      begin(smallerPyramid),
                      end(smallerPyramid));
    }

    return result;
}

void renderTriangle(vector<Vertex> const
                        &vertices) {
    int const NUMBER_OF_VERTICES = vertices.size();
    constexpr int NUMBER_OF_COORDINATES = 3;
    constexpr int NUMBER_OF_COORDINATES_UV = 2;

    // Fill VBO and VAO
    glBindVertexArray(vertexArrayObject);
    {
        glBindBuffer(GL_ARRAY_BUFFER, vertexBufferObject);
        {
            glBufferData(GL_ARRAY_BUFFER,
                         NUMBER_OF_VERTICES * sizeof(Vertex),
                         vertices.data(),
                         GL_STATIC_DRAW);

            glVertexAttribPointer(0,
                                  NUMBER_OF_COORDINATES,
                                  GL_FLOAT,
                                  GL_FALSE,
                                  (NUMBER_OF_COORDINATES + NUMBER_OF_COORDINATES_UV) * sizeof(float),
                                  nullptr);
            glEnableVertexAttribArray(0);
            glVertexAttribPointer(1,
                                  NUMBER_OF_COORDINATES_UV,
                                  GL_FLOAT,
                                  GL_FALSE,
                                  (NUMBER_OF_COORDINATES + NUMBER_OF_COORDINATES_UV) * sizeof(float),
                                  (void *)(NUMBER_OF_COORDINATES * sizeof(float)));
            glEnableVertexAttribArray(1);
        }
        glBindBuffer(GL_ARRAY_BUFFER, 0);
    }
    glBindVertexArray(0);

    // Draw the triangle
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_PROGRAM_POINT_SIZE);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texture);

    glBindVertexArray(vertexArrayObject);
    {
        glDrawArrays(GL_POINTS, 0, NUMBER_OF_VERTICES);
    }
    glBindVertexArray(0);
}

// //////////////////////////////////////////////////////////// Textures //
void generateTextures() {
    // Generate OpenGL resource
    glGenTextures(1, &texture);

    // Setup the texture
    glBindTexture(GL_TEXTURE_2D, texture);
    {
        // Set texture parameters
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        // Load texture from file
        stbi_set_flip_vertically_on_load(true);

        int imageWidth, imageHeight, imageNumberOfChannels;
        unsigned char *textureData = stbi_load(
            "res/textures/triangle.jpg",
            &imageWidth, &imageHeight,
            &imageNumberOfChannels, 0);

        if (textureData == nullptr) {
            throw exception("Failed to load texture!");
        }

        // Pass image to OpenGL
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB,
                     imageWidth, imageHeight, 0, GL_RGB,
                     GL_UNSIGNED_BYTE, textureData);

        // Generate mipmap for loaded texture
        glGenerateMipmap(GL_TEXTURE_2D);

        // After loading into OpenGL - release the raw resource
        stbi_image_free(textureData);
    }
}
// ////////////////////////////////////////////////////// User interface //
void setupDearImGui() {
    constexpr char const *GLSL_VERSION = "#version 430";

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();

    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init(GLSL_VERSION);

    ImGui::StyleColorsLight();
}

void prepareUserInterfaceWindow() {
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
    ImGui::Begin("Piramida Sierpinskiego");
    {
        ImGui::SliderInt("Poziom rekurencji",
                         &currentRecursionDepthLevel,
                         RECURSION_DEPTH_LEVEL_MIN,
                         RECURSION_DEPTH_LEVEL_MAX);
        ImGui::ColorEdit3("Kolor fraktala",
                          (float *)&fractalColor);
        ImGui::SliderAngle("Obrot X", &pyramidRotationAngleX, 0.0f, 360.0f);
        ImGui::SliderAngle("Obrot Y", &pyramidRotationAngleY, 0.0f, 360.0f);
        if (ImGui::Button("Tryb siatki")) {
            wireframeMode = !wireframeMode;
        }
        ImGui::SetWindowPos(ImVec2(0.0f, 0.0f));
        ImGui::SetWindowSize(ImVec2(375.0f, 200.0f));
    }
    ImGui::End();
    ImGui::Render();
}

// //////////////////////////////////////////////////////// Setup OpenGL //
void setupGLFW() {
    glfwSetErrorCallback(
        [](int const errorNumber,
           char const *description) {
            cerr << "GLFW;"
                 << "Error " << errorNumber
                 << "; "
                 << "Description: "
                 << description;
        });
    if (!glfwInit()) {
        throw exception("glfwInit error");
    }
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE,
                   GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
}

void createWindow() {
    window = glfwCreateWindow(WINDOW_WIDTH,
                              WINDOW_HEIGHT,
                              WINDOW_TITLE,
                              nullptr,
                              nullptr);
    if (window == nullptr) {
        throw exception("glfwCreateWindow error");
    }
    glfwMakeContextCurrent(window);
    glfwSwapInterval(
        1);  // Enable vertical synchronization
}

void initializeOpenGLLoader() {
    bool failedToInitializeOpenGL = false;
#if defined(IMGUI_IMPL_OPENGL_LOADER_GL3W)
    failedToInitializeOpenGL = (gl3wInit() != 0);
#elif defined(IMGUI_IMPL_OPENGL_LOADER_GLEW)
    failedToInitializeOpenGL = (glewInit() != GLEW_OK);
#elif defined(IMGUI_IMPL_OPENGL_LOADER_GLAD)
    failedToInitializeOpenGL = !gladLoadGLLoader(
        (GLADloadproc)glfwGetProcAddress);
#endif
    if (failedToInitializeOpenGL) {
        throw exception(
            "Failed to initialize OpenGL loader!");
    }
}

void createVertexBuffersAndArrays() {
    glGenVertexArrays(1, &vertexArrayObject);
    glGenBuffers(1, &vertexBufferObject);
}

void setupOpenGL() {
    setupGLFW();
    createWindow();
    initializeOpenGLLoader();
    createVertexBuffersAndArrays();
    generateTextures();
    shader = make_unique<Shader>("res\\shaders\\vertex.glsl",
                                 "res\\shaders\\geometry.glsl",
                                 "res\\shaders\\fragment.glsl");
    setupDearImGui();
}

// //////////////////////////////////////////////////////////// Clean up //
void cleanUp() {
    shader = nullptr;
    
    glDeleteBuffers(1, &vertexBufferObject);
    glDeleteVertexArrays(1, &vertexArrayObject);

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    glfwDestroyWindow(window);
    glfwTerminate();
}

// /////////////////////////////////////////////////////////// Main loop //
void performMainLoop() {
    while (!glfwWindowShouldClose(window)) {
        // --------------------------------------------------- Events -- //
        glfwPollEvents();

        // ----------------------------------- Get current frame size -- //
        int displayWidth, displayHeight;
        glfwMakeContextCurrent(window);
        glfwGetFramebufferSize(window, &displayWidth,
                               &displayHeight);

        // -------------------------------- Calculate transformations -- //
        mat4 const identity = mat4(1.0f);

        mat4 const projection = perspective(radians(60.0f),
                                            ((float)displayWidth) / ((float)displayHeight),
                                            0.01f, 100.0f);

        mat4 const view = lookAt(vec3(0.0f, 0.0f, 2.5f),
                                 vec3(0.0f, 0.0f, 0.0f),
                                 vec3(0.0f, 1.0f, 0.0f));

        mat4 model = identity;
        model = rotate(model, pyramidRotationAngleX, vec3(1.0, 0.0, 0.0));
        model = rotate(model, pyramidRotationAngleY, vec3(0.0, 1.0, 0.0));

        mat4 const transformation = projection * view * model;

        // ------------------------------------------- Clear viewport -- //
        glViewport(0, 0, displayWidth, displayHeight);
        glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // --------------------------------------- Set rendering mode -- //
        glPolygonMode(GL_FRONT_AND_BACK, wireframeMode ? GL_LINE : GL_FILL);

        // ------------------------------------- Set shader variables -- //
        shader->use();

        shader->uniformMatrix4fv("transform",
                                 value_ptr(transformation));

        shader->uniform3f("uniformColor",
                          fractalColor.x, fractalColor.y, fractalColor.z);

        shader->uniform1i("uniformTexture",
                          0);

        // ------------------------------------------- Render pyramid -- //
//        if (currentRecursionDepthLevel !=
//            previousRecursionDepthLevel) {
//            sierpinskiPyramid =
//                generateSierpinskiPyramidVertices(
//                    BASE_PYRAMID,
//                    currentRecursionDepthLevel);
//            previousRecursionDepthLevel = currentRecursionDepthLevel;
//        }
        renderTriangle(BASE_PYRAMID);

        // ------------------------------------------------------- UI -- //
        prepareUserInterfaceWindow();
        ImGui_ImplOpenGL3_RenderDrawData(
            ImGui::GetDrawData());

        // -------------------------------------------- Update screen -- //
        glfwMakeContextCurrent(window);
        glfwSwapBuffers(window);
    }
}

// //////////////////////////////////////////////////////////////// Main //
int main() {
    try {
        setupOpenGL();
        performMainLoop();
        cleanUp();
    } catch (exception const &exception) {
        cerr << exception.what();
        return 1;
    }
    return 0;
}

// ///////////////////////////////////////////////////////////////////// //
