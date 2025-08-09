@echo off
setlocal enabledelayedexpansion

REM Change this if glslangValidator is not in PATH
set GLSLANG=glslangValidator.exe

for /D %%D in (*) do (
    if exist "%%D\SPV" (
        echo Found SPV folder in %%D
        for %%F in ("%%D\*.vert" "%%D\*.frag" "%%D\*.comp" "%%D\*.geom" "%%D\*.tesc" "%%D\*.tese") do (
            if exist "%%F" (
                set "FILENAME=%%~nF"
				echo.
                echo Compiling %%F to %%D\SPV\!FILENAME!.spv
                "%GLSLANG%" -V "%%F" -o "%%D\SPV\!FILENAME!.spv"
				echo.
            )
        )
    )
)

echo Done.
pause

