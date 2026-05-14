//=============================================================================
// FileName: MoveAble.cpp
// Creater: ZhangXuedong
// Date: 2004.11.03
// Comment: CMoveAble class
//=============================================================================
#include "Core/stdafx.h"
#include "Combat/MoveAble.h"
#include "World/SubMap.h"
#include "App/GameAppNet.h"
#include "Core/CommFunc.h"
#include "App/GameApp.h"
unsigned long	g_ulElapse;
unsigned long	g_ulDist;

CMoveAble::CMoveAble()
{
	m_usHeartbeatFreq = 0;

	m_SMoveInit.STargetInfo.chType = 0;
	m_SMoveProc.sState = enumMSTATE_ARRIVE;
	m_SMoveProc.chRequestState = 0;
	m_SMoveProc.chLagMove = 0;

	m_SMoveProc.SNoticePoint.sNum = 2;
	m_SMoveProc.SNoticePoint.SList[0] = GetShape().Centre;
	m_SMoveProc.SNoticePoint.SList[1] = GetShape().Centre;

	m_SMoveRedu.ulLeftTime = 0;
	m_SMoveRedu.ulStartTick = 0;

	m_bOnMove = false;
	m_timeRun.Begin(1 * 300);
}

void CMoveAble::Initially()
{
	CFightAble::Initially();

	m_SMoveInit.STargetInfo.chType = 0;
	m_SMoveInitCache.STargetInfo.chType = 0;
	m_SMoveProc.sState = enumMSTATE_ARRIVE;
	m_SMoveProc.chRequestState = 0;
	m_SMoveProc.chLagMove = 0;

	m_SMoveProc.SNoticePoint.sNum = 2;
	m_SMoveProc.SNoticePoint.SList[0] = GetShape().Centre;
	m_SMoveProc.SNoticePoint.SList[1] = GetShape().Centre;

	m_SMoveRedu.ulLeftTime = 0;
	m_SMoveRedu.ulStartTick = 0;
	m_lSetPing = -1;

	m_bOnMove = false;
}

void CMoveAble::Finally()
{
	CFightAble::Finally();
}

void CMoveAble::ResetMove()
{
	m_SMoveProc.sState = enumMSTATE_ARRIVE;
	m_SMoveProc.chRequestState = 0;
	m_SMoveProc.chLagMove = 0;

	m_SMoveProc.SNoticePoint.sNum = 2;
	m_SMoveProc.SNoticePoint.SList[0] = GetShape().Centre;
	m_SMoveProc.SNoticePoint.SList[1] = GetShape().Centre;

	m_SMoveRedu.ulLeftTime = 0;
	m_SMoveRedu.ulStartTick = 0;

	m_bOnMove = false;
}

