@rem Copy library files (*.lib, *.pdb...) from local to global library output directory
@echo ---- %~n0: %~n1 ----
@if "%~1"=="" @goto ErrParams
@if "%~2"=="" @goto ErrParams

@goto OkParams
:ErrParams
@echo Error: Invalid parameters for %~nx0.
@exit 1
:OkParams

@set TargetFileMask=%~dpn1.*
@set ConfigDir=%~2

@set GlobalLibDir=%XRADInstall%\lib\MSVC2019\%ConfigDir%

@if exist "%GlobalLibDir%" @goto lSkipMd

@echo Create directory "%GlobalLibDir%".
@mkdir "%GlobalLibDir%"
@if errorlevel 1 @(
@echo Error: Failed to create directory "%GlobalLibDir%".
@exit 1
)
:lSkipMd

@echo xcopy /y /d "%TargetFileMask%" "%GlobalLibDir%"
@xcopy /y /d "%TargetFileMask%" "%GlobalLibDir%"
@if errorlevel 1 @(
@echo Error: Failed to copy "%TargetFileMask%" -^> "%GlobalLibDir%".
@exit 1
)

@echo ---- %~n0: %~n1 end ----
