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

#include "android_platform.h"

#include <chrono>
#include <unistd.h>
#include <unordered_map>

#include "common/error.h"

VKBP_DISABLE_WARNINGS()
#include <imgui.h>
#include <spdlog/sinks/android_sink.h>
VKBP_ENABLE_WARNINGS()

#include "common/logging.h"
#include "platform/input_events.h"
#include "vulkan_sample.h"

namespace vkb
{
namespace
{
inline KeyCode translate_key_code(int key)
{
	static const std::unordered_map<int, KeyCode> key_lookup =
	    {
	        {AKEYCODE_SPACE, KeyCode::Space},
	        {AKEYCODE_APOSTROPHE, KeyCode::Apostrophe},
	        {AKEYCODE_COMMA, KeyCode::Comma},
	        {AKEYCODE_MINUS, KeyCode::Minus},
	        {AKEYCODE_PERIOD, KeyCode::Period},
	        {AKEYCODE_SLASH, KeyCode::Slash},
	        {AKEYCODE_0, KeyCode::_0},
	        {AKEYCODE_1, KeyCode::_1},
	        {AKEYCODE_2, KeyCode::_2},
	        {AKEYCODE_3, KeyCode::_3},
	        {AKEYCODE_4, KeyCode::_4},
	        {AKEYCODE_5, KeyCode::_5},
	        {AKEYCODE_6, KeyCode::_6},
	        {AKEYCODE_7, KeyCode::_7},
	        {AKEYCODE_8, KeyCode::_8},
	        {AKEYCODE_9, KeyCode::_9},
	        {AKEYCODE_SEMICOLON, KeyCode::Semicolon},
	        {AKEYCODE_EQUALS, KeyCode::Equal},
	        {AKEYCODE_A, KeyCode::A},
	        {AKEYCODE_B, KeyCode::B},
	        {AKEYCODE_C, KeyCode::C},
	        {AKEYCODE_D, KeyCode::D},
	        {AKEYCODE_E, KeyCode::E},
	        {AKEYCODE_F, KeyCode::F},
	        {AKEYCODE_G, KeyCode::G},
	        {AKEYCODE_H, KeyCode::H},
	        {AKEYCODE_I, KeyCode::I},
	        {AKEYCODE_J, KeyCode::J},
	        {AKEYCODE_K, KeyCode::K},
	        {AKEYCODE_L, KeyCode::L},
	        {AKEYCODE_M, KeyCode::M},
	        {AKEYCODE_N, KeyCode::N},
	        {AKEYCODE_O, KeyCode::O},
	        {AKEYCODE_P, KeyCode::P},
	        {AKEYCODE_Q, KeyCode::Q},
	        {AKEYCODE_R, KeyCode::R},
	        {AKEYCODE_S, KeyCode::S},
	        {AKEYCODE_T, KeyCode::T},
	        {AKEYCODE_U, KeyCode::U},
	        {AKEYCODE_V, KeyCode::V},
	        {AKEYCODE_W, KeyCode::W},
	        {AKEYCODE_X, KeyCode::X},
	        {AKEYCODE_Y, KeyCode::Y},
	        {AKEYCODE_Z, KeyCode::Z},
	        {AKEYCODE_LEFT_BRACKET, KeyCode::LeftBracket},
	        {AKEYCODE_BACKSLASH, KeyCode::Backslash},
	        {AKEYCODE_RIGHT_BRACKET, KeyCode::RightBracket},
	        {AKEYCODE_ESCAPE, KeyCode::Escape},
	        {AKEYCODE_BACK, KeyCode::Back},
	        {AKEYCODE_ENTER, KeyCode::Enter},
	        {AKEYCODE_TAB, KeyCode::Tab},
	        {AKEYCODE_DEL, KeyCode::Backspace},
	        {AKEYCODE_INSERT, KeyCode::Insert},
	        {AKEYCODE_DEL, KeyCode::DelKey},
	        {AKEYCODE_SYSTEM_NAVIGATION_RIGHT, KeyCode::Right},
	        {AKEYCODE_SYSTEM_NAVIGATION_LEFT, KeyCode::Left},
	        {AKEYCODE_SYSTEM_NAVIGATION_DOWN, KeyCode::Down},
	        {AKEYCODE_SYSTEM_NAVIGATION_UP, KeyCode::Up},
	        {AKEYCODE_PAGE_UP, KeyCode::PageUp},
	        {AKEYCODE_PAGE_DOWN, KeyCode::PageDown},
	        {AKEYCODE_HOME, KeyCode::Home},
	        {AKEYCODE_CAPS_LOCK, KeyCode::CapsLock},
	        {AKEYCODE_SCROLL_LOCK, KeyCode::ScrollLock},
	        {AKEYCODE_NUM_LOCK, KeyCode::NumLock},
	        {AKEYCODE_BREAK, KeyCode::Pause},
	        {AKEYCODE_F1, KeyCode::F1},
	        {AKEYCODE_F2, KeyCode::F2},
	        {AKEYCODE_F3, KeyCode::F3},
	        {AKEYCODE_F4, KeyCode::F4},
	        {AKEYCODE_F5, KeyCode::F5},
	        {AKEYCODE_F6, KeyCode::F6},
	        {AKEYCODE_F7, KeyCode::F7},
	        {AKEYCODE_F8, KeyCode::F8},
	        {AKEYCODE_F9, KeyCode::F9},
	        {AKEYCODE_F10, KeyCode::F10},
	        {AKEYCODE_F11, KeyCode::F11},
	        {AKEYCODE_F12, KeyCode::F12},
	        {AKEYCODE_NUMPAD_0, KeyCode::KP_0},
	        {AKEYCODE_NUMPAD_1, KeyCode::KP_1},
	        {AKEYCODE_NUMPAD_2, KeyCode::KP_2},
	        {AKEYCODE_NUMPAD_3, KeyCode::KP_3},
	        {AKEYCODE_NUMPAD_4, KeyCode::KP_4},
	        {AKEYCODE_NUMPAD_5, KeyCode::KP_5},
	        {AKEYCODE_NUMPAD_6, KeyCode::KP_6},
	        {AKEYCODE_NUMPAD_7, KeyCode::KP_7},
	        {AKEYCODE_NUMPAD_8, KeyCode::KP_8},
	        {AKEYCODE_NUMPAD_9, KeyCode::KP_9},
	        {AKEYCODE_NUMPAD_DOT, KeyCode::KP_Decimal},
	        {AKEYCODE_NUMPAD_DIVIDE, KeyCode::KP_Divide},
	        {AKEYCODE_NUMPAD_MULTIPLY, KeyCode::KP_Multiply},
	        {AKEYCODE_NUMPAD_SUBTRACT, KeyCode::KP_Subtract},
	        {AKEYCODE_NUMPAD_ADD, KeyCode::KP_Add},
	        {AKEYCODE_NUMPAD_ENTER, KeyCode::KP_Enter},
	        {AKEYCODE_NUMPAD_EQUALS, KeyCode::KP_Equal},
	        {AKEYCODE_SHIFT_LEFT, KeyCode::LeftShift},
	        {AKEYCODE_CTRL_LEFT, KeyCode::LeftControl},
	        {AKEYCODE_ALT_LEFT, KeyCode::LeftAlt},
	        {AKEYCODE_SHIFT_RIGHT, KeyCode::RightShift},
	        {AKEYCODE_CTRL_RIGHT, KeyCode::RightControl},
	        {AKEYCODE_ALT_RIGHT, KeyCode::RightAlt}};

	auto key_it = key_lookup.find(key);

	if (key_it == key_lookup.end())
	{
		return KeyCode::Unknown;
	}

	return key_it->second;
}

inline KeyAction translate_key_action(int action)
{
	if (action == AKEY_STATE_DOWN)
	{
		return KeyAction::Down;
	}
	else if (action == AKEY_STATE_UP)
	{
		return KeyAction::Up;
	}

	return KeyAction::Unknown;
}

inline MouseButton translate_mouse_button(int button)
{
	if (button < 3)
	{
		return static_cast<MouseButton>(button);
	}

	return MouseButton::Unknown;
}

inline MouseAction translate_mouse_action(int action)
{
	if (action == AMOTION_EVENT_ACTION_DOWN)
	{
		return MouseAction::Down;
	}
	else if (action == AMOTION_EVENT_ACTION_UP)
	{
		return MouseAction::Up;
	}
	else if (action == AMOTION_EVENT_ACTION_MOVE)
	{
		return MouseAction::Move;
	}

	return MouseAction::Unknown;
}

inline TouchAction translate_touch_action(int action)
{
	action &= AMOTION_EVENT_ACTION_MASK;
	if (action == AMOTION_EVENT_ACTION_DOWN || action == AMOTION_EVENT_ACTION_POINTER_DOWN)
	{
		return TouchAction::Down;
	}
	else if (action == AMOTION_EVENT_ACTION_UP || action == AMOTION_EVENT_ACTION_POINTER_UP)
	{
		return TouchAction::Up;
	}
	else if (action == AMOTION_EVENT_ACTION_CANCEL)
	{
		return TouchAction::Cancel;
	}
	else if (action == AMOTION_EVENT_ACTION_MOVE)
	{
		return TouchAction::Move;
	}

	return TouchAction::Unknown;
}

void on_content_rect_changed(ANativeActivity *activity, const ARect *rect)
{
	LOGI("ContentRectChanged: {:p}\n", static_cast<void *>(activity));
	struct android_app *app = reinterpret_cast<struct android_app *>(activity->instance);
	auto                cmd = APP_CMD_CONTENT_RECT_CHANGED;

	app->contentRect = *rect;

	if (write(app->msgwrite, &cmd, sizeof(cmd)) != sizeof(cmd))
	{
		LOGE("Failure writing android_app cmd: {}\n", strerror(errno));
	}
}

void on_app_cmd(android_app *app, int32_t cmd)
{
	auto platform = reinterpret_cast<AndroidPlatform *>(app->userData);
	assert(platform && "Platform is not valid");

	switch (cmd)
	{
		case APP_CMD_INIT_WINDOW:
		{
			app->destroyRequested = !platform->get_app().prepare(*platform);
			break;
		}
		case APP_CMD_CONTENT_RECT_CHANGED:
		{
			// Get the new size
			auto width  = app->contentRect.right - app->contentRect.left;
			auto height = app->contentRect.bottom - app->contentRect.top;
			platform->get_app().resize(width, height);
			break;
		}
		case APP_CMD_GAINED_FOCUS:
		{
			platform->get_app().set_focus(true);
			break;
		}
		case APP_CMD_LOST_FOCUS:
		{
			platform->get_app().set_focus(false);
			break;
		}
		case APP_CMD_TERM_WINDOW:
		{
			platform->get_app().finish();
			break;
		}
	}
}

int32_t on_input_event(android_app *app, AInputEvent *input_event)
{
	auto platform = reinterpret_cast<AndroidPlatform *>(app->userData);
	assert(platform && "Platform is not valid");

	std::int32_t event_source = AInputEvent_getSource(input_event);

	if (event_source == AINPUT_SOURCE_KEYBOARD)
	{
		int32_t key_code = AKeyEvent_getKeyCode(input_event);
		int32_t action   = AKeyEvent_getAction(input_event);

		platform->get_app().input_event(KeyInputEvent{
		    *platform,
		    translate_key_code(key_code),
		    translate_key_action(action)});
	}
	else if (event_source == AINPUT_SOURCE_MOUSE)
	{
		std::int32_t action = AMotionEvent_getAction(input_event);

		float x = AMotionEvent_getX(input_event, 0);
		float y = AMotionEvent_getY(input_event, 0);

		platform->get_app().input_event(MouseButtonInputEvent{
		    *platform,
		    translate_mouse_button(0),
		    translate_mouse_action(action),
		    x, y});
	}
	else if (event_source == AINPUT_SOURCE_TOUCHSCREEN)
	{
		size_t       pointer_count = AMotionEvent_getPointerCount(input_event);
		std::int32_t action        = AMotionEvent_getAction(input_event);
		std::int32_t pointer_id    = AMotionEvent_getPointerId(input_event, 0);

		float x = AMotionEvent_getX(input_event, 0);
		float y = AMotionEvent_getY(input_event, 0);

		platform->get_app().input_event(TouchInputEvent{
		    *platform,
		    pointer_id,
		    pointer_count,
		    translate_touch_action(action),
		    x, y});
	}
	else
	{
		return 0;
	}

	return 1;
}
}        // namespace

AndroidPlatform::AndroidPlatform(android_app *app) :
    app{app}
{
	auto android_logger = spdlog::android_logger_mt("android", PROJECT_NAME);
	android_logger->set_pattern(LOGGER_FORMAT);
	spdlog::set_default_logger(android_logger);
}

bool AndroidPlatform::initialize(std::unique_ptr<Application> &&appplication)
{
	app->onAppCmd                                  = on_app_cmd;
	app->onInputEvent                              = on_input_event;
	app->activity->callbacks->onContentRectChanged = on_content_rect_changed;
	app->userData                                  = this;

	assert(appplication && "Appplication is not valid");
	active_app = std::move(appplication);

	return true;
}

VkSurfaceKHR AndroidPlatform::create_surface(VkInstance instance)
{
	if (instance == VK_NULL_HANDLE || !app->window)
	{
		return VK_NULL_HANDLE;
	}

	VkSurfaceKHR surface{};

	VkAndroidSurfaceCreateInfoKHR info{VK_STRUCTURE_TYPE_ANDROID_SURFACE_CREATE_INFO_KHR};

	info.window = app->window;

	VK_CHECK(vkCreateAndroidSurfaceKHR(instance, &info, nullptr, &surface));

	return surface;
}

void AndroidPlatform::main_loop()
{
	while (true)
	{
		android_poll_source *source;

		int ident;
		int events;

		while ((ident = ALooper_pollAll(0, nullptr, &events,
		                                (void **) &source)) >= 0)
		{
			if (source)
			{
				source->process(app, source);
			}
		}

		if (app->destroyRequested)
		{
			break;
		}

		if (app->window && active_app->is_focused())
		{
			active_app->step();
		}
	}
}

void AndroidPlatform::terminate()
{
	active_app.reset();
	spdlog::drop_all();
}

void AndroidPlatform::close() const
{
	ANativeActivity_finish(app->activity);
}

ANativeActivity *AndroidPlatform::get_activity()
{
	return app->activity;
}

float AndroidPlatform::get_dpi_factor() const
{
	return AConfiguration_getDensity(app->config) / static_cast<float>(ACONFIGURATION_DENSITY_MEDIUM);
}
}        // namespace vkb
