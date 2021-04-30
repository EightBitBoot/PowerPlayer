@ECHO OFF

if not exist bin\ (
    mkdir bin
)

if not exist build\ (
    mkdir build
)

rc -fo build\PowerPlayer.res PowerPlayer.rc
cl /Fobuild\PowerPlayer /Febin\PowerPlayer PowerPlayer.cpp user32.lib shell32.lib powrprof.lib /link /incremental:no build\PowerPlayer.res