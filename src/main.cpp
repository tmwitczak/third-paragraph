// //////////////////////////////////////////////////////////// Includes //
#include "opengl-headers.hpp"
#include "shader.hpp"
//#include "vertex.hpp"
#include "model.hpp"

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
using std::make_shared;
using std::string;
using std::stringstream;
using std::unique_ptr;
using std::shared_ptr;
using std::vector;

// /////////////////////////////////////////////////////////// Constants //
int const WINDOW_WIDTH = 982;
int const WINDOW_HEIGHT = 982;
char const *WINDOW_TITLE = "Tomasz Witczak 216920 - Zadanie 3";

// /////////////////////////////////////////////////////////// Variables //
// ----------------------------------------------------------- Window -- //
GLFWwindow *window = nullptr;

// ---------------------------------------------------------- Shaders -- //
shared_ptr<Shader> modelShader,
                   sphereShader;

// --------------------------------------------------- Rendering mode -- //
bool wireframeMode = false;

shared_ptr<Model> model;

// //////////////////////////////////////////////////////////// Textures //
GLuint loadTextureFromFile(string const &filename) {
    // Generate OpenGL resource
    GLuint texture;
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
            filename.c_str(),
            &imageWidth, &imageHeight,
            &imageNumberOfChannels, 0);

        if (textureData == nullptr) {
            throw exception("Failed to load texture!");
        }

        // Pass image to OpenGL
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB,
                     imageWidth, imageHeight, 0,
                     [&]() -> GLenum {
                         switch (imageNumberOfChannels) {
                             case 1:  return GL_RED;
                             case 3:  return GL_RGB;
                             case 4:  return GL_RGBA;
                             default: return GL_RGB;
                         }
                     }(),
                     GL_UNSIGNED_BYTE, textureData);

        // Generate mipmap for loaded texture
        glGenerateMipmap(GL_TEXTURE_2D);

        // After loading into OpenGL - release the raw resource
        stbi_image_free(textureData);
    }

    // Return texture's ID
    return texture;
}
class Sphere {
public:
    static constexpr int SUBDIVISION_LEVEL_MIN = 1;
    static constexpr int SUBDIVISION_LEVEL_MAX = 7;

private:
    GLuint vao, vbo;
    GLuint texture;

    vec3 const point {0.0f, 0.0f, 0.0f};

public:
    int subdivisionLevel = SUBDIVISION_LEVEL_MAX;

    Sphere() {
        glGenVertexArrays(1, &vao);
        glGenBuffers(1, &vbo);

        glBindVertexArray(vao);
            glBindBuffer(GL_ARRAY_BUFFER, vbo);
                glBufferData(GL_ARRAY_BUFFER, sizeof(Vertex), &point,
                        GL_STATIC_DRAW);

                glEnableVertexAttribArray(0);
                glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE,
                        sizeof(vec3), nullptr);

            glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindVertexArray(0);

        texture = loadTextureFromFile("res/textures/jupiter.jpg");
    }


    ~Sphere() {
        glDeleteBuffers(1, &vbo);
        glDeleteVertexArrays(1, &vao);
    }

    void render(shared_ptr<Shader> shader) {
        shader->use();

        sphereShader->uniform1i("subdivisionLevelHorizontal",
                subdivisionLevel + 2);
        sphereShader->uniform1i("subdivisionLevelVertical",
                subdivisionLevel + 1);

        sphereShader->uniform1i("texture0", 0);

        glEnable(GL_DEPTH_TEST);
        glEnable(GL_PROGRAM_POINT_SIZE);

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, texture);

        glBindVertexArray(vao);
            glDrawArrays(GL_POINTS, 0, 1);
        glBindVertexArray(0);
    }
};

unique_ptr<Sphere> sphere;


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
        ImGui::SliderInt("Szczegolowosc kuli",
                         &sphere->subdivisionLevel,
                         Sphere::SUBDIVISION_LEVEL_MIN,
                         Sphere::SUBDIVISION_LEVEL_MAX);
        if (ImGui::Button("Tryb siatki")) {
            wireframeMode = !wireframeMode;
        }
        // ImGui::SetWindowPos(ImVec2(0.0f, 0.0f));
        // ImGui::SetWindowSize(ImVec2(375.0f, 200.0f));
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
    glfwSwapInterval(1);  // Enable vertical synchronization
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


void setupOpenGL() {
    setupGLFW();
    createWindow();
    initializeOpenGLLoader();

    model = make_shared<Model>("res/models/orangelow.obj");

    modelShader = make_shared<Shader>("res/shaders/model/vertex.glsl",
                                      "res/shaders/model/geometry.glsl",
                                      "res/shaders/model/fragment.glsl");

    sphereShader = make_shared<Shader>("res/shaders/sphere/vertex.glsl",
                                       "res/shaders/sphere/geometry.glsl",
                                       "res/shaders/sphere/fragment.glsl");

    sphere = make_unique<Sphere>();

    setupDearImGui();
}

// //////////////////////////////////////////////////////////// Clean up //
void cleanUp() {
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    sphere = nullptr;

    sphereShader = nullptr;
    modelShader = nullptr;

    model = nullptr;

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
        model = glm::scale(model, vec3(0.025f));
        static float angle = 0.0f;
        angle += 0.01f;
         model = rotate(model, angle, vec3(0.0, 1.0, 0.0));
        // model = rotate(model, pyramidRotationAngleY, vec3(0.0, 1.0, 0.0));

        mat4 const transformation = projection * view * model;

        // ------------------------------------------- Clear viewport -- //
//        glFrontFace(GL_CW);
//        glCullFace(GL_BACK);
        glEnable(GL_DEPTH_TEST);
        glViewport(0, 0, displayWidth, displayHeight);
        glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // --------------------------------------- Set rendering mode -- //
        glPolygonMode(GL_FRONT_AND_BACK, wireframeMode ? GL_LINE : GL_FILL);

        // ------------------------------------- Set shader variables -- //
        sphereShader->use();

        sphereShader->uniformMatrix4fv("transform",
                value_ptr(transformation));

        modelShader->use();
        modelShader->uniformMatrix4fv("transform",
                                       value_ptr(transformation));

        // -------------------------------------------- Render sphere -- //
        //sphere->render(sphereShader);

        ::model->render(modelShader);

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
