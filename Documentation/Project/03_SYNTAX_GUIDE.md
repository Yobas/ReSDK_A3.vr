# Руководство по синтаксису

Проект использует язык SQF (Scripting Language for Arma 3), предоставляемый Платформой, с расширениями через систему макросов. В этом руководстве описаны особенности синтаксиса для разных частей проекта.

## Общие особенности

### Флаги компиляции

Проект использует условную компиляцию через флаги, определенные в `SETTINGS.h`:

- **`DEBUG`** - режим отладки (включены дополнительные проверки и логи)
- **`RELEASE`** - релизная версия (оптимизации, отключены отладочные функции)
- **`EDITOR`** - режим редактора (доступны редакторские функции)
- **`RBUILDER`** - режим сборки (доступны функции сборки)

### Локальные переменные

В SQF локальные переменные должны начинаться с нижнего подчеркивания `_`:

```sqf
private _localVar = 10;
private _result = _localVar + 5;
```

**Важно:** Всегда объявляйте локальные переменные с модификатором `private`.

### Регистрозависимость

**Функции и переменные:** НЕ регистрозависимы
- `myFunction`, `MYFUNCTION`, `MyFunction` - все это одна и та же функция
- `myVar`, `MYVAR`, `MyVar` - все это одна и та же переменная

**Макросы:** РЕГИСТРОЗАВИСИМЫ
- `callFunc`, `CALLFUNC`, `CallFunc` - разные макросы (если определены)
- Всегда используйте точный регистр, указанный в документации

**OOP система (классы):**
- Имена полей и методов НЕ регистрозависимы
- `getSelf(value)`, `getSelf(VALUE)`, `getSelf(Value)` - обращаются к одному и тому же полю
- `callSelf(myMethod)`, `callSelf(MYMETHOD)` - вызывают один и тот же метод

**Структуры (struct.hpp):**
- Имена полей и методов РЕГИСТРОЗАВИСИМЫ
- `getv(value)`, `getv(VALUE)`, `getv(Value)` - обращаются к РАЗНЫМ полям
- Всегда используйте точный регистр при работе со структурами

### Глобальные переменные

Глобальные переменные должны иметь префикс модуля:

```sqf
// Хорошо
myModule_globalVariable = 10;

// Плохо
globalVariable = 10; // может конфликтовать с другими модулями
```

### Функции модулей

Функции модулей должны иметь префикс модуля:

```sqf
// Хорошо
myModule_myFunction = {
    params ["_param1", "_param2"];
    // код функции
};

// Плохо
myFunction = { // без префикса модуля
    // код
};
```

### Возврат значений из функций

В SQF нет оператора `return`. Для возврата значений используются специальные конструкции:

**Для простого выхода из текущего скоупа (без возвращаемого значения):**
```sqf
// Используйте exitWith для выхода из текущего скоупа
if (condition) exitWith {
    error("Error message");
};
// код продолжается, если condition == false
```

**Для возврата значения из функции:**
```sqf
// Используйте FHEADER в начале функции и RETURN(val) для возврата
myFunction = {
    FHEADER;
    
    if (errorCondition) then {
        RETURN(false);  // Возвращает false и выходит из функции
    };
    
    private _result = someCalculation;
    RETURN(_result);  // Возвращает результат
};
```

**Важно:**
- `FHEADER` определяет скоуп с именем "main"
- `RETURN(val)` использует `breakout` для выхода из скоупа и возврата значения
- Для простого выхода без возвращаемого значения используйте `exitWith {}`
- Никогда не используйте оператор `return` - его нет в SQF

### Работа с null (nil) значениями

В SQF значение `null` (эквивалент `nil`) имеет особое поведение, которое важно учитывать:

**1. null значения пропускаются в операциях:**

```sqf
// Условные операторы - обе ветки никогда не выполнятся!
if (null) then {
    // Этот код НИКОГДА не выполнится
} else {
    // Этот код ТАКЖЕ НИКОГДА не выполнится
};
// Условие null считается ни истинным, ни ложным
```

**2. Добавление null в массивы:**

