// //////////////////////////////////////////////////////////// Includes //
#include "opengl-headers.h"

#include <exception>
#include <fstream>
#include <iostream>
#include <sstream>
#include <tuple>
#include <vector>

// ///////////////////////////////////////////////////////////// Structs //
struct Vertex {
    float x, y, z, u, v;

    Vertex(float a, float b, float c, float d, float e)
            : x(a), y(b), z(c), u(d), v(e) {}

    Vertex operator+(Vertex const &a) const {
        return {x + a.x, y + a.y, z + a.z, u + a.u, v + a.v};
    }

    Vertex operator/(float const a) const {
        return {x / a, y / a, z / a, u / a, v / a};
    }
};

// /////////////////////////////////////////////////////////// Constants //
unsigned int const WINDOW_WIDTH = 982;
unsigned int const WINDOW_HEIGHT = 982;
char const *WINDOW_TITLE = "Tomasz Witczak 216920 - Zadanie 2 "
                           "(Piramida Sierpińskiego)";

unsigned int const RECURSION_DEPTH_LEVEL_MIN = 0;
unsigned int const RECURSION_DEPTH_LEVEL_MAX = 8;

std::vector<Vertex> const PYRAMID
        = {{-1.0f, -1.0f, 1.0f,  0.0f, 0.0f}, // front
           {1.0f,  -1.0f, 1.0f,  1.0f, 0.0f},
           {0.0f,  1.0f,  0.0f,  0.5f, 1.0f},

           {0.0f,  -1.0f, -1.0f, 0.0f, 0.0f}, // left
           {-1.0f, -1.0f, 1.0f,  1.0f, 0.0f},
           {0.0f,  1.0f,  0.0f,  0.5f, 1.0f},

           {1.0f,  -1.0f, 1.0f,  0.0f, 0.0f}, // right
           {0.0f,  -1.0f, -1.0f, 1.0f, 0.0f},
           {0.0f,  1.0f,  0.0f,  0.5f, 1.0f},

           {1.0f,  -1.0f, 1.0f,  0.0f, 0.0f}, // bottom
           {-1.0f, -1.0f, 1.0f,  1.0f, 0.0f},
           {0.0f,  -1.0f, -1.0f, 0.5f, 1.0f}};


// /////////////////////////////////////////////////////////// Variables //
GLFWwindow *window = nullptr;

unsigned int vertexArrayObject;
unsigned int vertexBufferObject;
unsigned int vertexBufferObject2;

unsigned int texture;

float angleX = 0.0f;
float angleY = 0.0f;
float angleZ = 0.0f;

int shaderProgram;

int recursionDepthLevel = 4, previousRecursionDepthLevel = -1;
ImVec4 fractalColor = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);

std::vector<Vertex> sierpinskiTriangle;

