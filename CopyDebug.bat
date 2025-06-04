// 명령어		복사할 파일이 있는 위치			복사된 파일을 저장할 위치

//copy		.\Engine\*.h					.\Header\Engine\

//copy		.\Engine\DirectXTK\*.h			.\Header\Engine\

xcopy		.\Engine\Source\ThirdParty\ImGui\*.cpp	.\Client\Source\ThirdParty\ImGui\ /e /y
copy		.\Engine\x64\Debug\*.dll				.\Client\x64\Debug\Bin\
copy		.\Engine\x64\Debug\*.lib				.\Libraries\Debug\
xcopy 	.\Engine\x64\Debug\Shader			.\Client\Resources\Shader\	/e /h /k /y

