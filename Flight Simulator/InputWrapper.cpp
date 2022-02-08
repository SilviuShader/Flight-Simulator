#include "InputWrapper.h"

using namespace std;

InputWrapper* InputWrapper::g_instance = nullptr;

InputWrapper* InputWrapper::GetInstance()
{
	if (!g_instance)
		g_instance = new InputWrapper();

	return g_instance;
}

void InputWrapper::FreeInstance()
{
	if (g_instance)
	{
		delete g_instance;
		g_instance = nullptr;
	}
}

void InputWrapper::KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	if (m_glfwToKeysMapping.find(key) == m_glfwToKeysMapping.end())
		return;

	bool pressValue = action != GLFW_RELEASE;
	for (auto& currentKey : m_glfwToKeysMapping[key])
		m_keysStates[(int)currentKey] = pressValue;
}

void InputWrapper::Update()
{
	memset(m_getKeysDown, false, sizeof(bool) * (int)Keys::Last);
	memset(m_getKeysUp,   false, sizeof(bool) * (int)Keys::Last);

	for (int i = 0; i < (int)Keys::Last; i++)
	{
		if (m_keysStates[i] && !m_previousKeysStates[i])
			m_getKeysDown[i] = true;

		if (!m_keysStates[i] && m_previousKeysStates[i])
			m_getKeysUp[i] = true;
	}

	memcpy(m_previousKeysStates, m_keysStates, sizeof(bool) * (int)Keys::Last);
}

bool InputWrapper::GetKey(Keys key)
{
	return m_keysStates[(int)key];
}

bool InputWrapper::GetKeyDown(Keys key)
{
	return m_getKeysDown[(int)key];
}

bool InputWrapper::GetKeyUp(Keys key)
{
	return m_getKeysUp[(int)key];
}

InputWrapper::InputWrapper()
{
	memset(m_keysStates,         false, sizeof(bool) * (int)Keys::Last);
	memset(m_previousKeysStates, false, sizeof(bool) * (int)Keys::Last);

	memset(m_getKeysDown,        false, sizeof(bool) * (int)Keys::Last);
	memset(m_getKeysUp,          false, sizeof(bool) * (int)Keys::Last);

	m_glfwToKeysMapping[GLFW_KEY_W     ] = { Keys::Up    };
	m_glfwToKeysMapping[GLFW_KEY_A     ] = { Keys::Left  };
	m_glfwToKeysMapping[GLFW_KEY_S     ] = { Keys::Down  };
	m_glfwToKeysMapping[GLFW_KEY_D     ] = { Keys::Right };

	m_glfwToKeysMapping[GLFW_KEY_T     ] = { Keys::Debug };
	m_glfwToKeysMapping[GLFW_KEY_ESCAPE] = { Keys::Exit  };
}
