@echo off
python setup_win.py --generator gmake2
IF %ERRORLEVEL%==0 (
    cd ..
    cmd /k
) ELSE (
    PAUSE
)
