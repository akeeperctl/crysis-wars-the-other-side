# crysis-wars-the-other-side
Исходный код от модификации **The Other Side: Multiplayer**

## Описание

**The Other Side: Multiplayer** - **мультиплеерная** модификация к игре Crysis Wars, добавляющая в сетевую игру возможность играть за пришельцев. А также режим игры Zeus (как в Arma 3).

## Стэк технологий
- **CryENGINE 2** - игровой движок игры Crysis Wars
- **C++ 14** - язык программирования
- **Google Docs** - создание дизайн документов
- **Git** - система контроля версий
- **Trello** - координация с членами команды

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
