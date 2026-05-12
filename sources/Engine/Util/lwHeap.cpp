//

#include "stdafx.h"


#include "lwHeap.h"

namespace Corsairs::Engine::Render {
	// lwHeap
	LW_STD_IMPLEMENTATION(lwHeap);

	LW_RESULT lwHeap::Clone(lwIHeap** out_heap) {
		lwHeap* o = LW_NEW(lwHeap);

		o->_heap.Copy(&_heap);

		*out_heap = o;

		return LW_RET_OK;
	}

} // namespace Corsairs::Engine::Render