// ///////////////////////////////////////////////// Sierpinski triangle //
std::vector<Vertex> generateSierpinskiPyramidVertices(
        std::vector<Vertex> vertices,
        int const recursionDepth) {
    if (recursionDepth == 0) {
        for (int i = 0; i < 3 * 4; i++) {
            vertices[i].u = PYRAMID[i].u;
            vertices[i].v = PYRAMID[i].v;
        }
        return vertices;
    }

    std::vector<Vertex> centerPoints;
    centerPoints.reserve(3 * 4);
    for (int i = 0; i < 3 * 4; i++) {
        int start = 3 * (i / 3);
        centerPoints.push_back(
                (vertices[(0 + i) % 3 + start]
                 + vertices[(1 + i) % 3 + start]) / 2.0f);
    }

    std::vector<Vertex> result;
    for (int i = 0; i < 2; i++) {
        std::vector<Vertex> smallerPyramid =
                generateSierpinskiPyramidVertices(
                        {
                                vertices[(3 * i + 0) % 12],
                                centerPoints[(3 * i + 0) % 12],
                                centerPoints[(3 * i + 2) % 12],

                                centerPoints[(3 * i + 3) % 12],
                                vertices[(3 * i + 4) % 12],
                                centerPoints[(3 * i + 4) % 12],

                                centerPoints[(3 * i + 0) % 12],
                                centerPoints[(3 * i + 3) % 12],
                                centerPoints[(3 * i + 2) % 12],

                                vertices[(3 * i + 0) % 12],
                                centerPoints[(3 * i + 3) % 12],
                                centerPoints[(3 * i + 0) % 12],
                        },
                        recursionDepth - 1);


        result.insert(std::end(result),
                      std::begin(smallerPyramid),
                      std::end(smallerPyramid));
    }

    std::vector<Vertex> smallerPyramid =
            generateSierpinskiPyramidVertices(
                    {
                            centerPoints[0],
                            vertices[1],
                            centerPoints[1],

                            centerPoints[6],
                            centerPoints[0],
                            centerPoints[1],

                            vertices[1],
                            centerPoints[6],
                            centerPoints[8],

                            vertices[1],
                            centerPoints[0],
                            centerPoints[6],
                    },
                    recursionDepth - 1);


    result.insert(std::end(result),
                  std::begin(smallerPyramid),
                  std::end(smallerPyramid));

    smallerPyramid =
            generateSierpinskiPyramidVertices(
                    {
                            centerPoints[2],
                            centerPoints[1],
                            vertices[2],

                            centerPoints[5],
                            centerPoints[4],
                            vertices[5],

                            centerPoints[8],
                            centerPoints[7],
                            vertices[8],

                            centerPoints[2],
                            centerPoints[5],
                            centerPoints[1],
                    },
                    recursionDepth - 1);


    result.insert(std::end(result),
                  std::begin(smallerPyramid),
                  std::end(smallerPyramid));

    return result;
}

void renderTriangle(std::vector<Vertex> const
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
                         NUMBER_OF_VERTICES *
                         sizeof(Vertex),
                         vertices.data(),
                         GL_STATIC_DRAW);

            glVertexAttribPointer(0,
                                  3,
                                  GL_FLOAT,
                                  GL_FALSE,
                                  5 * sizeof(float),
                                  nullptr);
            glEnableVertexAttribArray(0);
            glVertexAttribPointer(1,
                                  2,
                                  GL_FLOAT,
                                  GL_FALSE,
                                  5 * sizeof(float),
                                  (void *) (3 *
                                            sizeof(float)));
            glEnableVertexAttribArray(1);
        }
        glBindBuffer(GL_ARRAY_BUFFER, 0);
    }
    glBindVertexArray(0);

    // Draw the triangle
    glEnable(GL_DEPTH_TEST);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texture);
    glBindVertexArray(vertexArrayObject);
    {
        glDrawArrays(GL_TRIANGLES, 0, NUMBER_OF_VERTICES);
    }
    glBindVertexArray(0);
}

// //////////////////////////////////////////////////////////// Textures //
void generateTextures() {
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);
    {
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S,
                        GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T,
                        GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D,
                        GL_TEXTURE_MIN_FILTER,
                        GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D,
                        GL_TEXTURE_MAG_FILTER,
                        GL_LINEAR);

        int width, height, numberOfChannels;
        stbi_set_flip_vertically_on_load(true);
        unsigned char *textureData = stbi_load(
                "res/textures/triangle.jpg",
                &width, &height,
                &numberOfChannels, 0);
        if (textureData == nullptr) {
            throw std::exception(
                    "Failed to load texture!");
        }
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width,
                     height, 0,
                     GL_RGB,
                     GL_UNSIGNED_BYTE, textureData);
        glGenerateMipmap(GL_TEXTURE_2D);
        stbi_image_free(textureData);
    }
}


