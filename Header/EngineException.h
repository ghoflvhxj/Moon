#pragma once
#ifndef __ENGINE_EXCEPTION_H__

class ENGINE_DLL EngineException : public std::exception
{
public:
	explicit EngineException(const int line, const char *file);
	virtual ~EngineException() = default;

public:
	virtual const char* what() const override;
	virtual const char* GetType() const;
	const std::string GetOriginString() const;

public:
	const int GetLine() const;
private:
	int m_line;

public:
	const std::string& GetFileName() const;
private:
	std::string m_fileName;

protected:
	mutable std::string m_whatBuffer;
}; 

#define __ENGINE_EXCEPTION_H__
#endif