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
{"����", "���������", "������", PoisonedWeak},
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
{"�������", "����", "�������", Weaken},
{"������", "��������", "�����", NoState},
{"��������", "��������", "��������", NoState},
{"�����", "���", "�����", NoState},
{"������", "��������", "������", NoState},
{"��������", "�������", "��������", NoState},
//
{"����", "�����", "�������", Sick},
{"����", "����", "���������� �����", Sick},
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

void creature::apply(state_s state, item_type_s magic, int quality, unsigned duration, bool interactive) {
	// Quality would be from 1 to 6 or from -1 to -4
	static damageinfo restore_hits[] = {{1, 5}, {5, 10}, {10, 20}, {15, 30}, {20, 40}, {30, 50}, {40, 60}};
	static damageinfo restore_mana[] = {{1, 3}, {4, 8}, {8, 16}, {12, 24}, {16, 32}, {20, 40}, {24, 48}};
	bool isartifact = (magic == Artifact);
	if(quality>0)
		duration *= quality;
	switch(state) {
	case Strenghted:
	case Dexterious:
	case Constitution:
	case Intellegenced:
	case Wisdowed:
	case Charismatic:
		if(true) {
			auto ability = get_state_ability(state);
			if(quality<0) {
				abilities[ability] += quality;
				if(abilities[ability] < 1)
					abilities[ability] = 1;
			} else if(isartifact)
				abilities[ability] += (quality-2); // This would be from 3-6, so effect would be from 1 to 3
			else
				set(state, duration + duration / 2);
		}
		break;
	case RestoreHits:
		if(quality<0) {
			auto dice = maptbl(restore_hits, -quality);
			damage(dice.roll(), Magic, false);
		} else {
			auto dice = maptbl(restore_hits, quality);
			if(isartifact) // Artifact permanently add health maximum
				mhp += xrand(2, 8);
			heal(dice.roll(), false);
		}
		break;
	case RestoreMana:
		if(quality<0) {
			auto dice = maptbl(restore_mana, -quality);
			consume(-dice.roll(), false);
		} else {
			auto dice = maptbl(restore_mana, quality);
			if(isartifact) // Artifact permanently add mana maximum
				mmp += xrand(2, 8);
			consume(dice.roll(), false);
		}
		break;
	case Experience:
		addexp(1000 * quality);
		break;
	default:
		if(quality<0 && state_data[state].cursed_state)
			set(state_data[state].cursed_state, duration);
		else
			set(state, duration);
		break;
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
	apply(state, it.getmagic(), it.getquality(), Hour, interactive);
	it.clear();
}