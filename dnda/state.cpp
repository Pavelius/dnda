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
	char temp[260]; it.getname(temp, zendof(temp), false); szlower(temp);
	auto itp = it.gettype();
	auto state = it.getstate();
	act("%����� �����%� %1", temp);
	if(it.isartifact())
		hint(" � ������������%� ���� ������� %1", state_data[state].namehow);
	else if(it.iscursed() && state_data[state].cursed)
		hint(" � ������������%� ���� %1", state_data[state].cursed);
	else if(state_data[state].namehow)
		hint(" � ������������%� ���� %1", state_data[state].namehow);
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