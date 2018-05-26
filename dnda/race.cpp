#include "main.h"

static constexpr struct race_info {
	const char*			name;
	cflags<skill_s>		skills;
} race_data[] = {{"��� ����"},
{"�������", {Bargaining, History, Gambling}},
{"����", {Smithing, Mining}},
{"����", {Survival, WeaponFocusBows}},
{"����������", {HideInShadow, Acrobatics}},
};
assert_enum(race, Halfling);
getstr_enum(race);

cflags<skill_s>	game::getskills(race_s value) {
	return race_data[value].skills;
}