# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Language

Always use english whenever possible.

## Project Overview

Tales of Pirate — MMORPG with two codebases:
- **C++23** (VS 2026 Preview, toolset v145, `/std:c++latest`, **x64-only** since 2026-04-22): client (DirectX 9 engine MindPower3D) + GameServer. Account/Gate/Group servers were migrated out of C++ and now live only in .NET.
- **Modern .NET 10** (F# + C#): replacement server infrastructure (Corsairs.*), Blazor admin panel

**C++ standard: C++23.** Prefer modern C++ constructs: `std::string`/`std::string_view` over `char*`, `std::format` over `sprintf`, `std::filesystem` over Win32 file API, structured bindings, `auto`, range-based for, `std::optional`, `std::span`, smart pointers. Avoid raw `new`/`delete` where possible.

**Именование полей:**
- **Поля класса (class)** — всегда начинаются с `_`, далее `camelCase`: `_id`, `_name`, `_currentTrack`, `_isPlaying`. Это инкапсулированное состояние с приватным/protected доступом. Никакой венгерской нотации (`nID`, `szName`, `m_nType`).
- **Открытые поля структуры (struct)** — в `PascalCase`, без `_`. struct в нашем коде — это data bag / DTO / POD, поля публичные по умолчанию и читаются как данные, а не состояние объекта. Правильно: `struct Point { int X; int Y; };`, `struct AudioInfo { AudioType Type; std::uint32_t Code; };`. Неправильно: `struct Point { int x; int y; }` или `struct Point { int _x; int _y; }`.
- Приватные поля внутри `struct` (если они есть) — так же как у класса: `_camelCase`. Но если появляются приватные поля — скорее всего это уже класс, а не структура.

**Имена методов:** методы класса/интерфейса и свободные функции — в `PascalCase`. Это распространяется и на виртуальные методы, и на перегрузки, и на static helper'ы в namespace.
- Правильно: `Init()`, `Instance()`, `GetResourceId()`, `IsValid()`, `SetVolume(float)`.
- Неправильно: `init()`, `get_instance()`, `get_resID()`, `is_valid()`, `set_volume(int)`.
- Аббревиатуры из 2+ букв — первая заглавная, остальные строчные (`GetResId`, а не `GetResID`; `ParseUrl`, а не `ParseURL`; `HttpClient`, а не `HTTPClient`). Так же, как в .NET-части `Corsairs.*`.
- Локальные переменные, параметры функций — `camelCase` (`const int maxValue = ...`); поля — `_camelCase` (см. правило выше).
- Исключения: сигнатуры, диктуемые сторонним API (WinAPI-колбэки с `WndProc`/`LRESULT CALLBACK`; STL customization-point'ы вроде `begin()`/`end()`/`size()`/`swap()` для совместимости с range-based for и `std::swap`). Только там — остаётся snake_case/CamelCase как требует API.

**Singleton:** у всех классов-одиночек в проекте стандартизированный аксессор — `static T& Instance()` (не `GetInstance()`, не `get_instance()`, не `Get()`). Реализация — Meyers' singleton через `static T instance;` внутри функции; конструктор `private`, copy/assign — `delete`. Пример — `AssetDatabase::Instance()`, `AudioSDL::Instance()`. При рефакторинге legacy-singleton'ов (`_singleton<T>::get_instance()`, `SomeClass::getInstance()`) — приводить к `Instance()`.

**Namespace'ы:** новые классы и код при рефакторинге обязательно оборачивать в namespace с корнем `Corsairs::` — согласовано с .NET-частью проекта (`Corsairs.*`). Схема: `Corsairs::<Раздел>[::<Подраздел>]`.
- `Corsairs::Client::*` — клиент (`sources/Client/`, `sources/Libraries/AudioSDL/`, и т.п.): `Corsairs::Client::Audio`, `Corsairs::Client::Net`, `Corsairs::Client::Ui`, `Corsairs::Client::Lua`.
- `Corsairs::Engine::*` — движок MindPower3D (`sources/Engine/`): `Corsairs::Engine::Render`, `Corsairs::Engine::Dx10`, `Corsairs::Engine::Animation`, `Corsairs::Engine::Font`, `Corsairs::Engine::Particle`.
- `Corsairs::Server::<Name>` — серверы (`sources/Server/<Name>Server/`): `Corsairs::Server::Game`, `Corsairs::Server::Gate`, `Corsairs::Server::Group`, `Corsairs::Server::Account`.
- `Corsairs::Common::*`, `Corsairs::Util::*` — общие библиотеки (`sources/Libraries/Common/`, `sources/Libraries/Util/`).

Пример: `sources/Libraries/AudioSDL/` → `Corsairs::Client::Audio`.

Стиль (C++23):
- Nested-namespace: `namespace Corsairs::Client::Audio { ... }`.
- Закрывающую скобку комментировать: `} // namespace Corsairs::Client::Audio`.
- В `.h` и соответствующем `.cpp` — один и тот же namespace, оба файла обёрнуты.
- **Никаких `using namespace ...` в глобальной области** `.h`-файлов. В `.cpp` — допустимо, но лучше квалификация по месту или локальный `using` внутри функции.

Исключения (не оборачивать):
- Сторонние библиотеки (`LuaJIT`, `LuaBridge`, `SDL3`, `SDL3_mixer`, `FreeType`, `fontstash`, `discord-rpc`, `stb_image`, `sqlite3`) — их заголовки и реализации остаются как есть, namespace не навязываем.
- Чистые C API (WinAPI, DirectX, ODBC, OpenSSL) — без обёртки.
- Legacy-код, не затронутый текущим рефакторингом, — не переносить массово в namespace; только при касании соседнего кода, согласованно с правилом про поля `_` и фиксированные типы.

**Защита заголовков:** в новых `.h` и при рефакторинге старых — использовать `#pragma once` в самом верху файла. C-style include guards (`#ifndef __FOO_H__ / #define __FOO_H__ / #endif`) — не применять.
- `#pragma once` поддерживается всеми актуальными компиляторами проекта (MSVC, clang, gcc), короче, и нельзя сломать опечаткой в символе guard'а или коллизией имён при копипасте.
- Исключение — сторонние заголовки (их не трогаем в принципе).

**Рефакторинг C-строк:** при рефакторинге и исправлении ошибок заменять C-функции на C++ аналоги:
- `strcpy`/`strncpy` → `std::string` присваивание или `.assign()`
- `strcmp`/`strncmp` → `==`, `!=`, `.compare()` или `std::string_view`
- `sprintf`/`snprintf` → `std::format`
- `strlen` → `.size()` / `.length()`
- `strcat` → `+=` или `std::string::append()`
- `atoi`/`atof` → `std::stoi`/`std::stof` или `std::from_chars`

**Целочисленные типы:** при рефакторинге и в новом коде — фиксированной ширины из `<cstdint>` вместо платформно-зависимых и Windows-typedef'ов.
- `int`, `long`, `short`, `unsigned int`, `unsigned long` → `std::int32_t` / `std::uint32_t` / `std::int64_t` / `std::uint64_t` / `std::int16_t` / `std::uint16_t` / `std::int8_t` / `std::uint8_t`
- `DWORD`, `ULONG`, `UINT` → `std::uint32_t`
- `LONG`, `INT` → `std::int32_t`
- `WORD`, `USHORT` → `std::uint16_t`
- `BYTE`, `UCHAR` → `std::uint8_t`
- `QWORD`, `ULONGLONG` → `std::uint64_t`
- `LONGLONG` → `std::int64_t`
- `size_t` → `std::size_t`, `ptrdiff_t` → `std::ptrdiff_t`, `intptr_t`/`uintptr_t` → `std::intptr_t`/`std::uintptr_t`
- Для handles и typed IDs предпочитать явные `std::uint32_t`/`std::uint64_t` вместо `DWORD`/`ULONGLONG` — скрытая ширина `DWORD` уже один раз привела к багу усечения указателя на x64 (см. `x64-playerptr-fix`).

Исключения (оставлять как есть):
- `bool`, `char` в `char*`/`std::string`, `wchar_t`, `float`, `double`;
- счётчики `for`-циклов без сохранения;
- **параметры и возвращаемые значения WinAPI** — передавать и принимать то, что требует API (`WPARAM`/`LPARAM`/`HRESULT`/`DWORD` в колбэках), но собственные поля класса и внутренние переменные — фиксированной ширины;
- legacy-места, не затронутые текущим рефакторингом, — не менять массово, только при касании соседнего кода.

**Перечисления:** в новом коде и при рефакторинге — **только `enum class`**, без plain `enum`. Обязательно с фиксированным underlying-типом из `<cstdint>` (`std::uint32_t` / `std::int32_t` / и т.п.).
- Правильно: `enum class TextureLoadMode : std::uint32_t { LOAD_TEXTURE_DDS = 0, LOAD_TEXTURE_USER_IMAGE = 1 };`
- Неправильно: `enum TextureLoadMode { ... };` (нет scope, неявная конверсия в int, underlying-тип неопределён).
- Plain enum допустим только в legacy-коде, который мы не трогаем (например, соседние `lwTexInfoTypeEnum`, `lwResStateEnum` и т.п. — переписывать массово не нужно). При расширении такого enum — на месте конвертировать в `enum class`, если затрагиваем большую часть колсайтов.
- Имена значений — `SCREAMING_CASE` с осмысленным префиксом (`LOAD_TEXTURE_*`, `TEX_TYPE_*`). Сам тип — `PascalCase` без `lw`/`Enum`-суффиксов: `TextureLoadMode`, не `lwTextureLoadModeEnum`.
- Колсайт обязан квалифицировать значение: `TextureLoadMode::LOAD_TEXTURE_DDS`. Никаких `using enum ...` в глобальной области заголовка.

**Обработка исключений:** `try { ... } catch (...) { ... }` — **только по явному согласованию с пользователем**. Самостоятельно оборачивать вызов в `try/catch` не нужно; пусть исключение пробрасывается и процесс падает. Подавление исключений прячет битые данные/ресурсы и приводит к тихим регрессиям. Исключение — если catch реально нужен для отката ресурса или перевода одного типа ошибки в другой, и это согласовано.

**Замер времени и таймауты:** в новом коде и при рефакторинге — `std::chrono` вместо `GetTickCount`/`timeGetTime`/`QueryPerformanceCounter` напрямую и вместо `DWORD`-таймстампов.
- Моменты времени — `std::chrono::steady_clock::time_point` (`steady_clock::now()`); для wall-clock — `system_clock`.
- Длительности — `std::chrono::milliseconds` / `std::chrono::seconds` / `std::chrono::microseconds`. Никаких `DWORD timeoutMs = 400;`.
- Сравнение/арифметика через chrono-операторы: `(now - start) > 400ms`, `start + 1s` и т.п.
- Sleep — `std::this_thread::sleep_for(...)` вместо `Sleep(N)`.
- Sentinel «не задано» — `time_point{}` (default-constructed = epoch); проверка `if (tp == steady_clock::time_point{})`.
- Исключение: callbacks WinAPI и legacy-API, требующие `DWORD` напрямую, — там оставляем как есть.

**Тайминги бизнес-логики — по времени, а не по кадрам.** Любой long-press, blink, cooldown, debounce, animation timing, ttl и т.п. должен считаться от `std::chrono`-таймстампов, не от инкремента счётчика каждый кадр. Frame-counter таймауты на 30 FPS превращаются в 2.4× более быстрые на 144 FPS — поведение ломается с переключением FPS. Если встречаешь паттерн `++counter; if (counter > N)` для timeout/blink — это **bug**, переписывай на `(now - start) > Nms`. Исключение — счётчики, которые семантически про **кадры**, а не про **время** (например, индекс анимационного кадра в спрайте), — там FPS-нормализация делается через `SteadyFrameSync::GetAnimMultiplier()`.

**Форматирование:** Однострочные `if` без фигурных скобок запрещены. Всегда использовать скобки и переносы:
```cpp
// Неправильно:
if (result.empty()) return false;

// Правильно:
if (result.empty()) {
    return false;
}
```

**Цепочки `.SetParam()`:** Каждый параметр на отдельной строке:
```cpp
// Правильно:
_db.CreateCommand("UPDATE character SET level = ? WHERE atorID = ?")
    .SetParam(1, level)
    .SetParam(2, chaId)
    .ExecuteNonQuery();
```

Main solution: `sources/TalesOfPirates.sln` — contains both C++ and .NET projects.

## Build Commands

### C++ (entire solution)

Main solution: `sources/talesofpirates.sln`. Platform is **x64-only** (no Win32 configuration). Default to Debug; build Release only when explicitly asked.

```bash
# MSBuild lives in VS 2026 Preview:
#   /c/Program\ Files/Microsoft\ Visual\ Studio/18/<edition>/MSBuild/Current/Bin/MSBuild.exe
msbuild sources/talesofpirates.sln /p:Configuration=Debug   /p:Platform=x64
msbuild sources/talesofpirates.sln /p:Configuration=Release /p:Platform=x64
```

Individual project `.vcxproj` files:
- Client: `sources/Client/Game.vcxproj`
- Engine: `sources/Engine/MindPower3D.vcxproj`
- GameServer: `sources/Server/GameServer/GameServer.vcxproj`

### .NET projects
```bash
# Build all .NET projects
dotnet build sources/Dotnet --configuration Release

# Build specific server
dotnet build sources/Dotnet/Servers/Account/Corsairs.AccountServer
dotnet build sources/Dotnet/Servers/Gate/Corsairs.GateServer
dotnet build sources/Dotnet/Servers/Group/Corsairs.GroupServer
dotnet build sources/Dotnet/Admin/Corsairs.Admin.Web
```

### Run .NET servers
```bash
dotnet run --project sources/Dotnet/Servers/Account/Corsairs.AccountServer  # TCP:9958, gRPC:15000
dotnet run --project sources/Dotnet/Servers/Gate/Corsairs.GateServer        # TCP:1973, gRPC:15001
dotnet run --project sources/Dotnet/Servers/Group/Corsairs.GroupServer      # Gate TCP:9957, Game TCP:9956, gRPC:15002
dotnet run --project sources/Dotnet/Admin/Corsairs.Admin.Web                # HTTP:5100
```

### Tests
```bash
# All .NET tests
dotnet test sources/Dotnet

# Specific test project
dotnet test sources/Dotnet/Shared/Corsairs.Platform.Network.Tests
dotnet test sources/Dotnet/Servers/Gate/Test/Corsairs.GateServer.Tests
```

Test framework: xUnit. No C++ test projects.

## Architecture

### C++ Client
- Entry: `sources/Client/src/Main.cpp` → `GameApp` class
- Game loop: `GameAppInit.cpp`, `GameAppFrameMove.cpp`, `GameAppRender.cpp`
- Engine: `sources/Engine/` (MindPower3D — DirectX 9 renderer, animation, particles, GUI)
- Network: `NetProtocol.cpp`, `PacketCmd*.cpp`, `Connection.cpp`
- State machine: `STAttack.cpp`, `STMove.cpp` (attack/movement states)
- Character system: `Character.cpp` → `CharacterModel.cpp` → `CharacterAction.cpp`
- Input: `Corsairs::Engine::Input::InputSystem` поверх `WM_KEY*` / `WM_MOUSE*` / `WM_CHAR` (DirectInput 8 снят 2026-04-24)
- Textures: `TextureLoader` + `TextureCache` (stb_image для BMP/TGA/PNG, ручной `DdsLoader` для DDS)
- Entity pooling: `GamePool` + `SlotMap<T, Capacity>` со стабильными handle'ами `(gen:12|slot:20)` / `(tag:8|serial:24)` — заменили raw-указатели после x86→x64
- PCH: все C++ проекты используют `stdafx.h` с `/FI` (ForcedIncludeFiles). **Не добавляйте `#include "stdafx.h"` вручную в `.cpp`.**

### C++ Server (GameServer — единственный C++ сервер)
- `sources/Server/GameServer/` — combat, quests, trading, guilds, NPCs (~130 source files)
- DB: ODBC, hardcoded connection string `DRIVER={ODBC Driver 17 for SQL Server};SERVER=localhost;DATABASE=gamedb;Trusted_Connection=Yes;` в `GameDB.cpp` / `TradeLogDB.cpp`
- Кодировка: MSSQL и сервер переведены на UTF-8 (миграция закрыта 2026-04-19, утилиты — `Libraries/Util/EncodingUtil.h`)
- Static data: читает общий `server/GameServer/gamedata.sqlite` через `AssetDatabase::Instance()` (см. ниже)

### .NET Server Infrastructure (Corsairs.*)
- **Platform.Network** (F#): SAEA IOCP networking, WPacket/RPacket binary protocol, AES/RSA encryption, TcpListener/Connector
- **Platform.Protocol** (F#): ICommandHandler interface, CommandRouter, PacketBuilder — command dispatch
- **Platform.Shared** (F#): Generic ServerHost, Serilog logging, endpoint config
- **Platform.Database** (C#): EF Core Code-First with multi-provider support (PostgreSQL, MySQL, MSSQL, SQLite)
- **Platform.Grpc.Contracts** (C#): Protobuf service definitions for inter-server and admin communication

### .NET Server Pattern
Each server follows the same architecture:
- `Program.fs` — entry point with `ServerHost` setup
- `Handlers/` — command handlers implementing `ICommandHandler`
- `Services/` — business logic and hosted services
- `Grpc/` — gRPC admin service implementation
- GateServer uses two system types: `DirectSystemCommand` (event-based, for GameServerSystem) and `ChannelSystemCommand` (poll-based, for ClientSystem)

### Admin.Web
- Blazor SSR with DaisyUI/Tailwind CSS
- Pages: Dashboard, Accounts, Online Players, Guilds, Chat Broadcast, Server Topology
- Communicates with servers via gRPC (ports 15000-15002)

### Attack Chain (key flow for combat debugging)
```
Client: mouse → ActAttackCha → CAttackState → SwitchState → Cmd_BeginSkill → server
Server: Cmd_BeginMove+Cmd_BeginSkill → CAction::Add → DesireMoveBegin+DesireFightBegin
Server → Client: CMD_MC_NOTIACTION (skill rep) or CMD_MC_FAILEDACTION
Client: NetActorSkillRep → state->ServerEnd(sState) / NetFailedAction → FailedAction
```

## Static Data Store

**`server/GameServer/gamedata.sqlite`** — общая БД статических данных (клиент + сервер, dev-окружение). Точка доступа: `AssetDatabase::Instance()`. Сюда постепенно переезжают все `.txt` / `.bin` из `server/GameServer/resource/` (миграция Lua→stores закрыта 2026-04-22; 53 корневых таблицы перенесены в `old.table/`). Для новых «таблиц» — создавать store (пример: `NpcRecordStore`, `ChaNameFilterStore`) с чтением через `AssetDatabase::Instance().GetDb()`. При миграции легаси-таблицы — **всегда в 2 этапа**: (1) стор + подключение к легаси-парсеру для переноса данных; (2) удаление легаси-пути после фактического импорта.

Клиентский `render.sqlite` (SQLAR + метаданные 3D/текстур) — в проработке, план в `memory/render-assets-plan.md`.

## Key Libraries (C++)

Все живут в `sources/Libraries/`:

- `common` — shared utilities (все проекты зависят; Windows SDK, PCH, базовые типы)
- `Util` — общие утилиты (`EncodingUtil.h` UTF-8, и т.п.)
- `CorsairsNet` — сетевой транспорт клиента и GameServer (заменил legacy `InfoNet`)
- `LuaJIT` (`lua51.lib`) — Lua 5.1 JIT, клиент + GameServer
- `LuaBridge` (header-only) — C++ Lua binding, auto-marshaling
- `AudioSDL` — аудио-обёртка клиента поверх **SDL3 3.4.4 + SDL3_mixer 3.2.0** (миграция 2026-04-23, track-модель API). OGG Vorbis — через встроенный stb_vorbis, `libogg` не нужен.
- `SDL3` — заголовки/импортные либы SDL3
- `FreeType` + `fontstash` — multi-page glyph-атлас для UI-шрифтов (DX9-backend; план в `memory/font-render-freetype-plan.md`)
- `sqlite3` — SQLite (для `AssetDatabase`, future `render.sqlite`)
- `Discord` — discord-rpc DLL
- `DirectX` — заголовки/либы DirectX 9. **Не включать `sources/Libraries/DirectX/include` на x64** — даёт C2733 из-за конфликта `winnt.h` со свежим Windows SDK (см. `memory/x64_migration_plan.md`)

## Important Caveats

- **Platform**: C++ сборки — **x64 only** (Win32-конфиги удалены из солюшена 2026-04-22). .NET таргетит `net10.0`.
- **RC files**: Edit tool ломает GBK-кодировку в `.rc` файлах — актуальный из C++ проектов `GameServer.rc`. Для правок ресурсников использовать `sed -i`.
- **PCH**: все три C++ проекта (`kop`, `MindPower3D`, `GameServer`) используют `stdafx.h` через `ForcedIncludeFiles` (`/FI`). **Не добавляйте `#include "stdafx.h"` в `.cpp` вручную.**
- **Client launch**: `Game.exe pKcfT0PcaX` (password argument required). Артефакты: `Client/system/Game.exe` (Debug) и `sources/Client/bin/system/Game.exe` (Release). SDL3 DLL (`SDL3.dll`, `SDL3_mixer.dll`) должны лежать рядом.
- **F# compilation order**: порядок файлов в `.fsproj` значим — новые файлы добавлять в порядке зависимостей.
- **sqlcmd из git-bash**: вызывать через `bash -c "unset SQLCMDUSER SQLCMDPASSWORD; ..."` + `cygpath -w` для `-i`; SSPI — флагом `-E`. См. `memory/feedback_mssql_sqlcmd.md`.
- **Лицензии сторонних компонентов**: при добавлении новой библиотеки/шрифта/ресурса в клиент — положить текст лицензии в `Client/licenses/` (шрифты — в `Client/licenses/fonts/`), добавить запись в `Client/licenses/README.md`. При удалении — убрать файл и строку из README. Тексты лицензий **только скачивать** (curl/cp), не генерировать.

<!-- rtk-instructions v2 -->
# RTK (Rust Token Killer) — Token-Optimized Commands

**Golden rule**: префиксуй `rtk` к командам, у которых есть фильтр. Если фильтра нет — `rtk` просто проксирует, так что вызов остаётся безопасным. Префиксовать нужно **каждое** звено в цепочках `&&`:

```bash
# ❌
git add . && git commit -m "msg" && git push
# ✅
rtk git add . && rtk git commit -m "msg" && rtk git push
```

Стек этого проекта — C++ (MSBuild) + .NET (F#/C#) + SQLite + Lua. Значимые RTK-фильтры для него:

### Git (59–80% savings)
```bash
rtk git status | log | diff | show | add | commit | push | pull | branch | fetch | stash | worktree
```
Passthrough работает для **всех** git-subcommand'ов, даже не перечисленных.

### GitHub (26–87% savings)
```bash
rtk gh pr view <num>    # Compact PR view
rtk gh pr checks
rtk gh run list
rtk gh issue list
rtk gh api
```

### Files & Search (60–75% savings)
```bash
rtk ls <path>      # Tree format, compact
rtk read <file>
rtk grep <pattern>
rtk find <pattern>
```

### Analysis & Debug (70–90% savings)
```bash
rtk err <cmd>          # Только ошибки из stdout/stderr
rtk log <file>         # Дедуп логов с счётчиками
rtk json <file>        # Структура JSON без значений
rtk summary <cmd>      # Умный summary вывода
rtk diff               # Ultra-compact diff
```

### Network (65–70% savings)
```bash
rtk curl <url>
rtk wget <url>
```

### Meta
```bash
rtk gain [--history]   # Статистика экономии токенов
rtk discover           # Поиск упущенных мест применения RTK в сессии
rtk proxy <cmd>        # Прогнать без фильтра (для дебага)
```

**Неприменимо к этому проекту** (и поэтому не используется): `rtk cargo`, `rtk tsc`, `rtk lint`, `rtk prettier`, `rtk next`, `rtk vitest`, `rtk playwright`, `rtk pnpm`, `rtk npm`, `rtk npx`, `rtk prisma`, `rtk docker`, `rtk kubectl`. Для сборки здесь — MSBuild (C++) и `dotnet build`/`dotnet test` (F#/C#); у них RTK-фильтров нет, пишем команды напрямую.
<!-- /rtk-instructions -->

---

# English Translation

> The sections below are an English translation of the Russian-language guidance above. Code identifiers, file paths, type names, and other technical terms are kept in their original form. In case of any discrepancy, the Russian text above is authoritative.

## Language

All documentation, comments, and communication — in Russian. Technical terms and code identifiers stay in original form.

## Project Overview

Tales of Pirate — MMORPG with two codebases:
- **C++23** (VS 2026 Preview, toolset v145, `/std:c++latest`, **x64-only** since 2026-04-22): client (DirectX 9 engine MindPower3D) + GameServer. Account/Gate/Group servers were migrated out of C++ and now live only in .NET.
- **Modern .NET 10** (F# + C#): replacement server infrastructure (Corsairs.*), Blazor admin panel

**C++ standard: C++23.** Prefer modern C++ constructs: `std::string`/`std::string_view` over `char*`, `std::format` over `sprintf`, `std::filesystem` over Win32 file API, structured bindings, `auto`, range-based for, `std::optional`, `std::span`, smart pointers. Avoid raw `new`/`delete` where possible.

**Field naming:**
- **Class (`class`) fields** — always start with `_`, then `camelCase`: `_id`, `_name`, `_currentTrack`, `_isPlaying`. This is encapsulated state with private/protected access. No Hungarian notation (`nID`, `szName`, `m_nType`).
- **Public struct (`struct`) fields** — `PascalCase`, without `_`. A struct in our code is a data bag / DTO / POD; fields are public by default and read as data, not as object state. Correct: `struct Point { int X; int Y; };`, `struct AudioInfo { AudioType Type; std::uint32_t Code; };`. Incorrect: `struct Point { int x; int y; }` or `struct Point { int _x; int _y; }`.
- Private fields inside a `struct` (if any) — same as a class: `_camelCase`. But if private fields appear — it's most likely a class, not a struct.

**Method names:** class/interface methods and free functions — `PascalCase`. This applies to virtual methods, overloads, and static helpers in a namespace.
- Correct: `Init()`, `Instance()`, `GetResourceId()`, `IsValid()`, `SetVolume(float)`.
- Incorrect: `init()`, `get_instance()`, `get_resID()`, `is_valid()`, `set_volume(int)`.
- Abbreviations of 2+ letters — first letter uppercase, the rest lowercase (`GetResId`, not `GetResID`; `ParseUrl`, not `ParseURL`; `HttpClient`, not `HTTPClient`). Same as in the .NET part `Corsairs.*`.
- Local variables and function parameters — `camelCase` (`const int maxValue = ...`); fields — `_camelCase` (see rule above).
- Exceptions: signatures dictated by third-party APIs (WinAPI callbacks with `WndProc`/`LRESULT CALLBACK`; STL customization points like `begin()`/`end()`/`size()`/`swap()` for compatibility with range-based for and `std::swap`). Only there — snake_case/CamelCase stays as the API requires.

**Singleton:** all singleton classes in the project use a standardized accessor — `static T& Instance()` (not `GetInstance()`, not `get_instance()`, not `Get()`). Implementation — Meyers' singleton via `static T instance;` inside the function; constructor `private`, copy/assign — `delete`. Example — `AssetDatabase::Instance()`, `AudioSDL::Instance()`. When refactoring legacy singletons (`_singleton<T>::get_instance()`, `SomeClass::getInstance()`) — bring them to `Instance()`.

**Namespaces:** new classes and refactored code must be wrapped in a namespace rooted at `Corsairs::` — aligned with the project's .NET part (`Corsairs.*`). Scheme: `Corsairs::<Section>[::<Subsection>]`.
- `Corsairs::Client::*` — client (`sources/Client/`, `sources/Libraries/AudioSDL/`, etc.): `Corsairs::Client::Audio`, `Corsairs::Client::Net`, `Corsairs::Client::Ui`, `Corsairs::Client::Lua`.
- `Corsairs::Engine::*` — MindPower3D engine (`sources/Engine/`): `Corsairs::Engine::Render`, `Corsairs::Engine::Dx10`, `Corsairs::Engine::Animation`, `Corsairs::Engine::Font`, `Corsairs::Engine::Particle`.
- `Corsairs::Server::<Name>` — servers (`sources/Server/<Name>Server/`): `Corsairs::Server::Game`, `Corsairs::Server::Gate`, `Corsairs::Server::Group`, `Corsairs::Server::Account`.
- `Corsairs::Common::*`, `Corsairs::Util::*` — shared libraries (`sources/Libraries/Common/`, `sources/Libraries/Util/`).

Example: `sources/Libraries/AudioSDL/` → `Corsairs::Client::Audio`.

Style (C++23):
- Nested-namespace: `namespace Corsairs::Client::Audio { ... }`.
- Comment the closing brace: `} // namespace Corsairs::Client::Audio`.
- In a `.h` and its corresponding `.cpp` — the same namespace, both files wrapped.
- **No `using namespace ...` at global scope** in `.h` files. In `.cpp` — allowed, but prefer qualification at the call site or a local `using` inside a function.

Exceptions (do not wrap):
- Third-party libraries (`LuaJIT`, `LuaBridge`, `SDL3`, `SDL3_mixer`, `FreeType`, `fontstash`, `discord-rpc`, `stb_image`, `sqlite3`) — their headers and implementations stay as-is; we do not impose a namespace.
- Pure C APIs (WinAPI, DirectX, ODBC, OpenSSL) — without a wrapper.
- Legacy code not touched by the current refactoring — do not bulk-move into a namespace; only when touching adjacent code, consistent with the rules about `_` fields and fixed-width types.

**Header guards:** in new `.h` files and when refactoring old ones — use `#pragma once` at the very top of the file. C-style include guards (`#ifndef __FOO_H__ / #define __FOO_H__ / #endif`) — do not use.
- `#pragma once` is supported by all of the project's current compilers (MSVC, clang, gcc), is shorter, and cannot be broken by a typo in the guard symbol or a name collision from copy-paste.
- Exception — third-party headers (we do not touch them at all).

**Refactoring C strings:** when refactoring and fixing bugs, replace C functions with their C++ equivalents:
- `strcpy`/`strncpy` → `std::string` assignment or `.assign()`
- `strcmp`/`strncmp` → `==`, `!=`, `.compare()` or `std::string_view`
- `sprintf`/`snprintf` → `std::format`
- `strlen` → `.size()` / `.length()`
- `strcat` → `+=` or `std::string::append()`
- `atoi`/`atof` → `std::stoi`/`std::stof` or `std::from_chars`

**Integer types:** when refactoring and in new code — fixed-width from `<cstdint>` instead of platform-dependent and Windows typedefs.
- `int`, `long`, `short`, `unsigned int`, `unsigned long` → `std::int32_t` / `std::uint32_t` / `std::int64_t` / `std::uint64_t` / `std::int16_t` / `std::uint16_t` / `std::int8_t` / `std::uint8_t`
- `DWORD`, `ULONG`, `UINT` → `std::uint32_t`
- `LONG`, `INT` → `std::int32_t`
- `WORD`, `USHORT` → `std::uint16_t`
- `BYTE`, `UCHAR` → `std::uint8_t`
- `QWORD`, `ULONGLONG` → `std::uint64_t`
- `LONGLONG` → `std::int64_t`
- `size_t` → `std::size_t`, `ptrdiff_t` → `std::ptrdiff_t`, `intptr_t`/`uintptr_t` → `std::intptr_t`/`std::uintptr_t`
- For handles and typed IDs prefer explicit `std::uint32_t`/`std::uint64_t` instead of `DWORD`/`ULONGLONG` — the hidden width of `DWORD` already caused a pointer-truncation bug on x64 once (see `x64-playerptr-fix`).

Exceptions (leave as-is):
- `bool`, `char` in `char*`/`std::string`, `wchar_t`, `float`, `double`;
- `for`-loop counters that aren't stored;
- **WinAPI parameters and return values** — pass and accept what the API requires (`WPARAM`/`LPARAM`/`HRESULT`/`DWORD` in callbacks), but your own class fields and internal variables — fixed-width;
- legacy spots not touched by the current refactoring — do not change in bulk, only when touching adjacent code.

**Enumerations:** in new code and when refactoring — **only `enum class`**, no plain `enum`. Always with a fixed underlying type from `<cstdint>` (`std::uint32_t` / `std::int32_t` / etc.).
- Correct: `enum class TextureLoadMode : std::uint32_t { LOAD_TEXTURE_DDS = 0, LOAD_TEXTURE_USER_IMAGE = 1 };`
- Incorrect: `enum TextureLoadMode { ... };` (no scope, implicit conversion to int, underlying type undefined).
- Plain enum is allowed only in legacy code we don't touch (e.g. neighboring `lwTexInfoTypeEnum`, `lwResStateEnum`, etc. — no need for a bulk rewrite). When extending such an enum — convert it to `enum class` in place if you're touching most of the call sites.
- Value names — `SCREAMING_CASE` with a meaningful prefix (`LOAD_TEXTURE_*`, `TEX_TYPE_*`). The type itself — `PascalCase` without `lw`/`Enum` suffixes: `TextureLoadMode`, not `lwTextureLoadModeEnum`.
- The call site must qualify the value: `TextureLoadMode::LOAD_TEXTURE_DDS`. No `using enum ...` at global header scope.

**Exception handling:** `try { ... } catch (...) { ... }` — **only with the user's explicit agreement**. Don't wrap a call in `try/catch` on your own; let the exception propagate and the process crash. Suppressing exceptions hides broken data/resources and leads to silent regressions. Exception — if a catch is genuinely needed to roll back a resource or translate one error type into another, and it's been agreed.

**Time measurement and timeouts:** in new code and when refactoring — `std::chrono` instead of `GetTickCount`/`timeGetTime`/`QueryPerformanceCounter` directly and instead of `DWORD` timestamps.
- Points in time — `std::chrono::steady_clock::time_point` (`steady_clock::now()`); for wall-clock — `system_clock`.
- Durations — `std::chrono::milliseconds` / `std::chrono::seconds` / `std::chrono::microseconds`. No `DWORD timeoutMs = 400;`.
- Comparison/arithmetic via chrono operators: `(now - start) > 400ms`, `start + 1s`, etc.
- Sleep — `std::this_thread::sleep_for(...)` instead of `Sleep(N)`.
- "Not set" sentinel — `time_point{}` (default-constructed = epoch); check `if (tp == steady_clock::time_point{})`.
- Exception: WinAPI callbacks and legacy APIs that require `DWORD` directly — leave those as-is.

**Business-logic timing — by time, not by frames.** Any long-press, blink, cooldown, debounce, animation timing, ttl, etc. must be measured from `std::chrono` timestamps, not from a counter incremented every frame. Frame-counter timeouts tuned at 30 FPS become 2.4× faster at 144 FPS — behavior breaks when the FPS changes. If you see the pattern `++counter; if (counter > N)` for a timeout/blink — that's a **bug**, rewrite it as `(now - start) > Nms`. Exception — counters that are semantically about **frames**, not **time** (e.g. the animation-frame index in a sprite) — there, FPS normalization is done via `SteadyFrameSync::GetAnimMultiplier()`.

**Formatting:** Single-line `if` without braces is forbidden. Always use braces and line breaks:
```cpp
// Incorrect:
if (result.empty()) return false;

// Correct:
if (result.empty()) {
    return false;
}
```

**`.SetParam()` chains:** Each parameter on its own line:
```cpp
// Correct:
_db.CreateCommand("UPDATE character SET level = ? WHERE atorID = ?")
    .SetParam(1, level)
    .SetParam(2, chaId)
    .ExecuteNonQuery();
```

Main solution: `sources/TalesOfPirates.sln` — contains both C++ and .NET projects.

## Build Commands

(See the build commands above — these are already in English and are not repeated here.)

## Architecture

### C++ Client
- Entry: `sources/Client/src/Main.cpp` → `GameApp` class
- Game loop: `GameAppInit.cpp`, `GameAppFrameMove.cpp`, `GameAppRender.cpp`
- Engine: `sources/Engine/` (MindPower3D — DirectX 9 renderer, animation, particles, GUI)
- Network: `NetProtocol.cpp`, `PacketCmd*.cpp`, `Connection.cpp`
- State machine: `STAttack.cpp`, `STMove.cpp` (attack/movement states)
- Character system: `Character.cpp` → `CharacterModel.cpp` → `CharacterAction.cpp`
- Input: `Corsairs::Engine::Input::InputSystem` on top of `WM_KEY*` / `WM_MOUSE*` / `WM_CHAR` (DirectInput 8 removed 2026-04-24)
- Textures: `TextureLoader` + `TextureCache` (stb_image for BMP/TGA/PNG, manual `DdsLoader` for DDS)
- Entity pooling: `GamePool` + `SlotMap<T, Capacity>` with stable handles `(gen:12|slot:20)` / `(tag:8|serial:24)` — replaced raw pointers after the x86→x64 migration
- PCH: all C++ projects use `stdafx.h` with `/FI` (ForcedIncludeFiles). **Do not manually add `#include "stdafx.h"` to `.cpp` files.**

### C++ Server (GameServer — the only C++ server)
- `sources/Server/GameServer/` — combat, quests, trading, guilds, NPCs (~130 source files)
- DB: ODBC, hardcoded connection string `DRIVER={ODBC Driver 17 for SQL Server};SERVER=localhost;DATABASE=gamedb;Trusted_Connection=Yes;` in `GameDB.cpp` / `TradeLogDB.cpp`
- Encoding: MSSQL and the server were converted to UTF-8 (migration closed 2026-04-19, utilities — `Libraries/Util/EncodingUtil.h`)
- Static data: reads the shared `server/GameServer/gamedata.sqlite` via `AssetDatabase::Instance()` (see below)

### .NET Server Infrastructure (Corsairs.*)
(Already in English above — Platform.Network, Platform.Protocol, Platform.Shared, Platform.Database, Platform.Grpc.Contracts.)

### .NET Server Pattern / Admin.Web / Attack Chain
(Already in English above.)

## Static Data Store

**`server/GameServer/gamedata.sqlite`** — shared static-data DB (client + server, dev environment). Access point: `AssetDatabase::Instance()`. All `.txt` / `.bin` files from `server/GameServer/resource/` are gradually moving here (the Lua→stores migration closed 2026-04-22; 53 root tables were moved to `old.table/`). For new "tables" — create a store (example: `NpcRecordStore`, `ChaNameFilterStore`) that reads via `AssetDatabase::Instance().GetDb()`. When migrating a legacy table — **always in 2 stages**: (1) the store + hookup to the legacy parser to transfer the data; (2) removal of the legacy path after the data has actually been imported.

The client-side `render.sqlite` (SQLAR + 3D/texture metadata) — in progress, plan in `memory/render-assets-plan.md`.

## Key Libraries (C++)

All live in `sources/Libraries/`:

- `common` — shared utilities (all projects depend on it; Windows SDK, PCH, base types)
- `Util` — shared utilities (`EncodingUtil.h` UTF-8, etc.)
- `CorsairsNet` — network transport for the client and GameServer (replaced legacy `InfoNet`)
- `LuaJIT` (`lua51.lib`) — Lua 5.1 JIT, client + GameServer
- `LuaBridge` (header-only) — C++ Lua binding, auto-marshaling
- `AudioSDL` — client audio wrapper on top of **SDL3 3.4.4 + SDL3_mixer 3.2.0** (migration 2026-04-23, track-model API). OGG Vorbis — via the built-in stb_vorbis, `libogg` is not needed.
- `SDL3` — SDL3 headers/import libs
- `FreeType` + `fontstash` — multi-page glyph atlas for UI fonts (DX9 backend; plan in `memory/font-render-freetype-plan.md`)
- `sqlite3` — SQLite (for `AssetDatabase`, future `render.sqlite`)
- `Discord` — discord-rpc DLL
- `DirectX` — DirectX 9 headers/libs. **Do not include `sources/Libraries/DirectX/include` on x64** — it produces C2733 due to a `winnt.h` conflict with the recent Windows SDK (see `memory/x64_migration_plan.md`)

## Important Caveats

- **Platform**: C++ builds — **x64 only** (Win32 configs removed from the solution 2026-04-22). .NET targets `net10.0`.
- **RC files**: the Edit tool breaks the GBK encoding in `.rc` files — the live one among the C++ projects is `GameServer.rc`. To edit resource files, use `sed -i`.
- **PCH**: all three C++ projects (`kop`, `MindPower3D`, `GameServer`) use `stdafx.h` via `ForcedIncludeFiles` (`/FI`). **Do not manually add `#include "stdafx.h"` to `.cpp` files.**
- **Client launch**: `Game.exe pKcfT0PcaX` (password argument required). Artifacts: `Client/system/Game.exe` (Debug) and `sources/Client/bin/system/Game.exe` (Release). The SDL3 DLLs (`SDL3.dll`, `SDL3_mixer.dll`) must sit next to it.
- **F# compilation order**: file order in the `.fsproj` matters — add new files in dependency order.
- **sqlcmd from git-bash**: invoke via `bash -c "unset SQLCMDUSER SQLCMDPASSWORD; ..."` + `cygpath -w` for `-i`; SSPI — with the `-E` flag. See `memory/feedback_mssql_sqlcmd.md`.
- **Third-party component licenses**: when adding a new library/font/resource to the client — place the license text in `Client/licenses/` (fonts — in `Client/licenses/fonts/`), add an entry to `Client/licenses/README.md`. When removing — delete the file and the README line. License texts must **only be downloaded** (curl/cp), never generated.

## RTK (Rust Token Killer) — Token-Optimized Commands

**Golden rule**: prefix `rtk` to commands that have a filter. If there's no filter — `rtk` just proxies, so the call stays safe. You must prefix **every** link in `&&` chains:

```bash
# ❌
git add . && git commit -m "msg" && git push
# ✅
rtk git add . && rtk git commit -m "msg" && rtk git push
```

This project's stack — C++ (MSBuild) + .NET (F#/C#) + SQLite + Lua. The meaningful RTK filters for it:

### Git (59–80% savings)
```bash
rtk git status | log | diff | show | add | commit | push | pull | branch | fetch | stash | worktree
```
Passthrough works for **all** git subcommands, even ones not listed.

### GitHub (26–87% savings)
```bash
rtk gh pr view <num>    # Compact PR view
rtk gh pr checks
rtk gh run list
rtk gh issue list
rtk gh api
```

### Files & Search (60–75% savings)
```bash
rtk ls <path>      # Tree format, compact
rtk read <file>
rtk grep <pattern>
rtk find <pattern>
```

### Analysis & Debug (70–90% savings)
```bash
rtk err <cmd>          # Only errors from stdout/stderr
rtk log <file>         # Log dedup with counts
rtk json <file>        # JSON structure without values
rtk summary <cmd>      # Smart summary of output
rtk diff               # Ultra-compact diff
```

### Network (65–70% savings)
```bash
rtk curl <url>
rtk wget <url>
```

### Meta
```bash
rtk gain [--history]   # Token-savings stats
rtk discover           # Find missed RTK opportunities in the session
rtk proxy <cmd>        # Run without a filter (for debugging)
```

**Not applicable to this project** (and therefore unused): `rtk cargo`, `rtk tsc`, `rtk lint`, `rtk prettier`, `rtk next`, `rtk vitest`, `rtk playwright`, `rtk pnpm`, `rtk npm`, `rtk npx`, `rtk prisma`, `rtk docker`, `rtk kubectl`. For building here — MSBuild (C++) and `dotnet build`/`dotnet test` (F#/C#); they have no RTK filters, so write those commands directly.