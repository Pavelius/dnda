#include "grammar.h"
#include "main.h"

struct names_object {
	race_s		race;
	gender_s		gender;
	const char*	name[2];
};
static names_object objects[] = {{Human, Male, {"Hawke", "�����"}},
{Human, Male, {"Rudiger", "�������"}},
{Human, Male, {"Gregor", "������"}},
{Human, Female, {"Brianne", "�����"}},
{Human, Male, {"Walton", "�������"}},
{Human, Male, {"Castor", "������"}},
{Human, Female, {"Shanna", "�����"}},
{Human, Female, {"Sonya", "����"}},
{Human, Female, {"Sun", "������"}},
{Human, Male, {"Ajax", "�����"}},
{Human, Male, {"Hob", "���"}},
{Halfling, Male, {"Finnegan", "�������"}},
{Halfling, Female, {"Olive", "������"}},
{Halfling, Male, {"Randolph", "��������"}},
{Halfling, Male, {"Bartleby", "�������"}},
{Halfling, Male, {"Aubrey", "������"}},
{Halfling, Male, {"Baldwin", "�������"}},
{Halfling, Female, {"Becca", "�����"}},
{Elf, Male, {"Elohiir", "�������"}},
{Elf, Female, {"Sharaseth", "�������"}},
{Elf, Male, {"Hasrith", "������"}},
{Elf, Male, {"Shevara", "�������"}},
{Elf, Male, {"Cadeus", "������"}},
{Elf, Male, {"Eldar", "������"}},
{Elf, Female, {"Kithracet", "��������"}},
{Elf, Female, {"Thelian", "��������"}},
{Dwarf, Male, {"Ozruk", "�����"}},
{Dwarf, Male, {"Surtur", "������"}},
{Dwarf, Female, {"Brunhilda", "���������"}},
{Dwarf, Female, {"Annika", "������"}},
{Dwarf, Male, {"Janos", "������"}},
{Dwarf, Female, {"Greta", "������"}},
{Dwarf, Male, {"Dim", "���"}},
{Dwarf, Male, {"Rundrig", "�������"}},
{Dwarf, Male, {"Jar", "����"}},
{Dwarf, Male, {"Xotoq", "������"}},
};

const char* game::getnamepart(unsigned short value) {
	return objects[value].name[1];
}

static int gen(race_s race, gender_s gender) {
	unsigned short data[sizeof(objects) / sizeof(objects[0]) + 1];
	auto p = data;
	for(unsigned i = 0; i < sizeof(objects) / sizeof(objects[0]); i++) {
		if(objects[i].race == race && objects[i].gender == gender)
			*p++ = i;
	}
	unsigned count = p - data;
	if(!count)
		return -1;
	return data[rand() % count];
}

unsigned short game::genname(race_s race, gender_s gender) {
	auto result = gen(race, gender);
	if(result == -1)
		result = gen(Human, gender);
	return result;
}