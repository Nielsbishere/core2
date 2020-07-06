#pragma once
#include "input_device.hpp"
#include "types/enum.hpp"

namespace oic {

	//Platform independent key code

	oicExposedEnum(

		Key, ButtonHandle,

		Key_0, Key_1, Key_2, Key_3, Key_4, Key_5, Key_6, Key_7, Key_8, Key_9,
		Key_a, Key_b, Key_c, Key_d, Key_e, Key_f, Key_g, Key_h, Key_i, Key_j,
		Key_k, Key_l, Key_m, Key_n, Key_o, Key_p, Key_q, Key_r, Key_s, Key_t,
		Key_u, Key_v, Key_w, Key_x, Key_y, Key_z,

		Key_backspace, Key_space, Key_tab, Key_shift, Key_ctrl, Key_alt,
		Key_pause, Key_caps_lock, Key_escape, Key_page_up, Key_page_down,
		Key_end, Key_home, Key_select, Key_print, Key_execute, Key_print_screen,
		Key_insert, Key_delete, Key_help, Key_menu, Key_num_lock, Key_scroll_lock,
		Key_apps, Key_back, Key_forward, Key_sleep, Key_refresh, Key_stop, Key_search,
		Key_favorites, Key_start, Key_mute, Key_volume_down, Key_volume_up,
		Key_skip, Key_previous, Key_clear, Key_zoom, Key_enter,

		Key_left, Key_right, Key_up, Key_down, 

		Key_numpad_1, Key_numpad_2, Key_numpad_3,
		Key_numpad_4, Key_numpad_5, Key_numpad_6,
		Key_numpad_7, Key_numpad_8, Key_numpad_9,
		Key_numpad_0, Key_numpad_multiply, Key_numpad_add,
		Key_numpad_decimal, Key_numpad_divide, Key_numpad_subtract,

		Key_f1, Key_f2, Key_f3, Key_f4, Key_f5, Key_f6, Key_f7, Key_f8, Key_f9,
		Key_f12, Key_f13, Key_f14, Key_f15, Key_f16, Key_f17, Key_f18, Key_f19,
		Key_f20, Key_f21, Key_f22, Key_f23, Key_f24,

		Key_plus, Key_comma, Key_minus, Key_period, 

		Key_slash, Key_tilde, Key_semicolon, Key_begin_bracket, Key_end_bracket, Key_pipe,
		Key_quote
	);

	//

	class Keyboard : public InputDevice {

	public:

		Keyboard(): InputDevice(Type::KEYBOARD, Key::count, 0) {}

		inline Handle handleByName(const String &str) const override final { return Handle(Key::idByName<EnumNameFormat::LOWERCASE_SPACE>(str)); }
		inline String nameByHandle(Handle id) const override final  { return Key::nameById<EnumNameFormat::LOWERCASE_SPACE>(id); }

		//Platform and device dependent
		bool isSupported(Handle handle) const final override;

	};

}