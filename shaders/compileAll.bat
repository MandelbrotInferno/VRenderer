@echo off
setlocal enabledelayedexpansion

:: Path to glslangValidator (adjust if not in PATH)
set GLSLANG_VALIDATOR=glslangValidator

:: Output directory for compiled SPIR-V shaders
set OUTPUT_DIR=SPIRV-CompiledShaders

:: Create output directory if it doesn't exist
if not exist "%OUTPUT_DIR%" (
    mkdir "%OUTPUT_DIR%"
)

:: List of shader extensions to compile
for %%F in (*.vert *.frag *.comp *.geom *.tesc *.tese) do (
    echo Compiling %%F ...
    %GLSLANG_VALIDATOR% -V "%%F" -o "%OUTPUT_DIR%\%%~nF.spv"
    if errorlevel 1 (
        echo Error compiling %%F
    ) else (
        echo Successfully compiled %%F to %OUTPUT_DIR%\%%~nF.spv
    )
)

echo.
echo All shaders processed.
pause