//bool CMoveAble::AreaOverlap(long &xdist, long &ydist)
//{
//	if (!m_submap)
//		return false;
//
//	std::uint16_t	usAreaAttr;
//	int16_t	sUnitSX, sUnitEX, sUnitSY, sUnitEY;
//	int16_t	sUnitWidth, sUnitHeight;
//	Corsairs::Util::Point	SPos = GetPos();
//
//	m_submap->GetTerrainCellSize(&sUnitWidth, &sUnitHeight);
//
//	sUnitSX = int16_t(SPos.x / sUnitWidth);
//	sUnitEX = int16_t((SPos.x + xdist) / sUnitWidth);
//	sUnitSY = int16_t(SPos.y / sUnitHeight);
//	sUnitEY = int16_t((SPos.y + ydist) / sUnitHeight);
//
//	if (sUnitSX == sUnitEX && sUnitSY == sUnitEY)
//	{
//		m_submap->GetTerrainCellAttr(sUnitSX, sUnitSY, usAreaAttr);
//		if (!g_IsMoveAble(static_cast<EChaCtrlType>(m_CChaAttr.GetAttr(ATTR_CHATYPE)), m_pCChaRecord->chTerritory, usAreaAttr))
//		{
//			xdist = 0, ydist = 0;
//			return true;
//		}
//		else
//			return false;
//	}
//
//	bool	bIs45Dir = false;
//	// 
//	if (xdist == ydist)
//	{
//		int16_t	sModelX = int16_t(SPos.x % sUnitWidth);
//		int16_t	sModelY = int16_t(SPos.y % sUnitHeight);
//		if (sModelX == sModelY)
//			bIs45Dir = true;
//	}
//	else if (-1 * xdist == ydist)
//	{
//		int16_t	sModelX = int16_t(SPos.x % sUnitWidth);
//		int16_t	sModelY = int16_t(SPos.y % sUnitHeight);
//		if (sUnitWidth - sModelX == sModelY || sModelX == sUnitHeight - sModelY)
//			bIs45Dir = true;
//	}
//
//	if (bIs45Dir)
//	{
//		char	chXDir = 1;
//		char	chYDir = 1;
//		if (sUnitSX > sUnitEX)
//			chXDir = -1;
//		if (sUnitSY > sUnitEY)
//			chYDir = -1;
//
//		int16_t	sLoop = (sUnitEX - sUnitSX) * chXDir;
//		for (int16_t i = 0; i <= sLoop; i++)
//		{
//			m_submap->GetTerrainCellAttr(sUnitSX + i * chXDir, sUnitSY + i * chYDir, usAreaAttr);
//			if (!g_IsMoveAble(static_cast<EChaCtrlType>(m_CChaAttr.GetAttr(ATTR_CHATYPE)), m_pCChaRecord->chTerritory, usAreaAttr))
//			{
//				xdist = 0, ydist = 0;
//				return true;
//			}
//		}
//	}
//	else
//	{
//		m_submap->GetTerrainCellAttr(sUnitEX, sUnitEY, usAreaAttr);
//		if (!g_IsMoveAble(static_cast<EChaCtrlType>(m_CChaAttr.GetAttr(ATTR_CHATYPE)), m_pCChaRecord->chTerritory, usAreaAttr))
//		{
//			xdist = 0, ydist = 0;
//			return true;
//		}
//
//		if (abs(xdist) >= abs(ydist))
//		{
//			if (xdist == 0)
//				return false;
//			std::int32_t	lRSX = SPos.x, lRSY = SPos.y;
//			if (sUnitSX > sUnitEX)
//			{
//				int16_t	sTemp = sUnitSX;
//				sUnitSX = sUnitEX;
//				sUnitEX = sTemp;
//				lRSX += xdist;
//				lRSY += ydist;
//			}
//			for (int16_t x = 0; x < sUnitEX - sUnitSX; x++)
//			{
//				std::int32_t y = ydist * ((sUnitWidth - lRSX % sUnitWidth) + x * sUnitWidth) / xdist + lRSY;
//				int16_t sRUnitY = int16_t(y / sUnitHeight);
//				m_submap->GetTerrainCellAttr(sUnitSX + x, sRUnitY, usAreaAttr);
//				if (!g_IsMoveAble(static_cast<EChaCtrlType>(m_CChaAttr.GetAttr(ATTR_CHATYPE)), m_pCChaRecord->chTerritory, usAreaAttr))
//				{
//					xdist = 0, ydist = 0;
//					return true;
//				}
//				m_submap->GetTerrainCellAttr(sUnitSX + x + 1, sRUnitY, usAreaAttr);
//				if (!g_IsMoveAble(static_cast<EChaCtrlType>(m_CChaAttr.GetAttr(ATTR_CHATYPE)), m_pCChaRecord->chTerritory, usAreaAttr))
//				{
//					xdist = 0, ydist = 0;
//					return true;
//				}
//			}
//		}
//		else
//		{
//			if (ydist == 0)
//				return false;
//			std::int32_t	lRSX = SPos.x, lRSY = SPos.y;
//			if (sUnitSY > sUnitEY)
//			{
//				int16_t	sTemp = sUnitSY;
//				sUnitSY = sUnitEY;
//				sUnitEY = sTemp;
//				lRSX += xdist;
//				lRSY += ydist;
//			}
//			for (int16_t y = 0; y < sUnitEY - sUnitSY; y++)
//			{
//				std::int32_t x = xdist * ((sUnitHeight - lRSY % sUnitHeight) + y * sUnitHeight) / ydist + lRSX;
//				int16_t sRUnitX = int16_t(x / sUnitWidth);
//				m_submap->GetTerrainCellAttr(sRUnitX, sUnitSY + y, usAreaAttr);
//				if (!g_IsMoveAble(static_cast<EChaCtrlType>(m_CChaAttr.GetAttr(ATTR_CHATYPE)), m_pCChaRecord->chTerritory, usAreaAttr))
//				{
//					xdist = 0, ydist = 0;
//					return true;
//				}
//				m_submap->GetTerrainCellAttr(sRUnitX, sUnitSY + y + 1, usAreaAttr);
//				if (!g_IsMoveAble(static_cast<EChaCtrlType>(m_CChaAttr.GetAttr(ATTR_CHATYPE)), m_pCChaRecord->chTerritory, usAreaAttr))
//				{
//					xdist = 0, ydist = 0;
//					return true;
//				}
//			}
//		}
//	}
//
//	return false;
//}

