#include "main.h"

struct ability_info {
	const char*		name;
} ability_data[] = {{"Сила"},
{"Ловкость"},
{"Телосложение"},
{"Интеллект"},
{"Мудрость"},
{"Харизма"},
};
assert_enum(ability, Charisma);
getstr_enum(ability);