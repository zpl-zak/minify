@echo off

ctime -begin minify.ctm

set CCF=-Od -MTd -nologo -fp:fast -fp:except- -Gm- -GR- -EHa- -Zo -Oi -WX -W4 -wd4201 -wd4100 -wd4189 -wd4505 -wd4127 -FC -Z7 /DEBUG
set CLF= -incremental:no -opt:ref

if not exist build mkdir build

pushd build

del *.pdb > NUL 2> NUL

echo Building Minify CSS...

cl %CCF% ../minify_css.cpp -Fmminify.map -LD /link -incremental:no -opt:ref -PDB:minify_%random%.pdb -EXPORT:MinifyFile -EXPORT:GetExtension

echo Building Minify JS...

cl %CCF% ../minify_js.cpp -Fmminify.map -LD /link -incremental:no -opt:ref -PDB:minify_%random%.pdb -EXPORT:MinifyFile -EXPORT:GetExtension

set LastError=%ERRORLEVEL%

echo Building Minify...

cl %CCF% ../win32_minify.cpp -Fmminify.map /link %CLF%

popd

ctime -end minify.ctm %LastError%

xcopy /Y build\minify_css.dll data\minify_css.dll
xcopy /Y build\minify_js.dll data\minify_js.dll