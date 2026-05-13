#pragma once

#include "Database/TableData.h"

// Запись таблицы эффектов (магия)

namespace Corsairs::Common::Effect {

class CMagicInfo : public EntityData
{
public:
	char    szName[16]{};
	char    szPhotoName[16]{};
	int     nPhotoTexID{0};

	int     nEffType{0};
	int     nObjType{0};

	int     nDummyNum{0};
	int     nDummy[8]{-1,-1,-1,-1,-1,-1,-1,-1};
	int     nDummy2{0};

	int     nHeightOff{0};

	float   fPlayTime{0};

	int     LightID{0};

	float   fBaseSize{0};
};

#define CEffectInfo   CMagicInfo
#define GetEffectInfo GetMagicInfo

} // namespace Corsairs::Common::Effect

