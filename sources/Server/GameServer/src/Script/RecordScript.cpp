// RecordScript.cpp
//---------------------------------------------------------
// См. RecordScript.h — биндинги сторов Item/Character/Skill/SkillState в Lua.
//---------------------------------------------------------
#include "Core/stdafx.h"
#include "Script/RecordScript.h"
#include "Script/Script.h"

#include "Item/ItemRecord.h"
#include "Item/ItemRecordStore.h"
#include "Character/CharacterRecord.h"
#include "Character/ChaRecordStore.h"
#include "Skill/SkillRecord.h"
#include "Skill/SkillRecordStore.h"
#include "Skill/SkillStateRecord.h"
#include "Skill/SkillStateRecordStore.h"

#include <string>

// ============================================================================
// Хелперы сериализации массивов в строку "v0,v1,..." (для совместимости с
// Lua-кодом, который раньше получал сырые txt-колонки).
// ============================================================================

namespace {

template <typename T>
std::string JoinArrayInt(const T* arr, int count) {
	std::string out;
	for (int i = 0; i < count; ++i) {
		if (i > 0) out += ',';
		out += std::to_string(static_cast<long long>(arr[i]));
	}
	return out;
}

std::string JoinPair(short v0, short v1) {
	return std::to_string(v0) + "," + std::to_string(v1);
}

} // namespace

// ============================================================================
// Глобальные функции-геттеры: GetItemRecord/GetChaRecord/GetSkillRecord/...
// Возвращают nullptr если запись не найдена (Lua получит nil).
// ============================================================================

static CItemRecord* LuaGetItemRecord(int id) {
	return ItemRecordStore::Instance()->Get(id);
}

static ChaRecord* LuaGetChaRecord(int id) {
	return ChaRecordStore::Instance()->Get(id);
}

static CSkillRecord* LuaGetSkillRecord(int id) {
	return SkillRecordStore::Instance()->Get(id);
}

static CSkillStateRecord* LuaGetSkillStateRecord(int id) {
	return SkillStateRecordStore::Instance()->Get(id);
}

// ============================================================================
// Итераторы: вызывают переданную Lua-функцию для каждой записи стора.
// fn(record)
// ============================================================================

static void LuaIterateItemRecords(luabridge::LuaRef fn) {
	if (!fn.isFunction()) {
		return;
	}
	ItemRecordStore::Instance()->ForEach([&fn](CItemRecord& rec) {
		try {
			fn(&rec);
		}
		catch (const luabridge::LuaException& e) {
			ToLogService("lua", LogLevel::Error, "IterateItemRecords fn error: {}", e.what());
		}
	});
}

static void LuaIterateChaRecords(luabridge::LuaRef fn) {
	if (!fn.isFunction()) {
		return;
	}
	ChaRecordStore::Instance()->ForEach([&fn](ChaRecord& rec) {
		try {
			fn(&rec);
		}
		catch (const luabridge::LuaException& e) {
			ToLogService("lua", LogLevel::Error, "IterateChaRecords fn error: {}", e.what());
		}
	});
}

static void LuaIterateSkillRecords(luabridge::LuaRef fn) {
	if (!fn.isFunction()) {
		return;
	}
	SkillRecordStore::Instance()->ForEach([&fn](CSkillRecord& rec) {
		try {
			fn(&rec);
		}
		catch (const luabridge::LuaException& e) {
			ToLogService("lua", LogLevel::Error, "IterateSkillRecords fn error: {}", e.what());
		}
	});
}

static void LuaIterateSkillStateRecords(luabridge::LuaRef fn) {
	if (!fn.isFunction()) {
		return;
	}
	SkillStateRecordStore::Instance()->ForEach([&fn](CSkillStateRecord& rec) {
		try {
			fn(&rec);
		}
		catch (const luabridge::LuaException& e) {
			ToLogService("lua", LogLevel::Error, "IterateSkillStateRecords fn error: {}", e.what());
		}
	});
}

// ============================================================================
// Регистрация классов и функций в Lua.
// ============================================================================

