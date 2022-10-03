#pragma once
#ifndef __INCLUDE_H__

#pragma warning(disable : 4251)

#include <iostream>
#include <sstream>
#include <memory>

#include <string>
#include <functional>
#include <exception>
#include <random>
#include <algorithm>
class __declspec(dllexport) std::exception;

#include <vector>
#include <list>
#include <map>
#include <unordered_map>

// Window Platform
#ifdef _WIN64
	#include <sdkddkver.h>
	#include <afx.h>
	#include <Windows.h>
	#include <Shlwapi.h>	// ���ڿ� ó����


	// DirectX
	#include <d3d11.h>
	#include <d3dcompiler.h>
	#include <Xinput.h>

	#define DIRECTINPUT_VERSION 0x0800
	#include <dinput.h>
#endif

#include "Type.h"
#include "Enum.h"
#include "Define.h"
#include "Extern.h"
#include "Macro.h"
#include "Function.h"

#define __INCLUDE_H__
#endif