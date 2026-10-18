//#define DEBUG_CONSOLE // Uncomment this if you want a debug console to start. You can use the Console class to print. You can use Console::inStrings to get input.

#include <4dm.h>
#include "4DKeyBinds.h"

using namespace fdm;

struct Rect
{
	int x, y, w, h;
};

Rect oldWindowRect{0,0,0,0};

// Initialize the DLLMain
initDLL

// https://stackoverflow.com/questions/21421074/how-to-create-a-full-screen-window-on-the-current-monitor-with-glfw
GLFWmonitor* getCurrentMonitor(GLFWwindow* window)
{
	GLFWmonitor* bestMonitor = glfwGetPrimaryMonitor();

	int wx, wy;
	int ww, wh;
	glfwGetWindowPos(window, &wx, &wy);
	glfwGetWindowSize(window, &ww, &wh);
	int nMonitors = 0;
	GLFWmonitor** monitors = glfwGetMonitors(&nMonitors);

	int bestOverlap = 0;
	for (int i = 0; i < nMonitors; ++i)
	{
		const GLFWvidmode* mode = glfwGetVideoMode(monitors[i]);

		int mx, my;
		int mw, mh;
		glfwGetMonitorPos(monitors[i], &mx, &my);
		mw = mode->width;
		mh = mode->height;

		int overlap =
			std::max(0, std::min(wx + ww, mx + mw) - std::max(wx, mx)) *
			std::max(0, std::min(wy + wh, my + mh) - std::max(wy, my));

		if (bestOverlap < overlap)
		{
			bestOverlap = overlap;
			bestMonitor = monitors[i];
		}
	}

	return bestMonitor;
}

//		void setFullscreenMode(GLFWwindow* window, bool fullscreenMode) 
$hook(void, StateSettings, setFullscreenMode, GLFWwindow* window, bool fullscreen)
{
	auto curMonitor = glfwGetWindowMonitor(window);
	if (!curMonitor && fullscreen)
	{
		glfwGetWindowSize(window, &oldWindowRect.w, &oldWindowRect.h);
		glfwGetWindowPos(window, &oldWindowRect.x, &oldWindowRect.y);

		curMonitor = getCurrentMonitor(window);
		auto mode = glfwGetVideoMode(curMonitor);
		glfwSetWindowMonitor(window, curMonitor, 0, 0, mode->width, mode->height, mode->refreshRate);
	}
	else if (curMonitor && !fullscreen)
	{
		curMonitor = nullptr;
		glfwSetWindowMonitor(window, curMonitor, oldWindowRect.x, oldWindowRect.y, oldWindowRect.w, oldWindowRect.h, 0);
	}
	self->fullscreen = fullscreen;
	self->fullscreenButton.setText(fullscreen ? "Exit Fullscreen" : "Enter Fullscreen");
}

$hook(void, StateIntro, init, StateManager& s)
{
	glfwInit();
	original(self, s);

	// initialize opengl stuff
	glewExperimental = true;
	glewInit();
}

static void toggleFullscreen(GLFWwindow* window, int action, int mods)
{
	if (action != GLFW_PRESS) return;

	StateSettings::instanceObj.setFullscreenMode(window, !StateSettings::instanceObj.fullscreen);
}

$hookStatic(void, main_cpp, keyCallback, GLFWwindow* window, int key, int scancode, int action, int mods)
{
	if (!KeyBinds::isLoaded())
	{
		if (key == GLFW_KEY_F11)
		{
			toggleFullscreen(window, action, mods);
		}
	}
	original(window, key, scancode, action, mods);
}

$exec
{
	KeyBinds::addBind("Better Fullscreen", "Toggle Fullscreen", glfw::Keys::F11, KeyBindsScope::GLOBAL, toggleFullscreen);
}