#include "main.h"

static struct magic_info {
	const char*		name;
} magic_data[] = {{""},
{"�����"},
{"���������"},
{"��������"},
{"��������"},
{"����������"},
// ��������
{"����"},
{"��������"},
{"������������"},
{"����������"},
{"��������"},
{"�������"}
};
assert_enum(magic, OfCharisma);
getstr_enum(magic);