bool CMoveAble::AreaOverlap(long &xdist, long &ydist)
{
	if (!m_submap)
		return false;

	std::uint16_t	usAreaAttr;
	int16_t	sUnitSX, sUnitEX, sUnitSY, sUnitEY;
	int16_t	sUnitWidth, sUnitHeight;
	Corsairs::Util::Point	SPos = GetPos();

	m_submap->GetTerrainCellSize(&sUnitWidth, &sUnitHeight);

	sUnitSX = int16_t(SPos.X / sUnitWidth);
	sUnitEX = int16_t((SPos.X + xdist) / sUnitWidth);
	sUnitSY = int16_t(SPos.Y / sUnitHeight);
	sUnitEY = int16_t((SPos.Y + ydist) / sUnitHeight);

	if (sUnitSX == sUnitEX && sUnitSY == sUnitEY)
	{
		m_submap->GetTerrainCellAttr(sUnitSX, sUnitSY, usAreaAttr);
		if (!g_IsMoveAble(static_cast<EChaCtrlType>(m_CChaAttr.GetAttr(ATTR_CHATYPE)), m_pCChaRecord->chTerritory, usAreaAttr))
		{
			xdist = 0, ydist = 0;
			return true;
		}
		else
			return false;
	}

	bool	bIs45Dir = false;
	// 
	if (xdist == ydist)
	{
		int16_t	sModelX = int16_t(SPos.X % sUnitWidth);
		int16_t	sModelY = int16_t(SPos.Y % sUnitHeight);
		if (sModelX == sModelY)
			bIs45Dir = true;
	}
	else if (-1 * xdist == ydist)
	{
		int16_t	sModelX = int16_t(SPos.X % sUnitWidth);
		int16_t	sModelY = int16_t(SPos.Y % sUnitHeight);
		if (sUnitWidth - sModelX == sModelY || sModelX == sUnitHeight - sModelY)
			bIs45Dir = true;
	}

	if (bIs45Dir)
	{
		char	chXDir = 1;
		char	chYDir = 1;
		if (sUnitSX > sUnitEX)
			chXDir = -1;
		if (sUnitSY > sUnitEY)
			chYDir = -1;

		int16_t	sLoop = (sUnitEX - sUnitSX) * chXDir;
		for (int16_t i = 0; i <= sLoop; i++)
		{
			m_submap->GetTerrainCellAttr(sUnitSX + i * chXDir, sUnitSY + i * chYDir, usAreaAttr);
			if (!g_IsMoveAble(static_cast<EChaCtrlType>(m_CChaAttr.GetAttr(ATTR_CHATYPE)), m_pCChaRecord->chTerritory, usAreaAttr))
			{
				xdist = 0, ydist = 0;
				return true;
			}
		}
	}
	else
	{
		if (sUnitSX > sUnitEX)
		{
			int16_t	sTemp = sUnitSX;
			sUnitSX = sUnitEX;
			sUnitEX = sTemp;
		}
		if (sUnitSY > sUnitEY)
		{
			int16_t	sTemp = sUnitSY;
			sUnitSY = sUnitEY;
			sUnitEY = sTemp;
		}

		float v0[2]; v0[0] = (float)SPos.X, v0[1] = (float)SPos.Y;
		float v1[2]; v1[0] = (float)(SPos.X + xdist), v1[1] = (float)(SPos.Y + ydist);
		float p1[2], p2[2], p3[2], p4[2];
		for (int16_t x = sUnitSX; x <= sUnitEX; x++)
		{
			for (int16_t y = sUnitSY; y <= sUnitEY; y++)
			{
				m_submap->GetTerrainCellAttr(x, y, usAreaAttr);
				if (!g_IsMoveAble(static_cast<EChaCtrlType>(m_CChaAttr.GetAttr(ATTR_CHATYPE)), m_pCChaRecord->chTerritory, usAreaAttr))
				{
					p1[0] = (float)(x * sUnitWidth), p1[1] = (float)(y * sUnitHeight);
					p2[0] = (float)(p1[0] + sUnitWidth - 1), p2[1] = p1[1];
					p3[0] = p2[0], p3[1] = (float)(p2[1] + sUnitHeight - 1);
					p4[0] = p1[0], p4[1] = p3[1];

					if(LineIntersection(v0, v1, p1, p2, FALSE) || LineIntersection(v0, v1, p2, p3, FALSE)
						|| LineIntersection(v0, v1, p3, p4, FALSE) || LineIntersection(v0, v1, p4, p1, FALSE))
					{
						xdist = 0, ydist = 0;
						return true;
					}
				}
			}
		}
	}

	return false;
}

bool CMoveAble::overlap(long &xdist, long &ydist)
{
	bool	b_retval	= false;

	if (Entity::overlap(xdist, ydist))
		b_retval = true;
	if (AreaOverlap(xdist, ydist))
		b_retval = true;

	return b_retval;
}

bool CMoveAble::DesireMoveBegin(SMoveInit *pSMoveInit)
{
	if (m_usHeartbeatFreq == 0)
	{
		if (IsCharacter()->IsPlayerCha())
			m_usHeartbeatFreq = 300;
		else
			m_usHeartbeatFreq = 600;
	}

	if (m_lSetPing >= 0)
		pSMoveInit->usPing = (std::uint16_t)m_lSetPing;

	if (pSMoveInit->SInflexionInfo.sNum > defMOVE_INFLEXION_NUM)
		pSMoveInit->SInflexionInfo.sNum = defMOVE_INFLEXION_NUM;
	std::uint32_t	ulNowTick = GetTickCount();

	if (ulNowTick >= m_SMoveRedu.ulStartTick)
	{
		SetExistState(enumEXISTS_MOVING);
		if (ulNowTick - m_SMoveRedu.ulStartTick >= m_SMoveRedu.ulLeftTime)
		{
			memcpy(&m_SMoveInit, pSMoveInit, sizeof(SMoveInit));
			m_SMoveProc.chRequestState = 0;
			m_SMoveProc.chLagMove = 0;
			m_SMoveRedu.ulLeftTime = 0;
			m_SMoveRedu.ulStartTick = 0xffffffff;
			BeginMove();
		}
		else
		{
			m_SMoveProc.ulCacheTick = ulNowTick;
			m_SMoveProc.chRequestState = 2;
			m_SMoveProc.chLagMove = 1;
			memcpy(&m_SMoveInitCache, pSMoveInit, sizeof(SMoveInit));
			OnMoveBegin();
				//m_SMoveRedu.ulLeftTime - (ulNowTick - m_SMoveRedu.ulStartTick), m_SMoveInit.usPing);
			}
	}
	else
	{
		return false;
	}

	return true;
}

