#pragma once

#include "Database/TableData.h"
#include <vector>

// Запись таблицы действий персонажа.
// Одна строка = одно действие (ActionNo) внутри типа персонажа (CharacterType).
// _keyFrames — массив ключевых кадров (может быть пустым).

namespace Corsairs::Common::Character {

class CCharacterActionInfo : public EntityData
{
public:
	short _characterType{};
	short _actionNo{};
	short _startFrame{};
	short _endFrame{};
	std::vector<short> _keyFrames;
};

} // namespace Corsairs::Common::Character

