-- Проверка orphan-записей по логическим FK (FK ещё не введены).
-- Для всех скалярных integer-FK: значение != 0 (0 = «не задано» в коде)
-- и нет соответствующей строки в parent.
-- CSV-поля проверяются отдельным скриптом (check_fk_csv_orphans.sql).

.mode column
.headers on

-- ============================================================
-- items
-- ============================================================
SELECT 'items.type→item_types.id'        AS link, COUNT(*) AS orphans
FROM items i WHERE i.type IS NOT NULL AND i.type <> 0
  AND NOT EXISTS (SELECT 1 FROM item_types t WHERE t.id = i.type);

-- ============================================================
-- characters
-- ============================================================
SELECT 'characters.weapon→items.id', COUNT(*)
FROM characters c WHERE c.weapon IS NOT NULL AND c.weapon <> 0
  AND NOT EXISTS (SELECT 1 FROM items x WHERE x.id = c.weapon);

SELECT 'characters.prefix→item_prefixes.id', COUNT(*)
FROM characters c WHERE c.prefix IS NOT NULL AND c.prefix <> 0
  AND NOT EXISTS (SELECT 1 FROM item_prefixes x WHERE x.id = c.prefix);

SELECT 'characters.eeff_id→effects.id', COUNT(*)
FROM characters c WHERE c.eeff_id IS NOT NULL AND c.eeff_id <> 0
  AND NOT EXISTS (SELECT 1 FROM effects x WHERE x.id = c.eeff_id);

SELECT 'characters.born_eff→effects.id', COUNT(*)
FROM characters c WHERE c.born_eff IS NOT NULL AND c.born_eff <> 0
  AND NOT EXISTS (SELECT 1 FROM effects x WHERE x.id = c.born_eff);

SELECT 'characters.die_eff→effects.id', COUNT(*)
FROM characters c WHERE c.die_eff IS NOT NULL AND c.die_eff <> 0
  AND NOT EXISTS (SELECT 1 FROM effects x WHERE x.id = c.die_eff);

SELECT 'characters.action_id→poses.id', COUNT(*)
FROM characters c WHERE c.action_id IS NOT NULL AND c.action_id <> 0
  AND NOT EXISTS (SELECT 1 FROM poses x WHERE x.id = c.action_id);

-- ============================================================
-- skills
-- ============================================================
SELECT 'skills.state_id→skill_states.id', COUNT(*)
FROM skills s WHERE s.state_id IS NOT NULL AND s.state_id <> 0
  AND NOT EXISTS (SELECT 1 FROM skill_states x WHERE x.id = s.state_id);

SELECT 'skills.target_effect→effects.id', COUNT(*)
FROM skills s WHERE s.target_effect IS NOT NULL AND s.target_effect <> 0
  AND NOT EXISTS (SELECT 1 FROM effects x WHERE x.id = s.target_effect);

SELECT 'skills.splash_effect→effects.id', COUNT(*)
FROM skills s WHERE s.splash_effect IS NOT NULL AND s.splash_effect <> 0
  AND NOT EXISTS (SELECT 1 FROM effects x WHERE x.id = s.splash_effect);

SELECT 'skills.target_effect_id→effects.id', COUNT(*)
FROM skills s WHERE s.target_effect_id IS NOT NULL AND s.target_effect_id <> 0
  AND NOT EXISTS (SELECT 1 FROM effects x WHERE x.id = s.target_effect_id);

SELECT 'skills.aground_effect_id→effects.id', COUNT(*)
FROM skills s WHERE s.aground_effect_id IS NOT NULL AND s.aground_effect_id <> 0
  AND NOT EXISTS (SELECT 1 FROM effects x WHERE x.id = s.aground_effect_id);

SELECT 'skills.water_effect_id→effects.id', COUNT(*)
FROM skills s WHERE s.water_effect_id IS NOT NULL AND s.water_effect_id <> 0
  AND NOT EXISTS (SELECT 1 FROM effects x WHERE x.id = s.water_effect_id);

SELECT 'skills.summon→characters.id', COUNT(*)
FROM skills s WHERE s.summon IS NOT NULL AND s.summon <> 0
  AND NOT EXISTS (SELECT 1 FROM characters x WHERE x.id = s.summon);

-- ============================================================
-- skill_states
-- ============================================================
SELECT 'skill_states.ch_id→characters.id', COUNT(*)
FROM skill_states ss WHERE ss.ch_id IS NOT NULL AND ss.ch_id <> 0
  AND NOT EXISTS (SELECT 1 FROM characters x WHERE x.id = ss.ch_id);

SELECT 'skill_states.free_state_id→skill_states.id', COUNT(*)
FROM skill_states ss WHERE ss.free_state_id IS NOT NULL AND ss.free_state_id <> 0
  AND NOT EXISTS (SELECT 1 FROM skill_states x WHERE x.id = ss.free_state_id);

SELECT 'skill_states.area_effect→effects.id', COUNT(*)
FROM skill_states ss WHERE ss.area_effect IS NOT NULL AND ss.area_effect <> 0
  AND NOT EXISTS (SELECT 1 FROM effects x WHERE x.id = ss.area_effect);

