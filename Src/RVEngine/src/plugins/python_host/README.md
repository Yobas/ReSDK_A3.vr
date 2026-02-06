# Python Host Plugin for RVEngine/Intercept

Плагин для интеграции Python в Arma 3 через RVEngine/Intercept.

> [!WARNING]
> Это экспериментальное решение и скорее всего оно не буде использоваться в основном проекте SDK

## Требования

1. **Python 3.10+** (рекомендуется 3.11)
   - Установить Python с официального сайта: https://www.python.org/downloads/
   - **ВАЖНО**: При установке отметить "Add Python to PATH" и "Install development headers"

2. **pybind11** (автоматически скачается через FetchContent или установить вручную)

## Установка зависимостей

### Вариант 1: Через vcpkg (рекомендуется)

```bash
# Установить vcpkg если еще не установлен
git clone https://github.com/microsoft/vcpkg.git
cd vcpkg
.\bootstrap-vcpkg.bat

# Установить pybind11
.\vcpkg install pybind11:x64-windows

# При конфигурации CMake указать тулчейн
cmake -B build -DCMAKE_TOOLCHAIN_FILE=[путь к vcpkg]/scripts/buildsystems/vcpkg.cmake -DBUILD_PYTHON_HOST=ON
```

### Вариант 2: Через pip (только pybind11 headers)

```bash
pip install pybind11
```

Затем добавить путь к pybind11 в CMakeLists.txt:
```cmake
set(pybind11_DIR "C:/Users/[user]/AppData/Local/Programs/Python/Python311/Lib/site-packages/pybind11/share/cmake/pybind11")
```

### Вариант 3: Git submodule

```bash
cd Src/RVEngine
git submodule add https://github.com/pybind/pybind11.git 3rdparty/pybind11
```

## Сборка

```bash
cd Src/RVEngine

# Конфигурация (укажите свой Python если нужно)
cmake -B build -G "Visual Studio 17 2022" -A x64 -DBUILD_PYTHON_HOST=ON

# Сборка
cmake --build build --config Release

# Результат: build/Release/plugins/python_host_x64.dll
```

## Структура мода

```
@YourMod/
├── addons/
│   └── ... (pbo файлы)
├── intercept/
│   └── python_host_x64.dll    <- Скопировать сюда
├── python/
│   ├── main.py                <- Главный скрипт (загружается автоматически)
│   ├── utils.py               <- Дополнительные модули
│   └── ...
└── mod.cpp
```

## Использование в Python

### Базовый пример (main.py)

```python
import arma
from arma import sqf

def post_init():
    """Вызывается при инициализации миссии"""
    player = sqf.player()
    pos = sqf.get_pos(player)
    sqf.hint(f"Player at {pos}")

def on_frame():
    """Вызывается каждый кадр (осторожно с производительностью!)"""
    pass
```

### Доступные функции

#### Позиция

```python
# Получить позицию
pos = sqf.get_pos(obj)        # [x, y, z]
pos = sqf.get_pos_asl(obj)    # ASL
pos = sqf.get_pos_atl(obj)    # ATL

# Установить позицию
sqf.set_pos(obj, (x, y, z))
sqf.set_pos(obj, [x, y, z])   # Можно list или tuple
sqf.set_pos_asl(obj, pos)
sqf.set_pos_atl(obj, pos)
```

#### Объекты

```python
player = sqf.player()         # Получить игрока
vehicle = sqf.vehicle(player) # Получить транспорт
```

#### Утилиты

```python
sqf.hint("Сообщение")
sqf.system_chat("Чат")
sqf.diag_log("Лог в RPT")
```

#### Универсальный вызов SQF

```python
# Nular (без аргументов)
time = sqf.call("time")
player = sqf.call("player")

# Unary (один аргумент)
damage = sqf.call("damage", player)
pos = sqf.call("getPos", player)

# Binary (два аргумента)
sqf.call("setVariable", player, ["varName", 42])
value = sqf.call("getVariable", player, ["varName", 0])
```

### Lifecycle хуки

```python
def pre_init():
    """CBA XEH_preInit"""
    pass

def post_init():
    """CBA XEH_postInit"""
    pass

def on_frame():
    """Каждый кадр - ОСТОРОЖНО с производительностью!"""
    pass

def mission_ended():
    """При завершении миссии"""
    pass

def on_shutdown():
    """При выгрузке Python"""
    pass
```

## Преобразование типов

| Python | Arma/SQF |
|--------|----------|
| `None` | `nil` |
| `bool` | `BOOL` |
| `int`/`float` | `SCALAR` |
| `str` | `STRING` |
| `list`/`tuple` | `ARRAY` |
| `GameValue` | Любой тип (object, group, etc) |

## Отладка

Логи пишутся в:
1. Консоль (stdout)
2. Arma 3 RPT файл

```python
sqf.diag_log("Debug message")
print("Also works for console")
```

## Производительность

- `on_frame()` вызывается КАЖДЫЙ кадр
- Используйте throttling:

```python
_frame_count = 0

def on_frame():
    global _frame_count
    _frame_count += 1

    # Только каждые 60 кадров (~1 сек при 60fps)
    if _frame_count % 60 != 0:
        return

    # Тяжелый код здесь
```

## Известные ограничения

1. Python GIL - все вызовы однопоточные
2. Нельзя создавать новые потоки для SQF вызовов
3. Большие массивы могут копироваться медленно
