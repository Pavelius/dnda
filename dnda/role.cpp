#include "main.h"

static struct role_info {
	const char*			name;
	race_s				race;
	gender_s			gender;
	alignment_s			alignment;
	class_s				type;
	char				level;
	adat<variant, 24>	features;
} role_data[] = {{"Гоблин", Goblin, Male, Chaotic, Monster, 0, {SwordShort}},
{"Гоблин", Goblin, Male, Chaotic, Monster, 0, {Rock}},
{"Орк", Orc, Male, Chaotic, Monster, 1, {SwordLong, StuddedLeatherArmour}},
{"Летучая мышь", Animal, Female, Chaotic, Monster, 0, {Bite, Dexterity}},
{"Крыса", Animal, Female, Chaotic, Monster, 0, {Bite, Dexterity}},
{"Крестьянин", Human, Male, Neutral, Commoner, 0},
{"Охранник", Human, Male, Neutral, Fighter, 1, {Spear}},
{"Ребенок", Human, Male, Neutral, Commoner},
{"Крестьянка", Human, Male, Neutral, Commoner, 1},
{"Владелец магазина", Human, Male, Neutral, Commoner, 1, {Bargaining}},
{"Кузнец", Dwarf, Male, Neutral, Commoner, 1, {HammerWar}},
{"Бартендер", Dwarf, Male, Neutral, Commoner, 1, },
{"Скелет", Undead, Male, Chaotic, Monster, 1, {Spear, Dexterity}},
{"Зомби", Undead, Male, Chaotic, Monster, 2, {Dagger, Strenght}},
{"Кобольд", Kobold, Male, Chaotic, Monster, 0, {BowShort, Dagger}},
{"Собака", Animal, Female, Neutral, Monster, 2, {Bite, Dexterity}},
{"Рысь", Animal, Female, Neutral, Monster, 4, {Bite, Dexterity, Strenght, Constitution}},
{"Лягушка", Animal, Female, Neutral, Monster, 1, {Bite, Dexterity}},
{"Муравей", Insect, Male, Neutral, Monster, 0, {Bite}},
{"Муравей-воин", Insect, Male, Neutral, Monster, 1, {Bite, Strenght}},
{"Матка муравьев", Insect, Female, Neutral, Monster, 5, {Bite, Strenght, Strenght, Constitution}},
{"Персонаж", Human, Male, Neutral, Commoner},
};
assert_enum(role, Character);
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
	item it;
	clear();
	auto& e = role_data[role];
	this->race = e.race;
	this->gender = e.gender;
	this->type = e.type;
	this->level = e.level;
	this->role = role;
	applyability();
	for(auto i : e.features) {
		switch(i.type) {
		case Item:
			it = item(i.item, 0, 15, 10, 35);
			equip(it);
			if(it.getammo())
				equip(item(it.getammo(), 0, 15, 10, 35));
			break;
		case Skill: raise(i.skill); break;
		case Ability: abilities[i.ability] += 2; break;
		default: break;
		}
	}
	// Восполним хиты
	mhp = 0;
	if(e.level) {
		for(int i = 0; i < e.level; i++)
			levelup();
	} else
		mhp = 1 + (rand() % 4);
	hp = getmaxhits();
	// Энергия
	mmp = 0;
	mp = getmaxmana();
}