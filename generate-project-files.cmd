@echo off
rem Just a wrapper to run the matching PowerShell script.
PowerShell.exe -NoProfile -NonInteractive -NoLogo -ExecutionPolicy Bypass -Command "& '%~dpn0.ps1'"
