#include "main.h"

static struct site_info {
	const char*		name;
} site_data[] = {{"Комната"},
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
assert_enum(site, ShopFood);
getstr_enum(site);