```sqf
private _array = [];

// ❌ НЕПРАВИЛЬНО - null значение НЕ будет добавлено
_array pushBack null;  // Ничего не произойдет

// ✅ ПРАВИЛЬНО - используйте append для добавления null
_array append [null];  // null будет добавлен в массив

// Также можно создать массив с null напрямую
private _arrayWithNull = [1, null, 2];
```

**3. Проверка на null:**

```sqf
// Проверка переменной на null
if (isNullVar(_myVar)) then {
    // _myVar равен null
};

// Проверка объекта на null reference
if (isNullReference(_obj)) then {
    // _obj является null reference
};

// Универсальная проверка (любые nullable значения)
if (!valid(_value)) then {
    // _value является null, nil, locationnull, objnull, etc.
};
```

**4. Передача null в функции:**

При передаче null значений в функции или макросы, операции могут быть пропущены. Всегда проверяйте значения перед использованием.

```sqf
// Плохо - может пропустить операцию
callFunc(_obj,method);  // Если _obj == null, операция пропустится

// Хорошо - проверка перед использованием
if (!valid(_obj)) exitWith {
    error("Object is null");
};
callFunc(_obj,method);
```

### Макрос `arg` - важная особенность макросов

**Критически важно:** Макросы в SQF - это простой текстовый реплейс (не контекстнозависимый препроцессор).

**Проблема:** Внутри макросов любая запятая воспринимается как разделитель аргументов (nextarg), даже если она находится внутри выражений или строк:

```sqf
// НЕ РАБОТАЕТ - ошибка компиляции
#define add(x,y) x + y
add([1,2] select 0, [2,3] select 0) // запятые внутри select воспринимаются как разделители аргументов!

// НЕ РАБОТАЕТ - запятые в строках затираются
#define msg(text) log(text)
msg("Привет, мир") // запятая в строке будет удалена макропроцессором
```

**Решение:** Используйте макрос `arg` вместо запятой для разделения аргументов макросов:

```sqf
// Правильно - arg заменяется на запятую, но только там где нужно
callFuncParams(this,printData, [1 arg 2 arg 3]);
// После препроцессинга: callFuncParams(this,printData, [1, 2, 3]);

// arg можно использовать в любых выражениях
private _result = [1 arg 2 arg 3] select 0; // работает корректно
```

**Важно:** В макросах OOP (например, `callFuncParams`, `getVar`, `setVar`, `getSelf`, `setSelf`) второй аргумент (имя метода/поля) **не должен содержать пробелов до или после запятой**. Макрос использует строкификацию (`#memberName`), которая захватывает все символы между запятыми, включая пробелы. Это приведет к обращению к неверному идентификатору (например, `" printData "` вместо `"printData"`).

```sqf
// Неправильно - пробелы до или после запятой
callFuncParams(this,printData , [1 arg 2 arg 3]); // будет обращение к методу "printData "
callFuncParams(this, printData, [1 arg 2 arg 3]); // будет обращение к методу " printData"
callFuncParams(this,printData  , [1 arg 2 arg 3]); // будет обращение к методу "printData  "

// Правильно - без пробелов до и после запятой
callFuncParams(this,printData, [1 arg 2 arg 3]); // корректное обращение к методу "printData"
```

**Важно понимать:**
- `arg` - это макрос, который заменяется на запятую во время препроцессинга
- Используйте `arg` везде, где запятая является частью синтаксиса макроса или может быть неправильно интерпретирована
- Внутри строк запятые все равно будут удалены макропроцессором - избегайте запятых в строковых литералах в макросах

## Особенности синтаксиса для host

Серверный код имеет доступ ко всем макросам и системам проекта.

### Макросы логирования из engine.hpp

