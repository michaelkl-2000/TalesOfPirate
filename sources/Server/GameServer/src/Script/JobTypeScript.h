// JobTypeScript.h
//---------------------------------------------------------
// Регистрация enum class Corsairs::Common::Character::JobType в Lua.
// Создаёт:
//   - namespace JobType.*  (JobType.XINSHOU, JobType.JUJS, ...)  — новый стиль
//   - legacy-глобалы JOB_TYPE_*  (JOB_TYPE_XINSHOU, JOB_TYPE_JUJS, ...) —
//     для совместимости с уже написанными Lua-скриптами (AttrCalculate.lua и т.п.).
// Заменяет ручной server/GameServer/resource/script/calculate/JobType.lua —
// C++ enum (Libraries/common/src/Core/JobType.h, макрос JOB_TYPE_LIST) теперь
// единственный источник истины.
//---------------------------------------------------------
#pragma once

extern bool RegisterJobTypeScript();
