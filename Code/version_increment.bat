echo off

::Получение значения из файла
set /p Build=< "D:\Games\Crysis Wars\Mods\TheOtherSide\Code\version.txt"

::Изменение
set /a Build = %Build% + 1
set /a NextBuild = %Build% + 1

::Сохранение
echo %Build% > "D:\Games\Crysis Wars\Mods\TheOtherSide\Code\version.txt"

::Вывод в консоль
echo CURRENT BUILD NUMBER: %Build%
echo NEXT BUILD NUMBER: %NextBuild% 