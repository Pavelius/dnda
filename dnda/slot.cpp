#include "main.h"

static struct slot_info {
	const char*			name;
} slot_data[] = {{"������"},
{"���"},
{"������ ����"},
{"����� ����"},
{"�����"},
{"����"},
{"������ �����"},
{"����� �����"},
{"�����"},
{"����"},
{"�������������"},
{"��������"}
};
assert_enum(slot, Amunitions);
getstr_enum(slot);