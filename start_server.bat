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
timeout /t 5 /nobreak >nul

echo [start_server] Starting GroupServer...
start "GroupServer" cmd /k dotnet run --project sources/Dotnet/Servers/Group/Corsairs.GroupServer
timeout /t 5 /nobreak >nul

echo [start_server] Starting GateServer...
start "GateServer" cmd /k dotnet run --project sources/Dotnet/Servers/Gate/Corsairs.GateServer
timeout /t 5 /nobreak >nul

echo [start_server] Starting Admin.Web...
start "Admin.Web" cmd /k dotnet run --project sources/Dotnet/Admin/Corsairs.Admin.Web
timeout /t 5 /nobreak >nul

echo [start_server] Starting GameServer...
start "GameServer" /d "%~dp0server\GameServer" GameServer.exe

echo [start_server] All processes launched.