// ///////////////////////////////////////////////////////////// Shaders //
void checkForShaderCompileErrors(int const shader) {
    int compiledSuccessfully;

    constexpr int INFO_LOG_LENGTH = 512;
    char infoLog[INFO_LOG_LENGTH];

    glGetShaderiv(shader, GL_COMPILE_STATUS,
                  &compiledSuccessfully);

    if (!compiledSuccessfully) {
        glGetShaderInfoLog(shader, INFO_LOG_LENGTH,
                           nullptr, infoLog);

        std::stringstream message;
        message << "Failed to compile shader!" << std::endl
                << infoLog;
        throw std::exception(message.str().c_str());
    }
}

void compileShader(int const shader,
                   std::string const &source) {
    char const *shaderSourceCode = source.c_str();
    glShaderSource(shader, 1, &shaderSourceCode, nullptr);
    glCompileShader(shader);

    checkForShaderCompileErrors(shader);
}

void checkForShaderLinkingErrors(int const shader) {
    int linkedSuccessfully;

    constexpr int INFO_LOG_LENGTH = 512;
    char infoLog[INFO_LOG_LENGTH];

    glGetProgramiv(shader, GL_LINK_STATUS,
                   &linkedSuccessfully);

    if (!linkedSuccessfully) {
        glGetProgramInfoLog(shader, INFO_LOG_LENGTH,
                            nullptr, infoLog);

        std::stringstream message;
        message << "Failed to link shader!" << std::endl
                << infoLog;
        throw std::exception(message.str().c_str());
    }
}

void linkShaderProgram(int const vertexShader,
                       int const fragmentShader) {
    shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);

    checkForShaderLinkingErrors(shaderProgram);
}

void createShaderProgram() {
    int vertexShaderNumber = glCreateShader(
            GL_VERTEX_SHADER);
    int fragmentShaderNumber = glCreateShader(
            GL_FRAGMENT_SHADER);

    std::string const vertexShaderSourceCode =
            "#version 430 core" "\n"
            "layout (location = 0) in vec3 inPosition;" "\n"
            "layout (location = 1) in vec2 inTextureCoordinates;" "\n"
            "out vec2 texCoord;" "\n"
            "uniform mat4 transform;" "\n"
            "void main()" "\n"
            "{" "\n"
            "    gl_Position = transform * vec4(inPosition, 1.0);" "\n"
            "    texCoord = inTextureCoordinates;" "\n"
            "}" "\n";

    std::string const fragmentShaderSourceCode =
            "#version 430 core" "\n"
            "uniform vec3 uniformColor;" "\n"
            "uniform sampler2D uniformTexture;" "\n"
            "in vec2 texCoord;" "\n"
            "out vec4 outColor;" "\n"
            "void main()" "\n"
            "{" "\n"
            "    outColor = texture(uniformTexture, " "\n"
            "                   texCoord) * vec4(uniformColor,1);" "\n"
            "}" "\n";

    compileShader(vertexShaderNumber,
                  vertexShaderSourceCode);
    compileShader(fragmentShaderNumber,
                  fragmentShaderSourceCode);
    linkShaderProgram(vertexShaderNumber,
                      fragmentShaderNumber);

    glDeleteShader(fragmentShaderNumber);
    glDeleteShader(vertexShaderNumber);
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
    ImGui::Begin("Trojkat Sierpinskiego");
    {
        ImGui::SliderInt("Poziom rekurencji",
                         &recursionDepthLevel,
                         RECURSION_DEPTH_LEVEL_MIN,
                         RECURSION_DEPTH_LEVEL_MAX);
        ImGui::ColorEdit3("Kolor fraktala",
                          (float *) &fractalColor);
        ImGui::SliderAngle("Obrot X", &angleX);
        ImGui::SliderAngle("Obrot Y", &angleY);
        ImGui::SliderAngle("Obrot Z", &angleZ);
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
                std::cerr << "GLFW;"
                          << "Error " << errorNumber
                          << "; "
                          << "Description: "
                          << description;

            });
    if (!glfwInit()) {
        throw std::exception("glfwInit error");
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
        throw std::exception("glfwCreateWindow error");
    }
    glfwMakeContextCurrent(window);
    glfwSwapInterval(
            1);          // Enable vertical synchronization
}

