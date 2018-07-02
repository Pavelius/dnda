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

const char* creature::getname(tile_s id) {
	return tile_data[id].description;
}

int	creature::getmoverecoil() const {
	auto tile = game::gettile(position);
	int result = 3 + tile_data[tile].movement;
	return result;
}