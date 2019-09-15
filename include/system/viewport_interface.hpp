#pragma once
#include "types/types.hpp"

namespace oic {

	struct ViewportInfo;

	class ViewportInterface {

	public:

		virtual ~ViewportInterface() {}
		virtual void init(ViewportInfo *viewport) = 0;
		virtual void resize(const Vec2u &size) = 0;
		virtual void render() = 0;

	};

}