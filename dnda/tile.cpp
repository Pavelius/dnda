#include "main.h"

static struct tile_info {
	const char*		name;
	char			movement;
} tile_data[] = {{},
{"�������", 1},
{"����", 10},
{"���"},
{"�����", 10},
{"������", 0},
{"������", 4},
{"����", 2},
{"�����"},
};

int	creature::getmoverecoil() const {
	auto tile = game::gettile(position);
	int result = 3 + tile_data[tile].movement;
	return result;
}