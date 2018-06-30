#include "main.h"

static struct state_info {
	const char*		name;
	const char*		nameof;
	const char*		namehow;
	const char*		cursed;
} state_data[] = {{""},
{"����", "������", "����", 0},
{"����������", "�����", "����������", 0},
{"������������", "�������������", "����������", 0},
{"��������", "�������", "��������", 0},
{"��������", "����������", "������", 0},
{"������", "��������", "�����", "�������"},
{"��������", "��������", "��������", "�������"},
{"�������", "�����������", "����������", 0},
{"������", "��������", "������", 0},
{"�����", "���", "�����", 0},
{"��������", "�����", "�������", 0},
{"��������", "����������", "����", "������� ����"},
{"��������", "���", "����", "������� ����"},
{"��������", "�������� ���", "����", "������� ����"},
{"���", "������", "����������", 0},
{"�������", "�����", "����������", 0},
{"������", "���", "����", 0},
{"������", "����", "�������", "������"},
{"����", "��������", "������", "������"},
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
	act("%����� ������%� %1", getstr(itp));
	if(it.isartifact())
		hint(" � ������������%� ���� ������� %1.", state_data[itp].namehow);
	else if(it.iscursed() && state_data[itp].cursed)
		hint(" � ������������%� ���� %1.", state_data[itp].cursed);
	else if(state_data[itp].namehow)
		hint(" � ������������%� ���� %1.", state_data[itp].namehow);
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