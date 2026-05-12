#include "Core/stdafx.h"
#include "Event/EventHandler.h"
#include "App/GameAppNet.h"
#include "Character/Character.h"
#include "Core/CommFunc.h"



//-------------------------------------
//  : 
//-------------------------------------
void CEventHandler::Event_ChaEmotion(CCharacter *pCha, short sEmotionNo)
{
	//  :  
	auto wpk = Corsairs::Net::Msg::serialize(Corsairs::Net::Msg::McChaEmotionMessage{pCha->GetID(), (int64_t)(short)sEmotionNo});
	pCha->NotiChgToEyeshot(wpk, false);
}


CEventHandler g_EventHandler;
