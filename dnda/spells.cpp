#include "main.h"

bool setstate(sceneparam& e, creature& opponent, bool run) {
	if(run) {
		auto duration = e.state.duration * e.level;
		opponent.set(e.state.type, duration);
		switch(e.state.type) {
		case Scared:
			if(!opponent.saving(e.interactive, ResistCharm, 0)) {
				opponent.set(e.state.type, duration);
				opponent.sethorror(&e.player);
			}
			break;
		case Charmed:
			if(!opponent.saving(e.interactive, ResistCharm, 0)) {
				opponent.set(e.state.type, duration);
				opponent.setcharmer(&e.player);
			}
			break;
		case Paralized:
			if(!opponent.saving(e.interactive, ResistParalize, 0))
				opponent.set(e.state.type, duration);
			break;
		case Slowed:
			if(!opponent.saving(e.interactive, ResistParalize, 10))
				opponent.set(e.state.type, duration);
			break;
		default:
			opponent.set(e.state.type, duration);
			break;
		}
	}
	return true;
}

static bool damage(sceneparam& e, creature& opponent, bool run) {
	if(run) {
		auto damage = e.damage;
		if(e.level > 1) {
			auto level = e.level - 1;
			damage.min += level;
			if(damage.max > 10)
				damage.max += level * 2;
			else
				damage.max += level;
		}
		opponent.damage(damage.roll(), e.damage.type, true);
	}
	return true;
}

static bool heal(sceneparam& e, creature& v, bool run) {
	if(run)
		v.heal(e.damage.roll() + e.level - 1, true);
	return true;
}

static bool repair(sceneparam& e, item& v, bool run) {
	if(run)
		v.repair(e.level);
	return true;
}

static bool bless(sceneparam& e, item& v, bool run) {
	if(run) {
		// Bless item has chance to make item cursed and depend on level.
		auto chance = 60 + e.level * 5;
		if(d100() < chance)
			v.set(BlessedItem);
		else
			v.set(Cursed);
	}
	return true;
}

static bool identify(sceneparam& e, item& v, bool run) {
	if(run)
		v.setidentify(true);
	return true;
}

static struct spell_info {
	const char*			name;
	const char*			nameof;
	short unsigned		cost;
	effect_info			effect;
} spell_data[] = {{"", ""},
{"�����", "�����", 10, {setstate, You, {}, {Armored, 4 * Hour}, 0, {0, "%����� ������%��� ����� ������."}}},
{"�������������", "�������������", 8, {setstate, Close, {}, {Blessed, Turn}, 0, {0, "%����� ������%��� ������ ������."}}},
{"������������ �������", "�������������", 10, {bless, Identified}},
{"��������� �������", "�����", 13, {setstate, Hostile | Near, {}, {Charmed, Day}, 0, {0, "�������� %����� ����%� ����� ���� ����������."}}},
{"���������� ���", "����������� ���", 12, {identify, Nearest | Conceal | Hostile}},
{"���������� �����", "����������� �����", 20, {identify, Nearest | Conceal}},
{"�����", "������", 5, {setstate, Hostile | Near | Splash, {}, {Scared, 5 * Minute}, 0, {0, "%����� �����������%� � �����%� ������."}}},
{"�������", "�������", 7, {heal, Friendly | Damaged | Close, {1, 8, Magic}, {}, 0, {0, "%����� �������� ����� ������."}}},
{"�������� �������", "���������", 15, {identify, Conceal}},
{"�����������", "�����������", 8, {setstate, Friendly | Close, {}, {Hiding, Hour}, 0, {0, "%����� �����%�� �� ����."}}},
{"����", "�����", 1, {setstate, Friendly | Close, {}, {Lighted, Hour}, 0, {"������ %����� ��������� ��������� ���������� �������."}}},
{"��������� ������", "����������", 3, {damage, Hostile | Near, {2, 8, Magic}, {}, 0, {0, "�� ������� %����� �������� ��������� ���������� �������, ������� ����������� � ������."}}},
{"�������", "�������", 10, {repair, Damaged, {}, {}, 0, {0, "%����� �� ��������� �����%��� ����� ������ � ������ �������� �� ����� ���������."}}},
{"�������� ��", "������� ���", 15, {setstate, Friendly | Close, {}, {RemovePoison}, 0, {0, "%����� �� ��������� �������� ������ ���������."}}},
{"�������� �������", "������� ��������", 15, {setstate, Friendly, {}, {RemoveSick}, 0, {0, "%����� �� ��������� �������� ������� ���������."}}},
{"���", "����", 6, {setstate, You, {}, {Shielded, Hour / 2}, 0, {0, "����� %������ �������� �������������� ������."}}},
{"���������� ������", "�������������", 4, {damage, Hostile | Close, {3, 12, Electricity}, {}, 0, {0, "������������� ������ ������� %�����."}}},
{"���������", "���������", 5, {setstate, Hostile | Reach, {}, {Sleeped, 2 * Minute}, 0, {0, "�������� %����� ������%�."}}},
{"��������� ��������", "����������", 7, {setstate, Hostile|Near, {}, {Slowed, 5 * Minute}, 0, {0, "�������� %����� ����%� ��������� ���������."}}},
};
assert_enum(spell, LastSpell);
getstr_enum(spell);

const char* item::getname(spell_s value) {
	return spell_data[value].nameof;
}

int creature::getcost(spell_s value) const {
	return spell_data[value].cost;
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
	return true;
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

//spell_s creature::aispell(aref<creature*> creatures, target_s target) {
//	return NoSpell;
//}
//
//bool creature::aiusewand(aref<creature*> creatures, target_s target) {
//	return false;
//}