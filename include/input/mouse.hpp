#pragma once
#include "input_device.hpp"
#include "types/enum.hpp"

namespace oic {

	//Platform independent mouse buttons and axes

	oicExposedEnum(
		MouseButton, ButtonHandle, 
		BUTTON_LEFT, BUTTON_RIGHT, BUTTON_MIDDLE, BUTTON_BACK, BUTTON_FORWARD
	);

	oicExposedEnum(
		MouseAxis, AxisHandle,
		AXIS_X, AXIS_Y, AXIS_DELTA_X, AXIS_DELTA_Y, AXIS_WHEEL
	);

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
		bool isSupported(Handle handle) const;
	};

}