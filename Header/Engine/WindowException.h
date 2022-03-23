#pragma once
#ifndef __WINDOW_EXCEPTION_H__

#include "EngineException.h"

class WindowException : public EngineException
{
public:
	explicit WindowException(const int line, const char *file, const HRESULT hr);

public:
	static const std::string TranslateErrorCode(const HRESULT hr);
	virtual const char *what() const override;
	virtual const char *GetType() const override;

public:
	const std::string GetErrorString() const;

public:
	const HRESULT GetErrorCode() const;
private:
	HRESULT m_hResult;
};

#define WINDOW_EXCEPTION(hr) WindowException(__LINE__, __FILE__, hr);

#define __WINDOW_EXCEPTION_H__
#endif