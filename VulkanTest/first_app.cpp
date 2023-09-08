#include "first_app.hpp"

void lve::FirstApp::run()
{
	while (!lveWindow.shouldClose())
	{
		glfwPollEvents();
	}
}
