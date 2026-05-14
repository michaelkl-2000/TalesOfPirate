// CharForge.h Created by knight-gongjian 2005.1.24.
//---------------------------------------------------------
#pragma once

#ifndef _CHARFORGE_H_
#define _CHARFORGE_H_


#include "Character/Character.h"

//---------------------------------------------------------
namespace mission
{
	class CForgeSystem
	{
	public:
		//
		void	ForgeItem( CCharacter& character, BYTE byIndex );
	};

}

extern mission::CForgeSystem g_ForgeSystem;

//---------------------------------------------------------

#endif // _CHARFORGE_H_
