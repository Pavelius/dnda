#include "main.h"

static struct tile_info {
	const char*		name;
	char			movement;
	const char*		description;
} tile_data[] = {{},
{"Равнина", 1, "Прекрасная зеленая равнина, покытая травой."},
{"Вода", 10, "Глубокая вода находится тут."},
{"Пол", 0, "Обычный пол комнаты или прохода."},
{"Стена", 10, "Стена преграждает путь."},
{"Дорога", 0, "Тут проходит дорога."},
{"Болото", 4, "Небольшая лужа, расположилась на траве."},
{"Холм", 2, "Невысокие бугорки затрудняют движение."},
{"Город"},
};

static char	dex_move_slowing[] = {7,
6, 5, 4, 3, 2, 2, 1, 1, 0,
0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
0, 0, 0, 0, 0
};

const char* creature::getname(tile_s id) {
	return tile_data[id].description;
}

int	creature::getmoverecoil() const {
	auto tile = game::gettile(position);
	auto a = get(Dexterity);
	auto result = 3;
	result += tile_data[tile].movement;
	result += maptbl(dex_move_slowing, a);
	// RULE: Encumbrance slow movement.
	if(getencumbrance())
		result += 4;
	return result;
}