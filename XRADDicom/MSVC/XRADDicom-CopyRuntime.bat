@echo ---- %~n0 ----
:: README !!!
:: Use "call :setprint" instead of "set" and "echo" and "call :DoCopy" instead of "copy"

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

@call :setprint OutDir "%~1"
@call :setprint PlatformTarget "%~2"
@call :setprint ConfigurationName "%~3"

@call :setprint dcmtk_path "%XRADThirdParty%\dcmtk-icu\install\%PlatformTarget%\%ConfigurationName%"
@call :setprint dcmtk_externals_path "%XRADThirdParty%\dcmtk-icu\externals\%PlatformTarget%"
@call :setprint openjpeg_path "%XRADThirdParty%\openjpeg\install\%PlatformTarget%\%ConfigurationName%"

:: dcmtk
@call :DoCopyDll "%dcmtk_path%\bin\charls.dll" "%OutDir%"
@call :DoCopyDll "%dcmtk_path%\bin\dcmdata.dll" "%OutDir%"
@call :DoCopyDll "%dcmtk_path%\bin\dcmimgle.dll" "%OutDir%"
@call :DoCopyDll "%dcmtk_path%\bin\dcmjpeg.dll" "%OutDir%"
@call :DoCopyDll "%dcmtk_path%\bin\dcmjpls.dll" "%OutDir%"
@call :DoCopyDll "%dcmtk_path%\bin\ijg12.dll" "%OutDir%"
@call :DoCopyDll "%dcmtk_path%\bin\ijg16.dll" "%OutDir%"
@call :DoCopyDll "%dcmtk_path%\bin\ijg8.dll" "%OutDir%"
@call :DoCopyDll "%dcmtk_path%\bin\oflog.dll" "%OutDir%"
@call :DoCopyDll "%dcmtk_path%\bin\ofstd.dll" "%OutDir%"
@call :DoCopyDll "%dcmtk_path%\bin\dcmnet.dll" "%OutDir%"
@call :DoCopy "%dcmtk_path%\etc\dcmtk\storescp.cfg" "%OutDir%"

:: dcmtk - icu
@set QM_ICU_Bin=_error_
@if "%PlatformTarget%"=="x86" @set QM_ICU_Bin=bin
@if "%PlatformTarget%"=="x64" @set QM_ICU_Bin=bin64
@call :DoCopyDll "%dcmtk_externals_path%\icu4c-64_2\%QM_ICU_Bin%\icudt64.dll" "%OutDir%"
@call :DoCopyDll "%dcmtk_externals_path%\icu4c-64_2\%QM_ICU_Bin%\icuuc64.dll" "%OutDir%"
@set QM_ICU_Bin=

:: openjpeg
@call :DoCopyDll "%openjpeg_path%\bin\openjp2.dll" "%OutDir%"

@endlocal
@echo ---- %~n0 end ----
@goto :EOF
::==================================================================================

:setprint
:: parameters: 1 - name of variable, 2 - value of variable
@echo set %~1 = %~2
@set %~1=%~2
@goto :EOF

:DoCopyDll
@call :DoCopy "%~1" "%~2"
@if exist "%~dpn1.pdb" @call :DoCopy "%~dpn1.pdb" "%~2"
@goto :EOF

:DoCopy
@echo xcopy /y /d "%~1" "%~2"
@xcopy /y /d "%~1" "%~2"
@if errorlevel 1 @(
@echo Error: Failed to copy "%~1" -^> "%~2".
@exit 1
)
@goto :EOF
