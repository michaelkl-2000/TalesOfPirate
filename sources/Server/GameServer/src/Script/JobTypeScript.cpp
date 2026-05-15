// JobTypeScript.cpp
//---------------------------------------------------------
// См. JobTypeScript.h. Использует X-macro JOB_TYPE_LIST из JobType.h,
// чтобы сгенерировать таблицу { имя -> JobType } один в один с C++ enum.
//---------------------------------------------------------
#include "Core/stdafx.h"
#include "Script/JobTypeScript.h"
#include "Script/Script.h"
#include "Core/JobType.h"

#include <string>

namespace {

struct JobEntry {
    const char* Name;
    Corsairs::Common::Character::JobType Value;
};

// Таблица генерируется из того же X-macro, что и сам enum class —
// рассинхронизация между enum и Lua физически невозможна.
constexpr JobEntry kJobEntries[] = {
#define X(name, value, comment) { #name, Corsairs::Common::Character::JobType::name },
    JOB_TYPE_LIST(X)
#undef X
};

} // namespace

bool RegisterJobTypeScript() {
    lua_State* L = g_pLuaState;

    // 1) Новый стиль: JobType.XINSHOU / JobType.JUJS / ... через LuaBridge.
    //    addVariable знает про std::is_enum_v и сам приводит к underlying type.
    {
        auto ns = luabridge::getGlobalNamespace(L).beginNamespace("JobType");
        for (const auto& entry : kJobEntries) {
            ns.addVariable(entry.Name, entry.Value);
        }
        ns.endNamespace();
    }

    // 2) Legacy-глобалы JOB_TYPE_* для уже написанных скриптов
    //    (AttrCalculate.lua, functions.lua, skilleffect.lua и т.п.).
    //    Прямой lua_setglobal — addVariable нельзя звать на global namespace.
    for (const auto& entry : kJobEntries) {
        std::string globalName = "JOB_TYPE_";
        globalName += entry.Name;
        lua_pushinteger(L, static_cast<lua_Integer>(entry.Value));
        lua_setglobal(L, globalName.c_str());
    }

    return true;
}
