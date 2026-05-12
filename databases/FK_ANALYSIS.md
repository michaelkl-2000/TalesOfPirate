# Анализ FOREIGN KEY для `databases/gamedata.sqlite`

Дата: 2026-05-13.
БД: `D:\Projects\MMORPG\TalesOfPirate\databases\gamedata.sqlite` (6.6 MB, 45 таблиц).

## Резюме

- В БД **нет ни одного FK** и почти нет вторичных индексов. `PRAGMA foreign_keys = 0`.
- Связи между таблицами восстановлены по `Record`/`Store`-классам в `sources/Libraries/common/`.
- **Sentinel-значения «не задано»** — это `0` для большинства полей, **и `-1`** для `areas.music`, `skill_states.charge_link`, `skill_states.ch_id` (диапазон `-128..-1`). Это значит, при добавлении FK условия должны быть `value > 0`, а не `value <> 0`.
- Найдены реальные **orphan'ы по данным** в 5 связях (см. ниже).
- Несколько гипотез из обзора кода **не подтвердились данными** и сняты.

## Подтверждённые FK (готовы к включению)

Все ниже — проверены: 0 orphan-ов при условии `value > 0`.

| Дочерняя колонка | Родитель | Семантика |
|---|---|---|
| `characters.weapon` | `items.id` | оружие по умолчанию |
| `characters.prefix` | `item_prefixes.id` | префикс имени |
| `characters.eeff_id` | `effects.id` | постоянный эффект |
| `characters.born_eff` | `effects.id` | эффект рождения |
| `characters.die_eff` | `effects.id` | эффект смерти |
| `skills.state_id` | `skill_states.id` | накладываемое состояние |
| `skills.target_effect` | `effects.id` | эффект на цель |
| `skills.splash_effect` | `effects.id` | эффект всплеска |
| `skills.target_effect_id` | `effects.id` | дубль `target_effect` |
| `skills.water_effect_id` | `effects.id` | эффект на воде |
| `skills.summon` | `characters.id` | призываемый персонаж |
| `skill_states.free_state_id` | `skill_states.id` | self-FK |
| `skill_states.area_effect` | `effects.id` | эффект области |
| `skill_states.bit_effect` | `effects.id` | bit-эффект |
| `hair.item_id` | `items.id` | предмет причёски |
| `stone_info.item_id` | `items.id` | предмет камня |
| `events.effect` | `effects.id` | эффект события |
| `events.born_effect` | `effects.id` | эффект рождения |
| `events.music` | `music.id` | музыка события |
| `events.main_cha_type` | `characters.id` | персонаж события |
| `scene_objects.attach_effect_id` | `effects.id` | прикреплённый эффект |
| `effects.light_id` | `animated_lights.light_no` | анимированный свет (PK part) |
| `eff_params.light_id` | `animated_lights.light_no` | анимированный свет |
| `item_refine_effects.light_id` | `animated_lights.light_no` | анимированный свет |
| `character_models.character_type` | `characters.id` | модель персонажа |
| `character_actions.character_type` | `characters.id` | анимация (composite) |
| `pose_data.character_type` | `characters.id` | поза (composite) |

Для `*.light_id → animated_lights.light_no` SQLite требует **уникального индекса на `animated_lights(light_no)`** (отдельная колонка, не часть составного PK). Сейчас уникальности нет: `light_no` — часть составного PK `(light_no, key_no)`, в БД 44 строки, 18 уникальных `light_no`. Можно либо:

1. Создать `UNIQUE INDEX idx_animated_lights_light_no ON animated_lights(light_no)` (требует, чтобы фактически было по одной строке на `light_no`). Сейчас НЕ выполняется — `key_no` тоже есть.
2. Использовать обычный (не FK) индекс и не вводить FK на `light_no`.

Я предлагаю **вариант 2** — обычный индекс без FK. Иначе придётся менять структуру `animated_lights`.

## FK с orphan-данными (нужно решение)

| FK | Orphans | Характер |
|---|---|---|
| `skills.aground_effect_id → effects.id` | 5 | пять skill'ов (Flash Bomb Lv1-5) ссылаются на `effects.id = 223`, которой нет. Скорее всего удалена/перенесена. **Рекомендую обнулить (`=0`) после ручной проверки или восстановить запись 223.** |
| `skill_states.effect → effects.id` | 9 | 8 строк ссылаются на effects 830-837, одна — на 222. Их нет в `effects`. **Рекомендую обнулить или восстановить пропущенные effects.** |
| `skill_states.ch_id → characters.id` | 71 | все 71 имеют значение в диапазоне `-128..-1` — sentinel «не задано» (`signed char` поле в C++). **FK безопасен, если условие `ch_id > 0` (см. `skill_states` ниже).** SQLite не поддерживает CHECK в FK, но FK с NULLABLE срабатывает только при не-NULL значении. Решение — заменить отрицательные значения на `NULL` перед включением FK. |
| `skill_states.charge_link → skill_states.id` | 192 | 191 значение `-1` (sentinel) + 3 положительных orphan'а (5, 7, 8 в Thunderstorm/Fog/Tornado). Эти 3 значения **не соответствуют ни skill_states, ни skills**. Возможно — legacy/мусор. **Заменить `-1` на NULL; для 3 orphan-значений — обнулить после ревью.** |
| `areas.music → music.id` | 120 | 119 значений `-1` (sentinel) + 1 реальный orphan (`areas.id=135 «Xmas Village» music=15`). **Заменить `-1` на NULL; для `music=15` — обнулить или добавить music id=15.** |

