#include "main.h"

static constexpr struct spell_info {
	const char*			name;
	short unsigned		cost;
	effectc				effect;
	unsigned			duration;
	cflags<state_s>		state;
	char				damage[2];
	const char*			text_success;
} spell_data[] = {{"Благословение", 8, {{TargetNotHostileCreature, 2}}, Turn, {Blessed}},
{"Очаровать персону", 13, {{TargetCreature, 4}, {SaveAbility, Wisdow}}, Week, {Charmed}, {}, "Внезапно %герой стал%а вести себя очень дружелюбно."},
{"Определить зло", 12, {{NoTarget}}, Instant, {}},
{"Опознать предмет", 20, {{TargetItemUnidentified}}, Instant, {}},
{"Волшебный снаряд", 4, {{TargetHostileCreature}}, Instant, {}, {2, 8}},
};
assert_enum(spell, MagicMissile);
getstr_enum(spell);

int creature::getcost(spell_s value) const {
	return spell_data[value].cost;
}

static void detect_evil(item& it, creature& caster, bool interactive) {
	char temp[260];
	if(it.getidentify() < KnowMagic && it.iscursed()) {
		caster.act("%1 осветился красным светом.", it.getname(temp, zendof(temp)));
		it.set(KnowMagic);
	}
}

static void foreach(creature& e, void(*proc)(item& it, creature& caster, bool interactive)) {
	bool interactive = creature::getplayer()->canhear(e.position);
	for(auto& i : e.wears) {
		if(i)
			proc(i, e, interactive);
	}
	for(auto& i : e.backpack) {
		if(i)
			proc(i, e, interactive);
	}
}

bool creature::use(spell_s value) {
	auto cost = getcost(value);
	if(getmana() < cost)
		return false;
	char temp[260];
	targets ti;
	auto& e = spell_data[value];
	if(e.effect.type.target && !gettarget(ti, e.effect.type))
		return false;
	act("%герой прокричал%а мистическую формулу.");
	mp -= cost;
	if(e.effect.saving(ti, true))
		return true;
	if(e.state) {
		for(auto s : e.state)
			ti.cre->set(s, e.duration);
	}
	if(e.text_success) {
		if(ti.cre)
			ti.cre->act(e.text_success);
	}
	switch(value) {
	case CharmPerson:
		ti.cre->charmer = this;
		break;
	case DetectEvil:
		foreach(*this, detect_evil);
		break;
	case Identify:
		act("%1 озарилось синим светом.", ti.itm->getname(temp, zendof(temp)));
		ti.itm->set(KnowEffect);
		break;
	}
	return true;
}

static int compare(const void* p1, const void* p2) {
	auto e1 = *((spell_s*)p1);
	auto e2 = *((spell_s*)p2);
	return strcmp(getstr(e1), getstr(e2));
}

bool logs::choose(creature& e, spell_s& result) {
	auto count = 0;
	spell_s source[sizeof(spell_data) / sizeof(spell_data[0])];
	for(auto i = Bless; i <= Identify; i = (spell_s)(i + 1)) {
		if(e.get(i) <= 0)
			continue;
		source[count++] = i;
	}
	qsort(source, count, sizeof(source[0]), compare);
	return logs::choose(e, result, source, count);
}