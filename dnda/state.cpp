#include "main.h"

static struct state_info {
	const char*			name;
} state_data[] = {{"����"},
{"������������"},
{"��������"},
{"��������"},
{"������"},
{"��������"},
{"�������"},
{"�������"}
};
assert_enum(state, Scared);
getstr_enum(state);