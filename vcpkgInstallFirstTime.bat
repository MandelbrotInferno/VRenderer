@echo off
REM Clone the vcpkg repository
git clone https://github.com/microsoft/vcpkg.git

REM Bootstrap vcpkg
call .\vcpkg\bootstrap-vcpkg.bat

REM Change directory to vcpkg
cd vcpkg

REM Integrate vcpkg with your environment
vcpkg integrate install

pause
