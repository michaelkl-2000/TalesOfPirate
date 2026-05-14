//=============================================================================
// FileName: Item.h
// Creater: ZhangXuedong
// Date: 2004.09.21
// Comment: CItem class
//=============================================================================

#ifndef ITEM_H
#define ITEM_H

#include "Character/Character.h"
#include "Item/ItemRecord.h"
#include "Network/CompCommand.h"

#define defITEM_ON_TICK		3 * 60 * 1000

enum EItemProtType // 
{
	enumITEM_PROT_OWN,	// 
	enumITEM_PROT_TEAM,	// 
};

class CItem : public Entity
{
public:
	CItem();

	void	Initially();
	void	Finally();

	virtual void	Run(std::uint32_t ulCurTick);

	SItemGrid	*GetGridContent(void) {return &m_SGridContent;}
	void		SetFromID(std::int32_t lFromEntityID) {m_lFromEntityID = lFromEntityID;}
	void		SetSpawnType(char chType) {m_chSpawType = chType;}
	void		SetStartTick(std::uint32_t ulTick) {m_ulStartTick = ulTick;}
	void		SetOnTick(std::uint32_t ulOnTick) {m_ulOnTick = ulOnTick;}
	void		SetProtOnTick(std::uint32_t ulProtOnTick) {m_ulProtOnTick = ulProtOnTick;}
	void		SetProtCha(std::uint32_t ulChaID, std::uint32_t ulChaHandle) {if (m_ulProtOnTick == 0) m_ulProtID = 0; else m_ulProtID = ulChaID, m_ulProtHandle = ulChaHandle;}
	std::uint32_t	GetProtChaID(void) {return m_ulProtID;}
	std::uint32_t	GetProtChaHandle(void) {return m_ulProtHandle;}
	void		SetProtType(char chType = enumITEM_PROT_OWN) {m_chProtType = chType;}
	char	GetProtType(void) {return m_chProtType;}

	char			chValid;
	CItemRecord		*m_pCItemRecord;

	SItemGrid		m_SGridContent;

protected:
	virtual CItem *IsItem(){return this;}

private:
	char	m_chSpawType;
	std::int32_t	m_lFromEntityID;
	std::uint32_t	m_ulStartTick;
	std::uint32_t	m_ulOnTick;		// 0

	char	m_chProtType;	// 
	std::uint32_t	m_ulProtOnTick;	// 0
	std::uint32_t	m_ulProtID;		// ID
	std::uint32_t	m_ulProtHandle;	// Handle

	virtual void OnBeginSeen(CCharacter *pCCha);
	virtual void OnEndSeen(CCharacter *pCCha);
};

#endif // ITEM_H