## Снятые гипотезы (не FK)

| Гипотеза | Что вместо |
|---|---|
| `items.type → item_types.id` (988 orphan'ов) | `items.type` — это значение `enum EItemType` (1..99), а `item_types` — неполный словарь для UI (38 строк, не покрывают все типы). **НЕ делать FK** — иначе придётся либо вычистить 988 items, либо набить item_types отсутствующими 50+ типами. Лучше оставить как enum. |
| `characters.action_id → poses.id` (1406 orphan'ов) | `poses.id` — 1..54 — это «именованные группы анимаций» (BasePose+массив). `characters.action_id` действительно для 243 строк сходится с poses, для 77 — с composite `character_actions(character_type, action_no)`, а для 1406 не сходится ни с тем, ни с другим (характеры без своих анимаций). **НЕ делать FK.** |
| `mount_info.mount_id → characters.id` (20 orphan'ов из 20) | mount_id находится в зарезервированном диапазоне (9500-9515, 19692-19695), которого нет в `characters`. Это **отдельное пространство ID маунтов**, родительской таблицы в gamedata.sqlite нет. **НЕ FK** (пока не появится `mounts` / расширения characters). |

## CSV-поля (логические M2M, FK невозможны без нормализации)

В SQLite нельзя поставить FK на TEXT-поле с CSV. Эти связи документируются, но миграция требует нормализации в join-таблицу.

| Колонка | Парсер хранит как | Логическая цель |
|---|---|---|
| `items.effects`, `item_effect`, `area_effect`, `use_item_effect` | `short[N][2]` (id, ...) | `effects.id` |
| `characters.feff_id`, `skill_ids`, `item_ids`, `task_item_ids` | `short[N][2]` | `effects.id` / `skills.id` / `items.id` |
| `characters.effect_action_id` | `short[3]` | `effects.id` (0=base, 1=enhanced, 2=dummy) |
| `skills.premiss_skill` | `short[N][2]` | `skills.id` |
| `skills.action_effect` | `short[N]` | `effects.id` |
| `skills.item_effect_1`, `item_effect_2` | `short[N]` | `effects.id` |
| `skills.item_need_1/2/3` | `short[3][8][2]` | `items.id` |
| `skills.conch_need` | `short[8][3]` | `items.id` |
| `hair.need_items`, `fail_item_ids` | `dword[N][2]` / `dword[N]` | `items.id` |
| `forge.items` | `id,cnt,id,cnt,...` | `items.id` |
| `job_equip.items` | CSV | `items.id` |
| `mount_info.pose_ids` | `short[4]` | `poses.id` |
| `monster_info.monster_list` | `int[8]` | `characters.id` |
| `item_refine_effects.effect_ids` | `short[4][4]` | `effects.id` |

При желании их можно мигрировать в нормализованные таблицы (`item_effects(item_id, slot, effect_id)` и т.п.), но это **отдельная задача**, не часть текущего FK-восстановления.

## Дальнейшие шаги

1. (опционально) Очистить sentinel-значения `-1` → `NULL` в трёх колонках:
   - `UPDATE areas SET music = NULL WHERE music = -1;`
   - `UPDATE skill_states SET charge_link = NULL WHERE charge_link = -1;`
   - `UPDATE skill_states SET ch_id = NULL WHERE ch_id < 0;`
2. (опционально) Решить судьбу 5 реальных orphan-значений (см. таблицу выше).
3. Применить миграцию `migrate_add_foreign_keys.sql` — она пересоздаёт таблицы с FK через `ALTER TABLE RENAME` + `INSERT SELECT`.
4. После миграции — добавить `PRAGMA foreign_keys = ON;` в загрузчик (`SqliteDatabase::Open`).

Скрипты в директории `databases/`:
- `check_fk_orphans.sql` — orphan-проверка (можно перезапускать).
- `migrate_add_foreign_keys.sql` — миграция (см. ниже).
- `FK_ANALYSIS.md` — этот документ.
