#include "main.h"

static struct trap_i {
	const char*		name;
	attackinfo		attack;
} trap_data[] = {
	{""}, // Нет ловушки
{"Змеинная яма", {Bludgeon, 0, 0, {1, 6}}},
{"Яма с кислотой", {Bludgeon, 0, 0, {3, 18}}},
{"Самострел", {Bludgeon, 0, 0, {1, 6}}},
{"Проклятое место", {Bludgeon, 0, 0, {1, 6}}},
{"Плита под напряжением", {Bludgeon, 0, 0, {3, 18}}},
{"Огненная плита", {Fire, 0, 0, {3, 18}}},
{"Плита света", {Bludgeon, 0, 0, {1, 6}}},
{"Яма", {Bludgeon, 0, 0, {1, 6}}},
{"Яма с копьями", {Piercing, 0, 0, {2, 12}}},
{"Стрельба копьем", {Piercing, 0, 0, {1, 8}}},
{"Выезжающие ножи", {Slashing, 0, 0, {1, 6}}},
{"Корррозийные испарения", {Acid, 0, 0, {1, 3}}},
{"Гейзер", {WaterAttack, 0, 0, {1, 6}}},
};
assert_enum(trap, TrapWater);
getstr_enum(trap);

const attackinfo& getattackinfo(trap_s value) {
	return trap_data[value].attack;
}