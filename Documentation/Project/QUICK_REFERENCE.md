# Быстрый справочник макросов и функций

Краткий справочник основных макросов и функций проекта для быстрого доступа. Детальные описания см. в [Особенностях синтаксиса](03_SYNTAX_GUIDE.md) и [Структуре проекта](02_PROJECT_STRUCTURE.md).

## Макросы из engine.hpp

### Логирование

```sqf
log(message)                                    // Простое логирование (1 аргумент - строка)
logformat(formatText, values)                   // Логирование с форматированием (2 аргумента: строка с %1/%2 и значения)
warning(message)                                // Предупреждение (1 аргумент - строка)
warningformat(formatText, values)               // Предупреждение с форматированием (2 аргумента: строка и значения)
error(message)                                  // Ошибка (1 аргумент - строка)
errorformat(formatText, values)                 // Ошибка с форматированием (2 аргумента: строка и значения)
trace(message)                                  // Трассировка (только в DEBUG, 1 аргумент - строка)
traceformat(formatText, values)                 // Трассировка с форматированием (только в DEBUG, 2 аргумента: строка и значения)
```

**Важно:** 
- Используйте `log()` для простых сообщений без форматирования
- Используйте `logformat()` только когда нужна подстановка значений через `%1`, `%2` и т.д.
- `logformat` всегда требует 2 аргумента: строку с форматированием (префикс модуля может быть частью строки) и значения. Для нескольких значений используйте `arg`:
```sqf
logformat("MyModule: Values: %1, %2", value1 arg value2);
// или без префикса:
logformat("Values: %1, %2", value1 arg value2);
```

### Breakpoints

```sqf
breakpoint_setfile(filename)                    // Установить файл для breakpoints
breakpoint(data)                                // Точка останова (только в DEBUG)
```

### Утилиты для переменных

```sqf
null                                            // nil (псевдоним)
isNull(val)                                     // Проверка на null
isNullReference(obj)                            // Проверка ссылки на null
isNullVar(var)                                  // Проверка переменной на null
isNullPtr(obj)                                  // Проверка указателя на null
valid(ptr)                                      // Проверка валидности (C++ стиль)
toBoolean(val)                                  // Приведение к boolean
defIsNull(_v,_defval)                           // Значение по умолчанию
outRef(var,def)                                 // Распаковка ссылки
```

### Работа с массивами

```sqf
array_exists(arr,var)                           // Проверка существования элемента
array_copy(array)                               // Копирование массива
array_remove(array,el)                          // Удаление элемента
array_selectlast(arr)                           // Выбор последнего элемента
array_isempty(arr)                              // Проверка на пустоту
array_count(arr)                                // Количество элементов
MODARR(var,index,modif)                         // Модификация элемента массива
```

### Векторы

```sqf
vec1(x)                                         // 1D вектор
vec2(x,y)                                       // 2D вектор
vec3(x,y,z)                                     // 3D вектор
vec4(x,y,w,h)                                   // 4D вектор (обычно для координат)
```

### HashMap и HashSet

```sqf
createHashMap                                   // Создание HashMap (нативная функция)
createHashMapFromArray                          // Создание HashMap из массива (нативная функция)
hashSet_createEmpty()                           // Создание пустого HashSet
hashSet_toArray(hash)                           // Преобразование HashSet в массив
hashSet_exists(hash,item)                       // Проверка существования в HashSet
```

### Математические утилиты

```sqf
clamp(val,min,max)                              // Ограничение значения
clampangle(x,a,b)                               // Ограничение угла
inRange(numberToCheck,bottom,top)               // Проверка диапазона
rand(_beg,_end)                                 // Случайное число (включительно)
randInt(_beg,_end)                              // Случайное целое число (включительно)
prob(val)                                       // Вероятностная проверка (старая, не рекомендуется)
prob_new(val)                                   // Вероятностная проверка (новая, рекомендуется)
pow(a,b)                                        // Возведение в степень
precentage(checked,precval)                     // Процент от числа
formatTime(secs)                                // Форматирование времени
t_asMin(s)                                      // Секунды в минуты
t_asHrs(s)                                      // Секунды в часы
getdiff(a,b)                                    // Разница между значениями
```

### Система задержек и обновлений

```sqf
startUpdate(func,delay)                         // Запустить периодическое обновление
startUpdateParams(func,delay,params)            // Запустить обновление с параметрами
stopUpdate(handle)                              // Остановить обновление
nextFrame(code)                                 // Выполнить на следующем кадре
nextFrameParams(code,args)                      // Выполнить на следующем кадре с параметрами
invokeAfterDelay(code,delay)                    // Выполнить с задержкой
invokeAfterDelayParams(code,delay,params)       // Выполнить с задержкой с параметрами
asyncInvoke(cond,state,args,timeout,tim)       // Асинхронное выполнение с условием
tickTime                                        // Время в секундах (diag_tickTime)
netTickTime                                     // Время миссии (CBA_missionTime)
deltaTime                                       // Дельта времени (diag_deltaTime)
```