void CMoveAble::BeginMove(std::uint32_t ulElapse)
{
	if (GetPos() != m_SMoveInit.SInflexionInfo.SList[0]
	&& IsCharacter()->IsRangePoint2(m_SMoveInit.SInflexionInfo.SList[0], 25 * 25 * 2)) // 
	{
		if (m_SMoveInit.SInflexionInfo.sNum < defMOVE_INFLEXION_NUM)
		{
			int16_t i = m_SMoveInit.SInflexionInfo.sNum;
			for (; i > 0; i--) {
				m_SMoveInit.SInflexionInfo.SList[i] = m_SMoveInit.SInflexionInfo.SList[i - 1];
			}
			m_SMoveInit.SInflexionInfo.SList[i] = GetPos();
			m_SMoveInit.SInflexionInfo.sNum += 1;
		}
	}

	// log
	for (int16_t i = 0; i < m_SMoveInit.SInflexionInfo.sNum; i++)
	//

	g_ulElapse = 0;
	g_ulDist = 0;

	m_SMoveProc.sCurInflexion = 1;
	if (m_SMoveProc.chRequestState == 1)
		m_SMoveProc.sState |= enumMSTATE_CANCEL;
	else
		m_SMoveProc.sState = enumMSTATE_ON;

	Corsairs::Util::Square	STarShape = {{0, 0}, 0};
	std::uint32_t	ulDistXY2 = 0;
	if (m_SMoveInit.STargetInfo.chType > 0) // 
	{
		if (!GetMoveTargetShape(&STarShape)) // 
		{
			m_SMoveInit.STargetInfo.chType = 0;
			m_SMoveProc.chRequestState = 0;
			m_SMoveProc.sState |= enumMSTATE_NOTARGET;

			NotiSelfMov();
			SubsequenceMove();
			return;
		}

		Corsairs::Util::Point	SPos = GetShape().Centre;
		ulDistXY2 = (SPos.X - STarShape.Centre.X) * (SPos.X - STarShape.Centre.X)
			+ (SPos.Y - STarShape.Centre.Y) * (SPos.Y - STarShape.Centre.Y);
		if (ulDistXY2 <= m_SMoveInit.STargetInfo.ulDist * m_SMoveInit.STargetInfo.ulDist)
		{
			m_SMoveProc.sState |= enumMSTATE_INRANGE;

			m_SMoveProc.SNoticePoint.SList[0] = m_SMoveProc.SNoticePoint.SList[1] = GetPos();
			NotiSelfMov();
			SubsequenceMove();
			return;
		}
	}

	if (!IsCharacter()->GetActControl(ActControl::MOVE))
	{
		m_SMoveProc.sState |= enumMSTATE_CANCEL;

		NotiSelfMov();
		SubsequenceMove();
		return;
	}

	if (m_CChaAttr.GetAttr(ATTR_MSPD) == 0) // 
	{
		m_SMoveProc.sState |= enumMSTATE_CANTMOVE;

		NotiSelfMov();
		SubsequenceMove();
		return;
	}

	m_ulHeartbeatTick = GetTickCount();
	m_SMoveRedu.ulLeftTime = ulElapse + m_usHeartbeatFreq + m_SMoveInit.usPing;
	//m_SMoveRedu.ulLeftTime = ulElapse + m_usHeartbeatFreq + m_SMoveInit.usPing + 100;
	std::uint32_t	ulAttemptDist = m_SMoveRedu.ulLeftTime * (long)m_CChaAttr.GetAttr(ATTR_MSPD) / 1000;
	m_SMoveProc.sState |= AttemptMove(ulAttemptDist, false);

	if (m_SMoveProc.ulElapse > 0)
	{
		NotiMovToEyeshot();
		if (m_SMoveProc.sState != enumMSTATE_ON)
		{
			m_SMoveRedu.ulLeftTime = m_SMoveProc.ulElapse;
			SubsequenceMove();
		}
		else
		{
			OnMoveBegin();
		}
		AfterStepMove();
	}
	else
	{
		m_SMoveRedu.ulLeftTime = 0;
		m_SMoveProc.ulElapse = 0;
		NotiSelfMov();
		SubsequenceMove();
	}
}

void CMoveAble::EndMove()
{
	m_SMoveProc.chRequestState = 1;
	// log
	//
}

