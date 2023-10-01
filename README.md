# crysis-wars-the-other-side
Исходный код от модификации **The Other Side: Conquest**

## Описание

**The Other Side: Conquest** - **синглплеерная** модификация к игре Crysis Wars, добавляющая новый игровой режим, в котором игрок сможет поучавствовать в масштабной битве как против людей, так и пришельцев.

## Стэк технологий
- **CryENGINE 2** - игровой движок игры Crysis Wars
- **C++ 14** - язык программирования
- **Google Docs** - создание дизайн документов
- **Git** - система контроля версий
- **Trello** - координация с членами команды

## Игровой режим Conquest

Это новый игровой режим, сочетающий в себе противостояние нескольких (2 и более) фракций. 
Игрок может присоединиться к любой фракции и отстаивать её интересы на поле боя.
На данный момент реализовано 3 фракции, за которые можно играть: КНДР, США, Пришельцы.
Игроку также будет выдан отряд, которым он сможет командовать.
В меню лобби игрок может выбрать фракцию и класс за который будет играть.

**Трейлер:**  <https://www.youtube.com/watch?v=lOWJHera5Ao>

## Установка и запуск
1.  Перейти по ссылке на Moddb <https://www.moddb.com/mods/crysis-the-other-side/downloads/the-other-side-conquest-build-4011>
2.  Скачать файл и развернуть описание
3.  Найти и прочитать пункт `Installation`

## Настройка проекта и компиляция (разработчик)
1.	В исходном коде движка, поставляемым из **CrysisWars_ModSDK_SourceCode_v1.1**, 
зайти в папку `CrysisWars\Code\CryEngine\CryCommon`

2.	Открыть файл **StlUtils.h** и в 21 строке прописать `#define _SILENCE_STDEXT_HASH_DEPRECATION_WARNINGS`

3.	Открыть файл **NetHelpers.h** и 132 строку заменить на 
`return TNetMessageCallbackResult( ((cls*)p)->Handle##name( serialize, curSeq, oldSeq, pEntityId, pChannel ), reinterpret_cast<INetAtSyncItem*>(NULL) ); \`

4.	Перейти в папку `CrysisWars\Code\CryEngine\CryAction`

5.	Открыть файл **IGameObject.h** и заменить 
`template <class T_Derived, class T_Parent, size_t MAX_STATIC_MESSAGES = 32>` на 
`template <class T_Derived, class T_Parent, size_t MAX_STATIC_MESSAGES = 64>`
