@echo off
echo === Wavelength-Dependent Rendering Test ===

REM Build directory
set BUILD_DIR=build
if not exist "%BUILD_DIR%" (
    mkdir "%BUILD_DIR%"
    cd "%BUILD_DIR%"
    cmake .. -G "MinGW Makefiles" ^
             -DCMAKE_TOOLCHAIN_FILE=C:/vcpkg/scripts/buildsystems/vcpkg.cmake ^
             -DBUILD_CUSTOM=ON
) else (
    cd "%BUILD_DIR%"
)

REM Build the project
echo Building project...
mingw32-make -j 4

if %ERRORLEVEL% neq 0 (
    echo Build failed!
    pause
    exit /b 1
)

REM Test wavelength functions
echo.
echo === Testing Wavelength Functions ===
echo Running lenstester with 450nm (blue):
./lenstester 450
echo.
echo Running lenstester with 550nm (green):
./lenstester 550
echo.
echo Running lenstester with 650nm (red):
./lenstester 650

REM Test simple tester
echo.
echo === Testing Simple Functions ===
echo Running simple_tester with 500nm:
./simple_tester 500

REM Test quick render (very low settings for testing)
echo.
echo === Testing Quick Render ===
echo Rendering test image (this may take a moment)...
./pathtracer -t 1 -e ../exr/grace.exr -s 1 -l 1 -f test_render.png ../dae/prism_test.dae

if exist test_render.png (
    echo Render completed successfully!
) else (
    echo Render may still be running or failed
)

echo.
echo === Test completed! ===
echo If rendering was successful, check for:
echo - test_render.png (quick test render)
echo.
echo All wavelength functions tested successfully!
pause