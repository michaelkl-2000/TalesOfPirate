-- Миграция: добавление FOREIGN KEY и индексов в databases/gamedata.sqlite.
-- Стратегия: пересоздание таблиц через CREATE *_new -> INSERT SELECT -> DROP -> RENAME.
-- PRAGMA foreign_keys=OFF на время миграции (иначе SQLite будет проверять
-- ссылки до того как все таблицы пересозданы).
--
-- Перед запуском:
--   1. Сделать бэкап:  cp gamedata.sqlite gamedata.sqlite.bak.YYYYMMDD
--   2. Прогнать sanitize_sentinels.sql (заменяет -1/0 на NULL).
--
-- Запуск:
--   sqlite3 databases/gamedata.sqlite < migrate_add_foreign_keys.sql
--
-- Проверка после:
--   PRAGMA foreign_key_check;  -- должен быть пустой результат
--   PRAGMA integrity_check;    -- должен вернуть 'ok'

.bail on
PRAGMA foreign_keys = OFF;

BEGIN TRANSACTION;

-- ============================================================
-- 1) characters
-- ============================================================
CREATE TABLE characters_new (
			id                INTEGER PRIMARY KEY,
			name              TEXT NOT NULL,
			icon_name         TEXT,
			modal_type        INTEGER,
			ctrl_type         INTEGER,
			model             INTEGER,
			suit_id           INTEGER,
			suit_num          INTEGER,
			skin_info         TEXT,
			feff_id           TEXT,
			eeff_id           INTEGER REFERENCES effects(id),
			effect_action_id  TEXT,
			shadow            INTEGER,
			action_id         INTEGER,
			diaphaneity       INTEGER,
			footfall          INTEGER,
			whoop             INTEGER,
			dirge             INTEGER,
			control_able      INTEGER,
			territory         INTEGER,
			sea_height        INTEGER,
			item_type         TEXT,
			lengh             REAL,
			width             REAL,
			height            REAL,
			radii             INTEGER,
			birth_behave      TEXT,
			died_behave       TEXT,
			born_eff          INTEGER REFERENCES effects(id),
			die_eff           INTEGER REFERENCES effects(id),
			dormancy          INTEGER,
			die_action        INTEGER,
			hp_effect         TEXT,
			is_face           INTEGER,
			is_cyclone        INTEGER,
			script            INTEGER,
			weapon            INTEGER REFERENCES items(id),
			skill_ids         TEXT,
			skill_lvs         TEXT,
			item_ids          TEXT,
			item_counts       TEXT,
			max_show_item     INTEGER,
			all_show          REAL,
			prefix            INTEGER REFERENCES item_prefixes(id),
			task_item_ids     TEXT,
			task_item_counts  TEXT,
			ai_no             INTEGER,
			can_turn          INTEGER,
			vision            INTEGER,
			noise             INTEGER,
			get_exp           INTEGER,
			light             INTEGER,
			mobexp            INTEGER,
			lv                INTEGER,
			mx_hp             INTEGER,
			hp                INTEGER,
			mx_sp             INTEGER,
			sp                INTEGER,
			mn_atk            INTEGER,
			mx_atk            INTEGER,
			p_def             INTEGER,
			def               INTEGER,
			hit               INTEGER,
			flee              INTEGER,
			crt               INTEGER,
			mf                INTEGER,
			h_rec             INTEGER,
			s_rec             INTEGER,
			a_spd             INTEGER,
			a_dis             INTEGER,
			c_dis             INTEGER,
			m_spd             INTEGER,
			col               INTEGER,
			str               INTEGER,
			agi               INTEGER,
			dex               INTEGER,
			con               INTEGER,
			sta               INTEGER,
			luk               INTEGER,
			l_hand_val        INTEGER,
			guild             TEXT,
			title             TEXT,
			job               TEXT,
			c_exp             INTEGER,
			n_exp             INTEGER,
			fame              INTEGER,
			ap                INTEGER,
			tp                INTEGER,
			gd                INTEGER,
			spri              INTEGER,
			stor              INTEGER,
			mx_sail           INTEGER,
			sail              INTEGER,
			stasa             INTEGER,
			scsm              INTEGER,
			t_str             INTEGER,
			t_agi             INTEGER,
			t_dex             INTEGER,
			t_con             INTEGER,
			t_sta             INTEGER,
			t_luk             INTEGER,
			t_mx_hp           INTEGER,
			t_mx_sp           INTEGER,
			t_atk             INTEGER,
			t_def             INTEGER,
			t_hit             INTEGER,
			t_flee            INTEGER,
			t_mf              INTEGER,
			t_crt             INTEGER,
			t_h_rec           INTEGER,
			t_s_rec           INTEGER,
			t_a_spd           INTEGER,
			t_a_dis           INTEGER,
			t_spd             INTEGER,
			t_spri            INTEGER,
			t_scsm            INTEGER,
			scaling           TEXT
);
INSERT INTO characters_new SELECT * FROM characters;
DROP TABLE characters;
ALTER TABLE characters_new RENAME TO characters;

