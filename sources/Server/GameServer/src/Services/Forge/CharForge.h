// CharForge.h Created by knight-gongjian 2005.1.24.
//---------------------------------------------------------
#pragma once

#ifndef _CHARFORGE_H_
#define _CHARFORGE_H_


#include "Character/Character.h"

//---------------------------------------------------------
namespace Corsairs::Common::Mission
{
	class CForgeSystem
	{
	public:
		//
		void	ForgeItem( CCharacter& character, BYTE byIndex );
	};

}

extern Corsairs::Common::Mission::CForgeSystem g_ForgeSystem;

//---------------------------------------------------------

#endif // _CHARFORGE_H_
