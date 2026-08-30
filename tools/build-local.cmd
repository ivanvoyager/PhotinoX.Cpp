@echo off
powershell -ExecutionPolicy Bypass -File "%~dp0build.ps1" -Preset debug-local
pause