### Языковые расширения

```sqf
ifcheck(val,trueval,falseval)                   // Тернарный оператор
RETURN(val)                                     // Возврат из области видимости (требует FHEADER)
FHEADER                                         // Заголовок функции (определяет скоуп "main")
IF(val)                                         // if с then
IF_EXIT(val)                                    // if exitWith
```

**Важно о RETURN и FHEADER:**
- В SQF нет оператора `return`
- Для возврата значения используйте `FHEADER` в начале функции и `RETURN(val)` для возврата
- Для простого выхода без значения используйте `exitWith {}`

```sqf
// Пример функции с возвращаемым значением
myFunction = {
    FHEADER;
    if (errorCondition) then {
        RETURN(false);
    };
    RETURN(result);
};

// Пример простого выхода
if (condition) exitWith {
    error("Error");
};
```
FOR(init,start,end)                             // for цикл
WHILE(cond)                                     // while цикл
SWITCH(cond)                                    // switch
CASE(cond)                                      // case
fswitch(val)                                    // Функциональный switch
fcase(val)                                      // Функциональный case
fcasein(values)                                 // Функциональный case для массива
equals(obja,objb)                               // Сравнение значений (используйте вместо isEqualTo)
not_equals(obja,objb)                           // Неравенство значений
equalTypes(obja,objb)                           // Сравнение типов (используйте вместо isEqualType)
not_equalTypes(obja,objb)                       // Неравенство типов
```

### Загрузка модулей

```sqf
loadFile(path)                                  // Загрузить серверный модуль
importClient(path)                              // Импортировать клиентский модуль
importCommon(path)                              // Импортировать общий компонент
```

### Ассерты

```sqf
assert(condition)                               // Runtime ассерт (отключается в RELEASE)
assert_str(condition,message)                   // Runtime ассерт с сообщением
static_assert(condition)                        // Compile-time ассерт
static_assert_str(condition,message)            // Compile-time ассерт с сообщением
```

### Управление приложением

```sqf
appExit(exitCode)                               // Аварийное завершение
OBSOLETE(funcname)                              // Маркер устаревшей функции
NOTIMPLEMENTED(funcname)                        // Маркер нереализованной функции
```

### Утилиты для указателей

```sqf
generatePtr()                                   // Генерация строкового указателя
criptPtr(val)                                   // Обфускация указателя (отключается в DEBUG)
mem_alloc()                                     // Выделение памяти (location объект)
mem_set(ptr)                                    // Установка значения в память
mem_get(ptr,val)                                // Получение значения из памяти
mem_free(ptr)                                   // Освобождение памяти
```

### Утилиты для ссылок

```sqf
refcreate(value)                                // Создание ссылки
refget(val)                                     // Получение значения из ссылки
refset(ref,newvalue)                            // Установка значения в ссылку
```

### Утилиты для файлов

```sqf
fileExists(file)                                // Проверка существования файла
SHORT_PATH                                      // Сокращенный путь файла (удаление префикса миссии)
SHORT_PATH_CUSTOM(path)                         // Сокращенный путь (кастомный)
```

## Макросы OOP системы (oop.hpp)

### Объявление класса

```sqf
class(name)                                     // Начало объявления класса
extends(parentClass)                            // Наследование
endclass                                        // Конец объявления класса
```

### Объявление полей

```sqf
var(name,value)                                 // Базовое объявление поля
var_num(name)                                   // Числовое поле (0)
var_str(name)                                   // Строковое поле ("")
var_bool(name)                                  // Логическое поле (false)
var_array(name)                                 // Массив ([])
var_obj(name)                                   // Объект (objnull)
var_vobj(name)                                  // Виртуальный объект (locationnull)
var_hashmap(name)                               // HashMap (createHashMap)
varpair(name,pairs)                             // HashMap поле из пар
var_exprval(name,expr)                          // Поле с вычисляемым значением
autoref                                         // Модификатор автоматической очистки ссылок
```

### Объявление методов

```sqf
func(name)                                      // Объявление метода
getter_func(name,code)                          // Getter метод (с objParams())
getterconst_func(name,code)                     // Константный getter (без objParams())
abstract_func(name)                             // Абстрактный метод
proto_func(name)                                // Прототипный метод (пустая реализация)
verbList(list,parentClass)                      // Генерация метода getVerbs()
```

### Параметры методов

```sqf
objParams()                                     // Без параметров (устанавливает this)
objParams_1(a)                                  // С одним параметром
objParams_2(a,b)                                // С двумя параметрами
objParams_3(a,b,c)                              // С тремя параметрами
this                                            // Ссылка на текущий объект
```

### Работа с полями объекта

```sqf
getSelf(name)                                   // Получить значение поля
setSelf(name,val)                               // Установить значение поля
modSelf(name,val)                               // Модифицировать поле (например, +1)
initSelf(name,initial)                          // Инициализировать поле, если null (не используется в продакшене)
getSelfReflect(name)                            // Runtime версия getSelf
setSelfReflect(name,val)                        // Runtime версия setSelf
```

### Вызов методов объекта

```sqf
callSelf(methodName)                            // Вызов метода без параметров
callSelfParams(methodName,params)               // Вызов метода с параметрами
callSelfAfter(methodName,delay)                 // Отложенный вызов
callSelfReflect(methodName)                     // Runtime версия
getSelfFunc(methodName)                         // Получить ссылку на метод
```

### Вызов методов внешнего объекта

```sqf
callFunc(obj,methodName)                        // Вызов метода объекта
callFuncParams(obj,methodName,params)           // Вызов метода с параметрами
callFuncAfter(obj,methodName,delay)             // Отложенный вызов
getFunc(obj,methodName)                         // Получить ссылку на метод
```

### Работа с полями внешнего объекта

```sqf
getVar(obj,name)                                // Получить значение поля
setVar(obj,name,value)                          // Установить значение поля
modVar(obj,name,val)                            // Модифицировать поле
initVar(obj,name,initial)                       // Инициализировать поле (не используется в продакшене)
getVarReflect(obj,name)                         // Runtime версия getVar
```

### Наследование

```sqf
super()                                         // Автоматический вызов базового метода (рекомендуется)
callSuper(parentClass,methodName)               // Устаревший способ вызова базового метода
```

### Создание и удаление объектов

```sqf
new(ClassName)                                  // Создание объекта без параметров
newParams(ClassName,params)                     // Создание объекта с параметрами
delete(obj)                                     // Удаление объекта
isdeleted(obj)                                  // Проверка удаления объекта
instantiate(typeString)                         // Создание объекта по строковому имени (runtime)
instantiateParams(typeString,params)            // Создание объекта с параметрами (runtime)
```

### Проверки типов

```sqf
isTypeOf(obj,ClassName)                         // Проверка типа объекта
isTypeStringOf(obj,typeString)                  // Проверка типа по строке
getObjectsTypeOf(ClassName)                     // Список типов-наследников (без глубокого поиска)
getAllObjectsTypeOf(ClassName)                  // Все наследники (с глубоким поиском)
isExistsObject(obj)                             // Проверка валидности OOP объекта
isImplementFunc(obj,methodName)                 // Проверка существования метода
isImplementVar(obj,varName)                     // Проверка существования поля
```

### Утилиты для обновлений

```sqf
startSelfUpdate(methodName)                     // Запустить обновление метода объекта
startSelfUpdateWithDelay(methodName,delay)      // Запустить обновление с задержкой
startObjUpdate(obj,methodName)                  // Запустить обновление внешнего объекта
```

**Важно:** В макросах OOP второй аргумент (имя метода/поля) не должен содержать пробелов до или после запятой!

## Макросы структур (struct.hpp)

### Объявление структуры

```sqf
struct(name)                                    // Начало объявления структуры
base(parentStruct)                              // Наследование
endstruct                                       // Конец объявления структуры
inline_struct(name)                             // Создание структуры "на лету"
inline_endstruct                                // Конец inline структуры
```

### Объявление полей и методов

```sqf
def(name)                                       // Объявление поля или метода
def_null(name)                                  // Поле со значением null
def_ret(name)                                   // Метод с аннотацией возвращаемого значения
cast_def(TypeName)                              // Определение приведения типа
```

### Создание структур

```sqf
struct_new(StructName)                          // Создание без параметров
struct_newp(StructName,params)                  // Создание с параметрами (используйте arg)
struct_alloc(typeName,params)                   // Runtime создание по строковому имени
```

### Доступ к полям

```sqf
getv(fieldName)                                 // Получить значение поля
setv(fieldName,value)                           // Установить значение поля
modv(fieldName,operator)                        // Модифицировать поле (например, +1)
incv(fieldName)                                 // Увеличить поле на 1
decv(fieldName)                                 // Уменьшить поле на 1
self                                            // Ссылка на текущий экземпляр
```

### Вызов методов

```sqf
callv(methodName)                               // Вызов метода без параметров
callp(methodName,params)                        // Вызов метода с параметрами (используйте arg)
callbase(methodName)                            // Вызов метода родительской структуры
```

### Специальные методы

```sqf
def(init)                                       // Конструктор (вызывается при создании)
def(del)                                        // Деструктор (вызывается при удалении)
def(copy)                                       // Конструктор копирования
def(str)                                        // Строковое представление
```

### Проверки типов

```sqf
isinstance(obj,StructName)                      // Проверка типа структуры
isinstance_str(obj,typeString)                  // Проверка по строке
struct_isstruct(obj)                            // Проверка, является ли структурой
struct_typename(obj)                            // Получение имени типа
struct_existType(typeName)                      // Проверка существования типа
```

### Управление структурами

```sqf
struct_copy(obj)                                // Копирование структуры
struct_free(obj)                                // Принудительное удаление
struct_erase(obj)                               // Полное удаление
struct_isdeleted(obj)                           // Проверка удаления
struct_cast(obj,TypeName)                       // Приведение типа
```

**Важно:** Все значения полей одного типа располагаются по одному адресу в памяти. Для независимых значений инициализируйте поля в `def(init)`.

## RPC макросы

### Серверные RPC (serverRpc.hpp)

```sqf
rpcAdd(name,codeRef)                            // Регистрация обработчика на сервере (codeRef - ссылка на функцию)
rpcAddGlobal(name,codeRef)                      // Глобальная регистрация обработчика (codeRef - ссылка на функцию)
rpcRemove(name,id)                              // Удаление обработчика
rpcRemoveGlobal(name,id)                        // Удаление глобального обработчика
rpcCall(name,args)                              // Вызов RPC на сервере
rpcSendToClient(owner,name,args)                // Отправка клиенту
rpcSendToAll(name,args)                         // Отправка всем клиентам
rpcSendGlobal(name,args)                        // Глобальная отправка
```

### Клиентские RPC (clientRpc.hpp)

```sqf
rpcAdd(name,codeRef)                            // Регистрация обработчика на клиенте (codeRef - ссылка на функцию)
rpcAddGlobal(name,codeRef)                      // Глобальная регистрация обработчика (codeRef - ссылка на функцию)
rpcRemove(name,id)                              // Удаление обработчика
rpcRemoveGlobal(name,id)                        // Удаление глобального обработчика
rpcCall(name,args)                              // Вызов RPC на клиенте
rpcSendToServer(name,args)                      // Отправка на сервер
```

**Важно:** 
- Для `rpcAdd` и `rpcAddGlobal` второй аргумент должен быть **ссылкой на функцию** (переменная с кодом или глобальная функция), а не inline кодом `{ ... }`
- Для параметров используйте `arg` вместо запятой:
```sqf
// Правильно: передаем ссылку на функцию
private _handler = {
    params ["_data"];
    logformat("Received: %1", _data);
};
rpcAdd("myAction", _handler);

