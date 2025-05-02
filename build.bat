
@echo off


:: Set variables
set SRC_DIR=%~dp0
set TESTS_DIR=%SRC_DIR%/tests
set BIN_DIR=bin
set CFLAGS=/std:c17 /Zi
set MAIN_FILE=%SRC_DIR%/mayorana.cpp
set FLAGS=MAYORANA
if not exist %BIN_DIR% mkdir %BIN_DIR%

cd %BIN_DIR%

:: Create the bin directory if it doesn't exist

:: Set up MSVC environment (ensure it's the correct script)
call "../setup_cl_x64.bat"

:: Compile with MSVC
:: Using Fe: in stead of OUT, since we are using unity build.
cl /Fe:MayoranaFramework %CFLAGS% -I%SRC_DIR% -I%TESTS_DIR% %MAIN_FILE% -D%FLAGS% /link /SUBSYSTEM:CONSOLE /nologo


