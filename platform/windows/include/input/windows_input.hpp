#pragma once
#include "types/enum.hpp"

namespace oic {

	//Mapping to a USHORT VK keycode

	oicExposedEnum(

		WKey, u16,

		Key_0 = 0x30, Key_1, Key_2, Key_3, Key_4, Key_5, Key_6, Key_7, Key_8, Key_9,
		Key_a = 0x41, Key_b, Key_c, Key_d, Key_e, Key_f, Key_g, Key_h, Key_i, Key_j,
		Key_k, Key_l, Key_m, Key_n, Key_o, Key_p, Key_q, Key_r, Key_s, Key_t,
		Key_u, Key_v, Key_w, Key_x, Key_y, Key_z,

		Key_backspace = 0x08, Key_space = 0x20, Key_tab = 0x09, Key_shift = 0x10, Key_ctrl = 0x11, Key_alt = 0x12,
		Key_pause = 0x13, Key_caps_lock = 0x14, Key_escape = 0x1b, Key_page_up = 0x21, Key_page_down = 0x22,
		Key_end = 0x23, Key_home = 0x24, Key_select = 0x29, Key_print = 0x2a, Key_execute = 0x2b, Key_print_screen = 0x2c,
		Key_insert = 0x2d, Key_delete = 0x2e, Key_help = 0x2f, Key_menu = 0x5b, Key_num_lock = 0x90, Key_scroll_lock = 0x91,
		Key_apps = 0x5d, Key_back = 0xa6, Key_forward = 0xa7, Key_sleep = 0x5f, Key_refresh = 0xa8, Key_stop = 0xa9, Key_search = 0xaa,
		Key_favorites = 0xab, Key_start = 0xac, Key_mute = 0xad, Key_volume_down = 0xae, Key_volume_up = 0xaf,
		Key_skip = 0xb0, Key_previous = 0xb1, Key_clear = 0xfe, Key_zoom = 0xfb, Key_enter = 0x0d,

		Key_left = 0x25, Key_up = 0x26, Key_right = 0x27, Key_down = 0x28, 

		Key_numpad_1 = 0x61, Key_numpad_2, Key_numpad_3,
		Key_numpad_4, Key_numpad_5, Key_numpad_6,
		Key_numpad_7, Key_numpad_8, Key_numpad_9,
		Key_numpad_0 = 0x60, Key_numpad_multiply = 0x6a, Key_numpad_add = 0x6b,
		Key_numpad_decimal = 0x6e, Key_numpad_divide = 0x6f, Key_numpad_subtract = 0x6d,

		Key_f1 = 0x70, Key_f2, Key_f3, Key_f4, Key_f5, Key_f6, Key_f7, Key_f8, Key_f9,
		Key_f12, Key_f13, Key_f14, Key_f15, Key_f16, Key_f17, Key_f18, Key_f19,
		Key_f20, Key_f21, Key_f22, Key_f23, Key_f24,

		Key_plus = 0xbb, Key_comma = 0xbc, Key_minus = 0xbd, Key_period = 0xbe, 

		Key_slash = 0xbf, Key_tilde = 0xc0, Key_semicolon = 0xba, Key_begin_bracket = 0xdb, Key_end_bracket = 0xdd, Key_pipe = 0xdc,
		Key_quote = 0xde
	);

}