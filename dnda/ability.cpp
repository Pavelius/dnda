#include "main.h"

struct ability_info {
	const char*		name;
} ability_data[] = {{"����"},
{"��������"},
{"������������"},
{"���������"},
{"��������"},
{"�������"},
};
assert_enum(ability, Charisma);
getstr_enum(ability);