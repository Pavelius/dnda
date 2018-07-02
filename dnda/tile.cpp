#include "main.h"

static struct tile_info {
	const char*		name;
	char			movement;
	const char*		description;
} tile_data[] = {{},
{"�������", 1, "���������� ������� �������, ������� ������."},
{"����", 10, "�������� ���� ��������� ���."},
{"���", 0, "������� ��� ������� ��� �������."},
{"�����", 10, "����� ����������� ����."},
{"������", 0, "��� �������� ������."},
{"������", 4, "��������� ����, ������������� �� �����."},
{"����", 2, "��������� ������� ���������� ��������."},
{"�����"},
};

static char	dex_move_slowing[] = {7,
6, 5, 4, 3, 2, 2, 1, 1, 0,
0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
0, 0, 0, 0, 0
};

const char* creature::getname(tile_s id) {
	return tile_data[id].description;
}

int	creature::getmoverecoil() const {
	auto tile = game::gettile(position);
	auto a = get(Dexterity);
	return 3 + tile_data[tile].movement + maptbl(dex_move_slowing, a);
}