void CMoveAble::OnMove(std::uint32_t dwCurTime)
{
	if (!m_bOnMove || !m_submap)
		return;
	if (!IsCharacter()->IsPlayerCha() && !m_timeRun.IsOK(dwCurTime))
		return;

	std::uint32_t	ulCurTick = GetTickCount();
	std::uint32_t	ulWillElapse = ulCurTick - m_ulHeartbeatTick;
	std::uint32_t	ulAttemptDist;

	if (!IsCharacter()->GetActControl(ActControl::MOVE) && m_SMoveProc.sState == enumMSTATE_ON)
	{
		EndMove();
	}

	if (m_SMoveProc.chRequestState == 1 // 
			&& m_SMoveProc.chLagMove == 0 // 
			)
	{
		ulAttemptDist = ulWillElapse * (long)m_CChaAttr.GetAttr(ATTR_MSPD) / 1000;

		if (ulAttemptDist > 0)
		{
			if (AttemptMove(ulAttemptDist) != enumMSTATE_ON)
			{
				if (ulWillElapse > m_SMoveProc.ulElapse)
				{
					if (m_SMoveRedu.ulLeftTime > ulWillElapse - m_SMoveProc.ulElapse)
						m_SMoveRedu.ulLeftTime -= ulWillElapse - m_SMoveProc.ulElapse;
					else
						m_SMoveRedu.ulLeftTime = 0;
				}
			}
		}
		m_SMoveProc.chRequestState = 0;
		m_SMoveProc.sState |= enumMSTATE_CANCEL;

		m_SMoveProc.chLagMove = 0; //

		NotiMovToEyeshot();
		SubsequenceMove();
	}

	Corsairs::Util::Square	STarShape = {{0, 0}, 0};
	if (m_SMoveInit.STargetInfo.chType > 0) // 
	{
		if (!GetMoveTargetShape(&STarShape)) // 
		{
			m_SMoveProc.chRequestState = 0;
			m_SMoveProc.sState |= enumMSTATE_NOTARGET;
			m_SMoveInit.STargetInfo.chType = 0;

			m_SMoveProc.SNoticePoint.SList[1] = m_SMoveProc.SNoticePoint.SList[m_SMoveProc.SNoticePoint.sNum - 1];
			m_SMoveProc.SNoticePoint.sNum = 2;

			if (m_SMoveRedu.ulLeftTime > ulWillElapse)
				m_SMoveRedu.ulLeftTime -= ulWillElapse;
			else
				m_SMoveRedu.ulLeftTime = 0;

			NotiMovToEyeshot();
			SubsequenceMove();
		}
	}

	if (m_SMoveProc.chLagMove == 1 && m_SMoveProc.sState != enumMSTATE_ON)
	{
		//LG("", "\t %u %u %u\n", ulNowTick, m_SMoveRedu.ulLeftTime, ulNowTick - m_SMoveRedu.ulStartTick);
		std::int32_t	lBal = std::int32_t(ulCurTick - m_SMoveRedu.ulStartTick) - (std::int32_t)m_SMoveRedu.ulLeftTime;
		if (ulCurTick > m_SMoveRedu.ulStartTick && lBal >= 0)
		{

			m_SMoveProc.chLagMove = 0;
			m_SMoveRedu.ulLeftTime = 0;
			m_SMoveRedu.ulStartTick = 0xffffffff;
			m_SMoveProc.chLagMove = 0;
			memcpy(&m_SMoveInit, &m_SMoveInitCache, sizeof(SMoveInit));
			OnMoveEnd();
			BeginMove();
			return;
		}
	}

	if ((long)ulCurTick - (long)m_ulHeartbeatTick - (long)m_usHeartbeatFreq < -50)
		return;
	m_ulHeartbeatTick = ulCurTick;

	bool	bAttemptMove = false;
	if (m_SMoveProc.sState == enumMSTATE_ON)
	{
		ulAttemptDist = ulWillElapse * (long)m_CChaAttr.GetAttr(ATTR_MSPD) / 1000;
		if (ulAttemptDist > 0)
		{
			m_SMoveProc.sState = AttemptMove(ulAttemptDist);
			bAttemptMove = true;
		}
	}

	if (bAttemptMove) // 
	{
		NotiMovToEyeshot();
		if (m_SMoveProc.sState != enumMSTATE_ON)
		{
			if (ulWillElapse > m_SMoveProc.ulElapse)
			{
				if (m_SMoveRedu.ulLeftTime > ulWillElapse - m_SMoveProc.ulElapse)
					m_SMoveRedu.ulLeftTime -= ulWillElapse - m_SMoveProc.ulElapse;
				else
					m_SMoveRedu.ulLeftTime = 0;
			}
			SubsequenceMove();
		}
	}

	if (m_SMoveProc.sState != enumMSTATE_ON && m_SMoveProc.chLagMove == 0)
	{
		OnMoveEnd();
	}

	if (bAttemptMove)
		AfterStepMove(); // 

}

