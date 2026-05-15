#pragma once

//  Windows
#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x0501
#endif
#ifndef _WIN32_WINDOWS
#define _WIN32_WINDOWS _WIN32_WINNT_WIN7
#endif

#include <winsock2.h>
#define NOMINMAX
#include <windows.h>

#include <span>
#include <iostream>
#include <tchar.h>
#include <math.h>
#include <cstdio>
#include <cstdlib>
#include <string>
#include <array>
#include <vector>
#include <charconv>
#include <string_view>
#include <cstdint>

#include "util.h"
#include "Localization/i18n.h"

