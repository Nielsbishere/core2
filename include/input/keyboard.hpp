#pragma once
#include "input_device.hpp"
#include "types/enum.hpp"

namespace oic {

	//Platform independent key code

	oicExposedEnum(

		Key, ButtonHandle,

		KEY_0, KEY_1, KEY_2, KEY_3, KEY_4, KEY_5, KEY_6, KEY_7, KEY_8, KEY_9,
		KEY_A, KEY_B, KEY_C, KEY_D, KEY_E, KEY_F, KEY_G, KEY_H, KEY_I, KEY_J,
		KEY_K, KEY_L, KEY_M, KEY_N, KEY_O, KEY_P, KEY_Q, KEY_R, KEY_S, KEY_T,
		KEY_U, KEY_V, KEY_W, KEY_X, KEY_Y, KEY_Z,

		KEY_BACKSPACE, KEY_SPACE, KEY_TAB, KEY_SHIFT, KEY_CTRL, KEY_ALT,
		KEY_PAUSE, KEY_CAPS_LOCK, KEY_ESCAPE, KEY_PAGE_UP, KEY_PAGE_DOWN,
		KEY_END, KEY_HOME, KEY_SELECT, KEY_PRINT, KEY_EXECUTE, KEY_PRINT_SCREEN,
		KEY_INSERT, KEY_DELETE, KEY_HELP, KEY_MENU, KEY_NUM_LOCK, KEY_SCROLL_LOCK,
		KEY_APPS, KEY_BACK, KEY_FORWARD, KEY_SLEEP, KEY_REFRESH, KEY_STOP, KEY_SEARCH,
		KEY_FAVORITES, KEY_START, KEY_MUTE, KEY_VOLUME_DOWN, KEY_VOLUME_UP,
		KEY_SKIP, KEY_PREVIOUS, KEY_CLEAR, KEY_ZOOM,

		KEY_LEFT, KEY_RIGHT, KEY_UP, KEY_DOWN, 

		KEY_NUMPAD_1, KEY_NUMPAD_2, KEY_NUMPAD_3,
		KEY_NUMPAD_4, KEY_NUMPAD_5, KEY_NUMPAD_6,
		KEY_NUMPAD_7, KEY_NUMPAD_8, KEY_NUMPAD_9,
		KEY_NUMPAD_0, KEY_NUMPAD_MULTIPLY, KEY_NUMPAD_ADD,
		KEY_NUMPAD_DECIMAL, KEY_NUMPAD_DIVIDE, KEY_NUMPAD_SUBTRACT,

		KEY_F1, KEY_F2, KEY_F3, KEY_F4, KEY_F5, KEY_F6, KEY_F7, KEY_F8, KEY_F9,
		KEY_F12, KEY_F13, KEY_F14, KEY_F15, KEY_F16, KEY_F17, KEY_F18, KEY_F19,
		KEY_F20, KEY_F21, KEY_F22, KEY_F23, KEY_F24,

		KEY_PLUS, KEY_COMMA, KEY_MINUS, KEY_PERIOD, 

		KEY_SLASH, KEY_TILDE, KEY_SEMICOLON, KEY_BEGIN_BRACKET, KEY_END_BRACKET, KEY_PIPE,
		KEY_QUOTE
	);

	//

	class Keyboard : public InputDevice {

	public:

		Keyboard(): InputDevice(Type::KEYBOARD, Key::count, 0) {}

		inline Handle handleByName(const String &str) const override final { return Handle(Key::idByName<EnumNameFormat::LOWERCASE_SPACE>(str)); }
		inline String nameByHandle(Handle id) const override final  { return Key::nameById<EnumNameFormat::LOWERCASE_SPACE>(id); }

		//Platform and device dependent
		bool isSupported(Handle handle) const;

	};

}