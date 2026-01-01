start Pack.bat ".\x64\Release\Bin"

set BANDIZIP_PATH="C:\ProgramFiles\Bandizip\Bandizip.exe"
set SRC=.\x64\Release\Bin
set OUT=.\x64\Release\Bin.zip

start /wait Bandizip.exe c "%OUT%" "%SRC%"

copy .\x64\Release\Bin.zip D:\공유폴더\

pause