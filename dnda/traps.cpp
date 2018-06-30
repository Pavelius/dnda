#include "main.h"

static struct trap_i {
	const char*		name;
	attackinfo		attack;
} trap_data[] = {{""}, // Нет ловушки
{"Змеинная яма", {0, {1, 6}}},
{"Яма с кислотой", {0, {3, 18, Acid}}},
{"Самострел", {0, {1, 6, Piercing}, 4}},
{"Проклятое место", {0, {1, 6}}},
{"Плита под напряжением", {0, {3, 18, Electricity}}},
{"Огненная плита", {0, {3, 18, Fire}}},
{"Плита света", {0, {1, 6}}},
{"Яма", {0, {1, 6}}},
{"Яма с копьями", {0, {2, 12, Piercing}}},
{"Стрельба копьем", {0, {1, 8, Piercing}}},
{"Выезжающие ножи", {0, {1, 6, Slashing}}},
{"Корррозийные испарения", {0, {1, 3, Acid}}},
{"Гейзер", {0, {1, 6, WaterAttack}}},
};
assert_enum(trap, TrapWater);
getstr_enum(trap);

const attackinfo& game::getattackinfo(trap_s value) {
	return trap_data[value].attack;
}