//=============================================================================
// distance
// SMoveProc.sState
//=============================================================================
char CMoveAble::AttemptMove(double dPreMoveDist, bool bNotiInflexion)
{
	char	chRet = enumMSTATE_ON;
	double	dLeftDist = dPreMoveDist;
	std::uint32_t	ulElapse = 0;
	Corsairs::Util::Point	SAttemptTar, SSrc;
	char	chAttemptMove;
	const int16_t csStep = 150; // 
	double	dCurStep;
	Corsairs::Util::Point	SPos, SBeforePos;

	SAttemptTar = m_SMoveInit.SInflexionInfo.SList[m_SMoveProc.sCurInflexion];
	SBeforePos = SSrc = GetShape().Centre;

	m_SMoveProc.SNoticePoint.sNum = 0;
	m_SMoveProc.SNoticePoint.SList[m_SMoveProc.SNoticePoint.sNum++] = SSrc;

	m_SMoveProc.ulElapse = 0;

	dCurStep = dLeftDist;
	Corsairs::Util::Square	SReqShape = {{0, 0}, 0};
	std::int32_t	lReqDist = 0;
	if (m_SMoveInit.STargetInfo.chType > 0) // 
	{
		dCurStep = csStep;

		GetMoveTargetShape(&SReqShape);
		lReqDist = m_SMoveInit.STargetInfo.ulDist;
	}

	std::int32_t	lMoveDist = 0;
	std::int32_t	lDistX2, lDistY2;
	std::int32_t	lPreMoveDist = 0;
	if (!bNotiInflexion)
		lPreMoveDist = m_SMoveRedu.ulLeftTime * (long)m_CChaAttr.GetAttr(ATTR_MSPD) / 1000;
	Corsairs::Util::MPTimer t; t.Begin();
	while (true)
	{
		if (dCurStep > dLeftDist)
			dCurStep = dLeftDist;

		chAttemptMove = LinearAttemptMove(SAttemptTar, dCurStep, &ulElapse);
		m_SMoveProc.ulElapse += ulElapse;

		lDistX2 = (SSrc.X - GetPos().X) * (SSrc.X - GetPos().X);
		lDistY2 = (SSrc.Y - GetPos().Y) * (SSrc.Y - GetPos().Y);
		lMoveDist += (std::int32_t)sqrt((double)lDistX2 + lDistY2);
		SPos = GetShape().Centre;
		if (m_SMoveInit.STargetInfo.chType > 0) // 
		{
			long	lDistX2 = (SPos.X - SReqShape.Centre.X)
				* (SPos.X - SReqShape.Centre.X);
			long	lDistY2 = (SPos.Y - SReqShape.Centre.Y)
				* (SPos.Y - SReqShape.Centre.Y);
			if (lDistX2 + lDistY2 < lReqDist * lReqDist) // 
			{
				chRet |= enumMSTATE_INRANGE;
				break;
			}
		}

		if (chAttemptMove == -2) // 
		{
			chRet |= enumMSTATE_BLOCK;
			break;
		}
		else if (chAttemptMove == -1) // 
		{
			m_SMoveProc.sCurInflexion ++;

			if (m_SMoveProc.sCurInflexion < m_SMoveInit.SInflexionInfo.sNum)
			{
				SAttemptTar = m_SMoveInit.SInflexionInfo.SList[m_SMoveProc.sCurInflexion];
				if (bNotiInflexion || lMoveDist >= lPreMoveDist)
				{
					if (ulElapse > 0)
						m_SMoveProc.SNoticePoint.SList[m_SMoveProc.SNoticePoint.sNum++] = m_SMoveInit.SInflexionInfo.SList[m_SMoveProc.sCurInflexion - 1];
				}
			}
			else
			{
				chRet |= enumMSTATE_ARRIVE;
				break;
			}
		}
		else if (chAttemptMove == 1)
		{
			if (GetPos() == SSrc)
				break;
		}

		dLeftDist -= sqrt(double(SSrc.X - SPos.X) * (SSrc.X - SPos.X)
			+ (SSrc.Y - SPos.Y) * (SSrc.Y - SPos.Y));
		if (dLeftDist < 1)
			break;
		SSrc = SPos;
	}

	SPos = GetShape().Centre;
	if (ulElapse > 0)
	{
		m_sAngle = Corsairs::Util::Arctan(SSrc, SPos);
		m_SMoveProc.SNoticePoint.SList[m_SMoveProc.SNoticePoint.sNum++] = SPos;
	}
	if (m_SMoveProc.SNoticePoint.sNum == 1) // 
		m_SMoveProc.SNoticePoint.SList[m_SMoveProc.SNoticePoint.sNum++] = SPos;
	DWORD dwMoveTime = t.End();

	t.Begin();
	m_shape.Centre = SBeforePos;
	m_submap->MoveTo(this, SPos);
	DWORD dwEyeMoveTime = t.End();

	if (dwMoveTime + dwEyeMoveTime >= 60 )
		//LG("map_time", "\t\t[%s] time = %u%u%u\n", GetLogName(), dwMoveTime + dwEyeMoveTime, dwMoveTime, dwEyeMoveTime);
		ToLogService("common", "\t\troll[{}]move cost time too long, time = {}thereinto position cost{}eye shot cost{}", GetLogName(), dwMoveTime + dwEyeMoveTime, dwMoveTime, dwEyeMoveTime);

	g_ulElapse += m_SMoveProc.ulElapse;
	g_ulDist += lMoveDist;

	return chRet;
}

