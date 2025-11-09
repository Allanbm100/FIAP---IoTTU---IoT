@echo off
REM
REM
cd /d "%~dp0"
node pg-executor.js %*
