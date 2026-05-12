// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently

#pragma once

//  Windows 
#include <tchar.h>

//  C standard 
#include <cstdio>
#include <cstdlib>
#include <cmath>
#include <ctime>
#include <cassert>
#include <cstring>

//  C++ standard 
#include <algorithm>
#include <array>
#include <atomic>
#include <charconv>
#include <deque>
#include <format>
#include <fstream>
#include <functional>
#include <iostream>
#include <map>
#include <memory>
#include <mutex>
#include <print>
#include <span>
#include <sstream>
#include <string>
#include <string_view>
#include <thread>
#include <unordered_map>
#include <vector>

//    () 
#include "DBCCommon.h"
#include "excp.h"
#include "util.h"
#include "Core/i18n.h"
#include "point.h"
#include "Core/Timer.h"
#include "Network/CompCommand.h"
#include "Item/ItemContent.h"
#include "Core/CommFunc.h"
#include "Core/GameCommon.h"
#include "CommandMessages.h"
#include "logutil.h"

namespace Corsairs::Common::World {

}

// Bring Corsairs::Common::Localization (GetLanguageString, RES_STRING, etc.) into global scope
using namespace Corsairs::Common::Localization;
using namespace Corsairs::Common::Skill;
using namespace Corsairs::Common::Network;
using namespace Corsairs::Common::Inventory;
using namespace Corsairs::Common::Character;
using namespace Corsairs::Common::Item;
using namespace Corsairs::Common::Database;
using namespace Corsairs::Common::World;
