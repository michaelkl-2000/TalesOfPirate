//
#include "lwHeader.h"
#include "SlotMap.h"

namespace Corsairs::Engine::Render {
	typedef SlotMapVoidPtr1024 lwObjectPoolSkeleton;
	typedef SlotMapVoidPtr1024 lwObjectPoolSkin;


	class lwSysCharacter {
	private:
		lwObjectPoolSkeleton* _pool_skeleton;
		lwObjectPoolSkin* _pool_skinmesh;

	public:
		lwSysCharacter();
		~lwSysCharacter();

		LW_RESULT QuerySkeleton(DWORD* ret_id, std::string_view file);
	};

} // namespace Corsairs::Engine::Render
