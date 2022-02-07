#include "glad/glad.h"

#include <iostream>
#include <fstream>
#include <streambuf>

#include "World.h"
#include "Skybox.h"
#include "Chunk.h"
#include "Camera.h"
#include "Light.h"

#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "stb_image.h"
#include "Shapes.h"

using namespace std;
using namespace glm;

constexpr auto WINDOW_WIDTH = 1280;
constexpr auto WINDOW_HEIGHT = 720;

float lastMouseX = WINDOW_WIDTH / 2.0f;
float lastMouseY = WINDOW_HEIGHT / 2.0f;

World* g_world;

void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    glViewport(0, 0, width, height);

    if (g_world)
        g_world->UpdateWindowSize(width, height);
}

void mouse_callback(GLFWwindow* window, double posX, double posY)
{
    float diffX = posX - lastMouseX;
    float diffY = posY - lastMouseY;
    lastMouseX = posX;
    lastMouseY = posY;

    if (g_world)
        g_world->ProcessMouseInput(diffX, diffY);
}

void processInput(GLFWwindow* window)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    if (g_world)
        g_world->ProcessKeyboardInput(window);
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

    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    
    g_world = new World(WINDOW_WIDTH, WINDOW_HEIGHT);

    //glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

    while (!glfwWindowShouldClose(window))
    {
        float currentTime = glfwGetTime();
        float deltaTime = currentTime - previousTime;

        processInput(window);
        g_world->Update(deltaTime);

        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glViewport(0, 0, g_world->GetCamera()->GetWidth(), g_world->GetCamera()->GetHeight());

        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        g_world->Draw();

        glfwSwapBuffers(window);
        glfwPollEvents();

        previousTime = currentTime;
    }

    Shapes::FreeInstance();

    if (g_world)
    {
        delete g_world;
        g_world = nullptr;
    }

    glfwTerminate();

    return 0;
}