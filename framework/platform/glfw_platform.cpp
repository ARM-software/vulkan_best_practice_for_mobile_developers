/* Copyright (c) 2019, Arm Limited and Contributors
 *
 * SPDX-License-Identifier: MIT
 *
 * Permission is hereby granted, free of charge,
 * to any person obtaining a copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation the rights to
 * use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software,
 * and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED,
 * INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

#include "glfw_platform.h"

#include <unordered_map>

#include "common/error.h"

VKBP_DISABLE_WARNINGS()
#include <GLFW/glfw3.h>
#include <GLFW/glfw3native.h>
#include <spdlog/sinks/stdout_color_sinks.h>
VKBP_ENABLE_WARNINGS()

#include "application.h"
#include "common/logging.h"
#include "input_events.h"

namespace vkb
{
namespace
{
void error_callback(int error, const char *description)
{
	LOGE("GLFW Error (code {}): {}", error, description);
}

void window_close_callback(GLFWwindow *window)
{
	glfwSetWindowShouldClose(window, GLFW_TRUE);
}

void window_size_callback(GLFWwindow *window, int width, int height)
{
	if (auto platform = reinterpret_cast<GlfwPlatform *>(glfwGetWindowUserPointer(window)))
	{
		platform->get_app().resize(width, height);
	}
}

void window_focus_callback(GLFWwindow *window, int focused)
{
	if (auto platform = reinterpret_cast<GlfwPlatform *>(glfwGetWindowUserPointer(window)))
	{
		platform->get_app().set_focus(focused ? true : false);
	}
}

inline KeyCode translate_key_code(int key)
{
	static const std::unordered_map<int, KeyCode> key_lookup =
	    {
	        {GLFW_KEY_SPACE, KeyCode::Space},
	        {GLFW_KEY_APOSTROPHE, KeyCode::Apostrophe},
	        {GLFW_KEY_COMMA, KeyCode::Comma},
	        {GLFW_KEY_MINUS, KeyCode::Minus},
	        {GLFW_KEY_PERIOD, KeyCode::Period},
	        {GLFW_KEY_SLASH, KeyCode::Slash},
	        {GLFW_KEY_0, KeyCode::_0},
	        {GLFW_KEY_1, KeyCode::_1},
	        {GLFW_KEY_2, KeyCode::_2},
	        {GLFW_KEY_3, KeyCode::_3},
	        {GLFW_KEY_4, KeyCode::_4},
	        {GLFW_KEY_5, KeyCode::_5},
	        {GLFW_KEY_6, KeyCode::_6},
	        {GLFW_KEY_7, KeyCode::_7},
	        {GLFW_KEY_8, KeyCode::_8},
	        {GLFW_KEY_9, KeyCode::_9},
	        {GLFW_KEY_SEMICOLON, KeyCode::Semicolon},
	        {GLFW_KEY_EQUAL, KeyCode::Equal},
	        {GLFW_KEY_A, KeyCode::A},
	        {GLFW_KEY_B, KeyCode::B},
	        {GLFW_KEY_C, KeyCode::C},
	        {GLFW_KEY_D, KeyCode::D},
	        {GLFW_KEY_E, KeyCode::E},
	        {GLFW_KEY_F, KeyCode::F},
	        {GLFW_KEY_G, KeyCode::G},
	        {GLFW_KEY_H, KeyCode::H},
	        {GLFW_KEY_I, KeyCode::I},
	        {GLFW_KEY_J, KeyCode::J},
	        {GLFW_KEY_K, KeyCode::K},
	        {GLFW_KEY_L, KeyCode::L},
	        {GLFW_KEY_M, KeyCode::M},
	        {GLFW_KEY_N, KeyCode::N},
	        {GLFW_KEY_O, KeyCode::O},
	        {GLFW_KEY_P, KeyCode::P},
	        {GLFW_KEY_Q, KeyCode::Q},
	        {GLFW_KEY_R, KeyCode::R},
	        {GLFW_KEY_S, KeyCode::S},
	        {GLFW_KEY_T, KeyCode::T},
	        {GLFW_KEY_U, KeyCode::U},
	        {GLFW_KEY_V, KeyCode::V},
	        {GLFW_KEY_W, KeyCode::W},
	        {GLFW_KEY_X, KeyCode::X},
	        {GLFW_KEY_Y, KeyCode::Y},
	        {GLFW_KEY_Z, KeyCode::Z},
	        {GLFW_KEY_LEFT_BRACKET, KeyCode::LeftBracket},
	        {GLFW_KEY_BACKSLASH, KeyCode::Backslash},
	        {GLFW_KEY_RIGHT_BRACKET, KeyCode::RightBracket},
	        {GLFW_KEY_GRAVE_ACCENT, KeyCode::GraveAccent},
	        {GLFW_KEY_ESCAPE, KeyCode::Escape},
	        {GLFW_KEY_ENTER, KeyCode::Enter},
	        {GLFW_KEY_TAB, KeyCode::Tab},
	        {GLFW_KEY_BACKSPACE, KeyCode::Backspace},
	        {GLFW_KEY_INSERT, KeyCode::Insert},
	        {GLFW_KEY_DELETE, KeyCode::DelKey},
	        {GLFW_KEY_RIGHT, KeyCode::Right},
	        {GLFW_KEY_LEFT, KeyCode::Left},
	        {GLFW_KEY_DOWN, KeyCode::Down},
	        {GLFW_KEY_UP, KeyCode::Up},
	        {GLFW_KEY_PAGE_UP, KeyCode::PageUp},
	        {GLFW_KEY_PAGE_DOWN, KeyCode::PageDown},
	        {GLFW_KEY_HOME, KeyCode::Home},
	        {GLFW_KEY_END, KeyCode::End},
	        {GLFW_KEY_CAPS_LOCK, KeyCode::CapsLock},
	        {GLFW_KEY_SCROLL_LOCK, KeyCode::ScrollLock},
	        {GLFW_KEY_NUM_LOCK, KeyCode::NumLock},
	        {GLFW_KEY_PRINT_SCREEN, KeyCode::PrintScreen},
	        {GLFW_KEY_PAUSE, KeyCode::Pause},
	        {GLFW_KEY_F1, KeyCode::F1},
	        {GLFW_KEY_F2, KeyCode::F2},
	        {GLFW_KEY_F3, KeyCode::F3},
	        {GLFW_KEY_F4, KeyCode::F4},
	        {GLFW_KEY_F5, KeyCode::F5},
	        {GLFW_KEY_F6, KeyCode::F6},
	        {GLFW_KEY_F7, KeyCode::F7},
	        {GLFW_KEY_F8, KeyCode::F8},
	        {GLFW_KEY_F9, KeyCode::F9},
	        {GLFW_KEY_F10, KeyCode::F10},
	        {GLFW_KEY_F11, KeyCode::F11},
	        {GLFW_KEY_F12, KeyCode::F12},
	        {GLFW_KEY_KP_0, KeyCode::KP_0},
	        {GLFW_KEY_KP_1, KeyCode::KP_1},
	        {GLFW_KEY_KP_2, KeyCode::KP_2},
	        {GLFW_KEY_KP_3, KeyCode::KP_3},
	        {GLFW_KEY_KP_4, KeyCode::KP_4},
	        {GLFW_KEY_KP_5, KeyCode::KP_5},
	        {GLFW_KEY_KP_6, KeyCode::KP_6},
	        {GLFW_KEY_KP_7, KeyCode::KP_7},
	        {GLFW_KEY_KP_8, KeyCode::KP_8},
	        {GLFW_KEY_KP_9, KeyCode::KP_9},
	        {GLFW_KEY_KP_DECIMAL, KeyCode::KP_Decimal},
	        {GLFW_KEY_KP_DIVIDE, KeyCode::KP_Divide},
	        {GLFW_KEY_KP_MULTIPLY, KeyCode::KP_Multiply},
	        {GLFW_KEY_KP_SUBTRACT, KeyCode::KP_Subtract},
	        {GLFW_KEY_KP_ADD, KeyCode::KP_Add},
	        {GLFW_KEY_KP_ENTER, KeyCode::KP_Enter},
	        {GLFW_KEY_KP_EQUAL, KeyCode::KP_Equal},
	        {GLFW_KEY_LEFT_SHIFT, KeyCode::LeftShift},
	        {GLFW_KEY_LEFT_CONTROL, KeyCode::LeftControl},
	        {GLFW_KEY_LEFT_ALT, KeyCode::LeftAlt},
	        {GLFW_KEY_RIGHT_SHIFT, KeyCode::RightShift},
	        {GLFW_KEY_RIGHT_CONTROL, KeyCode::RightControl},
	        {GLFW_KEY_RIGHT_ALT, KeyCode::RightAlt},
	    };

	auto key_it = key_lookup.find(key);

	if (key_it == key_lookup.end())
	{
		return KeyCode::Unknown;
	}

	return key_it->second;
}

inline KeyAction translate_key_action(int action)
{
	if (action == GLFW_PRESS)
	{
		return KeyAction::Down;
	}
	else if (action == GLFW_RELEASE)
	{
		return KeyAction::Up;
	}
	else if (action == GLFW_REPEAT)
	{
		return KeyAction::Repeat;
	}

	return KeyAction::Unknown;
}

void key_callback(GLFWwindow *window, int key, int /*scancode*/, int action, int /*mods*/)
{
	KeyCode   key_code   = translate_key_code(key);
	KeyAction key_action = translate_key_action(action);

	if (auto platform = reinterpret_cast<GlfwPlatform *>(glfwGetWindowUserPointer(window)))
	{
		platform->get_app().input_event(KeyInputEvent{*platform, key_code, key_action});
	}
}

