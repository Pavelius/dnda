#include "main.h"

struct map_object_info {
	const char*		name;
} map_object_data[] = {{"нет объекта"},
{"дверь"},
{"дерево"},
{"алтарь"},
{"стату€"},
{"ловушка"},
{"лестница вверх"},
{"лестница вниз"},
};
assert_enum(map_object, StairsDown);
getstr_enum(map_object);