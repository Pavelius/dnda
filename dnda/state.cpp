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
{"Пьян", "опьянения", "пьянее", PoisonedWeak},
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
{"Сильный", "силы", "сильнее", Weaken},
{"Ловкий", "ловкости", "ловче", NoState},
{"Здоровый", "здоровья", "здоровее", NoState},
{"Умный", "ума", "умнее", NoState},
{"Мудрый", "мудрости", "мудрее", NoState},
{"Красивый", "красоты", "красивее", NoState},
//
{"Опыт", "опыта", "опытнее", Sick},
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

void creature::apply(state_s state, item_type_s magic, int quality_raw, unsigned duration, bool interactive) {
	static damageinfo healing[] = {{1, 5}, {5, 10}, {10, 20}, {15, 30}, {20, 40}, {30, 50}, {40, 60}};
	int quality = quality_raw + 1; // this get quality 1-4
	bool iscursed = (magic == Cursed);
	bool isartifact = (magic == Artifact);
	switch(magic) {
	case Cursed: quality = -quality; break;
	case Magical: quality = quality + 1; break; // this would be 2-5
	case Artifact: quality = quality + 2; break; // this would be 3-6
	default: break;
	}
	if(!iscursed)
		duration = duration * quality;
	switch(state) {
	case Strenghted:
	case Dexterious:
	case Constitution:
	case Intellegenced:
	case Wisdowed:
	case Charismatic:
		if(true) {
			auto ability = get_state_ability(state);
			if(iscursed) {
				abilities[ability] -= 1 + quality_raw;
				if(abilities[ability] < 1)
					abilities[ability] = 1;
			} else if(isartifact)
				abilities[ability] += quality;
			else
				set(state, duration + duration / 2);
		}
		break;
	case HealState:
		if(!iscursed) {
			auto dice = maptbl(healing, quality-1);
			if(isartifact) // Artifact permanently add health maximum
				mhp += xrand(2, 8);
			heal(dice.roll(), false);
		} else {
			auto dice = maptbl(healing, -(quality-1));
			damage(dice.roll(), Magic, false);
		}
		break;
	case ExperienceState:
		addexp(quality * 1000);
		break;
	default:
		if(iscursed && state_data[state].cursed_state)
			set(state_data[state].cursed_state, duration);
		else
			set(state, duration);
		break;
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
	apply(state, it.getmagic(), it.getqualityraw(), Hour, interactive);
	it.clear();
}