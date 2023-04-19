@echo off
python setup_win.py
IF %ERRORLEVEL%==0 (
    cd ..
    cmd /k
) ELSE (
    PAUSE
)
