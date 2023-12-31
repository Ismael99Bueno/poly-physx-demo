@echo off
python src/setup_win.py --generator vs2022
IF %ERRORLEVEL%==0 (
    cd ..
    cmd /k
) ELSE (
    PAUSE
)
