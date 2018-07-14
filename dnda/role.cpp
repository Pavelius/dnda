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
} role_data[] = {{"������", Goblin, Male, Chaotic, Fighter, 0, {SwordShort}},
{"���", Orc, Male, Chaotic, Fighter, 1, {SwordLong, StuddedLeatherArmour}},
{"������� ����", Animal, Female, Chaotic, Fighter, 0, {Bite}},
{"�����", Animal, Female, Chaotic, Fighter, 0, {Bite}},
{"����������", Human, Male, Neutral, Commoner, 0},
{"��������", Human, Male, Neutral, Fighter, 1, {Spear}},
{"�������", Human, Male, Neutral, Commoner},
{"����������", Human, Male, Neutral, Commoner},
{"�������� ��������", Human, Male, Neutral, Commoner, 0, {Bargaining}},
{"������", Dwarf, Male, Neutral, Commoner, 0, {HammerWar}},
{"���������", Dwarf, Male, Neutral, Commoner, 0, },
{"������", Human, Male, Chaotic, Fighter, 1, {Spear, Dexterity}},
{"�����", Human, Male, Chaotic, Fighter, 2, {Dagger, Strenght}},
{"��������", Human, Male, Neutral, Fighter},
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