// Неправильно: inline код
// rpcAdd("myAction", { params ["_data"]; logformat("Received: %1", _data); });  // НЕ РАБОТАЕТ

// Параметры с arg:
rpcCall("myAction", [data1 arg data2 arg data3]);
rpcSendToClient(_owner, "clientAction", [data1 arg data2]);
```

## Критически важные макросы

### arg
Макрос для безопасной передачи аргументов с запятыми в другие макросы. Заменяется на запятую во время препроцессинга.

```sqf
callFuncParams(this,printData, [1 arg 2 arg 3]);
logformat("Values: %1, %2", value1 arg value2);
```

**См. также:** [Особенности синтаксиса](03_SYNTAX_GUIDE.md#макрос-arg---важная-особенность-макросов)

## Типичные паттерны

### Проверка на null перед использованием

```sqf
if (!valid(_obj)) exitWith {
    error("MyModule: Invalid object");  // Простое сообщение - используйте error()
};
callFunc(_obj,doSomething);
```

### Создание и использование объекта

```sqf
private _obj = new(MyClass);
if (!valid(_obj)) exitWith {
    error("Failed to create object");
};
callFunc(_obj,initialize);
delete(_obj);
```

### Логирование

```sqf
// Простые сообщения (без форматирования)
log("MyModule: Initializing...");              // Простое сообщение
log("MyModule: Module loaded");                // Простое сообщение

// Сообщения с форматированием (с подстановкой значений)
logformat("MyModule: Value: %1", _value);                    // Одно значение (префикс в строке)
logformat("Value: %1", _value);                              // Одно значение (без префикса)
logformat("MyModule: Values: %1, %2", _val1 arg _val2);     // Несколько значений (префикс в строке)
```

### Вызов метода с параметрами

```sqf
callSelfParams(processData, [data1 arg data2 arg data3]);
callFuncParams(_obj,processData, [data1 arg data2]);
```

### Работа с массивами

```sqf
private _copy = array_copy(_original);
if (array_exists(_array, _item)) then {
    array_remove(_array, _item);
};
private _last = array_selectlast(_array);
```

## Что дальше?

- ➡️ [Особенности синтаксиса](03_SYNTAX_GUIDE.md) - детальное описание макросов
- ➡️ [Структура проекта](02_PROJECT_STRUCTURE.md) - описание заголовочных файлов
- ➡️ [Глоссарий](GLOSSARY.md) - определения терминов

