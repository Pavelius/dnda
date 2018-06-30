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
{"Страшен"},
{"Спящий"}
};
assert_enum(state, Sleeped);
getstr_enum(state);