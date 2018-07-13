#include "main.h"

static struct role_info {
	const char*		name;
	race_s			race;
	gender_s		gender;
	alignment_s		alignment;
	class_s			type;
	char			level;
	char			abilities[Charisma + 1];
	adat<variant, 24> features;
	adat<special_s, 4> special;
} role_data[] = {{"Гоблин", Goblinoid, Male, Chaotic, Fighter, 0, {8, 12, 8, 8, 8, 8}, {SwordShort}},
{"Орк", Goblinoid, Male, Chaotic, Fighter, 1, {13, 10, 10, 8, 8, 8}, {SwordLong}},
{"Летучая мышь", Animal, Female, Chaotic, Fighter, 0, {8, 12, 4, 3, 3, 3}, {Bite}},
{"Крыса", Animal, Female, Chaotic, Fighter, 0, {6, 10, 4, 3, 3, 3}, {Bite}},
{"Крестьянин", Human, Male, Neutral, Commoner, 0, {10, 10, 10, 10, 10, 10}},
{"Охранник", Human, Male, Neutral, Fighter, 1, {15, 10, 10, 10, 10, 10}, {Spear}},
{"Ребенок", Human, Male, Neutral, Commoner, 0, {8, 8, 6, 6, 6, 10}},
{"Крестьянка", Human, Male, Neutral, Commoner, 0, {10, 10, 10, 10, 10, 10}},
{"Владелец магазина", Human, Male, Neutral, Commoner, 0, {10, 10, 10, 11, 10, 13}, {Bargaining}},
{"Кузнец", Dwarf, Male, Neutral, Commoner, 0, {16, 10, 12, 10, 10, 10}, {HammerWar}},
{"Бартендер", Dwarf, Male, Neutral, Commoner, 0, {10, 10, 12, 10, 12, 10}},
{"Скелет", Human, Male, Chaotic, Fighter, 1, {10, 13, 10, 10, 10, 10}, {Spear}},
{"Зомби", Human, Male, Chaotic, Fighter, 2, {15, 10, 16, 10, 10, 10}, {Dagger}},
};
assert_enum(role, Character-1);
getstr_enum(role);

bool creature::is(special_s value) const {
	if(role == Character)
		return false;
	return role_data[role].special.is(value);
}

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
	for(auto i : e.features) {
		switch(i.type) {
		case Item: equip(item(i.item, 0, 10, 10, 30)); break;
		case Skill: raise(i.skill); break;
		default:
			break;
		}
	}
	// Восполним хиты
	mhp = 0;
	if(level) {
		for(int i = 0; i < level; i++)
			mhp += 1 + (rand() % 6);
	} else
		mhp = 1 + (rand() % 4);
	hp = getmaxhits();
	// Энергия
	mmp = 0;
	mp = getmaxmana();
}