#include "Core/stdafx.h"
#include "Character/Character.h"
#include "World/SubMap.h"
#include "NPC/NPC.h"

//--------------------------------------------------------
//                   AI
//--------------------------------------------------------

void CCharacter::SrcFightTar(CFightAble *pTar, int16_t sSkillID)
{
	Corsairs::Util::Point	Path[2] = {GetShape().Centre, pTar->GetShape().Centre};
	Cmd_BeginSkillDirect(sSkillID, pTar);
}
