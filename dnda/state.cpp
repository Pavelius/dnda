#include "main.h"

static struct state_info {
	const char*		name;
	const char*		nameof;
	const char*		namehow;
	state_s			cursed_state;
} state_data[] = {{""},
{"Злой", "злости", "злее", PoisonedWeak},
{"Бронирован", "брони", "защищенней", PoisonedWeak},
{"Благославлен", "благословения", "защищенней", PoisonedWeak},
{"Очарован", "очарования", "дурнее", PoisonedWeak},
{"Невидим", "невидимости", "прозрачнее", PoisonedWeak},
{"Щедрый", "щедрости", "щедрее", Anger},
{"Светится", "света", "светлее", PoisonedWeak},
{"Отравлен", "отравления", "хуже", PoisonedStrong},
{"Отравлен", "яда", "хуже", PoisonedStrong},
{"Отравлен", "сильного яда", "хуже", PoisonedStrong},
{"Щит", "защиты", "защищенней", PoisonedWeak},
{"Болен", "болезни", "слабее", PoisonedStrong},
{"Страшен", "ужаса", "испуганней", PoisonedWeak},
{"Спящий", "сна", "хуже", PoisonedWeak},
{"Ослаблен", "ослабления", "слабее", Sick},
//
{"Сильный", "силы", "сильнее", NoState},
{"Ловкий", "ловкости", "ловче", NoState},
{"Здоровый", "здоровья", "здоровее", NoState},
{"Умный", "ума", "умнее", NoState},
{"Мудрый", "мудрости", "мудрее", NoState},
{"Красивый", "красоты", "красивее", NoState},
//
{"Лечение", "лечения", "здоровее", Sick},
{"Исцелить болезнь", "исцеления болезни", "здоровее", Sick},
{"Исцелить яд", "противоядия", "здоровее", Poisoned},
};
assert_enum(state, LastEffectState);
getstr_enum(state);

static const char* ability_worse[] = {"слабее", "неулюже", "больнее",
"глупее", "легкомысленнее", "уродливее"
};

const char* item::getname(state_s value) {
	return state_data[value].nameof;
}

const char* creature::getname(state_s id, bool cursed) {
	if(cursed) {
		if(state_data[id].cursed_state)
			id = state_data[id].cursed_state;
		else if(id >= Strenghted && id <= Charismatic)
			return ability_worse[id - Strenghted];
		else
			return "хуже";
	}
	return state_data[id].namehow;
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
	static damageinfo healing[] = {{5, 10}, {10, 20}, {15, 30}, {20, 40}};
	char temp[260]; it.getname(temp, zendof(temp), false); szlower(temp);
	auto itp = it.gettype();
	auto state = it.getstate();
	act("%герой выпил%а %1", temp);
	if(it.isartifact())
		hint(" и почувствовал%а себя намного %1", getname(state, false));
	else if(it.iscursed())
		hint(" и почувствовал%а себя %1", getname(state, true));
	else
		hint(" и почувствовал%а себя %1", getname(state, false));
	act(".");
	int duration = Hour;
	auto quality = it.getquality();
	auto quality_raw = it.getqualityraw();
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
				abilities[ability] -= 1 + quality_raw;
				if(abilities[ability] < 1)
					abilities[ability] = 1;
			} else if(it.isartifact())
				abilities[ability] += (1 + quality_raw);
			else
				set(state, duration + duration / 2);
		}
		break;
	case HealState:
		if(!it.iscursed()) {
			auto dice = maptbl(healing, quality_raw);
			if(it.isartifact()) // Artifact permanently add health maximum
				mhp += xrand(2, 8);
			damage(-dice.roll(), false, false);
		} else {
			auto dice = maptbl(healing, quality_raw);
			damage(dice.roll(), false, false);
		}
		break;
	default:
		if(it.iscursed() && state_data[state].cursed_state)
			set(state_data[state].cursed_state, duration);
		else
			set(state, duration);
		break;
	}
	it.clear();
}