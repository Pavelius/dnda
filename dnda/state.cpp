#include "main.h"

static struct state_info {
	const char*		name;
	const char*		nameof;
	const char*		namehow;
	state_s			cursed_state;
} state_data[] = {{""},
{"����", "������", "����", PoisonedWeak},
{"����������", "�����", "����������", PoisonedWeak},
{"������������", "�������������", "����������", PoisonedWeak},
{"��������", "����������", "������", PoisonedWeak},
{"�������", "�����������", "����������", PoisonedWeak},
{"������", "��������", "������", Anger},
{"��������", "�����", "�������", PoisonedWeak},
{"��������", "����������", "����", PoisonedStrong},
{"��������", "���", "����", PoisonedStrong},
{"��������", "�������� ���", "����", PoisonedStrong},
{"���", "������", "����������", PoisonedWeak},
{"�����", "�������", "������", PoisonedStrong},
{"�������", "�����", "����������", PoisonedWeak},
{"������", "���", "����", PoisonedWeak},
{"��������", "����������", "������", Sick},
//
{"�������", "����", "�������", NoState},
{"������", "��������", "�����", NoState},
{"��������", "��������", "��������", NoState},
{"�����", "���", "�����", NoState},
{"������", "��������", "������", NoState},
{"��������", "�������", "��������", NoState},
//
{"�������", "�������", "��������", Sick},
{"�������� �������", "��������� �������", "��������", Sick},
{"�������� ��", "�����������", "��������", Poisoned},
};
assert_enum(state, LastEffectState);
getstr_enum(state);

static const char* ability_worse[] = {"������", "�������", "�������",
"������", "��������������", "���������"
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
			return "����";
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

void creature::drink(item& it, bool interactive) {
	static damageinfo healing[] = {{5, 10}, {10, 20}, {15, 30}, {20, 40}};
	char temp[260]; it.getname(temp, zendof(temp), false); szlower(temp);
	auto itp = it.gettype();
	auto state = it.getstate();
	act("%����� �����%� %1", temp);
	if(it.isartifact())
		hint(" � ������������%� ���� ������� %1", getname(state, false));
	else if(it.iscursed())
		hint(" � ������������%� ���� %1", getname(state, true));
	else
		hint(" � ������������%� ���� %1", getname(state, false));
	act(".");
	int duration = Hour;
	auto quality = it.getquality();
	auto quality_raw = it.getqualityraw();
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
				abilities[ability] -= 1 + quality_raw;
				if(abilities[ability] < 1)
					abilities[ability] = 1;
			} else if(it.isartifact())
				abilities[ability] += (1 + quality_raw);
			else
				set(state, duration + duration / 2);
		}
		break;
	case HealState:
		if(!it.iscursed()) {
			auto dice = maptbl(healing, quality_raw);
			if(it.isartifact()) // Artifact permanently add health maximum
				mhp += xrand(2, 8);
			damage(-dice.roll(), false, false);
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