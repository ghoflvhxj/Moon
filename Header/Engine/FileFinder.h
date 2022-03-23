#pragma once
class FileFinder
{
	using FileList = std::vector<std::wstring>;

public:
	explicit FileFinder(WCHAR path[]);
	~FileFinder();

private:
	void reserveFileList(WCHAR path[]);
	void addFile(WCHAR path[]);
	void getRelativePath(WCHAR toPath[], WCHAR relativePath[]);

public:
	const uint32 getPathFileNum(WCHAR path[]);

public:
	const FileList& getFileList() const;
private:
	FileList _fileList;
};

