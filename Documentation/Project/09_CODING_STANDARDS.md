# Расширенные стандарты кодирования

Этот документ расширяет базовый [CODE-STANDARDS.md](../../CODE-STANDARDS.md) с дополнительными паттернами, анти-паттернами и особенностями работы с проектом.

## Базовые стандарты

См. [CODE-STANDARDS.md](../../CODE-STANDARDS.md) для базовых стандартов:
- Отступы и форматирование
- Именование
- Комментарии
- Объявления переменных и функций

## Паттерны проектирования

### Модульная архитектура

Каждый модуль должен быть независимым:

```sqf
// Хорошо - модуль независим
myModule_process = {
    params ["_data"];
    // логика модуля
};

// Плохо - прямая зависимость от другого модуля
otherModule_process = {
    // использует внутренности другого модуля напрямую
};
```

### Использование OOP системы

Используйте классы для сложных сущностей:

```sqf
// Хорошо - используем классы
class(MyEntity) extends(BaseClass)
    var(data, []);
    
    func(process)
    {
        objParams();
        // логика обработки
    };
endclass

// Плохо - глобальные функции и переменные
myEntity_data = [];
myEntity_process = {
    // логика
};
```

### Инкапсуляция через методы

Используйте методы для доступа к данным:

```sqf
class(MyClass) extends(BaseClass)
    var(privateData, 0);
    
    // Публичный метод для доступа
    getter_func(getPrivateData, getSelf(privateData));
    
    func(setPrivateData)
    {
        objParams_1(_value);
        // Валидация
        if (_value >= 0) then {
            setSelf(privateData, _value);
        };
    };
endclass
```

## Анти-паттерны и что избегать

### Глобальные переменные без префикса

```sqf
// Плохо
counter = 0;

// Хорошо
myModule_counter = 0;
```

### Прямой доступ к внутренностям классов

```sqf
// Плохо - прямой доступ к полю
private _value = getVar(_obj, privateField);

// Хорошо - через публичный метод
private _value = callFunc(_obj, getPrivateField);
```

### Игнорирование objParams()

```sqf
// Плохо - this не определен
func(myMethod)
{
    private _val = getSelf(value); // ошибка!
};

// Хорошо
func(myMethod)
{
    objParams();
    private _val = getSelf(value);
};
```

### Дублирование кода

```sqf
// Плохо - дублирование
moduleA_process = {
    // логика
};

moduleB_process = {
    // та же логика
};

// Хорошо - вынесено в общую функцию
common_process = {
    // общая логика
};

moduleA_process = common_process;
moduleB_process = common_process;
```

### Игнорирование ошибок

```sqf
// Плохо - игнорирование ошибок
myFunction = {
    params ["_obj"];
    // нет проверки на null
    callFunc(_obj, doSomething);
};

// Хорошо - проверка ошибок
myFunction = {
    params ["_obj"];
    if (!valid(_obj)) then {
        errorformat("MyModule", "Invalid object");
        return;
    };
    callFunc(_obj, doSomething);
};
```

## Особенности работы с OOP системой

### Использование наследования

```sqf
// Базовый класс
class(BaseItem) extends(Item)
    func(use)
    {
        objParams();
        logformat("BaseItem", "Used");
    };
endclass

// Наследник
class(SpecificItem) extends(BaseItem)
    func(use)
    {
        objParams();
        // Вызов базового метода
        callSuper(BaseItem, use);
        // Дополнительная логика
        logformat("SpecificItem", "Special action");
    };
endclass
```

### Работа с объектами

```sqf
// Создание объекта
private _obj = new(MyClass);

// Проверка валидности
if (!valid(_obj)) then {
    error("Failed to create object");
};

// Использование объекта
callFunc(_obj, doSomething);

// Удаление объекта
delete(_obj);
```

### Автоматическая очистка ресурсов

Используйте `autoref` для автоматической очистки:

```sqf
class(MyClass) extends(BaseClass)
    autoref
    var(refObject, nullPtr);
endclass
```

## Работа с памятью и производительностью

### Избегайте утечек памяти

