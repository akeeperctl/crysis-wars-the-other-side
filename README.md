# crysis-wars-the-other-side
Исходный код от модификации **The Other Side: Multiplayer**

## Описание

**The Other Side: Multiplayer** - **мультиплеерная** модификация к игре Crysis Wars, добавляющая в сетевую игру возможность играть за пришельцев.

## Стэк технологий
- **CryENGINE 2** - игровой движок игры Crysis Wars
- **C++ 14** - язык программирования
- **Google Docs** - создание дизайн документов
- **Git** - система контроля версий
- **Trello** - координация с членами команды

## Игровой режим Conquest

Это новый игровой режим, сочетающий в себе противостояние нескольких (2 и более) фракций. Игрок может присоединиться к любой фракции и отстаивать её интересы на поле боя. У каждой фракции существуют свои + и -.

**Трейлер:**  <https://www.youtube.com/watch?v=lOWJHera5Ao>

## Установка и запуск (игрок)
1.  Перейти по ссылке на Moddb <https://www.moddb.com/mods/crysis-the-other-side/downloads/the-other-side-conquest-build-4011>
2.  Скачать файл и развернуть описание
3.  Найти и прочитать пункт `Installation`

## Настройка проекта и компиляция (разработчик)
1.	В исходном коде движка, поставляемым из **CrysisWars_ModSDK_SourceCode_v1.1**, 
зайти в папку `CrysisWars\Code\CryEngine\CryCommon`

2.	Открыть файл **StlUtils.h** и в 21 строке прописать
      ```cpp
      #define _SILENCE_STDEXT_HASH_DEPRECATION_WARNINGS
      ```

4.	Открыть файл **NetHelpers.h** и 132 строку заменить на
      ```cpp
      return TNetMessageCallbackResult( ((cls*)p)->Handle##name( serialize, curSeq, oldSeq, pEntityId, pChannel ), reinterpret_cast<INetAtSyncItem*>(NULL) ); \
      ```

6.	Перейти в папку `CrysisWars\Code\CryEngine\CryAction`

7.	Открыть файл **IGameObject.h** и заменить
      ```cpp
      template <class T_Derived, class T_Parent, size_t MAX_STATIC_MESSAGES = 32>
      ```
      на
      ```cpp
      template <class T_Derived, class T_Parent, size_t MAX_STATIC_MESSAGES = 64>
      ```

8. Зайти в свойства проекта, **Компоновщик->Ввод** и в поле **Дополнительные зависимости** указать
      ```
      version.lib;%(AdditionalDependencies)
      ```