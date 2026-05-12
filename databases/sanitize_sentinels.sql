-- ОПЦИОНАЛЬНЫЙ скрипт: перед добавлением FK заменить sentinel-значения и
-- известные orphan'ы на NULL, чтобы FK срабатывал только для значащих ссылок.
--
-- Применять ТОЛЬКО после ревью отчёта FK_ANALYSIS.md. Скрипт обратимый:
-- старые значения не сохраняются, но это статика — её можно перезалить из исходников.
--
-- Запуск:
--   sqlite3 databases/gamedata.sqlite < sanitize_sentinels.sql

.bail on
BEGIN TRANSACTION;

-- 1) "−1" как sentinel «не задано» → NULL
UPDATE areas        SET music       = NULL WHERE music       = -1;
UPDATE skill_states SET charge_link = NULL WHERE charge_link = -1;
UPDATE skill_states SET ch_id       = NULL WHERE ch_id       <  0;

-- 2) Реальные orphan'ы (нет соответствующих строк в parent):
-- 2a) skills.aground_effect_id = 223 — отсутствует в effects (5 строк Flash Bomb)
UPDATE skills SET aground_effect_id = NULL
WHERE aground_effect_id IS NOT NULL AND aground_effect_id <> 0
  AND NOT EXISTS (SELECT 1 FROM effects e WHERE e.id = skills.aground_effect_id);

-- 2b) skill_states.effect = 222 или 830..837 — отсутствуют в effects (9 строк)
UPDATE skill_states SET effect = NULL
WHERE effect IS NOT NULL AND effect <> 0
  AND NOT EXISTS (SELECT 1 FROM effects e WHERE e.id = skill_states.effect);

-- 2c) skill_states.charge_link = 5/7/8 — отсутствуют в skill_states (3 строки)
UPDATE skill_states SET charge_link = NULL
WHERE charge_link IS NOT NULL AND charge_link <> 0
  AND NOT EXISTS (SELECT 1 FROM skill_states x WHERE x.id = skill_states.charge_link);

-- 2d) areas.music = 15 — отсутствует в music (1 строка, «Xmas Village»)
UPDATE areas SET music = NULL
WHERE music IS NOT NULL AND music <> 0
  AND NOT EXISTS (SELECT 1 FROM music m WHERE m.id = areas.music);

-- 3) Нормализуем "0" к NULL для FK-полей, где код считает 0 = «не задано».
-- (NULL семантически чище, FK-проверка не срабатывает.)
UPDATE characters   SET weapon            = NULL WHERE weapon            = 0;
UPDATE characters   SET prefix            = NULL WHERE prefix            = 0;
UPDATE characters   SET eeff_id           = NULL WHERE eeff_id           = 0;
UPDATE characters   SET born_eff          = NULL WHERE born_eff          = 0;
UPDATE characters   SET die_eff           = NULL WHERE die_eff           = 0;

UPDATE skills       SET state_id          = NULL WHERE state_id          = 0;
UPDATE skills       SET target_effect     = NULL WHERE target_effect     = 0;
UPDATE skills       SET splash_effect     = NULL WHERE splash_effect     = 0;
UPDATE skills       SET target_effect_id  = NULL WHERE target_effect_id  = 0;
UPDATE skills       SET aground_effect_id = NULL WHERE aground_effect_id = 0;
UPDATE skills       SET water_effect_id   = NULL WHERE water_effect_id   = 0;
UPDATE skills       SET summon            = NULL WHERE summon            = 0;

UPDATE skill_states SET ch_id             = NULL WHERE ch_id             = 0;
UPDATE skill_states SET free_state_id     = NULL WHERE free_state_id     = 0;
UPDATE skill_states SET area_effect       = NULL WHERE area_effect       = 0;
UPDATE skill_states SET effect            = NULL WHERE effect            = 0;
UPDATE skill_states SET bit_effect        = NULL WHERE bit_effect        = 0;
UPDATE skill_states SET charge_link       = NULL WHERE charge_link       = 0;

UPDATE hair         SET item_id           = NULL WHERE item_id           = 0;
UPDATE stone_info   SET item_id           = NULL WHERE item_id           = 0;

UPDATE events       SET effect            = NULL WHERE effect            = 0;
UPDATE events       SET born_effect       = NULL WHERE born_effect       = 0;
UPDATE events       SET music             = NULL WHERE music             = 0;
UPDATE events       SET main_cha_type     = NULL WHERE main_cha_type     = 0;

UPDATE areas        SET music             = NULL WHERE music             = 0;

UPDATE scene_objects SET attach_effect_id = NULL WHERE attach_effect_id  = 0;

UPDATE effects             SET light_id   = NULL WHERE light_id          = 0;
UPDATE eff_params          SET light_id   = NULL WHERE light_id          = 0;
UPDATE item_refine_effects SET light_id   = NULL WHERE light_id          = 0;

COMMIT;
.print "sanitize complete"
