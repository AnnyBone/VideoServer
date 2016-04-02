@echo off

pushd %~dp0
premake5 vs2015
popd

if errorlevel 1 pause

