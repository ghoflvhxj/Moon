#pragma once    
#pragma warning(disable : 4251)

#include <iostream>
#include <sstream>
#include <memory>

#include <string>
#include <functional>
#include <exception>
#include <random>
#include <algorithm>
#include <filesystem>
class __declspec(dllexport) std::exception;

#include <vector>
#include <list>
#include <queue>
#include <map>
#include <unordered_map>
#include <set>
#include <unordered_set>
#include <array>

// Window Platform
#ifdef _WIN64
	#include <sdkddkver.h>
	#include <afx.h>        //mfc제거하기
	#include <Windows.h>
	#include <Shlwapi.h>	// 문자열 처리용


	// DirectX
	#include <d3d11.h>
	#include <d3dcompiler.h>
    #include <DirectXCollision.h>
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