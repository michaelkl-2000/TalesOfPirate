//=============================================================================
// FileName: ChaAttr.cpp
// Comment: CChaAttr class
//=============================================================================

#include "Character/ChaAttr.h"
#include "Character/CharacterRecord.h"
#include "Network/CompCommand.h"

#include <cstdint>

namespace Corsairs::Common::Character {

std::array<std::int32_t, ATTR_MAX_NUM> g_attrMax{};

CChaAttr::CChaAttr() {
    Clear();
}

void CChaAttr::Clear() {
    _attribute.fill(0);
}

void CChaAttr::Init(std::int32_t id, bool applyProgressionDefaults) {
    ResetChangeFlag();

    // Дефолты для item/state-коэффициентов: 1000 = "100.0%" в fixed-point.
    // Item/state-значения и boat-skill-значения стартуют с нуля.
    for (std::int32_t i = ATTR_ITEMC_STR;        i <= ATTR_ITEMC_PDEF;        ++i) _attribute[i] = 1000;
    for (std::int32_t i = ATTR_STATEC_STR;       i <= ATTR_STATEC_PDEF;       ++i) _attribute[i] = 1000;
    for (std::int32_t i = ATTR_BOAT_SKILLC_MNATK; i <= ATTR_BOAT_SKILLC_MXSPLY; ++i) _attribute[i] = 1000;
    // ITEMV/STATEV/BOAT_SKILLV/EXTEND zero-initialized в Clear().

    ChaRecord* pCChaRecord = GetChaRecordInfo(id);
    if (!pCChaRecord) {
        return;
    }

    _id = id;
    if (applyProgressionDefaults) {
        _attribute[ATTR_LV]    = static_cast<std::int32_t>(pCChaRecord->Lv);
        _attribute[ATTR_HP]    = static_cast<std::int32_t>(pCChaRecord->MxHp);
        _attribute[ATTR_SP]    = static_cast<std::int32_t>(pCChaRecord->MxSp);
        _attribute[ATTR_CEXP]  = pCChaRecord->CExp;
        _attribute[ATTR_NLEXP] = pCChaRecord->NExp;
        _attribute[ATTR_FAME]  = static_cast<std::int32_t>(pCChaRecord->Fame);
        _attribute[ATTR_AP]    = static_cast<std::int32_t>(pCChaRecord->Ap);
        _attribute[ATTR_TP]    = static_cast<std::int32_t>(pCChaRecord->Tp);
        _attribute[ATTR_GD]    = static_cast<std::int32_t>(pCChaRecord->Gd);
    }
    _attribute[ATTR_SPRI]        = static_cast<std::int32_t>(pCChaRecord->Spri);
    _attribute[ATTR_CHATYPE]     = static_cast<std::int32_t>(pCChaRecord->CtrlType);
    _attribute[ATTR_STR]         = static_cast<std::int32_t>(pCChaRecord->Str);
    _attribute[ATTR_DEX]         = static_cast<std::int32_t>(pCChaRecord->Dex);
    _attribute[ATTR_AGI]         = static_cast<std::int32_t>(pCChaRecord->Agi);
    _attribute[ATTR_CON]         = static_cast<std::int32_t>(pCChaRecord->Con);
    _attribute[ATTR_STA]         = static_cast<std::int32_t>(pCChaRecord->Sta);
    _attribute[ATTR_LUK]         = static_cast<std::int32_t>(pCChaRecord->Luk);
    _attribute[ATTR_LHAND_ITEMV] = static_cast<std::int32_t>(pCChaRecord->LHandVal);
    _attribute[ATTR_MXHP]        = static_cast<std::int32_t>(pCChaRecord->MxHp);
    _attribute[ATTR_MXSP]        = static_cast<std::int32_t>(pCChaRecord->MxSp);
    _attribute[ATTR_MNATK]       = static_cast<std::int32_t>(pCChaRecord->MnAtk);
    _attribute[ATTR_MXATK]       = static_cast<std::int32_t>(pCChaRecord->MxAtk);
    _attribute[ATTR_DEF]         = static_cast<std::int32_t>(pCChaRecord->Def);
    _attribute[ATTR_HIT]         = static_cast<std::int32_t>(pCChaRecord->Hit);
    _attribute[ATTR_FLEE]        = static_cast<std::int32_t>(pCChaRecord->Flee);
    _attribute[ATTR_MF]          = static_cast<std::int32_t>(pCChaRecord->Mf);
    _attribute[ATTR_CRT]         = static_cast<std::int32_t>(pCChaRecord->Crt);
    _attribute[ATTR_HREC]        = static_cast<std::int32_t>(pCChaRecord->HRec);
    _attribute[ATTR_SREC]        = static_cast<std::int32_t>(pCChaRecord->SRec);
    _attribute[ATTR_ASPD]        = static_cast<std::int32_t>(pCChaRecord->ASpd);
    _attribute[ATTR_ADIS]        = static_cast<std::int32_t>(pCChaRecord->ADis);
    _attribute[ATTR_MSPD]        = static_cast<std::int32_t>(pCChaRecord->MSpd);
    _attribute[ATTR_COL]         = static_cast<std::int32_t>(pCChaRecord->Col);
    _attribute[ATTR_PDEF]        = static_cast<std::int32_t>(pCChaRecord->PDef);

    if (applyProgressionDefaults) {
        _attribute[ATTR_BSTR]  = static_cast<std::int32_t>(pCChaRecord->Str);
        _attribute[ATTR_BDEX]  = static_cast<std::int32_t>(pCChaRecord->Dex);
        _attribute[ATTR_BAGI]  = static_cast<std::int32_t>(pCChaRecord->Agi);
        _attribute[ATTR_BCON]  = static_cast<std::int32_t>(pCChaRecord->Con);
        _attribute[ATTR_BSTA]  = static_cast<std::int32_t>(pCChaRecord->Sta);
        _attribute[ATTR_BLUK]  = static_cast<std::int32_t>(pCChaRecord->Luk);
        _attribute[ATTR_BMXHP] = static_cast<std::int32_t>(pCChaRecord->MxHp);
        _attribute[ATTR_BMXSP] = static_cast<std::int32_t>(pCChaRecord->MxSp);
    }
    _attribute[ATTR_BMNATK] = static_cast<std::int32_t>(pCChaRecord->MnAtk);
    _attribute[ATTR_BMXATK] = static_cast<std::int32_t>(pCChaRecord->MxAtk);
    _attribute[ATTR_BDEF]   = static_cast<std::int32_t>(pCChaRecord->Def);
    _attribute[ATTR_BHIT]   = static_cast<std::int32_t>(pCChaRecord->Hit);
    _attribute[ATTR_BFLEE]  = static_cast<std::int32_t>(pCChaRecord->Flee);
    _attribute[ATTR_BMF]    = static_cast<std::int32_t>(pCChaRecord->Mf);
    _attribute[ATTR_BCRT]   = static_cast<std::int32_t>(pCChaRecord->Crt);
    _attribute[ATTR_BHREC]  = static_cast<std::int32_t>(pCChaRecord->HRec);
    _attribute[ATTR_BSREC]  = static_cast<std::int32_t>(pCChaRecord->SRec);
    _attribute[ATTR_BASPD]  = static_cast<std::int32_t>(pCChaRecord->ASpd);
    _attribute[ATTR_BADIS]  = static_cast<std::int32_t>(pCChaRecord->ADis);
    _attribute[ATTR_BMSPD]  = static_cast<std::int32_t>(pCChaRecord->MSpd);
    _attribute[ATTR_BCOL]   = static_cast<std::int32_t>(pCChaRecord->Col);
    _attribute[ATTR_BPDEF]  = static_cast<std::int32_t>(pCChaRecord->PDef);
}

std::int32_t CChaAttr::GetAttr(std::int32_t no) const {
    if (!IsValidAttr(no)) {
        return -1;
    }
    return _attribute[no];
}

std::int32_t CChaAttr::GetAttrMaxVal(std::int32_t no) const {
    if (!IsValidAttr(no)) {
        return -1;
    }
    return g_attrMax[no];
}

// Возвращает 0 — невалидный индекс; 1 — значение было capped; 2 — записано как есть.
std::int32_t CChaAttr::SetAttr(std::int32_t no, std::int32_t val) {
    if (!IsValidAttr(no)) {
        return 0;
    }

    std::int32_t ret = 2;
    // Сравниваем как unsigned-расширенный int64, чтобы неинициализированный
    // g_attrMax==-1 не отсёк все значения (legacy-поведение).
    const auto maxVal = static_cast<std::int64_t>(static_cast<std::uint32_t>(g_attrMax.at(no)));
    if (val > maxVal) {
        ret = 1;
        // EXP-атрибуты семантически unsigned (см. ATTR_CEXP..NLV_LIFEEXP):
        // overflow в negative ловится в Lua-чтении как unsigned int. Поэтому
        // capping к ним не применяется — иначе ломается прогресс прокачки.
        if (!IsExpAttr(no)) {
            val = g_attrMax.at(no);
            ToLogService("common", "set Cha[{}] Attrib[{}] greater than max value [{}]", _name, no, val);
        }
    }

    if (_attribute[no] != val) {
        SetChangeBitFlag(no);
        _attribute[no] = val;
    }

    return ret;
}

std::int32_t CChaAttr::DirectSetAttr(std::int32_t no, std::int32_t val) {
    if (!IsValidAttr(no)) {
        return 0;
    }

    if (_attribute[no] != val) {
        SetChangeBitFlag(no);
        _attribute[no] = val;
    }

    return 1;
}

std::int32_t CChaAttr::AddAttr(std::int32_t no, std::int32_t val) {
    if (!IsValidAttr(no)) {
        return 0;
    }

    SetAttr(no, _attribute[no] + val);
    return 1;
}

void CChaAttr::SetChangeFlag() {
    _dirty.set();
    _changeNumClient = static_cast<std::int16_t>(ATTR_CLIENT_MAX);
}

void CChaAttr::ResetChangeFlag() {
    _dirty.reset();
    _changeNumClient = 0;
}

void CChaAttr::SetChangeBitFlag(std::int32_t bit) {
    if (!IsValidAttr(bit)) {
        return;
    }
    if (IsClientAttr(bit) && !_dirty.test(bit)) {
        ++_changeNumClient;
    }
    _dirty.set(bit);
}

bool CChaAttr::GetChangeBitFlag(std::int32_t bit) const {
    if (!IsValidAttr(bit)) {
        return false;
    }
    return _dirty.test(bit);
}

} // namespace Corsairs::Common::Character
