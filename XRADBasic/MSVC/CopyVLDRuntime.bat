@rem Copy VLD binaries for Debug
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

@if not "%~3"=="Debug" (
@echo VLD is required for Debug configuration only. Skipped.
@goto lEnd
)

@setlocal

@set OutDir=%~1
@set PlatformTarget=%~2
@set ConfigDebug=%~3
@set VLDRuntimeDir=%XRADThirdParty%\vld\bin
@echo VLDRuntimeDir="%VLDRuntimeDir%"

@if not exist "%VLDRuntimeDir%" (
@echo Error: VLDRuntimeDir="%VLDRuntimeDir%" doesn't exist.
@exit 1
)

@if not exist "%OutDir%" (
@echo Error: OutDir="%OutDir%" doesn't exist.
@exit 1
)

@set PlatformDir=
@if "%PlatformTarget%"=="x86" set PlatformDir=x86
@if "%PlatformTarget%"=="x64" set PlatformDir=x64

@if "%PlatformDir%"=="" (
@echo Error: Invalid PlatformTarget="%PlatformTarget%".
@exit 1
)

@set LibSrcDir=%VLDRuntimeDir%\%PlatformDir%
@for /F "usebackq tokens=*" %%i in ("%~dpn0-%PlatformTarget%.txt") do @call :DoCopy "%%~i" "%LibSrcDir%" "%OutDir%"

@endlocal
:lEnd
@echo ---- %~n0 end ----
@goto :EOF

:DoCopy
@rem Skip empty lines and UTF-8 BOM
@if "%~1"=="" goto :EOF
@if "%~1"=="я╗┐" goto :EOF
@echo xcopy /y /d "%~2\%~1" "%~3"
@xcopy /y /d "%~2\%~1" "%~3"
@if errorlevel 1 @(
@echo Error: Failed to copy "%~2\%~1" -^> "%~3".
@exit 1
)
@goto :EOF
