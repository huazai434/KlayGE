CALL "%VS80COMNTOOLS%..\..\VC\vcvarsall.bat" x86
devenv Tools.sln /Build "Release|Win32"
pause