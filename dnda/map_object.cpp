#include "main.h"

struct map_object_info {
	const char*		name;
} map_object_data[] = {{"��� �������"},
{"�����"},
{"������"},
{"������"},
{"������"},
{"�������"},
{"�������� �����"},
{"�������� ����"},
};
assert_enum(map_object, StairsDown);
getstr_enum(map_object);