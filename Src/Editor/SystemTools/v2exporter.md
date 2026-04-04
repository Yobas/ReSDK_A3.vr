# v2exporter JSON schema

Документ описывает формат файла, который генерирует `systools_generateV2Prototypes` из [v2exporter.sqf](Src/Editor/SystemTools/v2exporter.sqf).

## Назначение

Экспорт формирует подробный реестр `v1`-классов для будущего `v2` porter-а. Это не экспорт карты. Это библиотека типов и их metadata:

- базовая идентификация класса
- цепочка наследования
- модель и базовые display-поля
- class-level attributes
- вычисленные теги
- Arma visual metadata для носимых предметов
- seat metadata для статических сидений
- door metadata для дверей
- diagnostics для потенциально проблемных getter-ов

## Выходной файл

По текущей реализации JSON сохраняется в:

```text
src\editor\bin\v1_class_registry.json
```

Экспорт падает с ошибкой, если целевая директория не существует.

## Верхний уровень JSON

Файл имеет вид:

```json
{
  "meta": { ... },
  "classes": [ ... ]
}
```

### `meta`

Служебный блок экспорта.

Поля:

- `exporter: string`
  Идентификатор exporter-а. Сейчас: `v1_v2exporter`.
- `generatedAt: array`
  Значение `systemTime` на момент генерации.
- `editorVersion: string`
  Версия редактора.
- `mapVersion: number`
  Версия карты из common storage, если доступна. Иначе `-1`.
- `source: string`
  Источник экспорта. Сейчас: `resdk_fork.vr`.
- `classCountSource: number`
  Сколько классов прошло в исходный список до фильтрации.
- `classCountExported: number`
  Сколько классов реально попало в `classes`.
- `classCountSkipped: number`
  Сколько классов было отброшено фильтрацией.
- `outputFile: string`
  Путь выходного JSON относительно миссии.

## `classes`

Массив объектов-классов.

Каждый элемент имеет обязательный минимум:

```json
{
  "className": "SomeClass"
}
```

Остальные поля добавляются только если exporter смог получить значение.

## Схема записи класса

### Базовые поля

- `className: string`
  Имя класса в `v1`.
- `baseClass: string`
  Прямой родитель (`__motherClass`).
- `inheritanceChain: string[]`
  Цепочка родителей в порядке OOP runtime (`__inhlistCase`).
- `declInfo: array`
  Декларативная информация о месте объявления (`__decl_info__`).
- `modelPath: string`
  Результат `model/getModel`.
- `name: string`
  Базовое имя класса или display-name.
- `desc: string`
  Базовое описание класса.
- `material: string | array | number | object`
  Значение поля `material`, если доступно.
- `weight: number`
  Значение поля `weight`, если доступно.

### `classAttributes`

HashMap с class-level editor attributes.

Формат зависит от того, как attribute зарегистрирован в `GOAsm`. Exporter копирует class attribute storage как есть, без нормализации под отдельную схему.

Пример:

```json
{
  "classAttributes": {
    "EditorGenerated": [],
    "ColorClass": ["#ffaa00"]
  }
}
```

### `tags`

Массив вычисленных тегов. Это derived metadata, а не просто копия raw attributes.

Возможные текущие значения:

- `item`
- `mob`
- `decor`
- `struct`
- `door`
- `container`
- `light`
- `seat`
- `effect`
- `weapon`
- `wearable`
- `electronic`

Exporter не гарантирует, что тег будет у каждого подходящего класса. Тег добавляется только если его можно уверенно определить штатной логикой.

## `equipmentVisual`

Добавляется только для классов, у которых реально определён `armaClass`.

Назначение:

- дать porter-у нормализованный блок для одежды, шлемов, масок, бронежилетов, рюкзаков и других wearable-визуалов

Формат:

```json
{
  "equipmentVisual": {
    "armaClass": "Some_Arma_Class",
    "allowedSlots": [ ... ],
    "examine3dType": "cloth",
    "kind": "cloth"
  }
}
```

Поля:

- `armaClass: string | number`
  Значение `armaClass`. Чаще всего строка, но exporter не нормализует тип принудительно.
- `allowedSlots: array`
  Значение поля `allowedSlots`, если доступно.
- `examine3dType: string`
  Результат `getExamine3dItemType`, если доступен.
- `kind: string`
  Derived category. Сейчас одно из:
  - `cloth`
  - `armor`
  - `helmet`
  - `mask`
  - `backpack`

`kind` сначала пытается браться из `getExamine3dItemType`, а если его нет, вычисляется по `allowedSlots`.

## `seat`

Добавляется только для классов, у которых `isSeat == true`.

Назначение:

- сохранить static seat offsets для будущих статических сидений в `v2`

Формат:

```json
{
  "seat": {
    "offsetPos": [0, 0, 0],
    "offsetDir": 180
  }
}
```

Поля:

- `offsetPos: array | number`
  Результат `getChairOffsetPos`
  Может быть:
  - один `vec3`
  - массив `vec3`
- `offsetDir: array | number`
  Результат `getChairOffsetDir`
  Может быть:
  - один угол
  - массив углов

Exporter не нормализует seat offsets в единый flat-format. Он сохраняет структуру как возвращает getter.

## `door`

Добавляется только для классов, у которых `isDoor == true`.

Назначение:

- сохранить static metadata для дверей
- отличить safe const-data от getter-ов, зависящих от состояния объекта

