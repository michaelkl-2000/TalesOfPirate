//

#include "stdafx.h"


#include "Heap.h"

namespace Corsairs::Engine::Render {
	// Heap
	LW_STD_IMPLEMENTATION(Heap);

	LW_RESULT Heap::Clone(IHeap** out_heap) {
		Heap* o = LW_NEW(Heap);

		o->_heap.Copy(&_heap);

		*out_heap = o;

		return LW_RET_OK;
	}

} // namespace Corsairs::Engine::Render
