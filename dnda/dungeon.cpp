#include "main.h"

static dungeon dungeon_data[] = {
	{"Город Центриус", 100, 10, 0, {{AreaDungeon, 20}}}
};

aref<dungeon> game::getdungeons() {
	return dungeon_data;
}