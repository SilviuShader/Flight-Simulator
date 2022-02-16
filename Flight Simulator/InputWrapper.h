#pragma once
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <unordered_map>

class InputWrapper
{
public:

	enum class Keys
	{
		Up,
		Down,
		Left,
		Right,

		Debug,
		Exit,

		Last
	};

public:

	InputWrapper(const InputWrapper&)   = delete;
	void operator=(const InputWrapper&) = delete;

		   void          KeyCallback(GLFWwindow*, int, int, int, int);
		   void          MouseCallback(GLFWwindow*, double, double);
		   void          Update();

		   bool          GetKey(Keys)       const;
		   bool          GetKeyDown(Keys)   const;
		   bool          GetKeyUp(Keys)     const;

		   glm::vec2     GetMouseMoveDiff() const;

	static InputWrapper* GetInstance();
	static void          FreeInstance();

private:

	InputWrapper();

private:

	       bool                                       m_keysStates[(int)Keys::Last];
	       bool                                       m_previousKeysStates[(int)Keys::Last];

		   bool                                       m_getKeysDown[(int)Keys::Last];
		   bool                                       m_getKeysUp[(int)Keys::Last];

	       std::unordered_map<int, std::vector<Keys>> m_glfwToKeysMapping;

		   glm::vec2                                  m_mousePos;
		   glm::vec2                                  m_previousMousePos;
		   glm::vec2                                  m_mouseMoveDiff;

		   bool                                       m_movedMouseOnce;
		   
	static InputWrapper*                              g_instance;
};