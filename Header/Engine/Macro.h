#pragma once
#ifndef __MACRO_H__

// 가변인자 매크로
//#define EXPAND (x) x
//#define F(x, ...) X = x and VA_ARGS = __VA_ARGS__
//#define G(...) EXPAND( F(__VA_ARGS__) )
//
//#define GENERIC(_1, _2, x, ...) x
//#define FAILED_RETURN(...) EXPAND(GENERIC(__VA_ARGS__, FAILED_RETURN2, FAILED_RETURN1, UNUSED)(__VA_ARGS__))

#ifdef ENGINE
	#define ENGINE_DLL __declspec(dllexport)
#else
	#define ENGINE_DLL __declspec(dllimport)
#endif

#define ToFloat(x)	static_cast<float>(x)
#define ToUint32(x) static_cast<uint32>(x)

#define SAFE_ADDREF(x)		if( nullptr != x )											\
	{ x->AddRef(); }	

#define SAFE_RELEASE(x)		if( nullptr != x )											\
	{ ULONG refCount = x->Release(); if(0 == refCount) { x = nullptr; } }

#define FAILED_CHECK(_hr)	if( ((HRESULT)(_hr)) < 0 )									\
	{ MessageBox(NULL, L"Failed", L"System Error",MB_OK); return;}

#define FAILED_CHECK_RETURN(_hr, _return)	if( ((HRESULT)(_hr)) < 0 )					\
	{ MessageBox(NULL, L"Failed", L"System Error",MB_OK); return _return;}

#define FAILED_CHECK_MSG( _hr, _message)	if( ((HRESULT)(_hr)) < 0 )					\
	{ MessageBox(NULL, _message, L"System Message",MB_OK); return E_FAIL;}

#define FAILED_CHECK_RETURN_MSG( _hr, _return, _message)	if( ((HRESULT)(_hr)) < 0 )	\
	{ MessageBox(NULL, _message, L"System Message",MB_OK); return _return;}

#define FALSE_CHECK(_bool)	if( !_bool )												\
		{ return;}

#define FALSE_CHECK_RETURN(_bool, _return)	if( !_bool )								\
		{ return _return;}

#define FALSE_CHECK_MSG(_bool, _message)	if( !_bool )								\
		{ MessageBox(NULL, _message, L"System Message",MB_OK); return E_FAIL;}

#define FALSE_CHECK_RETURN_MSG(_bool, _return, _message)	if( !_bool )				\
		{ MessageBox(NULL, _message, L"System Message",MB_OK); return _return;}

#define FALSE_CHECK_ASSERT(_bool)	if( false == _bool)									\
		{ assert(false) } 

#define FALSE_CHECK_ASSERT_MSG(_bool, _msg)	if( false == _bool)							\
		{ assert(false && _msg); } 

#define TRUE_CHECK(_bool)	if( _bool )													\
		{ return;}

#define ASSERT_CONDITION_MSG(_condition, _msg)											\
		{ assert(_condition && TEXT(_msg)); }

#ifdef DEBUG																			
	#define DEV_ASSERT_MSG(_msg)														\
		{ assert(false && TEXT(_msg)); }														
#else																					
	#define DEV_ASSERT_MSG(_msg)														
#endif																							

#define __MACRO_H__
#endif