void initializeOpenGLLoader() {
    bool failedToInitializeOpenGL = false;
#if defined(IMGUI_IMPL_OPENGL_LOADER_GL3W)
    failedToInitializeOpenGL = (gl3wInit() != 0);
#elif defined(IMGUI_IMPL_OPENGL_LOADER_GLEW)
    failedToInitializeOpenGL = (glewInit() != GLEW_OK);
#elif defined(IMGUI_IMPL_OPENGL_LOADER_GLAD)
    failedToInitializeOpenGL = !gladLoadGLLoader(
            (GLADloadproc) glfwGetProcAddress);
#endif
    if (failedToInitializeOpenGL) {
        throw std::exception(
                "Failed to initialize OpenGL loader!");
    }
}

void createVertexBuffersAndArrays() {
    glGenVertexArrays(1, &vertexArrayObject);
    glGenBuffers(1, &vertexBufferObject);
    glGenBuffers(1, &vertexBufferObject2);
}

void setupOpenGL() {
    setupGLFW();
    createWindow();
    initializeOpenGLLoader();
    createVertexBuffersAndArrays();
    generateTextures();
    createShaderProgram();
    setupDearImGui();
}

// //////////////////////////////////////////////////////////// Clean up //
void cleanUp() {
    glDeleteBuffers(1, &vertexBufferObject2);
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

        // ------------------------------------------- Clear viewport -- //
        glm::mat4 trans = glm::mat4(1.0f);
        glm::mat4 projection = //glm::ortho(-3.0f, 3.0f, -3.0f, 3.0f, 0.01f, 200.0f);
                glm::perspective(glm::radians(30.0f),
                                 ((float) displayWidth) /
                                 (float) displayHeight,
                                 0.01f, 100.0f);
        glm::mat4 view = glm::lookAt(glm::vec3(0.0f, 0.0f, 50.0f),
                                     glm::vec3(0.0f, 0.0f, 0.0f),
                                     glm::vec3(0.0f, 1.0f, 0.0f));
        trans = projection * view *
                (glm::rotate(glm::mat4(1.0f), angleZ,
                             glm::vec3(0.0, 0.0, 1.0)) *
                 glm::rotate(glm::mat4(1.0f), angleY,
                             glm::vec3(0.0, 1.0, 0.0)) *
                 glm::rotate(glm::mat4(1.0f), angleX,
                             glm::vec3(1.0, 0.0, 0.0)) *
                             glm::scale(glm::mat4(1.0f), glm::vec3(7.5f)));
        //        trans = glm::translate(trans, glm::vec3(0.0f, 0.0f, 0.0f));
//        glEnable(GL_CULL_FACE);
//        glCullFace(GL_BACK);
//        glFrontFace(GL_CCW);
        glViewport(0, 0, displayWidth, displayHeight);
        glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // ------------------------------ Set fractal color in shader -- //
        glUseProgram(shaderProgram);
        glUniformMatrix4fv(
                glGetUniformLocation(shaderProgram,
                                     "transform"),
                1, GL_FALSE, glm::value_ptr(trans));
        glUniform3f(
                glGetUniformLocation(shaderProgram,
                                     "uniformColor"),
                fractalColor.x, fractalColor.y,
                fractalColor.z);
        glUniform1i(
                glGetUniformLocation(shaderProgram,
                                     "uniformTexture"),
                0);

        // ------------------------------------------ Render triangle -- //
        if (recursionDepthLevel !=
            previousRecursionDepthLevel) {
            sierpinskiTriangle =
                    generateSierpinskiPyramidVertices(
                            PYRAMID,
                            recursionDepthLevel);
            previousRecursionDepthLevel = recursionDepthLevel;
        }
        renderTriangle(sierpinskiTriangle);

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
    }
    catch (std::exception const &exception) {
        std::cerr << exception.what();
        return 1;
    }
    return 0;
}

// ///////////////////////////////////////////////////////////////////// //
