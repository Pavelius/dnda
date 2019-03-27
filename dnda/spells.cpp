#include "main.h"

void message(creature& player, const anyptr& ti, const char* format);

bool setstate(scene& sc, sceneparam& e, creature& opponent, bool run) {
	if(opponent.is(e.state.type))
		return false;
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

static bool damage(scene& sc, sceneparam& e, creature& opponent, bool run) {
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

static bool heal(scene& sc, sceneparam& e, creature& v, bool run) {
	if(run)
		v.heal(e.damage.roll() + e.level - 1, true);
	return true;
}

static bool repair(scene& sc, sceneparam& e, item& v, bool run) {
	if(run)
		v.repair(e.level);
	return true;
}

static bool bless(scene& sc, sceneparam& e, item& v, bool run) {
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

static bool identify(scene& sc, sceneparam& e, item& v, bool run) {
	if(run)
		v.setidentify(true);
	return true;
}

static struct spell_info {
	const char*			name;
	const char*			nameof;
	short unsigned		cost;
	effectinfo			effect;
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

bool creature::use(scene& sc, spell_s value) {
	auto cost = getcost(value);
	if(getmana() < cost) {
		hint("�� ������� ����.");
		return false;
	}
	if(!use(sc, value, 1, "%����� ���������%� ����������� �������."))
		return false;
	mp -= cost;
	wait(Minute);
	return true;
}

bool creature::use(scene& sc, spell_s value, int level, const char* format, ...) {
	if(level < 1)
		level = 1;
	auto& e = spell_data[value];
	anyptr ti;
	if(!choose(e.effect, sc, ti, isinteractive())) {
		hint("��� ���������� ����");
		return false;
	}
	sceneparam sp(e.effect, *this, true);
	sp.apply(sc, ti, format, xva_start(format));
	if(ti && e.effect.messages.success) {
		creature* cre = ti;
		if(cre)
			cre->act(e.effect.messages.success);
	}
	return true;
}