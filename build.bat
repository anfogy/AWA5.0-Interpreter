@echo off

set compiler_flags=/std:c++20 /O2 /permissive /EHsc /GL /nologo /Fe:awa.exe

if exist ".\build" rmdir /S /Q ".\build"
mkdir ".\build"
cd ".\build"

setlocal enabledelayedexpansion

set sources=
for %%f in (..\src\*.cpp) do (
    set sources=!sources! %%f
)

cl %compiler_flags% %sources%

endlocal