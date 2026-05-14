#pragma once

#include <cstdint>

// Состояние оружия персонажа («wield-mode»). Определяет, какой из 7 вариантов
// реальной анимации брать из CPoseInfo::RealPoseId[]. Значения 0..6 — это
// единственный валидный домен индекса в RealPoseId; Count держим как явный
// sentinel для статических проверок и аксессоров.

namespace Corsairs::Common::Character {

enum class WieldMode : std::uint8_t {
	SingleMelee   = 0, // голые руки / щит без оружия
	SingleMelee2  = 1, // одноручное (меч / blade / stick / hammer / ax / shovel)
	DoubleMelee   = 2, // двуручное (huge sword)
	DoubleWeapon  = 3, // парное оружие (sword + sword)
	SingleGun     = 4,
	DoubleBow     = 5,
	SingleDagger  = 6,
	Count         = 7,
};

inline constexpr std::size_t kPoseVariantCount = static_cast<std::size_t>(WieldMode::Count);

} // namespace Corsairs::Common::Character