Формат:

```json
{
  "door": {
    "animateData": [ ... ],
    "animateDataSource": "{ ... }",
    "interpSpeed": -0.35,
    "animateDataMaybeProblematic": true
  }
}
```

Поля:

- `animateData: array`
  Вычисленное значение `animateData`, но только если getter выглядит как safe pure-data getter.
- `animateDataSource: string`
  Исходный код getter-а `animateData` через `toString method`.
- `interpSpeed: number`
  Результат getter-а `interpSpeed`, если доступен.
- `animateDataMaybeProblematic: bool`
  Ставится, если exporter нашёл признаки runtime-зависимости в коде `animateData`.

### Правило безопасности для `animateData`

Exporter сначала получает исходник getter-а `animateData`.

Если в source есть признаки обращения к runtime-state, например:

- `getVariable`
- `getSelf`
- `callSelf`
- `this getVariable`
- `getv(`
- `getvar(`

тогда getter считается потенциально небезопасным для compile-time export:

- `animateData` не вычисляется
- `animateDataSource` сохраняется как строка
- класс помечается как potentially problematic

Если таких признаков нет, exporter делает вторую фазу:

- получает уже вычисленное значение через `oop_getFieldBaseValue`
- пишет его в `door.animateData`

Это нужно, чтобы не тащить в экспорт door-data, завязанную на текущее состояние конкретного объекта, например на `isOpen`.

## `diagnostics`

Добавляется только если exporter нашёл потенциально проблемные места.

Формат:

```json
{
  "diagnostics": {
    "maybeProblematic": true,
    "warnings": [
      "door_animateData_dynamic"
    ]
  }
}
```

Поля:

- `maybeProblematic: bool`
  Общий флаг, что запись класса требует ручной проверки.
- `warnings: string[]`
  Список кодов предупреждений.

Текущий список warning-code:

- `door_animateData_dynamic`
  Getter `animateData` выглядит зависящим от runtime state и не был безопасно вычислен.

## Фильтрация классов

Exporter пропускает только runtime-relevant классы и выкидывает class-level abstract/internal категории:

- `InterfaceClass`
- `HiddenClass`
- `NodeClass`
- `TemplatePrefab`

`EditorGenerated` классы не выкидываются и экспортируются как обычные.

## Пример

```json
{
  "className": "ChairLibrary",
  "baseClass": "Chair",
  "inheritanceChain": ["Chair", "IStruct", "GameObject"],
  "declInfo": ["Src\\host\\GameObjects\\Structures\\Furniture\\Chairs.sqf", "112"],
  "modelPath": "ml_exodusnew\\stalker_tun\\meb1.p3d",
  "tags": ["struct", "seat"],
  "seat": {
    "offsetPos": [0, -0.1, 0],
    "offsetDir": 180
  }
}
```

Пример проблемной двери:

```json
{
  "className": "SteelGridDoor",
  "tags": ["struct", "door"],
  "door": {
    "animateDataSource": "{ objParams(); [ [\"door_1_rot\", 2.5, ifcheck(!getSelf(isOpen),1.5,0.5)] ] }",
    "interpSpeed": -0.35,
    "animateDataMaybeProblematic": true
  },
  "diagnostics": {
    "maybeProblematic": true,
    "warnings": ["door_animateData_dynamic"]
  }
}
```

## Что exporter сейчас не гарантирует

- это не formal JSON Schema draft, а практическая схема текущего файла
- поля `material`, `classAttributes`, `declInfo` могут быть разных форматов в зависимости от исходного класса
- `seat.offsetPos` и `seat.offsetDir` не приводятся к единому нормализованному shape
- `door.animateDataSource` хранится как raw source getter-а, без AST-разбора
- `animateData` считается safe только по простой эвристике, а не по полноценному анализу const-expression

## Export current map

`systools_exportCurrentMap` генерирует второй JSON, не с библиотекой классов, а с объектами текущей сцены.

Текущий выходной файл:

```text
src\editor\bin\v1_map_export.json
```

Формат:

```json
{
  "meta": { ... },
  "objects": [ ... ]
}
```

### `meta`

Поля:

- `exporter: string`
  Сейчас: `v1_v2mapexporter`
- `generatedAt: array`
- `editorVersion: string`
- `mapName: string`
- `mapVersion: number`
- `source: string`
- `objectCountExported: number`
- `objectCountSkipped: number`
- `outputFile: string`

### `objects`

Массив world-объектов сцены, у которых есть hashdata.

Exporter берёт только реальные editor scene objects:

- объект должен иметь hashdata
- `golib_com_object` исключается

Минимальная схема объекта:

```json
{
  "type": "ChairLibrary",
  "pos": [123.4, 456.7, 8.9],
  "vdir": [0, 1, 0],
  "vup": [0, 0, 1],
  "customProps": {}
}
```

Поля:

- `type: string`
  Значение `class` из hashdata.
- `pos: number[3]`
  `getPosWorld`
- `vdir: number[3]`
  `vectorDirVisual`
- `vup: number[3]`
  `vectorUpVisual`
- `customProps: object`
  `customProps` из hashdata. Если их нет, exporter пишет пустой HashMap/object.

Текущий map-export намеренно плоский:

- без полной копии hashdata
- без raw init-кода
- без системных runtime полей
- только то, что нужно porter-у для расстановки объектов и переноса custom properties
