#include "main.h"

static struct site_info {
	const char*		name;
} site_data[] = {{"�������"},
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
assert_enum(site, ShopFood);
getstr_enum(site);