```sqf
#include "engine.hpp"

// Простое логирование
log("Сообщение");
// logformat принимает только 2 аргумента: формат-строку и значения для форматирования
// Формат-строка может включать префикс модуля/класса как часть строки (опционально)
logformat("Validated %1", _ext);
// Если нужно несколько значений, используйте arg для объединения их во второй аргумент:
logformat("vs::init() - voip system: %1; api version: %2", apiRequest(REQ_GET_VERSION) arg vs_apiversion);
// С префиксом класса/метода:
logformat("[ServerClient::DEBUG]: client:%1<%2>", getSelf(id) arg getSelf(name));

// Предупреждения
warning("Предупреждение");
warningformat("Object %1 (%2) already not flying", callSelf(getClassName) arg getSelf(pointer));
warningformat("No main action defined - %1", callSelf(getClassName));

// Ошибки
error("Критическая ошибка");
errorformat("GameObject::getBasicLoc() - Cant find basic loc for object %1 (%2)", this arg getSelf(pointer));
errorformat("sound3d::play() - Unknown type source object %1", tolower typename _source);

// Трассировка (только в DEBUG)
trace("Трассировка выполнения");
traceformat("CHUNKS COUNT: %1", count _chObjList);
traceformat("CHECK OPENSPACE %1 - %2", _x arg callFunc(_x,isInOpenSpace));
```

### OOP система из oop.hpp

**Важно о макросах OOP:** 
- Имена членов (полей и методов) в макросах OOP должны быть известны на этапе компиляции. Если имя строится динамически во время выполнения, используйте Reflect-версии макросов.
- **Регистрозависимость:** В OOP системе имена полей и методов НЕ регистрозависимы. `getSelf(value)`, `getSelf(VALUE)`, `getSelf(Value)` - все обращаются к одному и тому же полю.

```sqf
// Статическое имя - используйте обычные макросы
getSelf(myField)  // Корректно
setVar(obj,myField,value)  // Корректно - БЕЗ пробелов после запятых!
callSelf(getValue)  // Корректно

// Динамическое имя - используйте Reflect-версии
private _category = "weapon";
callSelfReflect("get" + _category + "sound");  // Имя метода строится динамически

private _fieldName = "contenders_" + str _priority;
getVarReflect(_roleData, _fieldName);  // Имя поля строится динамически

private _skillName = "get" + _strSkill;
callSelfReflect(_skillName);  // Имя метода из переменной
```

#### Объявление класса

```sqf
#include "oop.hpp"

class(MyClass) extends(BaseClass)
    // Поля класса
    var(myNumber, 10);
    var(myString, "");
    var(myArray, []);
    var(myBool, false);
    
    // Методы класса
    func(initialize)
    {
        objParams();
        setSelf(myNumber,0);
        setSelf(myString,"Initialized");
    };
    
    func(processData)
    {
        objParams_1(_data);
        private _result = getSelf(myNumber) + _data;
        setSelf(myNumber,_result);
        _result
    };
    
endclass
```

#### Работа с объектами

```sqf
// Создание объекта
private _obj = new(MyClass);

// Вызов методов
private _result = callFunc(_obj,processData,[5]);

// Получение полей
private _num = getVar(_obj,myNumber);  // Имя поля БЕЗ пробелов после запятых!

// Установка полей
setVar(_obj,myNumber,20);  // Имя поля БЕЗ пробелов после запятых!

// Вызов метода с параметрами
callFuncParams(_obj,processData,[10]);  // Имя метода БЕЗ пробелов после запятых!

// Вызов метода без параметров
callSelf(initialize); // внутри метода класса - имя БЕЗ пробелов после запятых!

// Вызов с параметрами внутри метода
callSelfParams(processData,[15]);  // Имя метода БЕЗ пробелов после запятых!

// ВАЖНО: Если имя строится динамически, используйте Reflect-версии
private _category = "weapon";
callSelfReflect("get" + _category + "sound");  // Динамическое имя метода
getVarReflect(_roleData, "contenders_" + str _priority);  // Динамическое имя поля
callSelfReflectParams("add" + _stat, [+_amount]);  // Динамическое имя метода с параметрами
```

#### Параметры методов

```sqf
// Без параметров
func(myMethod)
{
    objParams();
    // код
};

// С одним параметром
func(myMethod)
{
    objParams_1(_param1);
    // код
};

// С несколькими параметрами
func(myMethod)
{
    objParams_3(_param1,_param2,_param3);  // БЕЗ пробелов после запятых!
    // код
};
```

#### Getter и Setter

