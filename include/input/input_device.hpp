#pragma once
#include "types/types.hpp"
#include "system/system.hpp"
#include "system/log.hpp"

namespace oic {

	using ButtonHandle = u16;
	using AxisHandle = u16;
	using InputHandle = u32;

	//An optimized input device, expandible for other types of input
	//A 'deadzone' is defined as the zone where an axis collapses to nothing
	//If this is .1 in an axis with range 0-1, it means that 0-0.1 are 0 and 0.1-1 are 0-1
	//This will only get checked for axes; buttons will return 0 or 1
	//Converting an axis to a press requires a "deadzone", below it will collapse to 
	class InputDevice {

	public:

		enum Type : u8 {

			MOUSE,			//Devices that can send axes and button and have a cursor
			KEYBOARD,		//Devices that send keys
			CONTROLLER,		//Devices that can send axes and buttons but have no cursor
			
			TRANSFORM,		//Devices that send transform data (e.g. position/rotation/acceleration/speed/etc.)

			CUSTOM			//Devices that do something special

		};

		enum State : u8 { UP, PRESSED, RELEASED, DOWN };

		using Handle = InputHandle;

		InputDevice(Type type, ButtonHandle buttons, AxisHandle axes);
		virtual ~InputDevice();

		InputDevice(const InputDevice&) = delete;
		InputDevice(InputDevice&&) = delete;
		InputDevice &operator=(const InputDevice&) = delete;
		InputDevice &operator=(InputDevice&&) = delete;

		//Setting values

		inline void setState(ButtonHandle handle, bool b);
		inline void setPreviousState(ButtonHandle handle, bool b);
		inline void setAxis(AxisHandle handle, f64 v);
		inline void setPreviousAxis(AxisHandle handle, f64 v);

		//Only for buttons

		inline bool getCurrentState(ButtonHandle handle) const;
		inline bool getPreviousState(ButtonHandle handle) const;

		//Only for axes

		inline f64 getCurrentAxis(AxisHandle handle) const;
		inline f64 getPreviousAxis(AxisHandle handle) const;

		//Helper functions

		inline bool isDown(Handle handle) const;
		inline bool isUp(Handle handle) const;
		inline bool isReleased(Handle handle) const;
		inline bool isPressed(Handle handle) const;

		inline f64 getAxis(Handle handle) const;

		inline Type getType() const { return type; }
		inline void pushUpdate();
		inline bool isType(Type t) const { return type == t; }

		inline Handle getButtonStart() const { return 0; }
		inline Handle getButtonEnd() const { return buttonCount; }
		inline ButtonHandle getButtonCount() const { return buttonCount; }

		inline Handle getAxisStart() const { return buttonCount; }
		inline Handle getAxisEnd() const { return buttonCount + axisCount; }
		inline AxisHandle getAxisCount() const { return axisCount; }

		inline Handle begin() const { return 0; }
		inline Handle end() const { return buttonCount + axisCount; }

		inline bool isValid(Handle handle) const { return handle < getAxisEnd(); }

		//0x0 = up, 0x1 = pressed, 0x2 = released, 0x3 = down
		inline usz getState(Handle handle) const;

		//Get a human readable name as a handle (returns getAxisEnd() if invalid)
		virtual Handle handleByName(const String &name) const = 0;

		//Get a handle as a human readable name
		virtual String nameByHandle(Handle handle) const = 0;

		//If a handle is supported on the current device; if not, it will always be on 0
		virtual bool isSupported(Handle handle) const = 0;

	protected:

		inline bool collapseAxis(f64 val) const;

	private:

		static constexpr usz prevMask = usz(usz_BYTES == 8 ? 0xAAAAAAAAAAAAAAAA : 0xAAAAAAAA);

		usz *buttons{};		//A button is defined as 2 bits; previous and next state as boolean. 'next0' is located at 1, 'prev0' at 2
		f64 *axes{};		//An axis is defined as 2 doubles; previous and next value

		u16 buttonCount, axisCount;
		u16 buttonSize;		//Size in usz
		Type type;

	};

	inline void InputDevice::setState(ButtonHandle handle, bool b) {

		if (!b)
			buttons[handle >> (usz_BIT_SHIFT - 1)] &= ~(1_usz << (handle << 1_usz));
		else
			buttons[handle >> (usz_BIT_SHIFT - 1)] |= 1_usz << (handle << 1_usz);
	}

	inline void InputDevice::setPreviousState(ButtonHandle handle, bool b) {

		if (!b)
			buttons[handle >> (usz_BIT_SHIFT - 1)] &= ~(2_usz << (handle << 1_usz));
		else
			buttons[handle >> (usz_BIT_SHIFT - 1)] |= 2_usz << (handle << 1_usz);
	}

	inline void InputDevice::setAxis(AxisHandle handle, f64 v) {
		axes[handle << 1] = v;
	}

	inline void InputDevice::setPreviousAxis(AxisHandle handle, f64 v) {
		axes[(handle << 1) + 1] = v;
	}

	inline bool InputDevice::getCurrentState(ButtonHandle handle) const {
		return buttons[handle >> (usz_BIT_SHIFT - 1)] & (1_usz << (handle << 1_usz));
	}

	inline bool InputDevice::getPreviousState(ButtonHandle handle) const {
		return buttons[handle >> (usz_BIT_SHIFT - 1)] & (2_usz << (handle << 1_usz));
	}

	inline f64 InputDevice::getCurrentAxis(AxisHandle handle) const {
		return axes[handle << 1];
	}

	inline f64 InputDevice::getPreviousAxis(AxisHandle handle) const {
		return axes[(usz(handle) << 1) + 1];
	}

	inline bool InputDevice::collapseAxis(f64 val) const {
		return bool(val);		//TODO: Determine if it should be a bool
	}

	inline usz InputDevice::getState(Handle handle) const {

		if(handle < buttonCount)
			return (buttons[handle >> (usz_BIT_SHIFT - 1)] >> (handle << 1)) & 0x3;

		handle -= buttonCount;
		return usz(collapseAxis(axes[usz(handle) << 1])) | (usz(collapseAxis(axes[(usz(handle) << 1) + 1])) << 1);
	}

	inline bool InputDevice::isDown(Handle handle) const {
		return getState(handle) == 0x3;
	}

	inline bool InputDevice::isReleased(Handle handle) const{
		return getState(handle) == 0x2;
	}

	inline bool InputDevice::isPressed(Handle handle) const{
		return getState(handle) == 0x1;
	}

	inline bool InputDevice::isUp(Handle handle) const {
		return getState(handle) == 0x0;
	}

	inline f64 InputDevice::getAxis(Handle handle) const {

		if (handle < buttonCount)
			return getCurrentState(ButtonHandle(handle));

		return getCurrentAxis(AxisHandle(handle - buttonCount));
	}

	inline void InputDevice::pushUpdate() {

		//Since the previous keys are left of the next keys, we shift them into the previous key and mask out the next keys.
		//Meaning we update 32 keys in 5 operations

		for (int i = 0; i < buttonSize; ++i) {
			usz curr = buttons[i];
			curr <<= 1;
			buttons[i] = curr & prevMask;
		}
	}

}