rmdir /s /q build
mkdir build
copy release\GraphTool.exe build\%PRODUCT_NAME%.exe

windeployqt --qmldir source\ui\qml --no-angle --no-compiler-runtime --no-opengl-sw build\%PRODUCT_NAME%.exe || EXIT /B 1
xcopy "%CRTDIRECTORY%*.*" build || EXIT /B 1
xcopy "%UniversalCRTSdkDir%redist\ucrt\DLLs\x64\*.*" build || EXIT /B 1
REM signtool sign /f %SIGN_KEYSTORE_WINDOWS% /p %SIGN_PASSWORD% /tr %SIGN_TSA% /td SHA256 build\%PRODUCT_NAME%.exe || EXIT /B 1
makensis /NOCD /DPRODUCT_NAME=%PRODUCT_NAME% /DVERSION=%VERSION% "/XOutFile build\%PRODUCT_NAME%-%VERSION%-installer.exe" installers\windows\installer.nsi
REM signtool sign /f %SIGN_KEYSTORE_WINDOWS% /p %SIGN_PASSWORD% /tr %SIGN_TSA% /td SHA256 build\%PRODUCT_NAME%-%VERSION%-installer.exe || EXIT /B 1
