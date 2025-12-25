# Расширенные стандарты кодирования

Этот документ расширяет базовый [CODE-STANDARDS.md](../../CODE-STANDARDS.md) с дополнительными паттернами, анти-паттернами и особенностями работы с проектом.

## Базовые стандарты

См. [CODE-STANDARDS.md](../../CODE-STANDARDS.md) для базовых стандартов:
- Отступы и форматирование
- Именование
- Комментарии
- Объявления переменных и функций

### Использование макросов для сравнений

В проекте используются макросы вместо встроенных операторов Платформы для сравнений:
- Используйте `equals(a, b)` вместо `a isEqualTo b`
- Используйте `not_equals(a, b)` вместо `!a isEqualTo b`
- Используйте `equalTypes(a, b)` вместо `a isEqualType b`
- Используйте `not_equalTypes(a, b)` вместо `!a isEqualType b`

**Причины:**
- Макросы обеспечивают единообразие кода
- Макросы могут иметь дополнительные проверки и оптимизации
- Встроенные операторы Платформы (`isEqualTo`, `isEqualType`) не используются в продакшене

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
            setSelf(privateData,_value);
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
private _value = getVar(_obj,privateField);

// Хорошо - через публичный метод
private _value = callFunc(_obj,getPrivateField);
```

### Динамическое создание полей в конструкторе

```sqf
// Плохо - поле создается "на лету" в конструкторе
class(MyClass) extends(BaseClass)
    func(constructor)
    {
        objParams();
        setSelf(dynamicField, 123);  // Плохо: поле не объявлено явно
    };
endclass

// Хорошо - все поля объявлены явно в теле класса
class(MyClass) extends(BaseClass)
    var(dynamicField, 0);  // Хорошо: поле объявлено явно
    
    func(constructor)
    {
        objParams();
        setSelf(dynamicField, 123);  // Хорошо: устанавливаем значение уже объявленного поля
    };
endclass
```

**Почему это важно:**
- Структура класса должна быть видна сразу при чтении кода - все поля объявлены в одном месте
- Наследники класса могут полагаться на наличие определенных полей, которые должны быть явно объявлены
- Статический анализ и понимание кода становится невозможным, если поля определяются динамически
- Легко допустить ошибку, пытаясь использовать поле, которое еще не было инициализировано
- Усложняется отладка и поддержка кода - сложно понять, какие поля должны существовать у объекта

**Правило:** Все поля класса должны быть объявлены через `var()` в теле класса (между `class()` и `endclass`). `var()` и `func()` могут использоваться **только в теле класса**, но **не внутри методов**.

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
// Плохо - слепое добавление проверок везде без понимания контекста
myFunction = {
    params ["_obj"];
    // Неоправданная проверка - если _obj всегда валиден в этом контексте
    if (!valid(_obj)) then {
        error("MyModule: Invalid object");
    };
    callFunc(_obj,doSomething);
};

// Хорошо - проверка только там, где это действительно необходимо
myFunction = {
    params ["_obj"];
    // Проверка оправдана, если _obj может быть null (например, из внешнего источника)
    if (!valid(_obj)) exitWith {
        error("MyModule: Invalid object");
    };
    callFunc(_obj,doSomething);
};

// Также хорошо - если разработчик понимает, что _obj гарантированно валиден
myFunction = {
    params ["_obj"];  // _obj создан выше в коде и гарантированно валиден
    callFunc(_obj,doSomething);  // Проверка не нужна
};
```

**Важно:** Разработчик должен понимать контекст и добавлять проверки на null только там, где это действительно необходимо. Слепое добавление проверок везде - это признак непонимания кода и приводит к раздуванию кодовой базы без реальной пользы. Проверяйте объекты только в точках входа (публичные методы, обработчики событий, RPC), где данные могут прийти извне, а не в каждом внутреннем вызове.

## Особенности работы с OOP системой

### Использование наследования

```sqf
// Базовый класс
class(BaseItem) extends(Item)
    func(use)
    {
        objParams();
        log("BaseItem: Used");
    };
endclass

// Наследник
class(SpecificItem) extends(BaseItem)
    func(use)
    {
        objParams();
        // Вызов базового метода
        callSuper(BaseItem,use);
        // Дополнительная логика
        log("SpecificItem: Special action");
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
callFunc(_obj,doSomething);

// Удаление объекта
delete(_obj);
```

### Автоматическая очистка ресурсов

Используйте `autoref` для автоматической очистки ссылок на объекты при удалении объекта.

**Важное ограничение:** `autoref` работает **только для классов, унаследованных от `ManagedObject`**. В проекте все игровые объекты (`GameObject`, `Item`, `IStruct`, `Decor`, `BasicMob`, `Mob` и т.д.) наследуются от `ManagedObject`, поэтому для них `autoref` доступен.

