#include "glad/glad.h"

#include <iostream>
#include <GLFW/glfw3.h>
#include <fstream>
#include <streambuf>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "stb_image.h"

#include "Terrain.h"
#include "Camera.h"
#include "Light.h"

using namespace std;
using namespace glm;

constexpr auto WINDOW_WIDTH = 1280;
constexpr auto WINDOW_HEIGHT = 720;

float lastMouseX = WINDOW_WIDTH / 2.0f;
float lastMouseY = WINDOW_HEIGHT / 2.0f;

Camera* camera;

void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    glViewport(0, 0, width, height);

    camera->UpdateWindowSize(width, height);
}

void mouse_callback(GLFWwindow* window, double posX, double posY)
{
    float diffX = posX - lastMouseX;
    float diffY = posY - lastMouseY;
    lastMouseX = posX;
    lastMouseY = posY;

    camera->ProcessMouseInput(diffX, diffY);
}

void processInput(GLFWwindow* window)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    camera->ProcessKeybaordInput(window);
}

int main(int argc, char const* argv[])
{
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* window = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "Flight Simulator", NULL, NULL);

    if (window == NULL)
    {
        cout << "Failed to create GLFW window." << endl;
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        cout << "Failed to initialize GLAD" << endl;
        return -1;
    }

    glViewport(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT);

    camera = new Camera(radians(45.0f), (float)WINDOW_WIDTH, (float)WINDOW_HEIGHT, 0.1f, 1000.0f);

    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    stbi_set_flip_vertically_on_load(true);

    glEnable(GL_DEPTH_TEST);

    int maxPatchVertices = 0;
    glGetIntegerv(GL_MAX_PATCH_VERTICES, &maxPatchVertices);
    cout << "Max supported patch vertices: " << maxPatchVertices << endl;

    if (maxPatchVertices < 3)
    {
        cout << "The GPU must support at least 3 verices per patch" << endl;
        return 0;
    }

    glPatchParameteri(GL_PATCH_VERTICES, 3);

    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    glfwSetCursorPosCallback(window, mouse_callback);

    float previousTime = glfwGetTime();

    PerlinNoise* perlinNoise = new PerlinNoise();
    Light* light = new Light();

    light->SetAmbientColor(vec4(0.1f, 0.1f, 0.1f, 1.0f));
    light->SetDiffuseColor(vec4(1.0f, 1.0f, 1.0f, 1.0f));
    //light->SetLightDirection(vec3(1.0f, 0.0f, 0.0f));

    Terrain* terrain = new Terrain(perlinNoise);

    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    //glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

    while (!glfwWindowShouldClose(window))
    {
        float currentTime = glfwGetTime();
        float deltaTime = currentTime - previousTime;

        processInput(window);
        camera->Update(deltaTime);

        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glViewport(0, 0, camera->GetWidth(), camera->GetHeight());

        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        terrain->Draw(light, camera);

        glfwSwapBuffers(window);
        glfwPollEvents();

        previousTime = currentTime;
    }

    delete terrain;
    terrain = nullptr;

    delete light;
    light = nullptr;

    delete perlinNoise;
    perlinNoise = nullptr;
    
    delete camera;
    camera = nullptr;

    glfwTerminate();

    return 0;
}