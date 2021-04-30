@ECHO OFF

if not exist bin\ (
    mkdir bin
)

cl /Fobin\PowerPlayer /Febin\PowerPlayer PowerPlayer.cpp user32.lib shell32.lib powrprof.lib