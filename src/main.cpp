// //////////////////////////////////////////////////////////// Includes //
#include "opengl-headers.h"

#include <exception>
#include <fstream>
#include <iostream>
#include <sstream>
#include <vector>

// ///////////////////////////////////////////////////////////// Structs //
struct float3 {
    float x, y, z;

    float3 operator+(float3 a) const {
        return {x + a.x, y + a.y, z + a.z};
    }

    float3 operator/(float a) const {
        return {x / a, y / a, z / a};
    }
};

// /////////////////////////////////////////////////////////// Constants //
unsigned int const WINDOW_WIDTH = 982;
unsigned int const WINDOW_HEIGHT = 982;
char const *WINDOW_TITLE = "Tomasz Witczak 216920 - Zadanie 1 "
                           "(Trójkąt Sierpińskiego)";

unsigned int const RECURSION_DEPTH_LEVEL_MIN = 0;
unsigned int const RECURSION_DEPTH_LEVEL_MAX = 8;

std::vector<float3> const TRIANGLE = {{-1.0f, -1.0f, 0.0f},
                                      {1.0f,  -1.0f, 0.0f},
                                      {0.0f,  1.0f,  0.0f}};

// /////////////////////////////////////////////////////////// Variables //
GLFWwindow *window = nullptr;

unsigned int vertexArrayObject;
unsigned int vertexBufferObject;

int shaderProgram;

int recursionDepthLevel = 4, previousRecursionDepthLevel = -1;
ImVec4 fractalColor = ImVec4(0.0f, 0.0f, 0.0f, 0.0f);

std::vector<float3> sierpinskiTriangle;

// ///////////////////////////////////////////////// Sierpinski triangle //
std::vector<float3> generateSierpinskiTriangleVertices(
        std::vector<float3> const &vertices,
        int const recursionDepth) {
    if (recursionDepth == 0) {
        return vertices;
    }

    std::vector<float3> centerPoints;
    centerPoints.reserve(3);
    for (int i = 0; i < 3; i++) {
        centerPoints.push_back(
                (vertices[(0 + i) % 3] + vertices[(1 + i) % 3]) / 2.0f);
    }

    std::vector<float3> result;
    for (int i = 0; i < 3; i++) {
        std::vector<float3> smallerTriangle =
                generateSierpinskiTriangleVertices(
                        {
                                vertices[i],
                                centerPoints[i],
                                centerPoints[(i + 2) % 3]
                        },
                        recursionDepth - 1);
        result.insert(std::end(result),
                      std::begin(smallerTriangle),
                      std::end(smallerTriangle));
    }

    return result;
}

void renderTriangle(std::vector<float3> const &vertices) {
    int const NUMBER_OF_VERTICES = vertices.size();
    constexpr int NUMBER_OF_COORDINATES = 3;

    // Fill VBO and VAO
    glBindVertexArray(vertexArrayObject);
    {
        glBindBuffer(GL_ARRAY_BUFFER, vertexBufferObject);
        {
            glBufferData(GL_ARRAY_BUFFER,
                         NUMBER_OF_VERTICES * sizeof(float3),
                         vertices.data(),
                         GL_STATIC_DRAW);

            glVertexAttribPointer(0,
                                  NUMBER_OF_COORDINATES,
                                  GL_FLOAT,
                                  GL_FALSE,
                                  NUMBER_OF_COORDINATES * sizeof(float),
                                  nullptr);
            glEnableVertexAttribArray(0);
        }
        glBindBuffer(GL_ARRAY_BUFFER, 0);
    }
    glBindVertexArray(0);

    // Draw the triangle
    glBindVertexArray(vertexArrayObject);
    {
        glDrawArrays(GL_TRIANGLES, 0, NUMBER_OF_VERTICES);
    }
    glBindVertexArray(0);
}

// ///////////////////////////////////////////////////////////// Shaders //
void checkForShaderCompileErrors(int const shader) {
    int compiledSuccessfully;

    constexpr int INFO_LOG_LENGTH = 512;
    char infoLog[INFO_LOG_LENGTH];

    glGetShaderiv(shader, GL_COMPILE_STATUS, &compiledSuccessfully);

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

    glGetProgramiv(shader, GL_LINK_STATUS, &linkedSuccessfully);

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
    int vertexShaderNumber = glCreateShader(GL_VERTEX_SHADER);
    int fragmentShaderNumber = glCreateShader(GL_FRAGMENT_SHADER);

    std::string const vertexShaderSourceCode =
            "#version 430 core" "\n"
            "layout (location = 0) in vec3 inPosition;" "\n"
            "void main()" "\n"
            "{" "\n"
            "    gl_Position = vec4(inPosition, 1.0);" "\n"
            "}" "\n";

    std::string const fragmentShaderSourceCode =
            "#version 430 core" "\n"
            "uniform vec3 uniformColor;" "\n"
            "out vec4 outColor;" "\n"
            "void main()" "\n"
            "{" "\n"
            "    outColor = vec4(uniformColor, 1.0f);" "\n"
            "}" "\n";

    compileShader(vertexShaderNumber, vertexShaderSourceCode);
    compileShader(fragmentShaderNumber, fragmentShaderSourceCode);
    linkShaderProgram(vertexShaderNumber, fragmentShaderNumber);

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
        ImGui::SetWindowPos(ImVec2(0.0f, 0.0f));
        ImGui::SetWindowSize(ImVec2(375.0f, 88.0f));
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
                          << "Error " << errorNumber << "; "
                          << "Description: " << description;

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
    glfwSwapInterval(1);          // Enable vertical synchronization
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
        throw std::exception("Failed to initialize OpenGL loader!");
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
    createShaderProgram();
    setupDearImGui();
}

// //////////////////////////////////////////////////////////// Clean up //
void cleanUp() {
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
        glfwGetFramebufferSize(window, &displayWidth, &displayHeight);

        // ------------------------------------------- Clear viewport -- //
        glViewport(0, 0, displayWidth, displayHeight);
        glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        // ------------------------------ Set fractal color in shader -- //
        glUseProgram(shaderProgram);
        glUniform3f(glGetUniformLocation(shaderProgram, "uniformColor"),
                    fractalColor.x, fractalColor.y, fractalColor.z);

        // ------------------------------------------ Render triangle -- //
        if (recursionDepthLevel != previousRecursionDepthLevel) {
            sierpinskiTriangle =
                    generateSierpinskiTriangleVertices(TRIANGLE,
                                                       recursionDepthLevel);
            previousRecursionDepthLevel = recursionDepthLevel;
        }
        renderTriangle(sierpinskiTriangle);

        // ------------------------------------------------------- UI -- //
        prepareUserInterfaceWindow();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

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