```sqf
// Простой getter
getter_func(getValue, getSelf(value));

// Константный getter (без objParams)
// Для массивов используйте arg вместо запятых!
getterconst_func(getArray, [1 arg 2 arg 3]);
getterconst_func(getEmptyArray, []);  // Пустой массив
getterconst_func(getStringArray, ["дитя" arg "юноша" arg "взрослый"]);

// Getter с вычислением
getter_func(getSum, getSelf(a) + getSelf(b));
```

#### Наследование и вызов базового класса

```sqf
class(ChildClass) extends(ParentClass)
    func(overrideMethod)
    {
        objParams();
        // Вызов базового метода - просто super()
        super();  // Просто вызывает базовый метод
        // Или используем результат базового метода:
        private _result = super() + " дополнительный текст";
        _result
    };
endclass
```

### Структуры данных (struct.hpp)

**Важно о регистрозависимости:** В структурах имена полей и методов РЕГИСТРОЗАВИСИМЫ. `getv(value)`, `getv(VALUE)`, `getv(Value)` - обращаются к РАЗНЫМ полям. Всегда используйте точный регистр при работе со структурами.

```sqf
#include "struct.hpp"

// Объявление структуры
struct(MyStruct)
    def(count) 0;  // Числовое поле с значением по умолчанию
    def(name) "";   // Строковое поле с значением по умолчанию
    def(items) [];  // Массив
    
    def(init)  // Метод инициализации
    {
        params [["_count",self getv(count)],["_name",self getv(name)]];
        self setv(count,_count);
        self setv(name,_name);
    }
    
    def(getCount)  // Метод для получения значения
    {
        self getv(count)
    }
endstruct

// Создание структуры
private _struct = struct_new(MyStruct);  // Без параметров
private _struct2 = struct_newp(MyStruct,[10 arg "Test"]);  // С параметрами для init

// Использование полей извне
private _count = _struct getv(count);  // Получение значения
_struct setv(count,20);  // Установка значения
_struct setv(name,"NewName");

// Вызов методов структуры
private _countValue = _struct callv(getCount);  // Вызов метода без параметров
_struct callp(init,30 arg "NewName");  // Вызов метода с параметрами через callp
```

### Загрузка модулей

```sqf
// Загрузка серверного модуля
loadFile("src\host\MyModule\MyModule_init.sqf");
```

## Особенности синтаксиса для client

Клиентский код имеет ограничения и использует специальную систему загрузки модулей.

### Загрузка модулей через importClient()

```sqf
// В client/loader.hpp
importClient("src\client\MyModule\MyModule_init.sqf");
```

Макрос `importClient()` автоматически:
- Компилирует модуль
- Добавляет его в массив `allClientContents`
- Выполняет код сразу (если возможно)

### Ограничения клиентского кода

1. **Нет доступа к OOP системе** - клиент не использует классы из `oop.hpp`, так как рантайм OOP объявлен на стороне сервера
2. **Ограниченный доступ к серверным данным** - только через RPC
3. **UI через WidgetSystem** - все интерфейсы создаются через систему виджетов

### Работа с RPC

```sqf
#include "clientRpc.hpp"  // На клиенте
// или
#include "serverRpc.hpp"  // На сервере

// Регистрация обработчика на клиенте
// Второй аргумент - ссылка на функцию (переменная с кодом или глобальная функция)
private _myHandler = {
    params ["_data"];
    logformat("MyModule: Received data: %1", _data);
};
rpcAdd("myAction", _myHandler);  // Передаем ссылку на функцию, не inline код

// Вызов обработчика на клиенте
// Второй аргумент - массив, используйте arg для нескольких значений
rpcCall("myAction", [myData]);
rpcCall("myAction", [data1 arg data2 arg data3]);  // Несколько аргументов

// На сервере: регистрация обработчика
rpcAdd("serverAction", _serverHandler);

// Отправка с сервера на клиент
// Третий аргумент - массив, используйте arg для нескольких значений
rpcSendToClient(_clientOwner, "clientAction", [serverData]);
rpcSendToClient(_clientOwner, "clientAction", [data1 arg data2]);
```

### Работа с WidgetSystem

