# v2 map exporter JSON schema

Документ описывает формат файла, который генерирует `systools_exportCurrentMap` из [v2exporter.sqf](Src/Editor/SystemTools/v2exporter.sqf).

## Назначение

Экспорт формирует плоский JSON текущей карты для будущего `v2` porter-а.

Это не packed map builder и не полный дамп editor state. Экспорт берёт только игровые объекты сцены, у которых есть hashdata, и сохраняет минимальный набор данных для последующего импорта:

- тип объекта
- мировой transform
- custom properties

## Выходной файл

По текущей реализации JSON сохраняется в:

```text
src\editor\bin\v1_map_export.json
```

Экспорт падает с ошибкой, если целевая директория не существует.

## Верхний уровень JSON

Файл имеет вид:

```json
{
  "meta": { ... },
  "objects": [ ... ]
}
```

## `meta`

Служебный блок экспорта.

Поля:

- `exporter: string`
  Идентификатор exporter-а. Сейчас: `v1_v2mapexporter`.
- `generatedAt: array`
  Значение `systemTime` на момент генерации.
- `editorVersion: string`
  Версия редактора.
- `mapName: string`
  Имя карты из common storage. Если отсутствует, exporter пишет `unknown`.
- `mapVersion: number`
  Версия карты из common storage. Если отсутствует, exporter пишет `-1`.
- `source: string`
  Источник экспорта. Сейчас: `resdk_fork.vr`.
- `objectCountExported: number`
  Сколько объектов реально попало в `objects`.
- `objectCountSkipped: number`
  Сколько объектов было пропущено, потому что exporter не смог собрать корректную запись.
- `outputFile: string`
  Путь выходного JSON относительно миссии.

Пример:

```json
{
  "meta": {
    "exporter": "v1_v2mapexporter",
    "generatedAt": [2026, 4, 4, 23, 10, 5, 123],
    "editorVersion": "1.19",
    "mapName": "Minimap",
    "mapVersion": 17,
    "source": "resdk_fork.vr",
    "objectCountExported": 15234,
    "objectCountSkipped": 3,
    "outputFile": "src\\editor\\bin\\v1_map_export.json"
  }
}
```

## `objects`

Массив world-объектов сцены.

Exporter проходит по:

```sqf
all3DENEntities select 0
```

и включает в экспорт только те объекты, которые удовлетворяют условиям:

- объект имеет hashdata
- объект не равен `golib_com_object`
- в hashdata существует поле `class`

## Схема записи объекта

Каждый объект имеет фиксированный минимальный shape:

```json
{
  "type": "ChairLibrary",
  "pos": [123.4, 456.7, 8.9],
  "vdir": [0, 1, 0],
  "vup": [0, 0, 1],
  "customProps": {}
}
```

### `type`

```json
"type": "ChairLibrary"
```

- Тип: `string`
- Источник: `hashData["class"]`

Это `v1` className объекта. В будущем porter должен сопоставлять его с библиотекой `v1 class registry` и `v2 prototype/entity id`.

### `pos`

```json
"pos": [123.4, 456.7, 8.9]
```

- Тип: `number[3]`
- Источник: `getPosWorld object`

Это мировая позиция объекта.

### `vdir`

```json
"vdir": [0, 1, 0]
```

- Тип: `number[3]`
- Источник: `vectorDirVisual object`

Это визуальный forward-вектор объекта.

### `vup`

```json
"vup": [0, 0, 1]
```

- Тип: `number[3]`
- Источник: `vectorUpVisual object`

Это визуальный up-вектор объекта.

Пара `vdir + vup` используется вместо одного `dir`, чтобы porter мог восстановить полный orientation объекта, а не только yaw.

### `customProps`

```json
"customProps": {
  "name": "Key from storage",
  "keyowner": ["storage_a"]
}
```

- Тип: `object`
- Источник: `hashData["customProps"]`

Если `customProps` отсутствуют в hashdata, exporter пишет пустой объект.

Exporter не нормализует значения `customProps`. Они сохраняются в том виде, в каком лежат в hashdata:

- строки
- числа
- массивы
- вложенные структуры, если они сериализуемы в `toJson`

## Что намеренно не экспортируется

Текущий map export плоский и преднамеренно не включает:

- полную копию hashdata
- raw `init` код объекта
- editor-only служебные поля
- runtime-only поля
- модель объекта отдельным полем
- связи между объектами как отдельную нормализованную структуру
- packed map builder opcodes

Если какие-то данные нужны porter-у позже, они должны добавляться в схему явно, а не вытаскиваться неявно из старого builder pipeline.

## Пример полного файла

```json
{
  "meta": {
    "exporter": "v1_v2mapexporter",
    "generatedAt": [2026, 4, 4, 23, 10, 5, 123],
    "editorVersion": "1.19",
    "mapName": "Minimap",
    "mapVersion": 17,
    "source": "resdk_fork.vr",
    "objectCountExported": 2,
    "objectCountSkipped": 0,
    "outputFile": "src\\editor\\bin\\v1_map_export.json"
  },
  "objects": [
    {
      "type": "ChairLibrary",
      "pos": [123.4, 456.7, 8.9],
      "vdir": [0, 1, 0],
      "vup": [0, 0, 1],
      "customProps": {}
    },
    {
      "type": "SteelBlueCase",
      "pos": [130.0, 460.0, 9.0],
      "vdir": [1, 0, 0],
      "vup": [0, 0, 1],
      "customProps": {
        "countslots": 100
      }
    }
  ]
}
```

## Ограничения текущей схемы

- Это не formal JSON Schema draft, а практическое описание текущего файла.
- `customProps` не проходят отдельную нормализацию или типизацию.
- В export попадают только объекты с hashdata.
- Порядок объектов в `objects` не считается стабильным идентификатором.
- В текущей версии exporter не добавляет отдельный `model`, даже если porter потенциально мог бы его использовать.

