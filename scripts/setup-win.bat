@echo off
set ERRORLEVEL=
python setup_win.py
IF %ERRORLEVEL%==0 (
    cd ..
    cmd /k