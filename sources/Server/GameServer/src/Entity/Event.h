//=============================================================================
// FileName: Event.h
// Creater: ZhangXuedong
// Date: 2005.10.11
// Comment: Event
//=============================================================================

#ifndef EVENT_H
#define EVENT_H


#define defMAX_EVENT_NAME_LEN	256

class CEvent
{
public:
	CEvent() {Init();}

	void	Init(void)
	{
		m_usID = 0;
		m_usTouchType = 0;
		m_usExecType = 0;
		m_pTableRec = 0;
		m_szName[0] = '\0';
	}

	void	SetID(std::uint16_t usID) {m_usID = usID;}
	void	SetTouchType(std::uint16_t usTchTpe) {m_usTouchType = usTchTpe;}
	void	SetExecType(std::uint16_t usExcTpe) {m_usExecType = usExcTpe;}
	void	SetTableRec(void *pTblRec) {m_pTableRec = pTblRec;}
	void	SetName(const char *cszName) {if (!cszName) return; strncpy(m_szName, cszName, defMAX_EVENT_NAME_LEN - 1); m_szName[defMAX_EVENT_NAME_LEN -1] = '\0';}
	std::uint16_t	GetID(void) {return m_usID;}
	std::uint16_t	GetTouchType(void) {return m_usTouchType;}
	std::uint16_t	GetExecType(void) {return m_usExecType;}
	void*	GetTableRec(void) {return m_pTableRec;}
	const char*	GetName(void) const {return m_szName;}

	void	WriteInfo(Corsairs::Net::WPacket &pk);

protected:

private:
	std::uint16_t	m_usID;			//
	std::uint16_t	m_usTouchType;	//
	std::uint16_t	m_usExecType;	//
	void		*m_pTableRec;
	char	m_szName[defMAX_EVENT_NAME_LEN];		// 

};

inline void CEvent::WriteInfo(Corsairs::Net::WPacket &pk)
{
	pk.WriteInt64(m_usID);
	pk.WriteString(m_szName);
}

#endif // EVENT_H
