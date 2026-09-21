@echo off
setlocal
cd /d "%~dp0"
set "PYTHONUTF8=1"
set "BADGE_PYTHON=py -3"
where py >nul 2>nul
if errorlevel 1 set "BADGE_PYTHON=python"
%BADGE_PYTHON% --version
if errorlevel 1 goto missing_python
%BADGE_PYTHON% -c "import esptool; assert esptool.__version__.split('.')[0]=='4'" >nul 2>nul
if errorlevel 1 goto missing_esptool
%BADGE_PYTHON% tools\upgrade_badge.py
if errorlevel 1 goto failed
echo Upgrade verified. Keep the private backup; do not share it.
pause
exit /b 0
:missing_python
echo Install Python 3.10 or newer, then run this file again. See README.zh_CN.md.
pause
exit /b 1
:missing_esptool
echo Install the dependencies, then run this file again:
echo %BADGE_PYTHON% -m pip install -r requirements.txt
pause
exit /b 1
:failed
echo Upgrade stopped. Keep the backup and error output. Do not erase the device.
pause
exit /b 1