//=============================================================================
// STardistance*ulElapse
// 1distance
//        -1-2
//=============================================================================
char CMoveAble::LinearAttemptMove(Corsairs::Util::Point STar, double distance, std::uint32_t *ulElapse)
{
	std::uint32_t	ulMoveSpd = (long)m_CChaAttr.GetAttr(ATTR_MSPD);
	if (ulMoveSpd == 0)
	{
		ulElapse = 0;
		return 2;
	}

	char	l_retval = 1;
	long	l_elapse = long((distance * 1000) / m_CChaAttr.GetAttr(ATTR_MSPD));	//distance
	double	l_dist2 = distance * distance;

	const Corsairs::Util::Point l_src = GetShape().Centre;
	const int32_t	lc_xdist = STar.X - l_src.X;
	const int32_t	lc_ydist = STar.Y - l_src.Y;
	long	l_xdist	=lc_xdist;							//x
	long	l_ydist	=lc_ydist;							//y
	double	l_xdist2 = double(l_xdist) * l_xdist;
	double	l_ydist2 = double(l_ydist) * l_ydist;
	double	l_xydist2 = l_xdist2 + l_ydist2;
	bool	l_arraim;

	if ((l_dist2 > l_xydist2) || (abs(l_xydist2 - l_dist2) < 0.0001))
	{
		l_arraim = true; //
	}
	else
	{
		l_arraim = false; //
	}
	if(l_arraim) //
	{
		if (l_xdist || l_ydist)
		{
			double l_tmp = l_xydist2 * 1000 * 1000;
			l_tmp /= ulMoveSpd;
			l_tmp /= ulMoveSpd;
			l_elapse = long(sqrt(l_tmp));
			if (l_elapse ==0) //
			{
				l_elapse	=1;
			}
		}
		else
		{
			l_elapse		=0;
		}

		l_retval = -1;
	}

	bool	bIs45Dir = abs(lc_xdist) == abs(lc_ydist) ? true : false;
	char	chDirX = l_xdist < 0 ? -1 : 1;
	char	chDirY = l_ydist < 0 ? -1 : 1;
	if (!l_arraim) //(l_dist2 <l_xydist2)xy
	{
		l_xdist = long(sqrt((l_dist2 * l_xdist2) / l_xydist2)) * chDirX;
		l_ydist = long(sqrt((l_dist2 * l_ydist2) / l_xydist2)) * chDirY;
	}
	if (bIs45Dir)
	{
		if ((l_src.X + l_xdist) % m_submap->GetBlockCellWidth() == 0)
			l_xdist -= chDirX;
		l_ydist = abs(l_xdist) * chDirY;
	}

	if (l_elapse > 0) //
	{
		bool l_lap = overlap(l_xdist, l_ydist);
		m_shape.Centre.X = l_src.X + l_xdist;
		m_shape.Centre.Y = l_src.Y + l_ydist;
		if (l_lap) //
		{
			long l_x2y2 = l_xdist * l_xdist + l_ydist * l_ydist;
			double l_tmp = double(l_x2y2) * 1000 * 1000;
			l_tmp /= ulMoveSpd;
			l_tmp /= ulMoveSpd;
			l_elapse = long(sqrt(l_tmp));
			if ((l_elapse ==0) && (l_x2y2 >0)) //
			{
				l_elapse = 1;
			}

			l_retval = -2;
		}
	}

	*ulElapse = l_elapse;

	return l_retval;
}

//  :    ()
void CMoveAble::NotiMovToEyeshot()
{
	Corsairs::Net::Msg::McCharacterActionMessage msg{};
	msg.worldId = m_ID;
	msg.packetId = m_ulPacketID;
	msg.actionType = Corsairs::Net::Msg::ActionType::MOVE;
	Corsairs::Net::Msg::ActionMoveData move;
	move.moveState = m_SMoveProc.sState;
	move.stopState = m_SMoveInit.sStopState;
	auto* raw = reinterpret_cast<const uint8_t*>(m_SMoveProc.SNoticePoint.SList);
	move.waypoints.assign(raw, raw + sizeof(Corsairs::Util::Point) * m_SMoveProc.SNoticePoint.sNum);
	msg.data = std::move(move);
	auto pk = Corsairs::Net::Msg::serialize(msg);

	NotiChgToEyeshot(pk);
}

//  :    ()
void CMoveAble::NotiSelfMov()
{
	Corsairs::Net::Msg::McCharacterActionMessage msg{};
	msg.worldId = m_ID;
	msg.packetId = m_ulPacketID;
	msg.actionType = Corsairs::Net::Msg::ActionType::MOVE;
	Corsairs::Net::Msg::ActionMoveData move;
	move.moveState = m_SMoveProc.sState;
	move.stopState = m_SMoveInit.sStopState;
	auto* raw = reinterpret_cast<const uint8_t*>(m_SMoveProc.SNoticePoint.SList);
	move.waypoints.assign(raw, raw + sizeof(Corsairs::Util::Point) * m_SMoveProc.SNoticePoint.sNum);
	msg.data = std::move(move);
	auto pk = Corsairs::Net::Msg::serialize(msg);

	ReflectINFof(this,pk);
}

bool CMoveAble::GetMoveTargetShape(Corsairs::Util::Square *pSTarShape)
{
	if (m_SMoveInit.STargetInfo.chType == 1) // 
	{
		Entity	*pTarObj = g_pGameApp->IsMapEntity(m_SMoveInit.STargetInfo.lInfo1, m_SMoveInit.STargetInfo.lInfo2);
		if (!pTarObj)
			return false;
		if (pSTarShape)
		{
			*pSTarShape = pTarObj->GetShape();
		}
	}
	else if (m_SMoveInit.STargetInfo.chType == 2) // 
	{
		if (pSTarShape)
		{
			pSTarShape->Centre.X = m_SMoveInit.STargetInfo.lInfo1;
			pSTarShape->Centre.Y = m_SMoveInit.STargetInfo.lInfo2;
			pSTarShape->Radius = 0;
		}
	}

	return true;
}

