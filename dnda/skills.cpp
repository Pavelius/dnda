#include "main.h"

static const char* talk_subjects[] = {"������", "��������", "������", "�������", "�������", "�������", "�������"};
static const char* talk_object[] = {"���������", "��������� ������", "��������� ��������", "����", "�������� ������"};
static const char* talk_location[] = {"����������", "������", "�������", "�������", "����"};
static const char* talk_games[] = {"������", "�����", "���������"};

bool	setstate(sceneparam& e, creature& v, bool run);
int		compare_skills(const void* p1, const void* p2);

static bool heal(sceneparam& e, creature& v, bool run) {
	if(run) {
		auto damage = e.damage.max + e.roll.value / 10;
		v.heal(e.damage.roll(), true);
	}
	return true;
}

static bool removetrap(sceneparam& e, short unsigned index, bool run) {
	auto t = game::getobject(index);
	if(t != Trap)
		return false;
	if(run)
		game::set(index, NoTileObject);
	return true;
}

static bool bash(sceneparam& e, short unsigned index, bool run) {
	auto t = game::getobject(index);
	if(t != Door)
		return false;
	if(run)
		game::set(index, NoTileObject);
	return true;
}

static bool openlock(sceneparam& e, short unsigned index, bool run) {
	if(run)
		game::set(index, Sealed, false);
	return true;
}

//static bool test_pickpockets(effectparam& e) {
//	static const char* talk[] = {
//		"������ ������� �������. ��� ... ��� �� �� ����������? ... �����. �����, ����� � ������ ��� ��������.",
//		"� - ������ ��� ���?",
//		"�� �� ������ ��� ������ � %3? �, �������, � ��� �������� ������.",
//		"������ ��� ������� ��� %1 � %2? ���? � ����..."
//	};
//	e.player.say(maprnd(talk), maprnd(talk_subjects), maprnd(talk_object), maprnd(talk_location));
//	return true;
//}

static bool pickpockets(sceneparam& e, creature& v, bool run) {
	if(run) {
		auto count = xrand(3, 18);
		if(count > v.getmoney())
			count = v.getmoney();
		v.setmoney(v.getmoney() - count);
		e.player.setmoney(e.player.getmoney() + count);
		if(e.player.isplayer())
			e.player.act("%����� �����%� [%1i] �����.", count);
	}
	return true;
}

//static bool inbuilding(effectparam& e) {
//	if(!e.player.getsite()) {
//		e.player.hint("���� ����� ����� ��������� ������ � ������.");
//		return false;
//	}
//	return true;
//}

static bool literacy(sceneparam& e, item& v, bool run) {
	if(run)
		e.player.use(v);
	return true;
}

static bool dance(sceneparam& e, creature& v, bool run) {
	return true;
}

//static bool test_gamble(effectparam& e) {
//	if(!inbuilding(e))
//		return false;
//	e.param = 20 * (1 + e.player.get(Gambling) / 20);
//	if(e.player.getmoney() < e.param) {
//		e.player.hint("� ���� ��� ������������ ���������� �����.");
//		return false;
//	}
//	e.player.say("����� ������� � %1?", maprnd(talk_games));
//	if(e.cre->getmoney() < e.param) {
//		e.cre->say("���. � �� ����. � ������ ���.");
//		return false;
//	}
//	e.skill_bonus = -e.cre->get(Gambling) / 2;
//	return true;
//}

static bool gamble(sceneparam& e, creature& v, bool run) {
	if(run) {
		e.player.setmoney(e.player.getmoney() + e.param);
		v.setmoney(v.getmoney() - e.param);
		v.act("%����� ��������%� [%1i] �����.", e.param);
	}
	return true;
}

//static void failskill(effectparam& e) {
//	if(d100() < 40) {
//		e.player.hint("�� ����� ����� �������� ��������� ��� ��� ����, �� � ����� ������� �� �������.");
//		e.player.wait(Minute * xrand(2, 5));
//	} else
//		e.player.hint("������� �� �������.");
//}
//
//static void failgamble(effectparam& e) {
//	e.player.setmoney(e.player.getmoney() - e.param);
//	e.cre->setmoney(e.cre->getmoney() + e.param);
//	e.player.act("%����� ��������%� [%1i] �����.", e.param);
//}