inline MouseButton translate_mouse_button(int button)
{
	if (button < GLFW_MOUSE_BUTTON_6)
	{
		return static_cast<MouseButton>(button);
	}

	return MouseButton::Unknown;
}

inline MouseAction translate_mouse_action(int action)
{
	if (action == GLFW_PRESS)
	{
		return MouseAction::Down;
	}
	else if (action == GLFW_RELEASE)
	{
		return MouseAction::Up;
	}

	return MouseAction::Unknown;
}

void cursor_position_callback(GLFWwindow *window, double xpos, double ypos)
{
	if (auto platform = reinterpret_cast<GlfwPlatform *>(glfwGetWindowUserPointer(window)))
	{
		platform->get_app().input_event(MouseButtonInputEvent{
		    *platform,
		    MouseButton::Unknown,
		    MouseAction::Move,
		    static_cast<float>(xpos),
		    static_cast<float>(ypos)});
	}
}

void mouse_button_callback(GLFWwindow *window, int button, int action, int /*mods*/)
{
	MouseAction mouse_action = translate_mouse_action(action);

	if (auto platform = reinterpret_cast<GlfwPlatform *>(glfwGetWindowUserPointer(window)))
	{
		double xpos, ypos;
		glfwGetCursorPos(window, &xpos, &ypos);

		platform->get_app().input_event(MouseButtonInputEvent{
		    *platform,
		    translate_mouse_button(button),
		    mouse_action,
		    static_cast<float>(xpos),
		    static_cast<float>(ypos)});
	}
}
}        // namespace

