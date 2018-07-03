#include "main.h"

static struct role_info {
	const char*		name;
	race_s			race;
	gender_s		gender;
	alignment_s		alignment;
	char			level;
	char			abilities[Charisma + 1];
	item_s			equipment[4];
	cflags<skill_s> skills;
} role_data[] = {{"Гоблин", Goblinoid, Male, Chaotic, 0, {8, 12, 8, 8, 8, 8}, {SwordShort}},
{"Орк", Goblinoid, Male, Chaotic, 1, {13, 10, 10, 8, 8, 8}, {SwordLong}},
{"Летучая мышь", Animal, Female, Chaotic, 0, {8, 12, 4, 3, 3, 3}, {Bite}},
{"Крыса", Animal, Female, Chaotic, 0, {6, 10, 4, 3, 3, 3}, {Bite}},
{"Крестьянин", Human, Male, Neutral, 0, {10, 10, 10, 10, 10, 10}, {}},
{"Охранник", Human, Male, Neutral, 1, {15, 10, 10, 10, 10, 10}, {Spear}},
{"Ребенок", Human, Male, Neutral, 0, {8, 8, 6, 6, 6, 10}},
{"Крестьянка", Human, Male, Neutral, 0, {10, 10, 10, 10, 10, 10}},
{"Владелец магазина", Human, Male, Neutral, 0, {10, 10, 10, 11, 10, 13}, {}, {Bargaining}},
{"Кузнец", Dwarf, Male, Neutral, 0, {16, 10, 12, 10, 10, 10}, {HammerWar}},
{"Бартендер", Dwarf, Male, Neutral, 0, {10, 10, 12, 10, 12, 10}},
};
assert_enum(role, Bartender);
getstr_enum(role);

bool creature::isagressive() const {
	return role_data[role].alignment == Chaotic;
}

const char* creature::getmonstername() const {
	if(role == Character)
		return 0;
	return role_data[role].name;
}

creature::creature(role_s role) {
	clear();
	auto& e = role_data[role];
	this->race = e.race;
	this->gender = e.gender;
	this->type = Fighter;
	this->level = e.level;
	this->role = role;
	for(int i = Strenght; i <= Charisma; i++)
		abilities[i] = e.abilities[i];
	for(auto i : e.equipment)
		equip(item(i, 0, 10, 10, 30));
	// Восполним хиты
	mhp = 0;
	if(level) {
		for(int i = 0; i < level; i++)
			mhp += 1 + (rand() % 6);
	} else
		mhp = 1 + (rand() % 4);
	hp = getmaxhits();
	// Добавим навыки
	for(auto i : e.skills)
		raise(i);
	// Энергия
	mmp = 0;
	mp = getmaxmana();
}