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
{"�������"},
{"������"}
};
assert_enum(state, Sleeped);
getstr_enum(state);