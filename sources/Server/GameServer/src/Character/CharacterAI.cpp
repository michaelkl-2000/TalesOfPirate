#include "Core/stdafx.h"
#include "Character/Character.h"
#include "World/SubMap.h"
#include "NPC/NPC.h"

//--------------------------------------------------------
//                   AI
//--------------------------------------------------------

void CCharacter::SrcFightTar(CFightAble *pTar, dbc::Short sSkillID)
{
	Point	Path[2] = {GetShape().centre, pTar->GetShape().centre};
	Cmd_BeginSkillDirect(sSkillID, pTar);
}
