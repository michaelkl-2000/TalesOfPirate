//=============================================================================
// FileName: Action.h
// Creater: ZhangXuedong
// Date: 2004.10.08
// Comment: CAction class
//=============================================================================

#ifndef ACTION_H
#define ACTION_H


#include "App/GameAppNet.h"
#include "Entity/Entity.h"
#include "Combat/MoveAble.h"
#include "Combat/FightAble.h"

#define defMAX_ACTION_NUM	2

class CAction
{
public:
	struct SActionQueue
	{
		int16_t	sType;
		void		*pInit;
	};

public:
	CAction(Entity *);

	bool Add(int16_t sActionType, void *pActionData);
	bool DoNext(int16_t sActionType = 0, int16_t sActionState = 0);
	void End();
	void Interrupt();

	int16_t GetActionNum() {return m_sActionNum;}
	int16_t GetCurActionNo() {return m_sCurAction;}
	bool Has(int16_t sActionType, void *pActionData);

protected:

private:
	Entity			*m_pCEntity;
	SActionQueue	m_SAction[defMAX_ACTION_NUM];
	int16_t		m_sActionNum;
	int16_t		m_sCurAction;

	CMoveAble::SMoveInit m_SMoveInit;
	SFightInit			 m_SFightInit;

};

#define defMAX_CACHE_ACTION_PARAM_LEN	30

enum ECacheActionType
{
	enumCACHEACTION_MOVE,
	enumCACHEACTION_SKILL,
	enumCACHEACTION_SKILL2,
};

class CCharacter;
class CActionCache
{
public:
	struct SAction
	{
		int16_t	sCommand;
		char	szParam[defMAX_CACHE_ACTION_PARAM_LEN];
		char	chParamPos;

		SAction	*pSNext;
	};

	CActionCache(CCharacter *pCOwn);
	~CActionCache();

	void	AddCommand(int16_t sCommand);
	void	PushParam(void *pParam, char chSize);
	void	Run(void);
	void	ExecAction(SAction *pSCarrier);

protected:

private:
	CCharacter	*m_pCOwn;

	SAction	*m_pSExecQueue;	// 
	SAction	*m_pSFreeQueue;	// 

};

#endif // ACTION_H