**Принцип работы:**
1. `autoref` - это модификатор, который помечает поле как автоматически очищаемое
2. При регистрации класса система собирает список всех полей с модификатором `autoref` в `__autoref_list`
3. При удалении объекта (вызов `delete()`) деструктор `ManagedObject` проверяет метод `enableAutoRefGC()` (возвращает `true` по умолчанию)
4. Если `enableAutoRefGC` включен, система проходит по всем полям из `__autoref_list` и очищает их в зависимости от типа:
   - **Массив объектов (`ARRAY`)**: Удаляет все объекты из массива через `delete()`, затем устанавливает массив в `["<AUTOREF_NAN>"]`
   - **Одиночный объект (`nullPtr` / `Location`)**: Удаляет объект через `delete()`
   - **Handle обновления (`SCALAR` > -1)**: Останавливает обновление через `stopUpdate()`

**Что очищает:**
- Ссылки на другие OOP объекты (одиночные или в массивах)
- Handles обновлений (`startUpdate`), чтобы предотвратить выполнение кода после удаления объекта

**Где используется в проекте:**
- **Оружие (`RangedWeapon.sqf`)**: Патрон в патроннике (`bullet`) и магазин (`magazine`) - при удалении оружия автоматически удаляются связанные патрон и магазин
- **Магазин (`Magazines.sqf`)**: Массив патронов (`content`) - при удалении магазина автоматически удаляются все патроны из массива
- **Ловушки (`ITrapItem`)**: Структура ловушки (`trapStruct`) - при удалении предмета-ловушки автоматически удаляется структура ловушки на карте
- **Части тела (`Bodyparts.sqf`)**: Наложенный бинт (`bandage`) - при удалении части тела автоматически удаляется наложенный на неё бинт
- **Сетевые дисплеи (`Mob.sqf`, `BasicMob.sqf`)**: Ссылки на открытые дисплеи - автоматически закрываются при удалении персонажа
- **Handles обновлений**: Handles от `startUpdate()` - автоматически останавливаются при удалении объекта, предотвращая выполнение кода после удаления

**Синтаксис:**
```sqf
// Пример 1: Одиночный объект (из RangedWeapon.sqf)
class(RangedWeapon) extends(Item)  // Item наследуется от GameObject -> ManagedObject
    autoref var(bullet, nullPtr);  // Патрон в патроннике - будет удален при удалении оружия
    autoref var(magazine, nullPtr);  // Магазин - будет удален при удалении оружия
endclass

// Пример 2: Массив объектов (из Magazines.sqf)
class(Magazine) extends(Item)
    autoref var(content, []);  // Массив патронов - все патроны будут удалены при удалении магазина
endclass

// Пример 3: Handle обновления (из Campfires.sqf)
class(Campfire) extends(IStruct)
    autoref var(handleUpdate, -1);  // Handle обновления - будет остановлен при удалении костра
endclass
```

**Ограничения и предупреждения:**
- **Критично:** Нельзя использовать для массивов, которые изменяются при удалении объектов (например, содержимое контейнера). Если объект в массиве при удалении вызывает методы, которые изменяют сам массив (например, `removeItem`), это приведет к смещению индексов и утечкам памяти. В таких случаях нужно использовать явную очистку в деструкторе:
```sqf
func(destructor)
{
    objParams();
    {
        delete(_x);
    } foreach array_copy(getSelf(content));  // Явная очистка с копированием массива
};
```
- `autoref` не работает для классов, которые не наследуются от `ManagedObject` (например, обычный `object`)

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
    callFunc(_obj,process, [_x]);
    delete(_obj);
} forEach _largeArray;

// Хорошо - переиспользование объекта
private _processor = new(MyClass);
{
    callFunc(_processor,process, [_x]);
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

callFunc(_obj,criticalMethod);
```

## Сетевые взаимодействия и безопасность

### Валидация данных от клиента

```sqf
#include "serverRpc.hpp"  // На сервере

// На сервере - всегда валидируйте данные от клиента
private _clientActionHandler = {
    params ["_clientData"];
    
    // Валидация
    if (not_equalTypes(_clientData, [])) exitWith {
        error("Security: Invalid data from client");
    };
    
    // Обработка
};
rpcAdd("clientAction", _clientActionHandler);
```

### Проверка прав доступа

```sqf
// Проверяйте права перед выполнением действий
func(adminAction)
{
    objParams();
    
    if (!callFunc(_user,isAdmin)) exitWith {
        error("Security: Access denied");
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
    if (not_equalTypes(_input, "")) exitWith {
        error("Invalid input type");
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
setVar(_obj,value, 10);
private _val = getVar(_obj,value);
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
    FHEADER;
    if (not_equalTypes(_data, []) || count _data == 0) then {
        error("MyClass: Invalid data");
        RETURN([]);
    };
    private _result = _data select 0;
    RETURN(_result);
};
```

### Пример 3: Логирование

```sqf
// Плохо
systemChat "Error occurred";

// Хорошо
errorformat("MyModule: Error: %1", _errorMsg);
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
    
    FHEADER;
    if (_b == 0) then {
        error("Math: Division by zero");
        RETURN(0);
    };
    
    _a / _b
};
```

## Что дальше?

- ➡️ [Особенности синтаксиса](03_SYNTAX_GUIDE.md) - синтаксические особенности
- ➡️ [Модульная система](04_MODULE_SYSTEM.md) - создание модулей по стандартам

