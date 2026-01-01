// 임시주석

// 나중에 dll도 Library로 옮긴다면, 엔진 dll을 Library로 옮기고, 이걸 통채로 실행파일에 있는곳에 복사하면 됨
//copy	.\Engine\x64\Release\*.dll		.\Libraries\Release\
//

copy		.\Engine\x64\Release\*.dll		.\Editor\x64\Release\Bin\
copy		.\Engine\x64\Release\*.lib	        .\Libraries\Release\
xcopy 	.\Engine\x64\Release\Shader	.\Editor\Resources\Shader\	/e /h /k /y