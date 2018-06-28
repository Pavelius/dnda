#include "main.h"

enum save_s : char {
	NoSave, SaveAbility,
};

struct save_i {
	save_s				type;
	ability_s			ability;
	skill_s				skill;
};

static constexpr struct spell_info {
	const char*			name;
	short unsigned		cost;
	targetdesc			target;
	unsigned			duration;
	cflags<state_s>		state;
	save_i				save;
	char				damage[2];
	const char*			text_success;
} spell_data[] = {{"Благословение", 3, {TargetNotHostileCreature, 2}, Turn, {BlessedState}},
{"Очаровать персону", 10, {TargetCreature, 4}, Week, {Charmed}, {SaveAbility, Wisdow}, {}, "Внезапно %герой стал%а вести себя очень дружелюбно."},
{"Определить зло", 3, {NoTarget}, Instant, {}},
{"Опознать предмет", 8, {TargetItemUnidentified}, Instant, {}},
{"Волшебный снаряд", 2, {TargetHostileCreature}, Instant, {}, {}, {2, 8}},
};
assert_enum(spell, MagicMissile);
getstr_enum(spell);

int creature::getcost(spell_s value) const {
	return spell_data[value].cost;
}

void item::detectevil(bool interactive) {
	char temp[260];
	if(getidentify() < KnowMagic && iscursed()) {
		if(interactive)
			logs::add("%1 осветился красным светом.", getname(temp, zendof(temp)));
		set(KnowMagic);
	}
}

static void foreach(creature& e, void(item::*proc)(bool interactive)) {
	bool interactive = creature::getplayer()->canhear(e.position);
	for(auto& i : e.wears) {
		if(i)
			(i.*proc)(interactive);
	}
	for(auto& i : e.backpack) {
		if(i)
			(i.*proc)(interactive);
	}
}

bool creature::use(spell_s value) {
	char temp[260];
	targetinfo ti;
	auto& e = spell_data[value];
	if(e.target.target && !gettarget(ti, e.target))
		return false;
	auto cost = getcost(value);
	if(getstamina() < cost)
		return false;
	act("%герой прокричал%а мистическую формулу.");
	mp -= cost;
	if(ti.creature && e.save.type != NoSave) {
		auto chance_save = 0;
		switch(e.save.type) {
		case SaveAbility:
			chance_save = ti.creature->get(e.save.skill) * 4;
			break;
		}
		if(chance_save) {
			auto r = d100();
			if(r < chance_save) {
				if(getplayer()->canhear(*ti.creature))
				ti.creature->act("%герой перенес%ла эффект без последствий.");
				return true;
			}
		}
	}
	if(e.state) {
		for(auto s : e.state)
			ti.creature->set(s, e.duration);
	}
	if(e.text_success) {
		if(ti.creature)
			ti.creature->act(e.text_success);
	}
	switch(value) {
	case CharmPerson:
		ti.creature->charmer = this;
		break;
	case DetectEvil:
		foreach(*this, &item::detectevil);
		break;
	case Identify:
		if(getplayer()->canhear(position))
			logs::add("%1 озарилось синим светом.", ti.item->getname(temp, zendof(temp)));
		ti.item->set(KnowEffect);
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