#include "main.h"

static struct location_info {
	const char*		name;
} location_data[] = {{"�������"},
{"�������"},
{"�������� ����"},
{"�������� �����"},
{"���"},
{"����"},
{"�������"},
{"������� ������ � �����"},
{"������� ������� � ������"},
{"������� ���"},
};
assert_enum(location, ShopFood);
getstr_enum(location);