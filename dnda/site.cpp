#include "main.h"

static struct site_info {
	struct search_item { // This is for skill Alertness
		const char*		text;
		char			chance; // Bonus to skill Alertness
		char			quality; // Item quality
		char			magic; // Blessed or Cursed is fate decided chance to be artifact is magic/10.
		aref<item_s>	items;
		explicit operator bool() const { return text != 0; }
	};
	struct monster_info {
		role_s			owner;
		role_s			wandering;
	};
	const char*			name;
	const char*			entering; // Message appear to player when enter to room
	search_item			search;
	monster_info		appear;
	aref<effectinfo>	skills;
} site_data[] = {{"Комната"},
{"Комната"},
{"Лестница вниз"},
{"Лестница вверх"},
{"Дом"},
{"Храм"},
{"Таверна"},
{"Бараки"},
{"Городской совет"},
{"Магазин оружия и брони", "Небольшая комната в которой на столах и на стойках стояло множетсво оружия на продажу."},
{"Магазин свитков и зельев", "Зал был заставлен шкафами в которых лежали свитки и тома. Рядом стояли столы с лежащими на них бутылочках непонятного содержимымого."},
{"Магазин еды"},
};
assert_enum(site, ShopFood);
getstr_enum(site);

int	site::getfoundchance() const {
	if(found > 5)
		return 0;
	return 100 - (found * 20);
}