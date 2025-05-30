#include "Include.h"
#include "EngineException.h"

EngineException::EngineException(const int line, const char *file)
	: m_line{ line }
	, m_fileName{ file }
	, m_whatBuffer{ EMPTY_TEXT_A }
{
}

const char *EngineException::what() const
{
	std::ostringstream oss;
	oss << GetType() << std::endl
		<< GetOriginString();

	m_whatBuffer = oss.str();
	return m_whatBuffer.c_str();
}

const char *EngineException::GetType() const
{
	return "Engine Exception";
}

const std::string EngineException::GetOriginString() const
{
	std::ostringstream oss;
	oss << "[File] " << m_fileName << std::endl
		<< "[Line] " << m_line;
	return oss.str();
}

const int EngineException::GetLine() const
{
	return m_line;
}

const std::string& EngineException::GetFileName() const
{
	return m_fileName;
}
