#pragma once

#include "Network/CompCommand.h"
#include "Skill/SkillRecord.h"
#include "Character/CharacterRecord.h"
#include "Item/ItemRecord.h"
#include "Item/ItemAttrType.h"
#include "Core/JobType.h"
#include "Network/NetRetCode.h"
#include <cstdint>
#include <string>
#include <string_view>
#include <vector>

// TODO: убрать после обёртки Core в Corsairs::Common::Core (Ф-Co).
// CommFunc.h активно использует Network/Item/Character/Skill types и
// будет жить в Corsairs::Common::Core после миграции; пока — using в global.
using namespace Corsairs::Common::Network;
using namespace Corsairs::Common::Item;
using namespace Corsairs::Common::Skill;
using namespace Corsairs::Common::Character;


// Вставляет ёмкость инвентаря в начало сериализованной строки сумки в формате "{ёмкость}@...".
extern bool KitbagStringConv(short sKbCapacity, std::string& strData);

// Проверяет, валиден ли визуальный слот персонажа: nType в [1..PLAY_NUM], nPart в [0..enumEQUIP_NUM].
extern bool IsValidLook(int nType, int nPart, long nValue);

// Зона — море? Возвращает true, если AreaMask не содержит признака суши.
extern bool IsSea(unsigned short usAreaMask);

// Зона — суша или мост? Удобен для проверки доступности зон сухопутным персонажам.
extern bool IsLand(unsigned short usAreaMask);

// Какой скилл активируется парой экипированных предметов (левая/правая рука). Возвращает -1, если совпадения нет.
extern int GetItemSkill(int nLeftItemID, int nRightItemID);

// Корректна ли дистанция: квадрат расстояния между точками < dwDist*10000.
extern bool IsDist(int x1, int y1, int x2, int y2, std::uint32_t dwDist);

// Можно ли применить указанный скилл к данной цели — проверяет тип цели, состояние, область,
// тип скилла, дружественность и т.п. Возвращает enumESKILL_* код (SUCCESS или конкретная ошибка).
extern int IsRightSkillTar(int nTChaCtrlType, bool bTIsDie, bool bTChaBeSkilled, int nTChaArea,
                            int nSChaCtrlType, int nSSkillObjType, int nSSkillObjHabitat, int nSSkillEffType,
                            bool bIsTeammate, bool bIsFriend, bool bIsSelf);

// Удовлетворяет ли текущая экипировка требованиям скилла (item-need в трёх слотах + conch-need).
// Возвращает 1 — да, 0 — не подходит, -1 — скилла с таким ID нет.
extern int IsUseSkill(stNetChangeChaPart* pSEquip, int nSkillID);

// Реальный ли item ID (положительный и не зарезервированный под композитную экипировку).
extern bool IsRealItemId(int nItemID);

// Overload по CSkillRecord*: nullptr → -1, иначе перенаправляет на основной IsUseSkill.
extern int IsUseSkill(stNetChangeChaPart* pSEquip, CSkillRecord* p);

// Подходит ли указанный сухопутный/морской "fit" под скилл (поиск по item-need ID-условиям).
// Возвращает 1 — да, 0 — нет, -1 — скилл null.
extern int IsUseSeaLiveSkill(long lFitNo, CSkillRecord* p);

// Принадлежит ли персонаж игроку (включая питомца игрока)?
extern bool IsPlyCtrlCha(EChaCtrlType eChaCtrlType);

// Принадлежит ли персонаж монстру/добываемому объекту (дерево, шахта, рыба, лодка, ремонтируемое)?
extern bool IsMonsCtrlCha(EChaCtrlType eChaCtrlType);

// Принадлежит ли персонаж NPC или event-NPC?
extern bool IsNpcCtrlCha(EChaCtrlType eChaCtrlType);

// На противоположных сторонах ли два персонажа по типу контроллера (player vs не-player, mons vs не-mons).
// Используется, например, для агро-логики и фильтрации скиллов помощи/атаки.
extern bool IsChaEnemyCtrlSide(EChaCtrlType eSCtrlType, EChaCtrlType eTCtrlType);

// Существует ли файл по указанному пути. UTF-8 string_view конвертируется в std::filesystem::path.
extern bool FileExists(std::string_view szFileName);

