#include "input/input_device.hpp"
#include "system/system.hpp"
#include "system/allocator.hpp"
#include "utils/math.hpp"

namespace oic {

	InputDevice::InputDevice(Type type, ButtonHandle buttonCount, AxisHandle axisCount) :
		type(type), buttonCount(buttonCount), axisCount(axisCount), buttonSize() {

		if (buttonCount) {
			const usz allocationCount = usz(oic::Math::ceil(buttonCount / f64(usz_BITS >> 1)));
			buttons = new usz[allocationCount]{};
			buttonSize = u16(allocationCount);
		}

		if (axisCount)
			axes = new f64[usz(axisCount) << 1]{};
	}

	InputDevice::~InputDevice() {
		destroy(buttons, axes);
	}

}