bool GlfwPlatform::initialize(std::unique_ptr<Application> &&app)
{
	if (!glfwInit())
	{
		return false;
	}

	glfwSetErrorCallback(error_callback);

	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

	if (arguments.contains("offscreen"))
	{
		glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);
	}

	window = glfwCreateWindow(1280, 720, app->get_name().c_str(), NULL, NULL);

	if (!window)
	{
		return false;
	}

	glfwSetWindowUserPointer(window, this);

	glfwSetWindowCloseCallback(window, window_close_callback);
	glfwSetWindowSizeCallback(window, window_size_callback);
	glfwSetWindowFocusCallback(window, window_focus_callback);
	glfwSetKeyCallback(window, key_callback);
	glfwSetCursorPosCallback(window, cursor_position_callback);
	glfwSetMouseButtonCallback(window, mouse_button_callback);

	glfwSetInputMode(window, GLFW_STICKY_KEYS, 1);
	glfwSetInputMode(window, GLFW_STICKY_MOUSE_BUTTONS, 1);

	auto console = spdlog::stdout_color_mt("console");
	console->set_pattern(LOGGER_FORMAT);
	spdlog::set_default_logger(console);

	return Platform::initialize(std::move(app));
}

VkSurfaceKHR GlfwPlatform::create_surface(VkInstance instance)
{
	if (instance == VK_NULL_HANDLE || !window)
	{
		return VK_NULL_HANDLE;
	}

	VkSurfaceKHR surface;

	VkResult errCode = glfwCreateWindowSurface(instance, window, NULL, &surface);

	if (errCode != VK_SUCCESS)
	{
		return VK_NULL_HANDLE;
	}

	return surface;
}

void GlfwPlatform::main_loop()
{
	while (!glfwWindowShouldClose(window))
	{
		if (active_app->is_focused())
		{
			active_app->step();
		}

		glfwPollEvents();
	}
}

void GlfwPlatform::terminate()
{
	Platform::terminate();

	glfwTerminate();
}

void GlfwPlatform::close() const
{
	glfwSetWindowShouldClose(window, GLFW_TRUE);
}

/// @brief It calculates the dpi factor using the density from GLFW physical size
/// <a href="https://www.glfw.org/docs/latest/monitor_guide.html#monitor_size">GLFW docs for dpi</a>
float GlfwPlatform::get_dpi_factor() const
{
	auto primary_monitor = glfwGetPrimaryMonitor();
	auto vidmode         = glfwGetVideoMode(primary_monitor);

	int width_mm, height_mm;
	glfwGetMonitorPhysicalSize(primary_monitor, &width_mm, &height_mm);

	// As suggested by the GLFW monitor guide
	static const float inch_to_mm       = 25.0f;
	static const float win_base_density = 96.0f;

	auto dpi        = static_cast<uint32_t>(vidmode->width / (width_mm / inch_to_mm));
	auto dpi_factor = dpi / win_base_density;
	return dpi_factor;
}
}        // namespace vkb