-- ============================================================
-- 2) skills
-- ============================================================
CREATE TABLE skills_new (
			id                  INTEGER PRIMARY KEY,
			name                TEXT NOT NULL,
			fight_type          INTEGER,
			job_select          TEXT,
			item_need_1         TEXT,
			item_need_2         TEXT,
			item_need_3         TEXT,
			conch_need          TEXT,
			phase               INTEGER,
			type                INTEGER,
			helpful             INTEGER,
			level_demand        INTEGER,
			premiss_skill       TEXT,
			point_expend        INTEGER,
			src_type            INTEGER,
			tar_type            INTEGER,
			apply_distance      INTEGER,
			apply_target        INTEGER,
			apply_type          INTEGER,
			angle               INTEGER,
			radii               INTEGER,
			range_val           INTEGER,
			prepare             TEXT,
			range_state         TEXT,
			use_sp              TEXT,
			use_endure          TEXT,
			use_energy          TEXT,
			set_range           TEXT,
			use_script          TEXT,
			effect              TEXT,
			active              TEXT,
			inactive            TEXT,
			state_id            INTEGER REFERENCES skill_states(id),
			splash_para         INTEGER,
			target_effect       INTEGER REFERENCES effects(id),
			splash_effect       INTEGER REFERENCES effects(id),
			variation           INTEGER,
			summon              INTEGER REFERENCES characters(id),
			fire_speed          TEXT,
			action_harm         INTEGER,
			action_play_type    INTEGER,
			action_pose         TEXT,
			action_key_frame    INTEGER,
			whop                INTEGER,
			action_dummy_link   TEXT,
			action_effect       TEXT,
			action_effect_type  TEXT,
			item_dummy_link     INTEGER,
			item_effect_1       TEXT,
			item_effect_2       TEXT,
			sky_eff_key_frame   INTEGER,
			sky_eff_dummy_link  INTEGER,
			sky_eff_item_dummy  INTEGER,
			sky_effect          INTEGER,
			sky_spd             INTEGER,
			whoped              INTEGER,
			target_dummy_link   INTEGER,
			target_effect_id    INTEGER REFERENCES effects(id),
			target_effect_time  INTEGER,
			aground_effect_id   INTEGER REFERENCES effects(id),
			water_effect_id     INTEGER REFERENCES effects(id),
			icon                TEXT,
			play_time           INTEGER,
			operate             TEXT,
			describe_hint       TEXT,
			effect_hint         TEXT,
			expend_hint         TEXT
);
INSERT INTO skills_new SELECT * FROM skills;
DROP TABLE skills;
ALTER TABLE skills_new RENAME TO skills;

-- ============================================================
-- 3) skill_states
-- ============================================================
CREATE TABLE skill_states_new (
			id              INTEGER PRIMARY KEY,
			ch_id           INTEGER REFERENCES characters(id),
			name            TEXT,
			frequency       INTEGER,
			on_transfer     TEXT,
			add_state       TEXT,
			sub_state       TEXT,
			add_type        INTEGER,
			can_cancel      INTEGER,
			can_move        INTEGER,
			can_mskill      INTEGER,
			can_gskill      INTEGER,
			can_trade       INTEGER,
			can_item        INTEGER,
			can_unbeatable  INTEGER,
			can_itemmed     INTEGER,
			can_skilled     INTEGER,
			no_hide         INTEGER,
			no_show         INTEGER,
			opt_item        INTEGER,
			talk_to_npc     INTEGER,
			free_state_id   INTEGER REFERENCES skill_states(id),
			screen          INTEGER,
			act_behave      TEXT,
			charge_link     INTEGER REFERENCES skill_states(id),
			area_effect     INTEGER REFERENCES effects(id),
			is_show_center  INTEGER,
			is_dizzy        INTEGER,
			effect          INTEGER REFERENCES effects(id),
			dummy1          INTEGER,
			bit_effect      INTEGER REFERENCES effects(id),
			dummy2          INTEGER,
			icon_id         INTEGER,
			icons           TEXT,
			descriptor      TEXT,
			colour          INTEGER
);
INSERT INTO skill_states_new SELECT * FROM skill_states;
DROP TABLE skill_states;
ALTER TABLE skill_states_new RENAME TO skill_states;

