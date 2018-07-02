#include "main.h"

static struct state_info {
	const char*		name;
	const char*		nameof;
	const char*		namehow;
	const char*		cursed;
	state_s			cursed_state;
} state_data[] = {{""},
{"����", "������", "����", 0, PoisonedWeak},
{"����������", "�����", "����������", 0, PoisonedWeak},
{"������������", "�������������", "����������", 0, PoisonedWeak},
{"��������", "�������", "��������", "���������", PoisonedWeak},
{"��������", "����������", "������", 0, PoisonedWeak},
{"������", "��������", "�����", "�������", Sick},
{"��������", "��������", "��������", "�������", Sick},
{"�������", "�����������", "����������", 0, PoisonedWeak},
{"������", "��������", "������", 0, Anger},
{"�����", "���", "�����", "������", PoisonedWeak},
{"��������", "�����", "�������", 0, PoisonedWeak},
{"��������", "����������", "����", "������� ����", PoisonedStrong},
{"��������", "���", "����", "������� ����", PoisonedStrong},
{"��������", "�������� ���", "����", "������� ����", PoisonedStrong},
{"���", "������", "����������", 0, PoisonedWeak},
{"�����", "�������", "������", 0, RemoveSick},
{"�������", "�����", "����������", 0, PoisonedWeak},
{"������", "���", "����", 0, PoisonedWeak},
{"������", "����", "�������", "������", PoisonedWeak},
{"��������", "����������", "������", 0, Sick},
{"����", "��������", "������", "��������������", Sick},
//
{"�������", "�������", "��������", "�������", Sick},
{"�������� �������", "��������� �������", "��������", "�������", Sick},
{"�������� ��", "�����������", "��������", "�������", Poisoned},
};
assert_enum(state, LastEffectState);
getstr_enum(state);

const char* item::getname(state_s value) {
	return state_data[value].nameof;
}

const char* creature::getname(state_s id, bool cursed) {
	if(cursed && state_data[id].cursed)
		return state_data[id].cursed;
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
	auto quality = it.getquality();
	auto quality_raw = it.getqualityraw();
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
				abilities[ability] += 1 + quality_raw;
			else
				set(state, duration + duration / 2);
		}
		break;
	case HealState:
		if(!it.iscursed()) {
			auto dice = maptbl(healing, quality_raw);
			damage(-dice.roll(), false, false);
			if(it.isartifact()) // Artifact permanently add health
				mhp += xrand(2, 8);
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