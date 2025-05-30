#include "Include.h"
#include "WindowException.h"

WindowException::WindowException(const int line, const char *file, const HRESULT hr)
	: EngineException(line, file)
	, m_hResult{ hr }
{
}

const std::string WindowException::TranslateErrorCode(const HRESULT hr)
{
	char *pBuffer = nullptr;

	DWORD message = FormatMessageA(
		FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
		nullptr,
		hr,
		MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_US),
		(LPSTR)&pBuffer, 0, nullptr
	);

	if (message == 0)
	{
		return "Unidentified error code";
	}

	std::string errorString = pBuffer;
	LocalFree(pBuffer);

	return errorString;
}

const char *WindowException::what() const
{
	std::ostringstream oss;
	oss << GetType() << std::endl
		<< "[Error Code]" << GetErrorCode() << std::endl
		<< "[Description]" << GetErrorString() << std::endl
		<< GetOriginString();

	m_whatBuffer = oss.str();
	return m_whatBuffer.c_str();
}

const char *WindowException::GetType() const
{
	return "Engine Window Exception";
}

const std::string WindowException::GetErrorString() const
{
	return TranslateErrorCode(m_hResult);
}

const HRESULT WindowException::GetErrorCode() const
{
	return m_hResult;
}