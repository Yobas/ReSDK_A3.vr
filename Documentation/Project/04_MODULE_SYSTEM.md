# Модульная система

Проект организован как модульная система, где каждый модуль - независимая единица функциональности со своей точкой входа и четко определенными зависимостями.

## Концепция модулей

Модуль - это директория с кодом, которая:
- Имеет точку входа (чаще всего именуется по паттерну `*_init.sqf`)
- Регистрируется в соответствующем загрузчике (клиентский, серверный, редактор)
- Имеет префикс для своих функций и переменных
- Может иметь зависимости от других модулей

## Типы модулей

### Серверные модули (host/)

Выполняются только на сервере. Размещаются в `Src/host/`.

**Использование:**
- Игровая логика
- Управление игровым миром
- Обработка действий игроков
- Системы, требующие серверной авторизации

### Клиентские модули (client/)

Выполняются только на клиенте. Размещаются в `Src/client/`.

**Использование:**
- Пользовательский интерфейс
- Визуальные эффекты
- Обработка ввода
- Локальная оптимизация

### Редакторные модули (Editor/)

Выполняются только в редакторе. Размещаются в `Src/Editor/`.

**Использование:**
- Инструменты редактора
- Визуальные компоненты редактора
- Утилиты разработки

### Общие компоненты (host/CommonComponents/)

Используются и на клиенте, и на сервере. Размещаются в `Src/host/CommonComponents/`.

**Использование:**
- Общие структуры данных
- Утилиты, не зависящие от окружения
- Алгоритмы
- Математические функции

## Структура модуля

### Базовая структура

```
MyModule/
├── MyModule_init.sqf      # Точка входа модуля (обязательно)
├── functions.sqf          # Функции модуля (опционально)
├── config.hpp             # Конфигурация (опционально)
└── README.md              # Документация модуля (рекомендуется)
```

### Точка входа модуля

Файл `*_init.sqf` является точкой входа модуля:

```sqf
// ======================================================
// Copyright (c) 2017-2025 the ReSDK_A3 project
// sdk.relicta.ru
// ======================================================

// Подключаем необходимые заголовочные файлы
#include "..\engine.hpp"  // для host модулей
// или
#include "..\..\host\engine.hpp"  // для client модулей

// Загружаем файлы модуля (если есть)
#include "functions.sqf"
#include "moduleHeader.hpp"
#include "modulePrivateHeader.h"

// Инициализация модуля
myModule_initialize = {
    log("MyModule: Initializing...");
    // код инициализации
};

// Выполняем инициализацию в самом модуле (в конце файла) либо где-то в загрузчике модулей
call myModule_initialize;
```

## Регистрация модулей

### Регистрация серверного модуля

Добавьте загрузку модуля в `Src/host/init.sqf`:

```sqf
// После загрузки зависимостей
loadFile("src\host\MyModule\MyModule_init.sqf");
```

**Важно:** Порядок загрузки критичен! Загружайте модуль после всех его зависимостей.

### Регистрация клиентского модуля

Добавьте загрузку модуля в `Src/client/loader.hpp`:

```sqf
importClient("src\client\MyModule\MyModule_init.sqf"); cmplog("MyModule")
```

Макрос `importClient()`:
- Компилирует модуль
- Добавляет в `allClientContents` для передачи клиенту
- Выполняет код сразу (если `_canCallClientCode` разрешен)

**Важно:** Порядок важен из-за зависимостей. Смотрите комментарии в `loader.hpp`.

### Регистрация редакторного модуля

Добавьте загрузку модуля в `Src/Editor/Editor_init.sqf`:

```sqf
componentInit(MyComponent)
#include "MyComponent\MyComponent_init.sqf"
```

Макрос `componentInit()` автоматически логирует загрузку компонента.

### Регистрация общего компонента

Для компонентов в `host/CommonComponents/` используйте:

```sqf
// В client/loader.hpp
importCommon("MyComponent.sqf");
```

## Именование в модулях

### Префиксы

Все функции и глобальные переменные модуля должны иметь префикс:

```sqf
// Правильно
myModule_functionName = {
    // код
};

myModule_globalVariable = 10;

// Неправильно
functionName = { // нет префикса
    // код
};
```

### Конвенции именования

- **Функции:** `moduleName_functionName` (camelCase)
- **Глобальные переменные:** `moduleName_variableName` (camelCase)
- **Локальные переменные:** начинаются с `_`
- **Константы:** `MODULE_NAME_CONSTANT` (UPPER_CASE)

## Создание нового модуля

### Шаг 1: Создание структуры

