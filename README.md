Прошивка на STM32 "Blue Pill" для работы с LCD дисплеем + модуль для SD карт.

## Если подробнее

Прошивка для STM32F103C8T6, LCD1602 с I2C и модуля для SD карт. 

Выводит время на дисплей при помощи RTC и реализует бегущую строку.

Использует FreeRTOS для потоков, код для потоков предварительно конвертируется в бин через arm gnu toolchain и отправляется на SD-карту.


Всё написано в Arduino IDE.

## Необходимые библиотеки
1) [Библиотека для STM32](http://dan.drown.org/stm32duino/package_STM32duino_index.json)
2) LiquidCrystal_I2C.h
3) STM32FreeRTOS.h
4) STM32RTC.h
5) SD.h