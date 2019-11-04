@echo off
REM This command takes the output of the git command and then puts it in to the variable %%a for the echo command that then
REM builds the BuildId.h file
for /f %%a in ('"git rev-parse --abbrev-ref HEAD"') do echo #define GIT_BRANCH "%%a" > BuildId.h


