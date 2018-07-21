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
	monster_info		mosnter;
	adat<map_object_s, 4> objects;
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

void site::wait(unsigned value) {
	if(!recoil)
		recoil = segments;
	recoil += value;
}

short unsigned site::getposition() const {
	return game::get(x1 + width() / 2, y1 + height() / 2);
}

creature* site::add(role_s role) const {
	auto p = new creature(role);
	if(!p)
		return 0;
	p->move(game::getfree(getposition()));
	return p;
}

void site::initialize() {
	if(site_data[type].mosnter.owner)
		owner = add(site_data[type].mosnter.owner);
}

void site::entering(creature& player) {
	if(site_data[type].entering)
		player.hint(site_data[type].entering);
}