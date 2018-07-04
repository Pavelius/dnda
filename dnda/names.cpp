#include "grammar.h"
#include "main.h"

struct names_object {
	race_s		race;
	gender_s		gender;
	const char*	name[2];
};
static names_object objects[] = {{Human, Male, {"Hawke", "Хавки"}},
{Human, Male, {"Rudiger", "Рудигер"}},
{Human, Male, {"Gregor", "Грегор"}},
{Human, Female, {"Brianne", "Бриан"}},
{Human, Male, {"Walton", "Вальтон"}},
{Human, Male, {"Castor", "Кастор"}},
{Human, Female, {"Shanna", "Шанна"}},
{Human, Female, {"Sonya", "Соня"}},
{Human, Female, {"Sun", "Солнце"}},
{Human, Male, {"Ajax", "Айакс"}},
{Human, Male, {"Hob", "Хоб"}},
{Halfling, Male, {"Finnegan", "Финганн"}},
{Halfling, Female, {"Olive", "Оливия"}},
{Halfling, Male, {"Randolph", "Рэндольф"}},
{Halfling, Male, {"Bartleby", "Батлбай"}},
{Halfling, Male, {"Aubrey", "Аурбей"}},
{Halfling, Male, {"Baldwin", "Балдвин"}},
{Halfling, Female, {"Becca", "Бэкки"}},
{Elf, Male, {"Elohiir", "Эйлохир"}},
{Elf, Female, {"Sharaseth", "Харасез"}},
{Elf, Male, {"Hasrith", "Хазрич"}},
{Elf, Male, {"Shevara", "Шеварал"}},
{Elf, Male, {"Cadeus", "Кадиус"}},
{Elf, Male, {"Eldar", "Эльдар"}},
{Elf, Female, {"Kithracet", "Котораса"}},
{Elf, Female, {"Thelian", "Фелианна"}},
{Dwarf, Male, {"Ozruk", "Озрук"}},
{Dwarf, Male, {"Surtur", "Суртур"}},
{Dwarf, Female, {"Brunhilda", "Брундилла"}},
{Dwarf, Female, {"Annika", "Анника"}},
{Dwarf, Male, {"Janos", "Джанос"}},
{Dwarf, Female, {"Greta", "Гретта"}},
{Dwarf, Male, {"Dim", "Дим"}},
{Dwarf, Male, {"Rundrig", "Рундриг"}},
{Dwarf, Male, {"Jar", "Жарл"}},
{Dwarf, Male, {"Xotoq", "Ксоток"}},
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