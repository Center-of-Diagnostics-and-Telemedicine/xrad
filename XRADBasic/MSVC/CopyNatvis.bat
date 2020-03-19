@echo ---- %~n0 ----
@echo OutDir=%~1

@if "%~1"=="" @goto ErrParams

@goto OkParams
:ErrParams
@echo Error: Invalid parameters for %~nx0.
@exit 1
:OkParams

@setlocal

@set OutDir=%~1
@set SrcDir=%~dp0..

@for /F "usebackq tokens=*" %%i in ("%~dpn0-files.txt") do @call :DoCopy "%%~i" "%SrcDir%" "%OutDir%"

@endlocal
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
