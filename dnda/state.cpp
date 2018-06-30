#include "main.h"

static struct state_info {
	const char*			name;
} state_data[] = {{"Злой"},
{"Бронирован"},
{"Благославлен"},
{"Очарован"},
{"Прячется"},
{"Щедрый"},
{"Светится"},
{"Отравлен"},
{"Отравлен"},
{"Отравлен"},
{"Щит"},
{"Страшен"},
{"Спящий"}
};
assert_enum(state, LastState);
getstr_enum(state);