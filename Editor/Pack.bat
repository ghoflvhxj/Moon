@echo off
setlocal enabledelayedexpansion

REM === 대상 폴더 경로 설정 ===
if "%~1"=="" (
    echo Usage: copy.bat [TARGET_FOLDER]
    pause
    exit /b 1
)

set TARGET_DIR=%~1

REM === 대상 폴더 없으면 생성 ===
if not exist "%TARGET_DIR%" (
    mkdir "%TARGET_DIR%"
)

REM === 폴더 복사 (x64, source 제외) ===
for /d %%D in (*) do (
    if /i not "%%D"=="x64" if /i not "%%D"=="Source" (
        echo Copying folder: %%D
        xcopy "%%D" "%TARGET_DIR%\%%D" /E /I /Y
    )
)

REM === json 파일 복사 ===
for %%F in (*.json) do (
    echo Copying json file: %%F
    copy /Y "%%F" "%TARGET_DIR%"
)

echo.
echo Copy completed.
pause