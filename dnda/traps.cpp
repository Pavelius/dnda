#include "main.h"

static struct trap_i {
	const char*		name;
	attackinfo		attack;
} trap_data[] = {
	{""}, // ��� �������
{"�������� ���", {Bludgeon, 0, 0, {1, 6}}},
{"��� � ��������", {Bludgeon, 0, 0, {3, 18}}},
{"���������", {Bludgeon, 0, 0, {1, 6}}},
{"��������� �����", {Bludgeon, 0, 0, {1, 6}}},
{"����� ��� �����������", {Bludgeon, 0, 0, {3, 18}}},
{"�������� �����", {Fire, 0, 0, {3, 18}}},
{"����� �����", {Bludgeon, 0, 0, {1, 6}}},
{"���", {Bludgeon, 0, 0, {1, 6}}},
{"��� � �������", {Piercing, 0, 0, {2, 12}}},
{"�������� ������", {Piercing, 0, 0, {1, 8}}},
{"���������� ����", {Slashing, 0, 0, {1, 6}}},
{"������������ ���������", {Acid, 0, 0, {1, 3}}},
{"������", {WaterAttack, 0, 0, {1, 6}}},
};
assert_enum(trap, TrapWater);
getstr_enum(trap);

const attackinfo& getattackinfo(trap_s value) {
	return trap_data[value].attack;
}