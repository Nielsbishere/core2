#pragma once
#include "input_device.hpp"
#include "types/enum.hpp"
#define plimpl

namespace oic {

	//Platform independent mouse buttons and axes

	oicExposedEnum(
		MouseButton, ButtonHandle, 
		Button_left, Button_right, Button_middle, Button_back, Button_forward
	);

	//Delta x/y is the relative position from the last cursor; this supports multiple mice
	//X/y is the absolute position of the cursor; this only supports one mouse

	oicExposedEnum(
		MouseAxis, AxisHandle,
		Axis_x, Axis_y, Axis_delta_x, Axis_delta_y,  Axis_wheel_x, Axis_wheel_y
	);

	struct ViewportInfo;

	//

	class Mouse : public InputDevice {

	public:

		Mouse(): InputDevice(Type::MOUSE, MouseButton::count, MouseAxis::count) {}

		inline Handle handleByName(const String &name) const override final {
		
			usz i = MouseButton::idByName<EnumNameFormat::LOWERCASE_SPACE>(name);

			if (i != MouseButton::count)
				return u32(i);

			i = MouseAxis::idByName<EnumNameFormat::LOWERCASE_SPACE>(name);

			if (i != MouseAxis::count)
				return u32(i + MouseButton::count);

			return end();
		}

		inline String nameByHandle(Handle id) const override final {
		
			if (id >= end()) return "";

			if (id < MouseButton::count)
				return MouseButton::nameById(id);

			return MouseAxis::nameById(id - MouseButton::count);
		}

		//Platform and device dependent

		plimpl bool isSupported(Handle handle) const final override ;
	};

}