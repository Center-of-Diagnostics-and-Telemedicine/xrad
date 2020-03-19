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

@call :setprint OutDir "%~1"
@call :setprint PlatformTarget "%~2"
@call :setprint ConfigurationName "%~3"

@call :setprint dcmtk_path "%XRADThirdParty%\dcmtk\install\%PlatformTarget%\%ConfigurationName%"
@call :setprint openjpeg_path "%XRADThirdParty%\openjpeg\install\%PlatformTarget%\%ConfigurationName%"

:: dcmtk
@call :DoCopy "%dcmtk_path%\bin\charls.dll" "%OutDir%"
@call :DoCopy "%dcmtk_path%\bin\dcmdata.dll" "%OutDir%"
@call :DoCopy "%dcmtk_path%\bin\dcmimgle.dll" "%OutDir%"
@call :DoCopy "%dcmtk_path%\bin\dcmjpeg.dll" "%OutDir%"
@call :DoCopy "%dcmtk_path%\bin\dcmjpls.dll" "%OutDir%"
@call :DoCopy "%dcmtk_path%\bin\ijg12.dll" "%OutDir%"
@call :DoCopy "%dcmtk_path%\bin\ijg16.dll" "%OutDir%"
@call :DoCopy "%dcmtk_path%\bin\ijg8.dll" "%OutDir%"
@call :DoCopy "%dcmtk_path%\bin\oflog.dll" "%OutDir%"
@call :DoCopy "%dcmtk_path%\bin\ofstd.dll" "%OutDir%"
@call :DoCopy "%dcmtk_path%\bin\dcmnet.dll" "%OutDir%"
@call :DoCopy "%dcmtk_path%\etc\dcmtk\storescp.cfg" "%OutDir%"

:: openjpeg
@call :DoCopy "%openjpeg_path%\bin\openjp2.dll" "%OutDir%"

@echo ---- %~n0 end ----
@goto :EOF
::==================================================================================

:setprint
:: parameters: 1 - name of variable, 2 - value of variable
@echo set %~1 = %~2
@set %~1=%~2
@goto :EOF

:DoCopy
@echo xcopy /y /d "%~1" "%~2"
@xcopy /y /d "%~1" "%~2"
@if errorlevel 1 @(
@echo Error: Failed to copy "%~1" -^> "%~2".
@exit 1
)
@goto :EOF