-- ============================================================
-- 4) hair
-- ============================================================
CREATE TABLE hair_new (
			id             INTEGER PRIMARY KEY,
			color          TEXT,
			need_items     TEXT,
			money          INTEGER,
			item_id        INTEGER REFERENCES items(id),
			fail_item_ids  TEXT,
			is_cha_use     TEXT
);
INSERT INTO hair_new SELECT * FROM hair;
DROP TABLE hair;
ALTER TABLE hair_new RENAME TO hair;

-- ============================================================
-- 5) stone_info
-- ============================================================
CREATE TABLE stone_info_new (
			id        INTEGER PRIMARY KEY,
			data_name TEXT,
			item_id   INTEGER REFERENCES items(id),
			equip_pos TEXT,
			type      INTEGER,
			hint_func TEXT,
			item_rgb  INTEGER
);
INSERT INTO stone_info_new SELECT * FROM stone_info;
DROP TABLE stone_info;
ALTER TABLE stone_info_new RENAME TO stone_info;

-- ============================================================
-- 6) events
-- ============================================================
CREATE TABLE events_new (
			id            INTEGER PRIMARY KEY,
			name          TEXT,
			event_type    INTEGER,
			arouse_type   INTEGER,
			arouse_radius INTEGER,
			effect        INTEGER REFERENCES effects(id),
			music         INTEGER REFERENCES music(id),
			born_effect   INTEGER REFERENCES effects(id),
			cursor        INTEGER,
			main_cha_type INTEGER REFERENCES characters(id)
);
INSERT INTO events_new SELECT * FROM events;
DROP TABLE events;
ALTER TABLE events_new RENAME TO events;

-- ============================================================
-- 7) areas
-- ============================================================
CREATE TABLE areas_new (
			id          INTEGER PRIMARY KEY,
			name        TEXT,
			color       INTEGER,
			music       INTEGER REFERENCES music(id),
			env_color   INTEGER,
			light_color INTEGER,
			light_dir   TEXT,
			type        INTEGER
);
INSERT INTO areas_new SELECT * FROM areas;
DROP TABLE areas;
ALTER TABLE areas_new RENAME TO areas;

-- ============================================================
-- 8) scene_objects
-- ============================================================
CREATE TABLE scene_objects_new (
			id                 INTEGER PRIMARY KEY,
			data_name          TEXT,
			name               TEXT,
			type               INTEGER,
			point_color        TEXT,
			env_color           TEXT,
			range              INTEGER,
			attenuation        REAL,
			anim_ctrl_id       INTEGER,
			attach_effect_id   INTEGER REFERENCES effects(id),
			enable_env_light   INTEGER,
			enable_point_light INTEGER,
			style              INTEGER,
			flag               INTEGER,
			size_flag          INTEGER,
			env_sound          TEXT,
			env_sound_dis      INTEGER,
			shade_flag         INTEGER,
			is_really_big      INTEGER,
			fade_obj_num       INTEGER,
			fade_obj_seq       TEXT,
			fade_coefficient   REAL
);
INSERT INTO scene_objects_new SELECT * FROM scene_objects;
DROP TABLE scene_objects;
ALTER TABLE scene_objects_new RENAME TO scene_objects;

-- ============================================================
-- 9) character_models / character_actions / pose_data : character_type → characters.id
-- ============================================================
CREATE TABLE character_models_new (
			character_type INTEGER PRIMARY KEY REFERENCES characters(id),
			bone  TEXT NOT NULL DEFAULT '',
			skin1 TEXT NOT NULL DEFAULT '',
			skin2 TEXT NOT NULL DEFAULT '',
			skin3 TEXT NOT NULL DEFAULT '',
			skin4 TEXT NOT NULL DEFAULT '',
			skin5 TEXT NOT NULL DEFAULT ''
);
INSERT INTO character_models_new SELECT * FROM character_models;
DROP TABLE character_models;
ALTER TABLE character_models_new RENAME TO character_models;

