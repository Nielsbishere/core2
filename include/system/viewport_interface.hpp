#pragma once
#include "types/types.hpp"

namespace oic {

	class InputDevice;
	using InputHandle = u32;

	struct ViewportInfo;

	class ViewportInterface {

	public:

		virtual ~ViewportInterface() {}
		virtual void init(ViewportInfo*) = 0;
		virtual void release(const ViewportInfo*) = 0;
		virtual void resize(const ViewportInfo*, const Vec2u32&) = 0;
		virtual void render(const ViewportInfo*) = 0;

		virtual void update(const ViewportInfo*, f64) {}		//Can be called before or after render

		virtual void onDeviceConnect(const ViewportInfo*, const InputDevice*) {}
		virtual void onDeviceRemoval(const ViewportInfo*, const InputDevice*) {}

		virtual void onInputActivate(const ViewportInfo*, const InputDevice*, InputHandle) {}
		virtual void onInputDeactivate(const ViewportInfo*, const InputDevice*, InputHandle) {}
		virtual void onInputUpdate(const ViewportInfo*, const InputDevice*, InputHandle, bool) {}

	};

}