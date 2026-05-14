//=============================================================================
// FileName: MoveAble.h
// Creater: ZhangXuedong
// Date: 2004.11.03
// Comment: CMoveAble class
//=============================================================================

#ifndef MOVEABLE_H
#define MOVEABLE_H

#include "Combat/FightAble.h"

#define defMOVE_INFLEXION_NUM	32
#define defPOS_QUEUE_MEMBER_NUM	32

class	CMoveAble : public CFightAble
{
public:
	struct SPointList
	{
		Corsairs::Util::Point		SList[defMOVE_INFLEXION_NUM];
		int16_t	sNum;
	};

	struct STarget
	{
		// SFightInit
		struct
		{
			char		chType;	// 012
			std::int32_t		lInfo1;
			std::int32_t		lInfo2;
		};
		//
		std::uint32_t	ulDist;		//
	};

	struct SMoveInit
	{
		//std::uint32_t	ulSpeed;		// /
		std::uint16_t	usPing;			//

		SPointList	SInflexionInfo;	// 
		STarget		STargetInfo;	// 

		int16_t	sStopState;		// enumEXISTS_WAITING, enumEXISTS_SLEEPING
	};

	struct SMoveProc
	{
		int16_t	sCurInflexion;	// 

		std::uint32_t	ulElapse;		//
		std::uint32_t	ulCacheTick;

		int16_t	sState;		// CompCommand.hEMoveState
		char	chRequestState;	// 012
		char	chLagMove;		// 01

		SPointList	SNoticePoint;	// 
	};

	struct SMoveRedundance
	{
		std::uint32_t	ulStartTick;
		std::uint32_t	ulLeftTime;
	};

	SMoveInit	m_SMoveInit;
	SMoveInit	m_SMoveInitCache;
	SMoveProc	m_SMoveProc;
	SMoveRedundance	m_SMoveRedu;
	std::int32_t	m_lSetPing;

	CMoveAble();

	bool	DesireMoveBegin(SMoveInit *pSMove);
	void	DesireMoveStop() {EndMove();};
	void	OnMove(std::uint32_t ulTimePrecision);

	char AttemptMove(double dDistance, bool bNotiInflexion = true);
	char LinearAttemptMove(Corsairs::Util::Point STar, double distance, std::uint32_t *ulElapse);

	void	ResetMove();

protected:
	void	Initially();
	void	Finally();
	virtual bool	overlap(long& xdist,long& ydist);
	virtual CMoveAble* IsMoveAble() override { return this; }

	void	NotiSelfMov();
	void	NotiMovToEyeshot();

	bool		GetMoveTargetShape(Corsairs::Util::Square *pSTarShape);
	int16_t	GetMoveState() {return m_SMoveProc.sState;}
	void		SetMoveState(int16_t sState) {m_SMoveProc.sState = sState;}
	int16_t	GetMoveStopState(void) {return m_SMoveInit.sStopState;}
	const Corsairs::Util::Point	&GetMoveEndPos(void) {return m_SMoveInit.SInflexionInfo.SList[m_SMoveInit.SInflexionInfo.sNum - 1];}
	bool		SetMoveOnInfo(SMoveInit* pSMoveI);

private:
	virtual void AfterStepMove(void){}; // AttemptMoveAfterStepMoveAttemptMove
	virtual void SubsequenceMove(){};

	void BeginMove(std::uint32_t ulElapse = 0);
	void EndMove();
	void OnMoveBegin(void) {m_bOnMove = true;}
	void OnMoveEnd(void) {m_bOnMove = false;}

	bool AreaOverlap(long& xdist,long& ydist);

	Corsairs::Util::Point NearlyPointFromPointToLine(const Corsairs::Util::Point *pPort1, const Corsairs::Util::Point *pPort2, const Corsairs::Util::Point *pCenter);
	bool SegmentEnterCircle(Corsairs::Util::Point *pSPort1, Corsairs::Util::Point *pSPort2, Corsairs::Util::Circle *pSCircle, Corsairs::Util::Point *pResult);

	std::uint16_t	m_usHeartbeatFreq;	//
	std::uint32_t	m_ulHeartbeatTick;	//
	bool		m_bOnMove;

	CTimer		m_timeRun;

};

#endif // MOVEABLE_H
