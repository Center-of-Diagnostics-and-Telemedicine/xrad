@rem msvcp140.dll, vcruntime140.dll, vcomp140.dll, api-ms-*, ucrtbase*
@echo ---- %~n0 ----
@echo OutDir=%~1
@echo PlatformTarget=%~2
@echo Debug=%~3

@if "%~1"=="" @goto ErrParams
@if "%~2"=="" @goto ErrParams
@if "%~3"=="" @goto ErrParams
@if "%XRADThirdParty%"=="" @goto ErrParams

@goto OkParams
:ErrParams
@echo Error: Invalid parameters for %~nx0.
@exit 1
:OkParams

@setlocal

@set OutDir=%~1
@set PlatformTarget=%~2
@set ConfigDebug=%~3
@set VCRuntimeDir=%XRADThirdParty%\vcruntime-14-1
@echo VCRuntimeDir="%VCRuntimeDir%"

@if not exist "%VCRuntimeDir%" (
@echo Error: VCRuntimeDir="%VCRuntimeDir%" doesn't exist.
@exit 1
)

@if not exist "%OutDir%" (
@echo Error: OutDir="%OutDir%" doesn't exist.
@exit 1
)

@if not "%ConfigDebug%"=="Debug" @call :CopyVC
@if "%ConfigDebug%"=="Debug" @call :CopyVCD
@call :CopyUCRT
@if "%ConfigDebug%"=="Debug" @call :CopyUCRTD

@endlocal
@echo ---- %~n0 end ----
@goto :EOF

:CopyVC
@for /F "usebackq tokens=*" %%i in ("%~dpn0-vc.txt") do @call :DoCopy "%%~i" "%VCRuntimeDir%\%PlatformTarget%\vc" "%OutDir%"
@goto :EOF

:CopyVCD
@for /F "usebackq tokens=*" %%i in ("%~dpn0-vc-d.txt") do @call :DoCopy "%%~i" "%VCRuntimeDir%\%PlatformTarget%\vc-d" "%OutDir%"
@goto :EOF

:CopyUCRT
@for /F "usebackq tokens=*" %%i in ("%~dpn0-ucrt.txt") do @call :DoCopy "%%~i" "%VCRuntimeDir%\%PlatformTarget%\ucrt" "%OutDir%"
@goto :EOF

:CopyUCRTD
@for /F "usebackq tokens=*" %%i in ("%~dpn0-ucrt-d.txt") do @call :DoCopy "%%~i" "%VCRuntimeDir%\%PlatformTarget%\ucrt-d" "%OutDir%"
@goto :EOF

:DoCopy
@rem Skip empty lines and UTF-8 BOM
@if "%~1"=="" goto :EOF
@if "%~1"=="﻿" goto :EOF
@echo xcopy /y /d "%~2\%~1" "%~3"
@xcopy /y /d "%~2\%~1" "%~3"
@if errorlevel 1 @(
@echo Error: Failed to copy "%~2\%~1" -^> "%~3".
@exit 1
)
@goto :EOF
