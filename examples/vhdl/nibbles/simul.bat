@ECHO OFF
if defined GHDL_INSTALL_DIR goto start
call ..\ghdl\set_ghdl_path.bat
PATH=%PATH%;..\gtkw\bin;..\gtkw

:start
@ECHO ON
ghdl -a *.vhd
@if errorlevel 1 goto end
ghdl -e Simulate_CPU
@if errorlevel 1 goto end
ghdl -r Simulate_CPU --vcd=Simulate.vcd --stop-time=50us
@if errorlevel 1 goto end

start /b gtkwave.exe Simulate.vcd

:end