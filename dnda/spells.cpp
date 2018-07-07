#include "main.h"

void setstate(effectparam& e) {
	// Set state and increase duration by level
	e.cre->set(e.state.type, e.state.duration * e.level);
	if(e.state.type == Scared)
		e.cre->horror = &e.player;
	else if(e.state.type == Charmed)
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
	if(e.itm->getidentify() < KnowEffect && e.itm->iscursed()) {
		e.player.act(e.text, e.itm->getname(temp, zendof(temp)));
		e.itm->set(KnowEffect);
		e.param++;
	}
}

static void detect_magic(effectparam& e) {
	// Detect only first level items
	char temp[260];
	if(e.param >= e.level)
		return;
	if(e.itm->getidentify()<KnowEffect && e.itm->ismagical()) {
		e.player.act(e.text, e.itm->getname(temp, zendof(temp)));
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
{"�����", "�����", 10, 0, {{TargetSelf}, {}, {setstate}, {Armored, 4 * Hour}, "%����� ������%��� ����� ������."}},
{"�������������", "�������������", 8, 0, {{TargetFriendly, 2}, {}, {setstate}, {Blessed, Turn}, "%����� ������%��� ������ ������."}},
{"��������� �������", "�����", 13, 0, {{TargetFriendlySelf, 4}, ResistCharm, {setstate}, {Charmed, Day}, "�������� %����� ����%� ����� ���� ����������."}},
{"���������� ���", "����������� ���", 12, 0, {{TargetInvertory}, {}, {detect_evil}, {}, "%1 ��������� ������� ������."}},
{"���������� �����", "����������� �����", 8, 0, {{TargetInvertory}, {}, {detect_magic}, {}, "%1 ��������� ������� ���������."}},
{"�����", "������", 5, 0, {{TargetHostile, 5, 2}, ResistCharm, {setstate}, {Scared, 5 * Minute}, "%����� �����������%� � �����%� ������.", {}}},
{"�������", "�������", 7, 0, {{TargetFriendly, 1}, {}, {healdamage}, {}, "%����� �������� ����� ������.", {1, 8, Magic}}},
{"�������� �������", "���������", 20, 2, {{TargetItemUnidentified}, {}, {identify}, {}, "%1 ��������� ������� ������."}},
{"�����������", "�����������", 8, 0, {{TargetFriendly, 1}, {}, {setstate}, {Hiding, Hour}, "%����� �����%�� �� ����."}},
{"����", "�����", 1, 0, {{TargetFriendly, 1}, {}, {setstate}, {Lighted, Hour}, "������ %����� ��������� ��������� ���������� �������."}},
{"��������� ������", "����������", 3, 0, {{TargetHostile, 6}, {}, {setdamage}, {}, "�� ������� %����� �������� ��������� ���������� �������.", {2, 8, Magic}}},
{"�������� ��", "������� ���", 15, 1, {{TargetFriendly}, {}, {setstate}, {RemovePoison}, "%����� �� ��������� �������� ������ ���������."}},
{"�������� �������", "������� ��������", 15, 1, {{TargetFriendly}, {}, {setstate}, {RemoveSick}, "%����� �� ��������� �������� ������� ���������."}},
{"���", "����", 6, 0, {{TargetSelf}, {}, {setstate}, {Shielded, Hour / 2}, "����� %������ �������� �������������� ������."}},
{"���������� ������", "�������������", 4, 0, {{TargetHostile, 1}, {}, {setdamage}, {}, "������������� ������ ������� %�����.", {3, 12, Electricity}}},
{"���������", "���������", 5, 0, {{TargetHostile, 4}, ResistCharm, {setstate}, {Sleeped, Minute}, "�������� %����� ������%�.", {}}},
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
		hint("�� ������� ����.");
		return false;
	}
	auto result = use(value, 1, "%����� ���������%� ����������� �������.");
	if(result)
		mp -= cost;
	wait(Minute);
	return result;
}

bool creature::use(spell_s value, int level, const char* format, ...) {
	if(level < 1)
		level = 1;
	return apply(spell_data[value].effect, level, isplayer(), format, xva_start(format), 0, 0);
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

spell_s creature::aispell(aref<creature*> creatures) {
	adat<spell_s, LastSpell + 1> recomended;
	auto mana = getmana();
	for(auto i = (spell_s)1; i <= LastSpell; i = (spell_s)(i + 1)) {
		if(!spells[i])
			continue;
		if(spell_data[i].effect.type.target == NoTarget)
			continue;
		if(getcost(i)>mana)
			continue;
		if(!spell_data[i].effect.type.isallow(*this, creatures))
			continue;
		recomended.add(i);
	}
	if(recomended.count > 0)
		return recomended.data[rand() % recomended.count];
	return NoSpell;
}