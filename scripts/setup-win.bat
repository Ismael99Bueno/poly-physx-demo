@echo off
python setup_win.py
IF %ERRORLEVEL% EQ 0 (
    cd ..
    cmd /k
)