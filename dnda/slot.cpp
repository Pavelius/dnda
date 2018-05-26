#include "main.h"

static struct slot_info {
	const char*			name;
} slot_data[] = {{"Голова"},
{"Шея"},
{"Правая рука"},
{"Левая рука"},
{"Спина"},
{"Тело"},
{"Правый палец"},
{"Левый палец"},
{"Локти"},
{"Ноги"},
{"Дистанционное"},
{"Амуниция"}
};
assert_enum(slot, Amunitions);
getstr_enum(slot);