static bool killing(sceneparam& e, creature& v, bool run) {
	if(run) {
		auto total = e.roll.value;
		e.player.meleeattack(&v, total / 2, total / 30);
	}
	return true;
}

//static void killing_fail(effectparam& e) {
//	e.player.meleeattack(e.cre);
//}

static struct skill_info {
	const char*		name;
	const char*		nameof;
	ability_s		ability[2];
	sceneeffect		effect;
	enchantment_s	enchant;
	bool			deny_ability;
} skill_data[] = {{"��� ������"},
{"��������", "��������", {Charisma, Intellegence}},
{"����", "������", {Charisma, Dexterity}},
{"����������", "����������", {Charisma, Wisdow}, {setstate, Friendly | Close, {}, {Goodwill, Turn / 2}, 0, {0, "%����� �������%� �����������."}}},
//
{"����������", "����������", {Dexterity, Dexterity}},
{"��������������", "��������������", {Wisdow, Dexterity}},
{"��������", "��������", {Strenght, Dexterity}, {bash, Close, {}, {}, 20, {"%����� ������%�� ����� � �����."}}},
{"��������", "��������", {Dexterity, Dexterity}, {killing, Close, {}, {}, 50, {"%����� �����%�� ������ ����."}}},
{"������������", "�����������", {Wisdow, Wisdow}},
{"����������� �������", "�������", {Dexterity, Intellegence}, {removetrap, Close | Identified, {}, {}, 30, {"%����� ����������%� �������."}}},
{"������� �����", "�����", {Wisdow, Intellegence}},
{"��������� � ����", "����������", {Dexterity, Dexterity}, {setstate, You, {}, {Hiding, Turn / 2}, 0, {0, "%����� �������� �����%�� �� ���� ������."}}},
{"������� �����", "������", {Dexterity, Intellegence}, {openlock, Close, {}, {}, 50, {0, "%����� ������%� �����."}}},
{"�������� �������", "���������", {Dexterity, Charisma}, {pickpockets, Close, {}, {}, 25}},
{"�������", "�������", {Intellegence, Intellegence}},
{"�����", "������", {Dexterity, Charisma}, {dance, All | Near, {}, {}, 10, {0, "%����� ��������%� �������� �����."}}},
{"���������� ����", "���������", {Intellegence, Intellegence}},
{"�������� ����", "�������� ���", {Charisma, Dexterity}, {gamble, Close, {}, {}, 25}},
{"�������", "�������", {Intellegence, Intellegence}},
{"�������", "�������", {Wisdow, Intellegence}, {heal, Close | Friendly | Damaged, {1, 3}, {}, 5, {"%����� ���������%� ����."}}},
{"�����������", "������ � ������", {Intellegence, Intellegence}, {literacy, 0, {}, {}, 25}},
{"���������� ����", "����������� ����", {Strenght, Intellegence}},
{"��������� ����", "���������� ����", {Strenght, Intellegence}},
{"���������", "���������", {Wisdow, Constitution}},
{"��������", "��������", {Strenght, Constitution}},
//
{"�������� �����", "�������� �� ����", {Dexterity, Dexterity}},
{"�������� �����", "�������� �� �����", {Strenght, Dexterity}},
{"�������� �������", "�������� �� �������", {Strenght, Constitution}},
{"�������� ����� ��������", "�������� ������", {Strenght, Dexterity}},
//
{"������������� �������", "�������", {Dexterity, Constitution}, {}, OfAcidResistance, true},
{"������������� �����", "������� � �����", {Wisdow, Wisdow}, {}, OfCharmResistance},
{"������������� ������", "������", {Constitution, Strenght}, {}, OfColdResistance, true},
{"������������� �������������", "������", {Dexterity, Dexterity}, {}, OfElectricityResistance, true},
{"������������� ����", "����", {Constitution, Dexterity}, {}, OfFireResistance, true},
{"������������� ��������", "��������", {Constitution, Dexterity}, {}, OfFireResistance, true},
{"������������� ���", "���", {Constitution, Constitution}, {}, OfPoisonResistance},
{"������� �����", "����", {Strenght, Constitution}, {}, OfWaterproof, true},
};
assert_enum(skill, ResistWater);
getstr_enum(skill);

