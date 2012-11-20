#pragma once

#ifndef STATIC_BUILD
#ifdef XREX_SOURCE
#define XREX_API __declspec(dllexport)
#else
#define XREX_API __declspec(dllimport)
#endif
#else
#define XREX_API
#endif

#pragma warning(disable: 4251) // have member do not dllexport-ed

#if defined(DEBUG) || defined(_DEBUG)
#define XREX_DEBUG
#endif

#define USE_OPENGL_COMPATIBILITY_PROFILE

#include "Declare.hpp"

#include <string>
#include <vector>
#include <array>
#include <deque>
#include <queue>
#include <functional>
#include <algorithm>
#include <unordered_map>
#include <unordered_set>