```sqf
// Плохо - объекты не удаляются
{
    private _obj = new(MyClass);
    // объект не удаляется
} forEach _array;

// Хорошо - удаление объектов
{
    private _obj = new(MyClass);
    // использование объекта
    delete(_obj);
} forEach _array;
```

### Оптимизация циклов

```sqf
// Плохо - создание объектов в цикле
{
    private _obj = new(MyClass);
    callFunc(_obj, process, [_x]);
    delete(_obj);
} forEach _largeArray;

// Хорошо - переиспользование объекта
private _processor = new(MyClass);
{
    callFunc(_processor, process, [_x]);
} forEach _largeArray;
delete(_processor);
```

### Избегайте лишних проверок в горячих путях

```sqf
// В критичных местах используйте прямые проверки
#ifdef DEBUG
    if (!valid(_obj)) then {
        error("Invalid object");
    };
#endif

callFunc(_obj, criticalMethod);
```

## Сетевые взаимодействия и безопасность

### Валидация данных от клиента

```sqf
// На сервере - всегда валидируйте данные от клиента
serverrpc_registerHandler("clientAction", {
    params ["_clientData"];
    
    // Валидация
    if (!(_clientData isEqualType [])) then {
        errorformat("Security", "Invalid data from client");
        return;
    };
    
    // Обработка
});
```

### Проверка прав доступа

```sqf
// Проверяйте права перед выполнением действий
func(adminAction)
{
    objParams();
    
    if (!callFunc(_user, isAdmin)) then {
        errorformat("Security", "Access denied");
        return;
    };
    
    // выполнение действия
};
```

### Защита от инъекций

```sqf
// Всегда валидируйте строковые входные данные
func(processInput)
{
    objParams_1(_input);
    
    // Валидация
    if (!(_input isEqualType "")) then {
        error("Invalid input type");
        return;
    };
    
    // Обработка
};
```

## Примеры правильного и неправильного кода

### Пример 1: Работа с классами

```sqf
// Плохо
myGlobal_obj = createLocation [...];
myGlobal_obj setVariable ["value", 10];
private _val = myGlobal_obj getVariable "value";

// Хорошо
private _obj = new(MyClass);
setVar(_obj, value, 10);
private _val = getVar(_obj, value);
```

### Пример 2: Обработка ошибок

```sqf
// Плохо
func(processData)
{
    objParams_1(_data);
    // нет проверки
    private _result = _data select 0;
};

// Хорошо
func(processData)
{
    objParams_1(_data);
    if (!(_data isEqualType []) || count _data == 0) then {
        errorformat("MyClass", "Invalid data");
        return [];
    };
    private _result = _data select 0;
    _result
};
```

### Пример 3: Логирование

```sqf
// Плохо
systemChat "Error occurred";

// Хорошо
errorformat("MyModule", "Error: %1", _errorMsg);
```

## Комментирование кода

### Комментарии для сложной логики

```sqf
// Вычисляем расстояние с учетом рельефа местности
// Используется алгоритм A* для поиска пути
private _distance = [_start, _end] call calculatePathDistance;
```

### Документирование функций

```sqf
/*
    Обрабатывает данные игрока
    
    Parameters:
        _player - объект игрока
        _data - данные для обработки
    
    Returns:
        Boolean - успех операции
*/
func(processPlayerData)
{
    objParams_2(_player, _data);
    // код
};
```

### TODO комментарии

```sqf
// TODO: Оптимизировать этот алгоритм
// TODO: Добавить поддержку новых типов данных
```

## Тестирование кода

### Модульное тестирование

```sqf
#ifdef DEBUG
    // Тестовые функции только в режиме отладки
    testModule_runTests = {
        // тесты
    };
#endif
```

### Проверка граничных условий

```sqf
func(divide)
{
    objParams_2(_a, _b);
    
    if (_b == 0) then {
        errorformat("Math", "Division by zero");
        return 0;
    };
    
    _a / _b
};
```

## Что дальше?

- ➡️ [Особенности синтаксиса](03_SYNTAX_GUIDE.md) - синтаксические особенности
- ➡️ [Модульная система](04_MODULE_SYSTEM.md) - создание модулей по стандартам

