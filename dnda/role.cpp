#include "main.h"

static struct role_info {
	const char*		name;
	race_s			race;
	gender_s		gender;
	alignment_s		alignment;
	class_s			type;
	char			level;
	adat<variant, 24> features;
} role_data[] = {{"������", Goblin, Male, Chaotic, Fighter, 0, {SwordShort}},
{"���", Orc, Male, Chaotic, Fighter, 1, {SwordLong, StuddedLeatherArmour}},
{"������� ����", Animal, Female, Chaotic, Fighter, 0, {Bite, Dexterity}},
{"�����", Animal, Female, Chaotic, Fighter, 0, {Bite, Dexterity}},
{"����������", Human, Male, Neutral, Commoner, 0},
{"��������", Human, Male, Neutral, Fighter, 1, {Spear}},
{"�������", Human, Male, Neutral, Commoner},
{"����������", Human, Male, Neutral, Commoner},
{"�������� ��������", Human, Male, Neutral, Commoner, 0, {Bargaining}},
{"������", Dwarf, Male, Neutral, Commoner, 0, {HammerWar}},
{"���������", Dwarf, Male, Neutral, Commoner, 0, },
{"������", Undead, Male, Chaotic, Fighter, 1, {Spear, Dexterity}},
{"�����", Undead, Male, Chaotic, Fighter, 2, {Dagger, Strenght}},
{"�������", Kobold, Male, Chaotic, Fighter, 0, {BowShort, Dagger}},
{"��������", Human, Male, Neutral, Fighter},
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
	// ��������� ����
	mhp = 0;
	if(level) {
		for(int i = 0; i < level; i++)
			mhp += 1 + (rand() % 6);
	} else
		mhp = 1 + (rand() % 4);
	hp = getmaxhits();
	// �������
	mmp = 0;
	mp = getmaxmana();
}