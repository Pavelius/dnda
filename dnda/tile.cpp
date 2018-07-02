#include "main.h"

static struct tile_info {
	const char*		name;
	char			movement;
} tile_data[] = {{},
{"Равнина", 1},
{"Вода", 10},
{"Пол"},
{"Стена", 10},
{"Дорога", 0},
{"Болото", 4},
{"Холм", 2},
{"Город"},
};

int	creature::getmoverecoil() const {
	auto tile = game::gettile(position);
	int result = 3 + tile_data[tile].movement;
	return result;
}