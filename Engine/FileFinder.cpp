#include "stdafx.h"
#include "FileFinder.h"

FileFinder::FileFinder(WCHAR path[])
{
	PathCombine(path, path, TEXT("*.*"));

	reserveFileList(path);
	addFile(path);
}

FileFinder::~FileFinder()
{
}

void FileFinder::reserveFileList(WCHAR path[])
{
	uint32 count = 0;
	bool counted = false;

	CFileFind fileFind;
	BOOL bContinue = fileFind.FindFile(path);
	while (bContinue)
	{
		bContinue = fileFind.FindNextFile();

		if (fileFind.IsDots())
		{
			continue;
		}
		else if (fileFind.IsDirectory())
		{
			WCHAR subPath[MAX_PATH] = {};
			lstrcpy(subPath, fileFind.GetFilePath().GetString());

			reserveFileList(subPath);
		}
		else
		{
			if (fileFind.IsSystem())
			{
				continue;
			}

			if (false == counted)
			{
				WCHAR pathName[MAX_PATH] = {};
				lstrcpy(pathName, fileFind.GetFilePath().GetString());
				PathRemoveFileSpec(pathName);

				count += getPathFileNum(pathName);
			}
			
			bContinue = false;
		}
	}

	_fileList.reserve(count);
}

void FileFinder::addFile(WCHAR path[])
{
	CFileFind fileFind;
	BOOL bContinue = fileFind.FindFile(path);

	while (bContinue)
	{
		bContinue = fileFind.FindNextFile();

		if (fileFind.IsDots())
		{
			continue;
		}
		else if (fileFind.IsDirectory())
		{
			WCHAR subPath[MAX_PATH] = {};
			lstrcpy(subPath, fileFind.GetFilePath().GetString());
			addFile(subPath);
		}
		else
		{
			if (fileFind.IsSystem())
			{
				continue;
			}
			
			WCHAR pathAndFileName[MAX_PATH] = {};
			lstrcpy(pathAndFileName, fileFind.GetFilePath().GetString());	// C:\Users\ghofl\Desktop\ProjectMoon\Resource\Shader\VertexShader.cso

			WCHAR relativePathAndFileName[MAX_PATH] = {};
			getRelativePath(pathAndFileName, relativePathAndFileName);
			_fileList.push_back(relativePathAndFileName);
		}
	}
}

void FileFinder::getRelativePath(WCHAR toPath[], WCHAR relativePath[])
{
	TCHAR szCurPath[MAX_PATH]	= {};
	GetCurrentDirectory(MAX_PATH, szCurPath);

	PathRelativePathTo(relativePath, szCurPath, FILE_ATTRIBUTE_DIRECTORY, toPath, FILE_ATTRIBUTE_DIRECTORY);
}

const uint32 FileFinder::getPathFileNum(WCHAR path[])
{
	uint32 count = 0;

	WCHAR newPath[MAX_PATH] = {};
	lstrcpy(newPath, path);
	PathAppend(newPath, TEXT("*.*"));

	CFileFind find;
	BOOL bContinue = find.FindFile(newPath);
	while (bContinue)
	{
		bContinue = find.FindNextFile();

		if (find.IsDots() || find.IsSystem() || find.IsDirectory())
			continue;

		CString a = find.GetFileName();

		++count;
	}

	return count;
}

const FileFinder::FileList& FileFinder::getFileList() const
{
	return _fileList;
}
