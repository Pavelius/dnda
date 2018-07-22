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
	const char*			name;
	const char*			entering; // Message appear to player when enter to room
	search_item			search;
} site_data[] = {{"Комната"},
{"Комната"},
{"Лестница вниз"},
{"Лестница вверх"},
{"Дом"},
{"Логово", "В помещении валялось куча костей и много другого хлама."},
{"Храм"},
{"Таверна", "В комнате стояло много столов и вкусно пахло едой. Люди, которые здесь были отдыхали и пили."},
{"Бараки", "Несколько кроватей стояло вдоль стен. Похоже здесь ночует стража."},
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

void site::entering(creature& player) {
	if(site_data[type].entering)
		player.hint(site_data[type].entering);
}

void site::update() {
	if(recoil > segments)
		return;
	if(game::isdungeon())
		game::spawn(game::getfree(getposition()));
	wait(xrand(5 * Minute, 15 * Minute));
}

char* site::getname(char* result) const {
	zcpy(result, getstr(type));
	return result;
}