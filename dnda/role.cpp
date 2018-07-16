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
} role_data[] = {{"������", Goblin, Male, Chaotic, Monster, 0, {SwordShort}},
{"������", Goblin, Male, Chaotic, Monster, 0, {Rock}},
{"���", Orc, Male, Chaotic, Monster, 1, {SwordLong, StuddedLeatherArmour}},
{"������� ����", Animal, Female, Chaotic, Monster, 0, {Bite, Dexterity}},
{"�����", Animal, Female, Chaotic, Monster, 0, {Bite, Dexterity}},
{"����������", Human, Male, Neutral, Commoner, 1},
{"��������", Human, Male, Neutral, Fighter, 1, {Spear}},
{"�������", Human, Male, Neutral, Commoner},
{"����������", Human, Male, Neutral, Commoner, 1},
{"�������� ��������", Human, Male, Neutral, Commoner, 1, {Bargaining}},
{"������", Dwarf, Male, Neutral, Commoner, 1, {HammerWar}},
{"���������", Dwarf, Male, Neutral, Commoner, 1, },
{"������", Undead, Male, Chaotic, Monster, 1, {Spear, Dexterity}},
{"�����", Undead, Male, Chaotic, Monster, 2, {Dagger, Strenght}},
{"�������", Kobold, Male, Chaotic, Monster, 0, {BowShort, Dagger}},
{"������", Animal, Female, Neutral, Monster, 2, {Bite, Dexterity}},
{"����", Animal, Female, Chaotic, Monster, 4, {Bite, Fur, Dexterity, Strenght, Constitution}},
{"�������", Animal, Female, Neutral, Monster, 1, {Bite, Dexterity}},
{"�������", Insect, Male, Chaotic, Monster, 0, {Bite, Hitin}},
{"�������-����", Insect, Male, Chaotic, Monster, 1, {item(Bite, OfPoison), Hitin, Strenght}},
{"����� ��������", Insect, Female, Chaotic, Monster, 5, {item(Bite, OfPoison).setquality(2).set(BlessedItem), Hitin, Strenght, Strenght, Constitution}},
{"��������", Human, Male, Neutral, Commoner},
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
	this->role = role;
	applyability();
	for(auto i : e.features) {
		switch(i.type) {
		case Item:
		case ItemObject:
			if(i.type == Item)
				it = item(i.itemtype, 0, 15, 10, 35);
			else
				it = i.itemobject;
			it.set(KnowEffect);
			equip(it);
			if(it.getammo())
				equip(item(it.getammo(), 0, 15, 10, 35).set(KnowEffect));
			break;
		case Skill:
			raise(i.skill);
			break;
		case Ability:
			abilities[i.ability] += 2;
			break;
		default: break;
		}
	}
	// ��������� ����
	for(int i = 0; i < e.level; i++)
		levelup();
	hp = getmaxhits();
	mp = getmaxmana();
}