1. Создайте директорию в соответствующей папке:
   - `Src/host/MyModule/` - для серверного модуля
   - `Src/client/MyModule/` - для клиентского модуля
   - `Src/Editor/MyComponent/` - для редакторного модуля

2. Создайте файл `MyModule_init.sqf`:

```sqf
// ======================================================
// Copyright (c) 2017-2025 the ReSDK_A3 project
// sdk.relicta.ru
// ======================================================

#include "..\engine.hpp"

// Ваш код модуля
myModule_init = {
    log("MyModule: Module initialized");
};

call myModule_init;
```

### Шаг 2: Регистрация модуля

Добавьте модуль в соответствующий загрузчик (см. раздел "Регистрация модулей").

### Шаг 3: Определение зависимостей

Убедитесь, что зависимости загружены перед вашим модулем:

```sqf
// Если модуль зависит от GameObjects
// Убедитесь, что GameObjects загружен ранее в host/init.sqf
```

### Шаг 4: Тестирование

1. Запустите проект
2. Проверьте логи на ошибки загрузки
3. Проверьте работу модуля

## Зависимости между модулями

### Определение зависимостей

Модуль может зависеть от:
- Других модулей
- Заголовочных файлов (`.hpp`)
- Системных модулей (OOP_engine, Networking, etc.)

### Порядок загрузки

Критично загружать модули в правильном порядке:

```sqf
// В host/init.sqf

// 1. Сначала системные модули
loadFile("src\host\OOP_engine\oop_object.sqf");

// 2. Затем базовые системы
#include "GameObjects\loader.hpp"

// 3. Затем модули, зависящие от базовых
loadFile("src\host\MyModule\MyModule_init.sqf");
```

### Пример зависимостей

Модуль, работающий с игровыми объектами:

```sqf
// MyModule_init.sqf

#include "..\engine.hpp"
#include "..\oop.hpp"                    // Зависимость: OOP система
#include "..\GameObjects\GameConstants.hpp" // Зависимость: GameObjects

class(MyGameObject) extends(Item)
    // код класса
endclass
```

Убедитесь, что OOP_engine и GameObjects загружены раньше.

## Практические примеры

### Пример 1: Простой серверный модуль

```sqf
// host/SimpleModule/SimpleModule_init.sqf

#include "..\engine.hpp"

simpleModule_value = 0;

simpleModule_increment = {
    simpleModule_value = simpleModule_value + 1;
    logformat("SimpleModule: Value: %1", simpleModule_value);
};

simpleModule_getValue = {
    simpleModule_value
};

log("SimpleModule loaded");
```

### Пример 2: Модуль с классами

```sqf
// host/ItemModule/ItemModule_init.sqf

#include "..\engine.hpp"
#include "..\oop.hpp"
#include "..\GameObjects\GameConstants.hpp"

class(MyItem) extends(Item)
    var(durability, 100);
    
    func(useItem)
    {
        objParams();
        setSelf(durability,getSelf(durability) - 1);
        if (getSelf(durability) <= 0) then {
            callSelf(breakItem);
        };
    };
    
    func(breakItem)
    {
        objParams();
        log("MyItem: Item broken");
        // логика поломки
    };
endclass

log("ItemModule loaded");
```

### Пример 3: Клиентский модуль

```sqf
// client/UIControl/UIControl_init.sqf

#include "..\..\host\engine.hpp"

uiControl_showDialog = {
    params ["_title", "_content"];
    logformat("UIControl: Showing dialog: %1", _title);
    // логика отображения диалога
};

log("UIControl loaded");
```

## Лучшие практики

### Изоляция модулей

- Модуль не должен напрямую обращаться к внутренностям других модулей
- Используйте публичные API модулей
- Избегайте глобальных переменных без префикса модуля

### Документирование

Создайте `README.md` в модуле:

```markdown
# MyModule

Описание модуля.

## Зависимости
- OOP_engine
- GameObjects

## API

### Функции
- `myModule_functionName()` - описание
```

### Обработка ошибок

```sqf
myModule_init = {
    if (isNullVar(someDependency)) then {
        error("MyModule: Dependency not loaded");
        appExit(APPEXIT_REASON_UNDEFINEDMODULE);
    };
    // инициализация
};
```

## Что дальше?

- ➡️ [Особенности синтаксиса](03_SYNTAX_GUIDE.md) - синтаксис для работы с модулями
- ➡️ [Отладка](05_DEBUGGING.md) - отладка модулей
- ➡️ [Стандарты кодирования](09_CODING_STANDARDS.md) - правила написания кода модулей

