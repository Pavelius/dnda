#include "main.h"

static struct location_info {
	const char*		name;
} location_data[] = {{"Комната"},
{"Комната"},
{"Лестница вниз"},
{"Лестница вверх"},
{"Дом"},
{"Храм"},
{"Таверна"},
{"Магазин оружия и брони"},
{"Магазин свитков и зельев"},
{"Магазин еды"},
};
assert_enum(location, ShopFood);
getstr_enum(location);