SELECT 'skill_states.effect→effects.id', COUNT(*)
FROM skill_states ss WHERE ss.effect IS NOT NULL AND ss.effect <> 0
  AND NOT EXISTS (SELECT 1 FROM effects x WHERE x.id = ss.effect);

SELECT 'skill_states.bit_effect→effects.id', COUNT(*)
FROM skill_states ss WHERE ss.bit_effect IS NOT NULL AND ss.bit_effect <> 0
  AND NOT EXISTS (SELECT 1 FROM effects x WHERE x.id = ss.bit_effect);

SELECT 'skill_states.charge_link→skill_states.id', COUNT(*)
FROM skill_states ss WHERE ss.charge_link IS NOT NULL AND ss.charge_link <> 0
  AND NOT EXISTS (SELECT 1 FROM skill_states x WHERE x.id = ss.charge_link);

-- ============================================================
-- hair / mount_info / stone_info
-- ============================================================
SELECT 'hair.item_id→items.id', COUNT(*)
FROM hair h WHERE h.item_id IS NOT NULL AND h.item_id <> 0
  AND NOT EXISTS (SELECT 1 FROM items x WHERE x.id = h.item_id);

SELECT 'mount_info.mount_id→characters.id', COUNT(*)
FROM mount_info m WHERE m.mount_id IS NOT NULL AND m.mount_id <> 0
  AND NOT EXISTS (SELECT 1 FROM characters x WHERE x.id = m.mount_id);

SELECT 'stone_info.item_id→items.id', COUNT(*)
FROM stone_info s WHERE s.item_id IS NOT NULL AND s.item_id <> 0
  AND NOT EXISTS (SELECT 1 FROM items x WHERE x.id = s.item_id);

-- ============================================================
-- events / areas
-- ============================================================
SELECT 'events.effect→effects.id', COUNT(*)
FROM events e WHERE e.effect IS NOT NULL AND e.effect <> 0
  AND NOT EXISTS (SELECT 1 FROM effects x WHERE x.id = e.effect);

SELECT 'events.born_effect→effects.id', COUNT(*)
FROM events e WHERE e.born_effect IS NOT NULL AND e.born_effect <> 0
  AND NOT EXISTS (SELECT 1 FROM effects x WHERE x.id = e.born_effect);

SELECT 'events.music→music.id', COUNT(*)
FROM events e WHERE e.music IS NOT NULL AND e.music <> 0
  AND NOT EXISTS (SELECT 1 FROM music x WHERE x.id = e.music);

SELECT 'events.main_cha_type→characters.id', COUNT(*)
FROM events e WHERE e.main_cha_type IS NOT NULL AND e.main_cha_type <> 0
  AND NOT EXISTS (SELECT 1 FROM characters x WHERE x.id = e.main_cha_type);

SELECT 'areas.music→music.id', COUNT(*)
FROM areas a WHERE a.music IS NOT NULL AND a.music <> 0
  AND NOT EXISTS (SELECT 1 FROM music x WHERE x.id = a.music);

-- ============================================================
-- scene_objects
-- ============================================================
SELECT 'scene_objects.attach_effect_id→effects.id', COUNT(*)
FROM scene_objects so WHERE so.attach_effect_id IS NOT NULL AND so.attach_effect_id <> 0
  AND NOT EXISTS (SELECT 1 FROM effects x WHERE x.id = so.attach_effect_id);

-- ============================================================
-- effects / eff_params / item_refine_effects → animated_lights.light_no
-- (composite PK на (light_no, key_no) — проверяем существование любого ключа с этим light_no)
-- ============================================================
SELECT 'effects.light_id→animated_lights.light_no', COUNT(*)
FROM effects e WHERE e.light_id IS NOT NULL AND e.light_id <> 0
  AND NOT EXISTS (SELECT 1 FROM animated_lights x WHERE x.light_no = e.light_id);

SELECT 'eff_params.light_id→animated_lights.light_no', COUNT(*)
FROM eff_params p WHERE p.light_id IS NOT NULL AND p.light_id <> 0
  AND NOT EXISTS (SELECT 1 FROM animated_lights x WHERE x.light_no = p.light_id);

SELECT 'item_refine_effects.light_id→animated_lights.light_no', COUNT(*)
FROM item_refine_effects r WHERE r.light_id IS NOT NULL AND r.light_id <> 0
  AND NOT EXISTS (SELECT 1 FROM animated_lights x WHERE x.light_no = r.light_id);

-- ============================================================
-- character_models / character_actions / pose_data → characters.id
-- ============================================================
SELECT 'character_models.character_type→characters.id', COUNT(*)
FROM character_models cm
  WHERE NOT EXISTS (SELECT 1 FROM characters x WHERE x.id = cm.character_type);

SELECT 'character_actions.character_type→characters.id (distinct)', COUNT(*)
FROM (SELECT DISTINCT character_type FROM character_actions) ca
  WHERE NOT EXISTS (SELECT 1 FROM characters x WHERE x.id = ca.character_type);

SELECT 'pose_data.character_type→characters.id (distinct)', COUNT(*)
FROM (SELECT DISTINCT character_type FROM pose_data) p
  WHERE NOT EXISTS (SELECT 1 FROM characters x WHERE x.id = p.character_type);
