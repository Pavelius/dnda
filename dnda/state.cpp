#include "main.h"

static struct state_info {
	const char*			name;
} state_data[] = {{"Злой"},
{"Благославлен"},
{"Очарован"},
{"Прячется"},
{"Щедрый"},
{"Светится"},
{"Защищен"},
{"Страшен"}
};
assert_enum(state, Scared);
getstr_enum(state);