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
	char temp[260]; it.getname(temp, zendof(temp), false); szlower(temp);
	auto itp = it.gettype();
	auto state = it.getstate();
	act("%герой выпил%а %1", temp);
	if(it.isartifact())
		hint(" и почувствовал%а себя намного %1", state_data[state].namehow);
	else if(it.iscursed() && state_data[state].cursed)
		hint(" и почувствовал%а себя %1", state_data[state].cursed);
	else if(state_data[state].namehow)
		hint(" и почувствовал%а себя %1", state_data[state].namehow);
	act(".");
	int duration = Hour;
	if(!it.iscursed())
		duration += duration * it.getquality();
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
				abilities[ability] -= 1 + it.getqualityraw();
				if(abilities[ability] < 1)
					abilities[ability] = 1;
			} else if(it.isartifact())
				abilities[ability] += 1 + it.getqualityraw();
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