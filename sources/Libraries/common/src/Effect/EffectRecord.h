#pragma once

#include <array>
#include <cstdint>
#include <string>

#include "Database/TableData.h"

// DTO одной записи таблицы эффектов (магии). Соответствует строке в SQL-таблице
// `effects`, загружается через EffectRecordStore. Историческое имя — CMagicInfo;
// текущее имя домена (папка/store/таблица) — Effect.

namespace Corsairs::Common::Effect {

// Категория визуального эффекта. Определяет, как CEffectObj::Create
// инициализирует рендер-операцию (см. switch в EffectObj.cpp).
// EFFECT_NONE — без рендер-операции; EFFECT_KIND_1..5 — индекс в OpertionList[]
// берётся из AttachId записи.
enum class EffectKind : std::uint32_t {
	EFFECT_NONE   = 0,
	EFFECT_KIND_1 = 1,
	EFFECT_KIND_2 = 2,
	EFFECT_KIND_3 = 3,
	EFFECT_KIND_4 = 4,
	EFFECT_KIND_5 = 5,
};

struct CEffectRecord : public EntityData {
	std::string  PhotoName;
	std::int32_t PhotoTexId{0};         // runtime-кеш текстуры превью, не из БД

	EffectKind   Kind{EffectKind::EFFECT_NONE};

	// Индекс операции в OpertionList[] / dummy-слот для крепления.
	// Семантика наследуется от исторического "ObjType" — это не "type",
	// а ID места привязки, поэтому в use-сайтах кладётся в setAttachID и
	// m_iIdxRender.
	std::int32_t AttachId{0};

	std::int32_t                   DummyCount{0};
	std::array<std::int32_t, 8>    Dummies{{-1, -1, -1, -1, -1, -1, -1, -1}};
	std::int32_t                   Dummy2{0};

	std::int32_t HeightOff{0};

	float        PlayTime{0};

	std::int32_t LightId{0};

	float        BaseSize{0};
};

} // namespace Corsairs::Common::Effect