```sqf
#include "widgets.hpp"

// ВАЖНО: Сначала нужно получить display для создания виджетов
// Есть три способа получить display:

// 1. Открыть обычный дисплей (если еще не открыт)
// Обычный дисплей блокирует управление персонажем
private _display = call displayOpen;

// 2. Открыть динамический дисплей
// Динамический дисплей НЕ блокирует управление персонажем
private _display = call dynamicDisplayOpen;

// 3. Получить существующий GUI из uinamespace (если GUI уже создан)
private _display = getGUI;  // Макрос, получает GUI из uinamespace

// Проверка, открыт ли дисплей
if (isDisplayOpen) then {
    // Дисплей уже открыт
    private _display = getDisplay;  // Получить текущий открытый дисплей
};

// Создание виджета
// Параметры: display, тип виджета, позиция [x, y, width, height], опционально родитель
private _textWidget = [_display, TEXT, [10, 10, 80, 20]] call createWidget;
_textWidget ctrlSetStructuredText parseText "<t size='1.2'>Текст виджета</t>";

private _buttonWidget = [_display, BUTTON, [10, 40, 30, 10]] call createWidget;
_buttonWidget ctrlSetText "Кнопка";
_buttonWidget ctrlAddEventHandler ["MouseButtonUp", {
    log("Кнопка нажата");
}];

// Показать/скрыть виджет
_textWidget ctrlShow true;   // Показать
_textWidget ctrlShow false;  // Скрыть

// Удалить виджет
_textWidget call deleteWidget;
```

## Особенности синтаксиса для editor

Код редактора использует компонентную систему инициализации и специальные макросы для объявления функций.

### Компонентная инициализация

```sqf
#include "EditorEngine.h"

// Инициализация компонента
componentInit(MyComponent)
#include "MyComponent\MyComponent_init.sqf"
```

Макрос `componentInit()` автоматически логирует загрузку компонента.

### Объявление редакторных функций

Редакторные функции объявляются через специальный макрос `function()`:

```sqf
#include "EditorEngine.h"

// Обычная редакторная функция
function(myFunction)
{
    params ["_param1", "_param2"];
    logformat("Editor: Function called with %1 and %2", _param1 arg _param2);
    // код функции
}

// Функция инициализации - вызывается автоматически при загрузке редактора
init_function(myInitFunction)
{
    log("Editor component initialized");
    // код инициализации
}
```

**Важно:**
- `function(name)` - объявляет обычную функцию редактора
- `init_function(name)` - объявляет функцию инициализации, которая автоматически вызывается при загрузке редактора (добавляется в `functions_list_init`)
- Функции доступны глобально в редакторе

### Система событий редактора

Редактор использует систему событий для связи между компонентами:

```sqf
#include "EditorEngine.h"

// Регистрация обработчика события
["onDisplayOpen", {
    params ["_display"];
    logformat("Editor: Display opened: %1", _display);
}] call Core_addEventHandler;

// Регистрация нескольких обработчиков для одного события
private _handler1 = { log("Handler 1"); };
private _handler2 = { log("Handler 2"); };
["onDisplayClose", [_handler1, _handler2]] call Core_addEventHandler;

// Вызов события (обычно делается внутри системы редактора)
["onDisplayOpen", [currentDisplay]] call Core_callEvent;
```

**Типичные события редактора:**
- `onDisplayOpen` - при открытии дисплея
- `onDisplayClose` - при закрытии дисплея
- `onMouseAreaPressed` - при нажатии на область мыши

### Редакторские макросы

```sqf
// Проверка, что код выполняется в редакторе
#ifdef EDITOR
    log("Этот код выполняется только в редакторе");
#endif

// Проверка режима редактора
if (is3DEN) then {
    // код для редактора 3DEN
};
```

### Работа с визуальными компонентами

Визуальные компоненты - это система для редакторов с предпросмотром 3D объектов в редакторе карт. Используются для редактирования позиций моделей, частиц, освещения и других визуальных элементов.

