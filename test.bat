@echo off

pushd data
	xcopy /Y ..\build\minify_css.dll *

	cls

	..\build\win32_minify.exe test\*.* output
popd