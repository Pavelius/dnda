#include "main.h"

void setstate(effectparam& e) {
	// Set state and increase duration by level
	e.cre->set(e.state, e.duration * e.level);
	if(e.state == Scared)
		e.cre->horror = &e.player;
	else if(e.state == Charmed)
		e.cre->charmer = &e.player;
}

static void setdamage(effectparam& e) {
	// Apply damage to level
	auto damage = e.damage;
	if(e.level > 1) {
		auto level = e.level - 1;
		damage.min += level;
		if(damage.max > 10)
			damage.max += level * 2;
		else
			damage.max += level;
	}
	e.cre->damage(damage.roll(), e.damage.type, true);
}

void healdamage(effectparam& e) {
	// Heal damage according level
	e.cre->heal(e.damage.roll() + e.level - 1, true);
}

static void detect_evil(effectparam& e) {
	// Detect only first level items
	char temp[260];
	if(e.param >= e.level)
		return;
	if(e.itm->getidentify() < KnowMagic && e.itm->iscursed()) {
		e.player.act(e.text, e.itm->getname(temp, zendof(temp)));
		e.itm->set(KnowMagic);
		e.param++;
	}
}

static void identify(effectparam& e) {
	char temp[260];
	e.player.act(e.text, e.itm->getname(temp, zendof(temp)));
	e.itm->set(KnowEffect);
}

static struct spell_info {
	const char*			name;
	const char*			nameof;
	short unsigned		cost;
	short unsigned		cost_reduce_by_level;
	effectinfo			effect;
} spell_data[] = {{"", ""},
{"Броня", "брони", 10, 0, {{TargetSelf}, {}, {setstate}, 4 * Hour, Armored, "%герой озарил%ась синим светом."}},
{"Благословение", "благословения", 8, 0, {{TargetFriendly, 2}, {}, {setstate}, Turn, Blessed, "%герой озарил%ась желтым светом."}},
{"Очаровать персону", "шарма", 13, 0, {{TargetFriendlySelf, 4}, ResistCharm, {setstate}, Day, Charmed, "Внезапно %герой стал%а вести себя дружелюбно."}},
{"Определить зло", "определения зла", 12, 0, {{TargetInvertory}, {}, {detect_evil}, Instant, {}, "%1 осветился красным светом."}},
{"Страх", "страха", 5, 0, {{TargetHostile, 5, 2}, ResistCharm, {setstate}, 5 * Minute, Scared, "%герой запаниковал%а и начал%а бежать.", {}}},
{"Лечение", "лечения", 7, 0, {{TargetFriendly, 1}, {}, {healdamage}, Instant, {}, "%герой озарился белым светом.", {1, 8, Magic}}},
{"Опознать предмет", "опознания", 20, 2, {{TargetItemUnidentified}, {}, {identify}, Instant, {}, "%1 осветился голубым светом."}},
{"Невидимость", "невидимости", 8, 0, {{TargetFriendly, 1}, {}, {setstate}, Hour, Hiding, "%герой исчез%ла из виду."}},
{"Свет", "света", 1, 0, {{TargetFriendly, 1}, {}, {setstate}, Hour, {Lighted}, "Вокруг %героя появилось несколько светящихся шариков."}},
{"Волшебный снаряд", "колдовства", 3, 0, {{TargetHostile, 6}, {}, {setdamage}, Instant, {}, "Из пальцев %ГЕРОЯ вылетело несколько светящихся шариков.", {2, 8, Magic}}},
{"Исцелить яд", "лечения яда", 15, 1, {{TargetFriendly}, {}, {setstate}, Instant, RemoveSick, "%герой на мгновение окутался желтым свечением."}},
{"Исцелить болезнь", "лечения болезней", 15, 1, {{TargetFriendly}, {}, {setstate}, Instant, RemovePoison, "%герой на мгновение окутался зеленым свечением."}},
{"Щит", "щита", 6, 0, {{TargetSelf}, {}, {setstate}, Hour / 2, Shielded, "Перед %героем появился полупрозрачный барьер."}},
{"Шокирующая хватка", "электричества", 4, 0, {{TargetHostile, 1}, {}, {setdamage}, Instant, {}, "Электрический разряд поразил %героя.", {3, 12, Electricity}}},
{"Усыпление", "усыпления", 5, 0, {{TargetHostile, 4}, ResistCharm, {setstate}, Minute, Sleeped, "Внезапно %герой заснул%а.", {}}},
};
assert_enum(spell, LastSpell);
getstr_enum(spell);

const char* item::getname(spell_s value) {
	return spell_data[value].nameof;
}

int creature::getcost(spell_s value) const {
	auto level = get(value);
	if(level <= 1)
		return spell_data[value].cost;
	auto cost = spell_data[value].cost - spell_data[value].cost_reduce_by_level*(level - 1);
	if(cost < 1)
		cost = 1;
	return cost;
}

bool creature::use(spell_s value) {
	auto cost = getcost(value);
	if(getmana() < cost) {
		hint("Не хватает маны.");
		return false;
	}
	auto result = use(value, 1, "%герой прокричал%а мистическую формулу.");
	if(result)
		mp -= cost;
	return result;
}

bool creature::use(spell_s value, int level, const char* format, ...) {
	creature* source_data[256];
	auto creatures = getcreatures(source_data, position, getlos());
	effectparam ep(spell_data[value].effect, *this, creatures, isplayer());
	if(level < 1)
		level = 1;
	ep.level = level;
	return ep.applyv(format, xva_start(format));
}

static int compare(const void* p1, const void* p2) {
	auto e1 = *((spell_s*)p1);
	auto e2 = *((spell_s*)p2);
	return strcmp(getstr(e1), getstr(e2));
}

bool logs::choose(creature& e, spell_s& result) {
	unsigned count = 0;
	spell_s source[sizeof(spell_data) / sizeof(spell_data[0])];
	for(auto i = FirstSpell; i <= LastSpell; i = (spell_s)(i + 1)) {
		if(e.get(i) <= 0)
			continue;
		source[count++] = i;
	}
	qsort(source, count, sizeof(source[0]), compare);
	return logs::choose(e, result, {source, count});
}