bool RegisterRecordScript() {
	lua_State* L = g_pLuaState;

	// --- CItemRecord ---
	luabridge::getGlobalNamespace(L)
		.beginClass<CItemRecord>("CItemRecord")
			.addFunction("GetID",         +[](CItemRecord* self) -> int         { return static_cast<int>(self->lID); })
			.addFunction("GetName",       +[](CItemRecord* self) -> std::string { return self->szName; })
			.addFunction("GetIcon",       +[](CItemRecord* self) -> std::string { return self->szICON; })
			.addFunction("GetType",       +[](CItemRecord* self) -> int         { return static_cast<int>(self->sType); })
			.addFunction("GetStack",      +[](CItemRecord* self) -> int         { return self->nPileMax; })
			.addFunction("GetPrice",      +[](CItemRecord* self) -> int         { return static_cast<int>(self->lPrice); })
			.addFunction("GetNeedLv",     +[](CItemRecord* self) -> int         { return static_cast<int>(self->sNeedLv); })
			.addFunction("GetDurability", +[](CItemRecord* self) -> std::string { return JoinPair(self->sEndure[0], self->sEndure[1]); })
			.addFunction("GetFunction",   +[](CItemRecord* self) -> std::string { return self->szAttrEffect; })
			.addFunction("GetBody",       +[](CItemRecord* self) -> std::string { return JoinArrayInt(self->chBody.data(), defITEM_BODY); })
			.addFunction("GetJobMask",    +[](CItemRecord* self) -> std::string { return JoinArrayInt(self->szWork.data(), MAX_JOB_TYPE); })
		.endClass();

	// --- CChaRecord ---
	luabridge::getGlobalNamespace(L)
		.beginClass<ChaRecord>("CChaRecord")
			.addFunction("GetID",         +[](ChaRecord* self) -> int         { return static_cast<int>(self->lID); })
			.addFunction("GetName",       +[](ChaRecord* self) -> std::string { return self->szName; })
			.addFunction("GetLevel",      +[](ChaRecord* self) -> int         { return static_cast<int>(self->lLv); })
			.addFunction("GetHp",         +[](ChaRecord* self) -> int         { return static_cast<int>(self->lHp); })
			.addFunction("GetMaxHp",      +[](ChaRecord* self) -> int         { return static_cast<int>(self->lMxHp); })
			.addFunction("GetSp",         +[](ChaRecord* self) -> int         { return static_cast<int>(self->lSp); })
			.addFunction("GetMaxSp",      +[](ChaRecord* self) -> int         { return static_cast<int>(self->lMxSp); })
			.addFunction("GetCtrlType",   +[](ChaRecord* self) -> int         { return static_cast<int>(self->chCtrlType); })
			.addFunction("GetModalType",  +[](ChaRecord* self) -> int         { return static_cast<int>(self->chModalType); })
			.addFunction("GetExpReward",  +[](ChaRecord* self) -> int         { return static_cast<int>(self->lGetEXP); })
		.endClass();

	// --- CSkillRecord ---
	luabridge::getGlobalNamespace(L)
		.beginClass<CSkillRecord>("CSkillRecord")
			.addFunction("GetID",             +[](CSkillRecord* self) -> int         { return static_cast<int>(self->sID); })
			.addFunction("GetName",           +[](CSkillRecord* self) -> std::string { return self->szName; })
			.addFunction("GetPhase",          +[](CSkillRecord* self) -> int         { return static_cast<int>(self->chPhase); })
			.addFunction("GetType",           +[](CSkillRecord* self) -> int         { return static_cast<int>(self->chType); })
			.addFunction("GetLevelDemand",    +[](CSkillRecord* self) -> int         { return static_cast<int>(self->sLevelDemand); })
			.addFunction("GetApplyDistance",  +[](CSkillRecord* self) -> int         { return static_cast<int>(self->sApplyDistance); })
			.addFunction("GetApplyType",      +[](CSkillRecord* self) -> int         { return static_cast<int>(self->chApplyType); })
			.addFunction("GetApplyTarget",    +[](CSkillRecord* self) -> int         { return static_cast<int>(self->chApplyTarget); })
			.addFunction("GetHelpful",        +[](CSkillRecord* self) -> int         { return static_cast<int>(self->chHelpful); })
			.addFunction("GetStateID",        +[](CSkillRecord* self) -> int         { return self->nStateID; })
			.addFunction("GetFightType",      +[](CSkillRecord* self) -> int         { return static_cast<int>(self->chFightType); })
			.addFunction("GetPointExpend",    +[](CSkillRecord* self) -> int         { return static_cast<int>(self->chPointExpend); })
			.addFunction("GetAngle",          +[](CSkillRecord* self) -> int         { return static_cast<int>(self->sAngle); })
			.addFunction("GetRadii",          +[](CSkillRecord* self) -> int         { return static_cast<int>(self->sRadii); })
		.endClass();

	// --- CSkillStateRecord ---
	luabridge::getGlobalNamespace(L)
		.beginClass<CSkillStateRecord>("CSkillStateRecord")
			.addFunction("GetID",            +[](CSkillStateRecord* self) -> int         { return self->Id; })
			.addFunction("GetName",          +[](CSkillStateRecord* self) -> std::string { return self->szName; })
			.addFunction("GetFrequency",     +[](CSkillStateRecord* self) -> int         { return static_cast<int>(self->sFrequency); })
			.addFunction("GetDescriptor",    +[](CSkillStateRecord* self) -> std::string { return self->szDesc; })
			.addFunction("CanCancel",        +[](CSkillStateRecord* self) -> bool        { return self->bCanCancel; })
			.addFunction("CanMove",          +[](CSkillStateRecord* self) -> bool        { return self->bCanMove; })
			.addFunction("CanUseSkill",      +[](CSkillStateRecord* self) -> bool        { return self->bCanMSkill; })
			.addFunction("CanAttack",        +[](CSkillStateRecord* self) -> bool        { return self->bCanGSkill; })
			.addFunction("CanTrade",         +[](CSkillStateRecord* self) -> bool        { return self->bCanTrade; })
			.addFunction("CanUseItem",       +[](CSkillStateRecord* self) -> bool        { return self->bCanItem; })
			.addFunction("IsDizzy",          +[](CSkillStateRecord* self) -> bool        { return self->IsDizzy; })
			.addFunction("IsInvisible",      +[](CSkillStateRecord* self) -> bool        { return self->bNoHide; })
			.addFunction("GetColour",        +[](CSkillStateRecord* self) -> int         { return self->lColour; })
		.endClass();

	// --- Глобальные функции ---
	luabridge::getGlobalNamespace(L)
		.addFunction("GetItemRecord",       LuaGetItemRecord)
		.addFunction("GetChaRecord",        LuaGetChaRecord)
		.addFunction("GetSkillRecord",      LuaGetSkillRecord)
		.addFunction("GetSkillStateRecord", LuaGetSkillStateRecord)
		.addFunction("IterateItemRecords",       LuaIterateItemRecords)
		.addFunction("IterateChaRecords",        LuaIterateChaRecords)
		.addFunction("IterateSkillRecords",      LuaIterateSkillRecords)
		.addFunction("IterateSkillStateRecords", LuaIterateSkillStateRecords);

	return true;
}