const char* creature::getname(skill_s id) {
	return skill_data[id].nameof;
}

damageinfo creature::getraise(skill_s id) const {
	auto value = skills[id];
	if(value < 20)
		return {3, 12};
	else if(value < 40)
		return {3, 9};
	else
		return {2, 6};
}

void creature::raise(skill_s value) {
	auto dice = getraise(value);
	skills[value] += dice.roll();
}

int	creature::getbasic(skill_s value) const {
	return skills[value];
}

int creature::get(skill_s value) const {
	auto& e = skill_data[value];
	auto result = getbasic(value);
	if(!e.deny_ability)
		result += get(e.ability[0]) + get(e.ability[1]);
	if(e.enchant)
		result += getbonus(e.enchant) * 10;
	return result;
}

void creature::use(skill_s value) {
	if(is(Anger)) {
		hint("��� ���� ������� ������ � ���� � ����������.");
		return;
	}
	auto& e = skill_data[value];
	if(!e.effect) {
		hint("����� %1 �� ������������ �������� �������", getstr(value));
		return;
	}
	//apply(e.effect, 0, isplayer(), 0, 0, d100(), get(value), failskill);
	// Use skill is one minute
	wait(Minute);
}

skill_s creature::aiskill(aref<creature*> creatures) {
	if(is(Anger))
		return NoSkill;
	//adat<skill_s, LastSkill + 1> recomended;
	//for(auto i = (skill_s)1; i <= LastSkill; i = (skill_s)(i + 1)) {
	//	if(!skills[i])
	//		continue;
	//	if(!skill_data[i].effect.type.iscreature())
	//		continue;
	//	if(!skill_data[i].effect.type.isallow(*this, creatures))
	//		continue;
	//	recomended.add(i);
	//}
	//if(recomended.count > 0)
	//	return recomended.data[rand() % recomended.count];
	return NoSkill;
}

skill_s creature::aiskill() {
	if(is(Anger))
		return NoSkill;
	//aref<creature*> source = {};
	//adat<skill_s, LastSkill + 1> recomended;
	//for(auto i = (skill_s)1; i <= LastSkill; i = (skill_s)(i + 1)) {
	//	if(!skills[i])
	//		continue;
	//	if(!skill_data[i].effect.type.isposition())
	//		continue;
	//	if(!skill_data[i].effect.type.isallow(*this, source))
	//		continue;
	//	recomended.add(i);
	//}
	//if(recomended.count > 0)
	//	return recomended.data[rand() % recomended.count];
	return NoSkill;
}

void manual_ability_skills(stringbuffer& sc, manual& e) {
	adat<skill_s, LastResist + 1> source;
	for(auto i = (skill_s)1; i < LastResist; i = (skill_s)(i + 1)) {
		if(skill_data[i].ability[0] == e.value.ability || skill_data[i].ability[1] == e.value.ability)
			source.add(i);
	}
	qsort(source.data, source.count, sizeof(source.data[0]), compare_skills);
	if(!source.count)
		return;
	sc.add("[������]: ");
	auto p = zend(sc.result);
	for(auto i : source) {
		if(p[0])
			sc.add(", ");
		sc.add(getstr(i));
	}
	sc.add(".");
}

void manual_skill_ability(stringbuffer& sc, manual& e) {
	auto& ep = skill_data[e.value.skill];
	sc.add("[������� ��������]: ");
	if(ep.ability[0] == ep.ability[1])
		sc.add("%1 x 2", getstr(ep.ability[0]));
	else
		sc.add("%1 + %2", getstr(ep.ability[0]), getstr(ep.ability[1]));
}