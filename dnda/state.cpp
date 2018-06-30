#include "main.h"

static struct state_info {
	const char*		name;
	const char*		nameof;
	const char*		namehow;
	const char*		cursed;
} state_data[] = {{""},
{"Злой", "злости", "злее", 0},
{"Бронирован", "брони", "защищенней", 0},
{"Благославлен", "благословения", "защищенней", 0},
{"Красивый", "красоты", "красивее", 0},
{"Очарован", "очарования", "дурнее", 0},
{"Ловкий", "ловкости", "ловче", "неулюже"},
{"Здоровый", "здоровья", "здоровее", "больнее"},
{"Невидим", "невидимости", "прозрачнее", 0},
{"Щедрый", "щедрости", "щедрее", 0},
{"Умный", "ума", "умнее", 0},
{"Светится", "света", "светлее", 0},
{"Отравлен", "отравления", "хуже", "намного хуже"},
{"Отравлен", "яда", "хуже", "намного хуже"},
{"Отравлен", "сильного яда", "хуже", "намного хуже"},
{"Щит", "защиты", "защищенней", 0},
{"Страшен", "ужаса", "испуганней", 0},
{"Спящий", "сна", "хуже", 0},
{"Усилен", "силы", "сильнее", "слабее"},
{"Мудр", "Мудрости", "мудрее", "мудрее"},
};
assert_enum(state, LastState);
getstr_enum(state);

const char* item::getname(state_s value) {
	return state_data[value].nameof;
}

ability_s get_state_ability(state_s id) {
	switch(id) {
	case Dexterious: return Dexterity;
	case Healthy: return Constitution;
	case Intellegenced: return Intellegence;
	case Wisdowed: return Wisdow;
	case Charismatic: return Charisma;
	default: return Strenght;
	}
}

void creature::drink(item& it, bool interactive) {
	auto itp = it.gettype();
	act("%герой выпила%а %1", getstr(itp));
	if(it.isartifact())
		hint(" и почувствовал%а себя намного %1.", state_data[itp].namehow);
	else if(it.iscursed() && state_data[itp].cursed)
		hint(" и почувствовал%а себя %1.", state_data[itp].cursed);
	else if(state_data[itp].namehow)
		hint(" и почувствовал%а себя %1.", state_data[itp].namehow);
	act(".");
	auto state = it.getstate();
	auto duration = Hour;
	switch(state) {
	case Strenghted:
	case Dexterious:
	case Constitution:
	case Intellegenced:
	case Wisdowed:
	case Charismatic:
		if(true) {
			auto ability = get_state_ability(state);
			if(it.iscursed()) {
				if(abilities[ability])
					abilities[ability]--;
			} else if(it.isartifact())
				abilities[ability]++;
			else
				set(state, duration + duration / 2);
		}
		break;
	default:
		set(state, duration);
		break;
	}
	it.clear();
}