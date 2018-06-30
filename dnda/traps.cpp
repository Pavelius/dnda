#include "main.h"

static struct trap_i {
	const char*		name;
	attackinfo		attack;
} trap_data[] = {
	{""}, // ��� �������
{"�������� ���", {0, 0, {1, 6}}},
{"��� � ��������", {0, 0, {3, 18}}},
{"���������", {0, 0, {1, 6}}},
{"��������� �����", {0, 0, {1, 6}}},
{"����� ��� �����������", {0, 0, {3, 18}}},
{"�������� �����", {0, 0, {3, 18, Fire}}},
{"����� �����", {0, 0, {1, 6}}},
{"���", {0, 0, {1, 6}}},
{"��� � �������", {0, 0, {2, 12, Piercing}}},
{"�������� ������", {0, 0, {1, 8, Piercing}}},
{"���������� ����", {0, 0, {1, 6, Slashing}}},
{"������������ ���������", {0, 0, {1, 3, Acid}}},
{"������", {0, 0, {1, 6, WaterAttack}}},
};
assert_enum(trap, TrapWater);
getstr_enum(trap);

const attackinfo& game::getattackinfo(trap_s value) {
	return trap_data[value].attack;
}