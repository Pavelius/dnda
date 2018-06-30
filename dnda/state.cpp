#include "main.h"

static struct state_info {
	const char*			name;
} state_data[] = {{"Злой"},
{"Благославлен"},
{"Очарован"},
{"Прячется"},
{"Щедрый"},
{"Светится"},
{"Отравлен"},
{"Отравлен"},
{"Отравлен"},
{"Защищен"},
{"Страшен"},
{"Спящий"}
};
assert_enum(state, LastState);
getstr_enum(state);