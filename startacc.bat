@echo off
REM ==============================================================
REM  Tales of Pirate - server launcher
REM  Launches each .NET server in its own cmd window with a 5s
REM  delay between starts, then starts the C++ GameServer.
REM  Run from the repository root (the folder this file lives in).
REM ==============================================================

cd /d "%~dp0"

echo [start_server] Starting AccountServer...
start "AccountServer" cmd /k dotnet run --project sources/Dotnet/Servers/Account/Corsairs.AccountServer