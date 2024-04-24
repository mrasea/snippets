@echo off
set "EXE_PATH=%~dp0\snippets.exe"
reg add HKLM\Software\Microsoft\Windows\CurrentVersion\Run /v snippets /t REG_SZ /d "%EXE_PATH%" /f
echo %EXE_PATH% set to run at startup.
