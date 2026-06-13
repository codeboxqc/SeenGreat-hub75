@echo off
cd token
setlocal EnableDelayedExpansion

for %%F in (*.json) do (
    set "name=%%~nF"
    set "name=!name: =!"
    set "name=!name:(=!"
    set "name=!name:)=!"
    ren "%%F" "!name!%%~xF"
)

endlocal


pause