#pragma once
#include "system/allocator.hpp"

namespace oic::windows {

	class WAllocator : public Allocator {

	public:

		void *alloc(usz size, RangeHint hint, usz addressHint) final override;
		void free(void *v, usz size, bool isRange) final override;

	};

}