bool CMoveAble::SetMoveOnInfo(SMoveInit* pSMoveI)
{
	if (GetMoveState() != enumMSTATE_ON)
		return false;
	if (m_SMoveInit.STargetInfo.chType  != 0
		&& m_SMoveInit.STargetInfo.chType == (*pSMoveI).STargetInfo.chType
		&& m_SMoveInit.STargetInfo.lInfo1 == (*pSMoveI).STargetInfo.lInfo1
		&& m_SMoveInit.STargetInfo.lInfo2 == (*pSMoveI).STargetInfo.lInfo2
		)
	{
		m_SMoveInit = *pSMoveI;
		m_SMoveProc.sCurInflexion = 1;
		return true;
	}

	return false;
}

//=============================================================================
// pSPort1pSPort2pSReference
//=============================================================================
Corsairs::Util::Point CMoveAble::NearlyPointFromPointToLine(const Corsairs::Util::Point *pSPort1, const Corsairs::Util::Point *pSPort2, const Corsairs::Util::Point *pSReference)
{
	Corsairs::Util::Point	SNearlyPoint;
	std::int32_t	lMaxX, lMinX, lMaxY, lMinY;

	if (pSPort1->X > pSPort2->X)
		lMaxX = pSPort1->X, lMinX = pSPort2->X;
	else
		lMaxX = pSPort2->X, lMinX = pSPort1->X;
	if (pSPort1->Y > pSPort2->Y)
		lMaxY = pSPort1->Y, lMinY = pSPort2->Y;
	else
		lMaxY = pSPort2->Y, lMinY = pSPort1->Y;

	if (pSPort1->X == pSPort2->X) // Y
	{
		SNearlyPoint.X = pSPort1->X;
		if (pSReference->Y < lMinY)
			SNearlyPoint.Y = lMinY;
		else if (pSReference->Y > lMaxY)
			SNearlyPoint.Y = lMaxY;
		else
			SNearlyPoint.Y = pSReference->Y;
	}
	else if (pSPort1->Y == pSPort2->Y) // X
	{
		SNearlyPoint.Y = pSPort1->Y;
		if (pSReference->X < lMinX)
			SNearlyPoint.X = lMinX;
		else if (pSReference->X > lMaxX)
			SNearlyPoint.X = lMaxX;
		else
			SNearlyPoint.X = pSReference->X;
	}
	else // 0
	{
		double dSlope;
		// 
		dSlope = double(pSPort2->Y - pSPort1->Y) / double(pSPort2->X - pSPort1->X);
		// 
		SNearlyPoint.X = std::int32_t((dSlope * dSlope * pSPort1->X + dSlope * (pSReference->Y - pSPort1->Y) + pSReference->X) / (dSlope * dSlope + 1));
		SNearlyPoint.Y = std::int32_t(dSlope * (SNearlyPoint.X - pSPort1->X) + pSPort1->Y);
		// 
		if (SNearlyPoint.X < lMinX || SNearlyPoint.X > lMaxX
			|| SNearlyPoint.Y < lMinY && SNearlyPoint.Y > lMaxY) // 
		{
			if (double(SNearlyPoint.X - pSPort1->X) * double(SNearlyPoint.X - pSPort1->X)
				+ double(SNearlyPoint.Y - pSPort1->Y) * double(SNearlyPoint.Y - pSPort1->Y)
				< double(SNearlyPoint.X - pSPort2->X) * double(SNearlyPoint.X - pSPort2->X)
				+ double(SNearlyPoint.Y - pSPort2->Y) * double(SNearlyPoint.Y - pSPort2->Y))
				SNearlyPoint = *pSPort1;
			else
				SNearlyPoint = *pSPort2;
		}
	}

	return SNearlyPoint;
}

//=============================================================================
// pSPort1pSPort2pSCircle
// falsetruepResult
//=============================================================================
//bool CMoveAble::SegmentEnterCircle(Corsairs::Util::Point *pSPort1, Corsairs::Util::Point *pSPort2, Corsairs::Util::Circle *pSCircle, Corsairs::Util::Point *pResult)
//{
//	bool	bRet;
//	double	dDistP1C2, dDistP2C2; // 
//	double	dRadius2;
//	std::int32_t	lDist;
//
//	dDistP1C2 = double(pSPort1->x - pSCircle->centre.x) * double(pSPort1->x - pSCircle->centre.x)
//		+ double(pSPort1->y - pSCircle->centre.y) * double(pSPort1->y - pSCircle->centre.y);
//	dDistP2C2 = double(pSPort2->x - pSCircle->centre.x) * double(pSPort2->x - pSCircle->centre.x)
//		+ double(pSPort2->y - pSCircle->centre.y) * double(pSPort2->y - pSCircle->centre.y);
//	dRadius2 = pSCircle->radius * pSCircle->radius;
//
//	if (dDistP1C2 <= dRadius2) // 
//	{
//		bRet = true;
//		pResult = pSPort1;
//	}
//	else // 
//	{
//		if (pSPort1->x == pSPort2->x) // y
//		{
//			lDist = abs(pSPort1->x - pSCircle->centre.x);
//			if (lDist < pSCircle->radius)
//			{
//			}
//			else if (lDist == pSCircle->radius)
//			{
//				bRet = true;
//				pResult->x = pSPort1->x;
//				pResult->y = pSC
//			}
//			else
//			{
//			}
//		}
//	}
//
//	return bRet;
//}