// Конвертирует ITEM-attr тип в CHA-attr (диапазоны COE→ITEMC, VAL→ITEMV). Вне диапазонов возвращает 0.
extern long ConvItemAttrTypeToCha(long lItemAttrType);

// Сколько параметров требует указанный тип range (STICK/FAN — 2, SQUARE/CIRCLE — 1) + 1 на тип.
extern short GetRangeParamNum(char RangeType);

// Может ли персонаж двигаться по данной зоне с учётом своего "territory type" (LAND/SEA/DISCRETIONAL)
// и флага NOT_FIGHT (мирная зона запрещает движение монстрам).
extern bool IsMoveAble(EChaCtrlType eChaCtrlType, char chChaTerrType, unsigned short usAreaMask);

// Имя класса (Job) по ID. Out-of-range → имя класса 0 ("Newbie").
extern const char* GetJobName(short sJobID);

// Поиск Job ID по имени. Не найдено → 0 (Newbie).
extern short GetJobId(std::string_view szJobName);

// Имя стартового города по ID. Out-of-range → "".
extern const char* GetCityName(short sCityID);

// Поиск city ID по имени. Не найдено → -1.
extern short GetCityId(std::string_view szCityName);

// Текущая поза — "сидя"? (захардкодено: 16 = сидячая поза).
extern bool IsSeatPose(int pose);

// Валидно ли текущее боевое состояние (меньше enumFSTATE_TARGET_NO).
extern bool IsValidFightState(int nState);

// Состояние существования — "мёртв/увядает"? (>= enumEXISTS_WITHERING).
extern bool ExistStateIsDie(char chState);

// Описание ITEMATTR-бонуса (Strength Bonus, Defense Bonus, ...) по типу. COE и VAL дают одинаковый текст.
extern const char* GetItemAttrExplain(int v);

// Текстовое сообщение об ошибке сервера по коду ERR_*. Default-ветка генерирует диагностику с диапазоном
// "сервер-источник → сервер-получатель" через thread_local буфер (безопасно при многопоточности).
extern const char* GetServerError(int error_code);

// Все символы строки — буквы или цифры (ASCII isalnum).
extern bool IsAlphanumeric(std::string_view text);

// Все символы строки — цифры (ASCII isdigit).
extern bool IsNumeric(std::string_view text);

// Валидный e-mail (упрощённый regex {слово}(.|_)?{слово}@{слово}.{слово}+).
extern bool IsEmail(std::string_view email);

// Сериализация item-grid из CSV-строки. Формат: ID,Num,Endure[0..1],Energy[0..1],ForgeLv,DBParam*N,instAttrFlag,instAttr*N.
extern void String2Item(std::string_view pszData, SItemGrid* SGridCont);

// Сериализует look (внешний вид + экипировка) в строку с контрольной суммой. Используется для DB и сети.
extern bool LookData2String(const stNetChangeChaPart& pLook, std::string& strData);

// Парсит look из строки версии 112+ (с контрольной суммой) или старого формата. Возвращает false при несовпадении checksum.
extern bool String2LookData(stNetChangeChaPart& pLook, const std::string& strData);

// Сериализует горячие клавиши в char-буфер. Возвращает szShortcutBuf или nullptr при переполнении.
extern char* ShortcutData2String(const stNetShortCut* pShortcut, char* szShortcutBuf, int nLen);

// Парсит горячие клавиши из строки.
extern bool String2ShortcutData(stNetShortCut* pShortcut, std::string& strData);

// Валидное ли имя персонажа: latin/digit/UTF-8 multi-byte; запрещены 0xA1 0xA1 (двойной "пустой" символ GBK).
// TODO: legacy GBK-проверка после UTF-8 миграции работает корректно для ASCII, но запрещает имена,
// в которых после ведущего >=0x80 идёт continuation байт <=0x80 — нужно переписать как UTF-8 codepoint
// валидацию через Corsairs::Util::IsUtf8StartByte.
extern bool IsValidName(std::string_view name);

// Текстовое описание ошибки операции с инвентарём по коду enumITEMOPT_*.
extern const char* GetUseItemFailedInfo(short sErrorID);

// CTextFilter перенесён в Libraries/Util/src/Text/TextFilter.h (Corsairs::Util)
// в рамках Ф8 (2026-05). Включайте `TextFilter.h` напрямую.
