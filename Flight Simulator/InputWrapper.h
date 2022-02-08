#pragma once
#include <GLFW/glfw3.h>
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

	static InputWrapper* GetInstance();
	static void          FreeInstance();

		   void          KeyCallback(GLFWwindow*, int, int, int, int);
		   void          Update();

		   bool          GetKey(Keys);
		   bool          GetKeyDown(Keys);
		   bool          GetKeyUp(Keys);

private:

	InputWrapper();

private:

	       bool                                       m_keysStates[(int)Keys::Last];
	       bool                                       m_previousKeysStates[(int)Keys::Last];

		   bool                                       m_getKeysDown[(int)Keys::Last];
		   bool                                       m_getKeysUp[(int)Keys::Last];

	       std::unordered_map<int, std::vector<Keys>> m_glfwToKeysMapping;

	static InputWrapper*                              g_instance;
};