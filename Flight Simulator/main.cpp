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
#include "DebugHelper.h"
#include "InputWrapper.h"
#include "TextureLoadHelper.h"
#include "ShaderManager.h"
#include "RenderSettings.h"

using namespace std;
using namespace glm;

constexpr auto WINDOW_WIDTH = 1280;
constexpr auto WINDOW_HEIGHT = 720;

World* g_world;

void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    glViewport(0, 0, width, height);

    if (g_world)
        g_world->UpdateWindowSize(width, height);
}

void mouse_callback(GLFWwindow* window, double posX, double posY)
{
    InputWrapper::GetInstance()->MouseCallback(window, posX, posY);
}

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    InputWrapper::GetInstance()->KeyCallback(window, key, scancode, action, mods);
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

    TextureLoadHelper::GetInstance()->SetFlipVerticallyOnLoad(true);
    
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
    glfwSetKeyCallback(window, key_callback);

    float previousTime = (float)glfwGetTime();

    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    
    g_world = new World(WINDOW_WIDTH, WINDOW_HEIGHT);

    //glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

    while (!glfwWindowShouldClose(window))
    {
        float currentTime = (float)glfwGetTime();
        float deltaTime = currentTime - previousTime;

        InputWrapper::GetInstance()->Update();
        if (InputWrapper::GetInstance()->GetKey(InputWrapper::Keys::Exit))
            glfwSetWindowShouldClose(window, true);

        g_world->Update(deltaTime);

        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glViewport(0, 0, (int)g_world->GetCamera()->GetWidth(), (int)g_world->GetCamera()->GetHeight());

        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        g_world->Draw();

        glfwSwapBuffers(window);
        glfwPollEvents();

        previousTime = currentTime;
    }

    RenderSettings::FreeInstance();
    TextureLoadHelper::FreeInstance();
    InputWrapper::FreeInstance();
    DebugHelper::FreeInstance();
    ShaderManager::FreeInstance();

    if (g_world)
    {
        delete g_world;
        g_world = nullptr;
    }

    glfwTerminate();

    return 0;
}