```sqf
#include "EditorEngine.h"

// Открытие редактора относительных позиций модели
function(openRelativePositionEditor)
{
    params ["_modelObject"];
    // Открыть окно визуального редактора
    ["relpos"] call vcom_openWindow;
    // Или напрямую через функцию редактора
    [_modelObject] call vcom_relposEditorOpen;
}

// Открытие редактора частиц и освещения
function(openParticleEditor)
{
    params ["_object"];
    [_object] call vcom_emit_createVisualWindow;
}
```

**Примечание:** Визуальные компоненты - это специализированная система для работы в редакторе карт 3DEN и требуют глубокого понимания архитектуры редактора.

## Общие паттерны

### Проверка на null

```sqf
// Проверка переменной
if isNullVar(_var) then {
    _var = defaultValue;
};

// Проверка объекта
if isNullReference(_obj) then {
    // объект null
};

// Проверка валидности (C++ стиль)
if (valid(_ptr)) then {
    // указатель валидный
};
```

### Работа с массивами и параметрами

```sqf
// Использование arg для параметров в макросах
logformat("array: %1",[1 arg 2 arg 3]);

// Объявление параметров функции
fncname = {
    params ["_a", "_b", "_c"];
    // код функции
};
[1,2,3] call fncname;

// Распаковка параметров массива
[1,2,3] params ["_a", "_b", "_c"];

// Параметры со значением по умочанию
params ["_required", ["_optional", 123]];
```

### Условная компиляция

```sqf
#ifdef DEBUG
    log("Отладочное сообщение");
#endif

#ifndef RELEASE
    // код для не-релизной версии
#endif

#ifdef EDITOR
    // код только для редактора
#endif
```

### Обработка ошибок

```sqf
// Аварийное завершение
appExit(APPEXIT_REASON_CRITICAL);

// Проверка с сообщением
if (condition) then {
    errorformat("ClientInit: DLL validation error: %1", _signDLLErrorMessage);
    appExit(APPEXIT_REASON_RUNTIMEERROR);
};
```

### Форматирование строк

```sqf
// Использование format (вне макросов - обычные запятые)
private _msg = format["Значение: %1, Имя: %2", _value, _name];

// С logformat - макрос принимает только 2 аргумента: формат-строку и значения
// Формат-строка может включать префикс модуля/класса (опционально)
// Одно значение - просто передаем:
logformat("Validated %1", _ext);
// Несколько значений - используйте arg для объединения их во второй аргумент:
logformat("vs::init() - voip system: %1; api version: %2", apiRequest(REQ_GET_VERSION) arg vs_apiversion); 

// В макросах избегайте запятых в выражениях
// Одно значение - без arg
#define myLog(msg,val) logformat("[Module] " + msg, val)
myLog("Value: %1", 42); // Корректно

// Несколько значений - используйте arg
#define myLogMulti(msg,val1,val2) logformat("[Module] " + msg, val1 arg val2)
myLogMulti("Values: %1, %2", 42, 100); // Корректно но запятая в строке будет удалена макропроцессором!
```

## Работа с файлами

### Загрузка файлов

```sqf
// Загрузка и компиляция файла
loadFile("src\host\MyModule\code.sqf");

// Проверка существования файла
if (fileExists("path\to\file.sqf")) then {
    // файл существует
};
```

## Типичные ошибки

### Неправильное использование переменных

```sqf
// Плохо - переменная без префикса
myVar = 10;

// Хорошо - с префиксом модуля
myModule_myVar = 10;
```

### Пропущенный objParams()

```sqf
// Плохо
func(myMethod)
{
    // нет objParams() - this не определен
    private _val = getSelf(value);
};

// Хорошо
func(myMethod)
{
    objParams();
    private _val = getSelf(value);
};
```

### Неправильное использование callSelf

```sqf
// Плохо - вызов вне контекста объекта
callSelf(myMethod); // this не определен

// Хорошо - внутри метода класса
func(anotherMethod)
{
    objParams();
    callSelf(myMethod); // this определен через objParams()
};
```

## Что дальше?

- ➡️ [Модульная система](04_MODULE_SYSTEM.md) - создание новых модулей
- ➡️ [Стандарты кодирования](09_CODING_STANDARDS.md) - подробные правила кодирования

