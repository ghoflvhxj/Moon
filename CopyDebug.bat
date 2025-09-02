// 명령어		복사할 파일이 있는 위치			복사된 파일을 저장할 위치

xcopy		.\Engine\Source\ThirdParty\ImGui\*.cpp	.\Client\Source\ThirdParty\ImGui\ /e /y
copy		.\Engine\x64\Debug\*.dll				.\Client\x64\Debug\Bin\
xcopy 	.\Engine\x64\Debug\Shader			.\Client\Resources\Shader\	/e /h /k /y
copy		.\Engine\x64\Debug\*.lib				.\Libraries\Debug\

xcopy		.\Engine\Source\ThirdParty\ImGui\*.cpp	.\Editor\Source\ThirdParty\ImGui\ /e /y
copy		.\Engine\x64\Debug\*.dll				.\Editor\x64\Debug\Bin\
xcopy 	.\Engine\x64\Debug\Shader			.\Editor\Resources\Shader\	/e /h /k /y
copy		.\Engine\x64\Debug\*.lib				.\Libraries\Debug\