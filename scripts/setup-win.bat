@echo off
python setup_win.py
PAUSE
IF %ERRORLEVEL% EQ 0 (
    cd ..
    cmd /k
)