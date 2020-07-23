#include "input/keyboard.hpp"

namespace oic {

	c32 Keyboard::getKey(Key key) const {

		bool isShift = getCurrentState(Key::Key_shift);

		if (!isShift && key >= Key::Key_0 && key <= Key::Key_9)
			return '0' + c32(key - Key::Key_0);

		else if (key >= Key::Key_a && key <= Key::Key_z)
			return (isShift ? 'A' : 'a') + c32(key - Key::Key_a);

		switch (key.value) {

			case Key::Key_plus:				return isShift ? '=' : '+';
			case Key::Key_comma:			return isShift ? '<' : ',';
			case Key::Key_minus:			return isShift ? '_' : '-';
			case Key::Key_period:			return isShift ? '>' : '.';
			
			case Key::Key_slash:			return isShift ? '/' : '?';
			case Key::Key_tilde:			return isShift ? '~' : '`';
			case Key::Key_semicolon:		return isShift ? ':' : ';';
			case Key::Key_quote:			return isShift ? '"' : '\'';
			case Key::Key_begin_bracket:	return isShift ? '{' : '[';
			case Key::Key_end_bracket:		return isShift ? '}' : ']';
			case Key::Key_pipe:				return isShift ? '|' : '\\';

		}

		return 0;
	}

}