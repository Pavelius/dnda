#include "main.h"

static struct role_info {
	const char*			name;
	race_s				race;
	gender_s			gender;
	alignment_s			alignment;
	class_s				type;
	char				level;
	adat<variant, 16>	features;
	role_s				minions[4];
} role_data[] = {{"Гоблин", Goblin, Male, Chaotic, Monster, 0, {SwordShort}},
{"Гоблин", Goblin, Male, Chaotic, Monster, 0, {Rock}},
{"Орк", Orc, Male, Chaotic, Monster, 1, {SwordLong, StuddedLeatherArmour}},
{"Летучая мышь", Animal, Female, Chaotic, Monster, 0, {Bite, Dexterity}},
{"Крыса", Animal, Female, Chaotic, Monster, 0, {Bite, Dexterity}},
{"Крестьянин", Human, Male, Neutral, Commoner, 1},
{"Охранник", Human, Male, Neutral, Fighter, 1, {Spear}},
{"Ребенок", Human, Male, Neutral, Commoner},
{"Крестьянка", Human, Male, Neutral, Commoner, 1},
{"Владелец магазина", Human, Male, Neutral, Commoner, 1, {Bargaining}},
{"Кузнец", Dwarf, Male, Neutral, Commoner, 1, {HammerWar}},
{"Бартендер", Dwarf, Male, Neutral, Commoner, 1, },
{"Скелет", Undead, Male, Chaotic, Monster, 1, {Spear, Dexterity}},
{"Зомби", Undead, Male, Chaotic, Monster, 2, {Dagger, Strenght}},
{"Кобольд", Kobold, Male, Chaotic, Monster, 0, {BowShort, Dagger}},
{"Кобольд-шаман", Kobold, Male, Chaotic, Mage, 3, {Staff, Wand1, MagicMissile}},
{"Собака", Animal, Female, Neutral, Monster, 2, {Bite, Dexterity}},
{"Рысь", Animal, Female, Chaotic, Monster, 4, {Bite, Fur, Dexterity, Strenght, Constitution}},
{"Лягушка", Animal, Female, Neutral, Monster, 1, {Bite, Dexterity}},
{"Муравей", Insect, Male, Chaotic, Monster, 0, {Bite, Hitin}},
{"Муравей-воин", Insect, Male, Chaotic, Monster, 1, {item(Bite, OfPoison), Hitin, Strenght}},
{"Матка муравьев", Insect, Female, Chaotic, Monster, 5, {item(Bite, OfPoison).setquality(2).set(BlessedItem), Hitin, Strenght, Strenght, Constitution}, {AntWorker, AntWorker, AntWarrior, AntWarrior}},
{"Гнолл", Gnoll, Male, Chaotic, Monster, 2, {AxeBattle, Fur}},
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

aref<role_s> creature::select(aref<role_s> result, int min_level, int max_level, alignment_s aligment, const race_s races[4]) {
	auto pb = result.data;
	auto pe = result.data + result.count;
	for(auto& e : role_data) {
		if(e.level < min_level || e.level > max_level)
			continue;
		if(e.alignment != aligment)
			continue;
		if(races && e.race!=races[0] && e.race != races[1] && e.race != races[2] && e.race != races[3])
			continue;
		if(pb < pe)
			*pb++ = (role_s)(&e - role_data);
		else
			break;
	}
	return aref<role_s>(result.data, pb - result.data);
}

creature::creature(role_s role) {
	clear();
	auto& e = role_data[role];
	this->race = e.race;
	this->gender = e.gender;
	this->type = e.type;
	this->role = role;
	applyability();
	apply(e.features);
	// Восполним хиты
	for(int i = 0; i < e.level; i++)
		levelup(false);
	hp = getmaxhits();
	mp = getmaxmana();
}