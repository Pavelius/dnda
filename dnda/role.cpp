#include "main.h"

static struct role_info {
	const char*		name;
	race_s			race;
	gender_s		gender;
	alignment_s		alignment;
	class_s			type;
	char			level;
	adat<variant, 24> features;
	adat<special_s, 4> special;
} role_data[] = {{"Гоблин", Goblin, Male, Chaotic, Fighter, 0, {SwordShort}},
{"Орк", Orc, Male, Chaotic, Fighter, 1, {SwordLong, StuddedLeatherArmour}},
{"Летучая мышь", Animal, Female, Chaotic, Fighter, 0, {Bite}},
{"Крыса", Animal, Female, Chaotic, Fighter, 0, {Bite}},
{"Крестьянин", Human, Male, Neutral, Commoner, 0},
{"Охранник", Human, Male, Neutral, Fighter, 1, {Spear}},
{"Ребенок", Human, Male, Neutral, Commoner},
{"Крестьянка", Human, Male, Neutral, Commoner},
{"Владелец магазина", Human, Male, Neutral, Commoner, 0, {Bargaining}},
{"Кузнец", Dwarf, Male, Neutral, Commoner, 0, {HammerWar}},
{"Бартендер", Dwarf, Male, Neutral, Commoner, 0, },
{"Скелет", Human, Male, Chaotic, Fighter, 1, {Spear, Dexterity}},
{"Зомби", Human, Male, Chaotic, Fighter, 2, {Dagger, Strenght}},
{"Персонаж", Human, Male, Neutral, Fighter},
};
assert_enum(role, Character);
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
	applyability();
	for(auto i : e.features) {
		switch(i.type) {
		case Item: equip(item(i.item, 0, 15, 10, 35)); break;
		case Skill: raise(i.skill); break;
		case Ability: abilities[i.ability] += 2; break;
		default: break;
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