CREATE TABLE character_actions_new (
			character_type INTEGER NOT NULL REFERENCES characters(id),
			action_no      INTEGER NOT NULL,
			start_frame    INTEGER NOT NULL,
			end_frame      INTEGER NOT NULL,
			key_frames     TEXT,
			PRIMARY KEY (character_type, action_no)
);
INSERT INTO character_actions_new SELECT * FROM character_actions;
DROP TABLE character_actions;
ALTER TABLE character_actions_new RENAME TO character_actions;

CREATE TABLE pose_data_new (
			character_type INTEGER NOT NULL REFERENCES characters(id),
			action_id      INTEGER NOT NULL,
			start_frame    INTEGER NOT NULL,
			end_frame      INTEGER NOT NULL,
			keyframes      TEXT NOT NULL DEFAULT '',
			PRIMARY KEY (character_type, action_id)
);
INSERT INTO pose_data_new SELECT * FROM pose_data;
DROP TABLE pose_data;
ALTER TABLE pose_data_new RENAME TO pose_data;

-- ============================================================
-- Индексы для FK-колонок (ускоряют JOIN и каскад-проверки).
-- light_id → animated_lights.light_no без FK (parent column не уникальный) — обычный индекс.
-- ============================================================
CREATE INDEX IF NOT EXISTS idx_characters_weapon         ON characters(weapon);
CREATE INDEX IF NOT EXISTS idx_characters_prefix         ON characters(prefix);
CREATE INDEX IF NOT EXISTS idx_characters_eeff_id        ON characters(eeff_id);
CREATE INDEX IF NOT EXISTS idx_characters_born_eff       ON characters(born_eff);
CREATE INDEX IF NOT EXISTS idx_characters_die_eff        ON characters(die_eff);

CREATE INDEX IF NOT EXISTS idx_skills_state_id           ON skills(state_id);
CREATE INDEX IF NOT EXISTS idx_skills_target_effect      ON skills(target_effect);
CREATE INDEX IF NOT EXISTS idx_skills_splash_effect      ON skills(splash_effect);
CREATE INDEX IF NOT EXISTS idx_skills_target_effect_id   ON skills(target_effect_id);
CREATE INDEX IF NOT EXISTS idx_skills_aground_effect_id  ON skills(aground_effect_id);
CREATE INDEX IF NOT EXISTS idx_skills_water_effect_id    ON skills(water_effect_id);
CREATE INDEX IF NOT EXISTS idx_skills_summon             ON skills(summon);

CREATE INDEX IF NOT EXISTS idx_skill_states_ch_id        ON skill_states(ch_id);
CREATE INDEX IF NOT EXISTS idx_skill_states_free_state   ON skill_states(free_state_id);
CREATE INDEX IF NOT EXISTS idx_skill_states_charge_link  ON skill_states(charge_link);
CREATE INDEX IF NOT EXISTS idx_skill_states_area_effect  ON skill_states(area_effect);
CREATE INDEX IF NOT EXISTS idx_skill_states_effect       ON skill_states(effect);
CREATE INDEX IF NOT EXISTS idx_skill_states_bit_effect   ON skill_states(bit_effect);

CREATE INDEX IF NOT EXISTS idx_hair_item_id              ON hair(item_id);
CREATE INDEX IF NOT EXISTS idx_stone_info_item_id        ON stone_info(item_id);

CREATE INDEX IF NOT EXISTS idx_events_effect             ON events(effect);
CREATE INDEX IF NOT EXISTS idx_events_born_effect        ON events(born_effect);
CREATE INDEX IF NOT EXISTS idx_events_music              ON events(music);
CREATE INDEX IF NOT EXISTS idx_events_main_cha_type      ON events(main_cha_type);

CREATE INDEX IF NOT EXISTS idx_areas_music               ON areas(music);
CREATE INDEX IF NOT EXISTS idx_scene_objects_attach_eff  ON scene_objects(attach_effect_id);

CREATE INDEX IF NOT EXISTS idx_effects_light_id          ON effects(light_id);
CREATE INDEX IF NOT EXISTS idx_eff_params_light_id       ON eff_params(light_id);
CREATE INDEX IF NOT EXISTS idx_item_refine_eff_light_id  ON item_refine_effects(light_id);

COMMIT;

-- ============================================================
-- Верификация: проверяем что FK ни одной строкой не нарушены и БД целая
-- ============================================================
PRAGMA foreign_keys = ON;
PRAGMA foreign_key_check;
PRAGMA integrity_check;
.print "migration complete"
