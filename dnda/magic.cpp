#include "main.h"

static struct magic_info {
	const char*		name;
} magic_data[] = {{""},
{"брони"},
{"отражения"},
{"точности"},
{"скорости"},
{"разрушения"},
// Атрибуты
{"силы"},
{"ловкости"},
{"телосложения"},
{"интеллекта"},
{"мудрости"},
{"харизмы"}
};
assert_enum(magic, OfCharisma);
getstr_enum(magic);