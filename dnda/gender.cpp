#include "main.h"

struct gender_info {
	const char*		name;
} gender_data[] = {{"Трансгендер"},
{"Мужчина"},
{"Женщина"},
{"Оно"},
};
assert_enum(gender, They);
getstr_enum(gender);