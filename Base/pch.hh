//
// Created on Saturday 23rd April 2022 by e-erdal
//

#pragma once

#include <Windows.h>
#include <stdint.h>

#include <dxgiformat.h>
#include <d3dcommon.h>
#include <d3d11.h>

#include <EASTL/string.h>
#include <EASTL/string_view.h>
#include <EASTL/algorithm.h>
#include <EASTL/vector.h>
#include <EASTL/fixed_vector.h>
#include <EASTL/unordered_map.h>
#include <EASTL/map.h>
#include <EASTL/array.h>
#include <EASTL/atomic.h>
#include <EASTL/iterator.h>
#include <EASTL/queue.h>

#include <bx/bx.h>
#include <DirectXMath.h>
#include <DirectXPackedVector.h>

using namespace DirectX;
using namespace PackedVector;

#include "Utils/BitFlags.hh"
#include "Utils/Logger.hh"
#include "Utils/Math.hh"
#include "Utils/Timer.hh"
#include "Utils/Format.hh"

#define HRCall(func, message)                                                                                                                        \
    if (FAILED(hr = func))                                                                                                                           \
    {                                                                                                                                                \
        char *pError;                                                                                                                                \
        FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,                                   \
                      NULL,                                                                                                                          \
                      hr,                                                                                                                            \
                      MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),                                                                                     \
                      (LPTSTR)&pError,                                                                                                               \
                      0,                                                                                                                             \
                      NULL);                                                                                                                         \
        LOG_ERROR(message " HR: {}", pError);                                                                                                        \
        return;                                                                                                                                      \
    }

#define HRCallRet(func, message, ret)                                                                                                                \
    if (FAILED(hr = func))                                                                                                                           \
    {                                                                                                                                                \
        char *pError;                                                                                                                                \
        FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,                                   \
                      NULL,                                                                                                                          \
                      hr,                                                                                                                            \
                      MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),                                                                                     \
                      (LPTSTR)&pError,                                                                                                               \
                      0,                                                                                                                             \
                      NULL);                                                                                                                         \
        LOG_ERROR(message " HR: {}", pError);                                                                                                        \
        ret;                                                                                                                                         \
    }

#define SAFE_DELETE(var)                                                                                                                             \
    if (var)                                                                                                                                         \
    {                                                                                                                                                \
        delete var;                                                                                                                                  \
        var = NULL;                                                                                                                                  \
    }

#define SAFE_FREE(var)                                                                                                                               \
    if (var)                                                                                                                                         \
    {                                                                                                                                                \
        free(var);                                                                                                                                   \
        var = NULL;                                                                                                                                  \
    }

#define SAFE_RELEASE(var)                                                                                                                            \
    if (var != nullptr)                                                                                                                              \
    {                                                                                                                                                \
        var->Release();                                                                                                                              \
        var = nullptr;                                                                                                                               \
    }

#define _ZEROM(x, len) memset((void *)x, 0, len)

#define PACK_VERSION(major, minor, build) ((u32)((u8)major << 28 | ((u16)minor & 0x0fff) << 16 | _byteswap_ushort((u16)build)))
#define UNPACK_VERSION(packedVersion, major, minor, build)                                                                                           \
    {                                                                                                                                                \
        major = ((u8)((u32)packedVersion >> 24) >> 4);                                                                                               \
        minor = (u16)(((u32)packedVersion >> 16) & 0x0fff);                                                                                          \
        build = _byteswap_ushort((u16)packedVersion & 0x0000ffff);                                                                                   \
    }

typedef unsigned long long u64;
typedef signed long long i64;

typedef unsigned int u32;
typedef signed int i32;

typedef unsigned short u16;
typedef signed short i16;

typedef unsigned char u8;
typedef signed char i8;

template<typename T1, typename T2>
struct eastl::hash<eastl::pair<T1, T2>>
{
    size_t operator()(const eastl::pair<T1, T2> &s) const noexcept
    {
        size_t h1 = eastl::hash<T1>{}(s.first);
        size_t h2 = eastl::hash<T2>{}(s.second);
        return h1 ^ (h2 << 1);
    }
};