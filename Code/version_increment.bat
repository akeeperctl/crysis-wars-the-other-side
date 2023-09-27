echo off

::Получение значения из файла
set /p Build=<version.txt
echo CURRENT BUILD NUMBER: %Build%

::Изменение
set /a Build = %Build% + 1

::Сохранение
echo %Build% > version.txt

::Вывод в консоль
echo NEW BUILD NUMBER: %Build% 
pause