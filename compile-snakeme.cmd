@echo off
set EMXOMFLD_TYPE=WLINK
set EMXOMFLD_LINKER=wl.exe
set EMXOMFLD_PRELINK=0
make -f Makefile.os2 %1 > compile-snakeme.log 2>&1
if errorlevel 1 (echo Build FAILED. See compile-snakeme.log.) else (echo Build OK.)
