# Отладка и диагностика

Отладка - критически важный процесс при разработке проекта. В этом руководстве описаны инструменты и техники отладки для разных частей проекта.

## Инструменты отладки

### Встроенное логирование

Проект использует систему макросов для логирования из `engine.hpp`:

```sqf
#include "engine.hpp"

// Простое логирование
log("Сообщение");

// Логирование с форматированием
logformat("Модуль", "Значение: %1", _value);

// Предупреждения
warning("Предупреждение");
warningformat("Модуль", "Проблема: %1", _issue);

// Ошибки
error("Критическая ошибка");
errorformat("Модуль", "Ошибка: %1", _errorMsg);

// Трассировка (только в DEBUG)
trace("Точка выполнения");
traceformat("Модуль", "Данные: %1", _data);
```

### Breakpoints

Используйте макрос `breakpoint()` для установки точек останова:

```sqf
#ifdef __TRACE__ENABLED
breakpoint_setfile("MyModule.sqf");
breakpoint("Достигнута точка останова");
#endif
```

**Важно:** Breakpoints работают только при включенном флаге `__TRACE__ENABLED` (автоматически в DEBUG).

### Debug консоль

Используйте `debug_console` расширение для вывода в консоль:

```sqf
"debug_console" callExtension "message";
```

## Отладка host кода

### Логирование в RPT файлы

Серверные логи сохраняются в файлах `.rpt`:

**Расположение:**
- Windows: `%LOCALAPPDATA%\Arma 3\`
- Имя файла: `arma3server_YYYYMMDD_HHMMSS.rpt`

### Отладочные макросы и флаги

Используйте флаги компиляции для отладки:

```sqf
#ifdef DEBUG
    // Код выполняется только в режиме отладки
    logformat("Debug", "Значение: %1", _value);
#endif
```

### Работа с классами и объектами в отладчике

#### Проверка состояния объекта

```sqf
// Получение всех полей объекта
private _fields = (this getVariable PROTOTYPE_VAR_NAME) getVariable "__allfields";

// Получение значения поля
private _value = getSelf(fieldName);

// Вывод состояния объекта
logformat("Debug", "Object state: %1", _fields);
```

#### Трассировка методов

```sqf
func(myMethod)
{
    objParams();
    traceformat("MyClass", "myMethod called with: %1", _param);
    // код метода
};
```

### Профилирование производительности

Используйте `diag_tickTime` для измерения времени выполнения:

```sqf
private _startTime = diag_tickTime;
// ваш код
private _endTime = diag_tickTime;
private _duration = _endTime - _startTime;
logformat("Performance", "Execution time: %1ms", _duration * 1000);
```

## Отладка client кода

### Клиентские логи

Клиентские логи также сохраняются в `.rpt` файлах:

**Расположение:**
- Windows: `%LOCALAPPDATA%\Arma 3\`
- Имя файла: `arma3_YYYYMMDD_HHMMSS.rpt`

### Отладка через ClientStatistic

Модуль `ClientStatistic` предоставляет визуальную отладку:

```sqf
// Включение статистики
clientStatistic_enable = true;
```

**Что показывает:**
- FPS
- Использование памяти
- Версию клиента
- Состояние загрузки модулей

### Синхронизация с сервером

Отлаживайте сетевые взаимодействия:

```sqf
// На клиенте
logformat("Client", "Sending RPC: %1", _data);
serverRpc_call("myAction", [_data]);

// На сервере
serverrpc_registerHandler("myAction", {
    params ["_data"];
    logformat("Server", "Received RPC: %1", _data);
});
```

### Отладка UI (WidgetSystem)

```sqf
// Создание отладочного виджета
private _debugWidget = [widget_debugPanel] call widget_create;
[_debugWidget, "text", format["Value: %1", _value]] call widget_setProperty;
```

## Отладка editor кода

### Редакторские инструменты отладки

Редактор имеет встроенные инструменты отладки:

- **Логи редактора** - отображаются в консоли редактора
- **Инспектор объектов** - просмотр состояния игровых объектов
- **Отладчик визуального скриптинга** - для ReNode

### Тестирование компонентов

```sqf
#ifdef EDITOR
componentInit(TestComponent)
    testComponent_run = {
        log("Testing component...");
        // тестовый код
    };
    
    call testComponent_run;
#endif
```

### Симуляция

Используйте симуляцию в редакторе для тестирования:

1. Откройте редактор
2. Настройте сцену
3. Запустите симуляцию (F3)
4. Проверьте логи во время симуляции

## Специфичные техники

### Отладка NOEngine (репликация объектов)

NOEngine имеет специальные отладочные флаги:

```sqf
#ifdef DEBUG
    // Отображение чанков
    #define NOE_DEBUG_SHOW_CHUNKS
    
    // Отображение объектов
    #define NOE_DEBUG_SHOW_OBJECTS
#endif
```

**Логирование репликации:**
- Логи загрузки чанков
- Логи создания объектов
- Логи синхронизации состояния

### Отладка OOP системы

#### Проверка иерархии классов

```sqf
// Получение всех классов
private _allClasses = p_table_allclassnames;

// Проверка наследования
private _inheritance = p_table_inheritance;
logformat("OOP", "Inheritance: %1", _inheritance);
```

#### Отладка создания объектов

```sqf
private _obj = new(MyClass);
traceformat("OOP", "Created object: %1", _obj);

// Проверка типа объекта
private _className = callFunc(_obj, getClassName);
logformat("OOP", "Object class: %1", _className);
```

### Отладка сетевого взаимодействия (RPC)

#### Включение RPC логирования

```sqf
#ifdef DEBUG
    #define ENABLE_RPCLOG_CONSOLE_SERVER
    #define ENABLE_RPCLOG_CONSOLE_CLIENT
#endif
```

#### Трассировка RPC вызовов

```sqf
// На клиенте
serverRpc_call("myAction", [_data]);
traceformat("RPC", "Sent: myAction with %1", _data);

// На сервере
serverrpc_registerHandler("myAction", {
    params ["_data"];
    traceformat("RPC", "Received: myAction with %1", _data);
});
```

## Работа с логами

### Поиск в логах

Используйте поиск для нахождения ошибок:

**Ключевые слова для поиска:**
- `ERROR` - ошибки
- `WARNING` - предупреждения
- `TRACE` - трассировка
- Название модуля - логи конкретного модуля

### Анализ производительности

Ищите медленные операции в логах:

```sqf
// Включите таймеры
private _start = diag_tickTime;
// код
private _duration = diag_tickTime - _start;
if (_duration > 0.1) then {
    warningformat("Performance", "Slow operation: %1ms", _duration * 1000);
};
```

## Лучшие практики

### Структурированное логирование

```sqf
// Используйте префиксы для группировки
#define LOG_PREFIX "MyModule"

logformat(LOG_PREFIX, "Initializing...");
logformat(LOG_PREFIX, "Value: %1", _value);
```

### Условное логирование

```sqf
#ifdef DEBUG
    #define DEBUG_LOG(msg) logformat("Debug", msg)
#else
    #define DEBUG_LOG(msg)
#endif

DEBUG_LOG("This only logs in DEBUG mode");
```

### Проверка состояний

```sqf
// Проверка валидности перед использованием
if (!valid(_obj)) then {
    errorformat("MyModule", "Object is invalid");
    return;
};
```

## Что дальше?

- ➡️ [Решение проблем](10_TROUBLESHOOTING.md) - типичные проблемы и их решение
- ➡️ [Модульная система](04_MODULE_SYSTEM.md) - отладка модулей

