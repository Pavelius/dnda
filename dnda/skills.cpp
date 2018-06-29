#include "main.h"

static struct skillinfo {
	const char*		name;
	ability_s		ability[2];
	targetdesc		type;
	unsigned char	koef[2];
	unsigned		experience;
	const char*		text_success;
} skill_data[] = {{"��� ������"},
{"��������", {Charisma, Intellegence}},
{"����", {Charisma, Dexterity}},
{"����������", {Charisma, Wisdow}, {TargetCreature, 1}, {}, 0, "%����� �������%� �����������."},
//
{"����������", {Dexterity, Dexterity}},
{"��������������", {Wisdow, Dexterity}},
{"��������", {Strenght, Dexterity}},
{"����������� �������", {Dexterity, Intellegence}, {TargetTrap, 1}},
{"������� �����", {Wisdow, Intellegence}},
{"��������� � ����", {Dexterity, Dexterity}, {TargetSelf}, {}, 0, "%����� �������� �����%�� �� ���� ������."},
{"������� �����", {Dexterity, Dexterity}, {TargetDoor, 1}, {0}, 50, "%����� ������%� �����."},
{"�������� �������", {Dexterity, Dexterity}, {TargetCreature, 1}, {0}, 25},
{"�������", {Intellegence, Intellegence}},
{"�����", {Dexterity, Charisma}, {}, {0}, 10, "%����� ��������%� �������� �����."},
{"���������� ����", {Intellegence, Intellegence}},
{"�������� ����", {Charisma, Dexterity}, {TargetCreature, 1}, {0, 2}, 25},
{"�������", {Intellegence, Intellegence}},
{"�������", {Wisdow, Intellegence}, {TargetCreature, 1}, {}, 10, "%����� ���������� ����."},
{"�����������", {Intellegence, Intellegence}},
{"���������� ����", {Strenght, Intellegence}},
{"��������� ����", {Strenght, Intellegence}},
{"���������", {Wisdow, Constitution}},
//
{"�������� �����", {Dexterity, Dexterity}},
{"�������� �����", {Strenght, Dexterity}},
{"�������� �������", {Strenght, Constitution}},
{"�������� ����� ��������", {Strenght, Dexterity}},
};
assert_enum(skill, TwoWeaponFighting);
getstr_enum(skill);

static const char* talk_subjects[] = {"������", "��������", "������", "�������"};
static const char* talk_object[] = {"���������", "��������� ������", "��������� ��������", "����"};
static const char* talk_location[] = {"����������", "������", "�������", "�������", "����"};
static const char* talk_games[] = {"������", "�����", "���������"};

void creature::raise(skill_s value) {
	skills[value] += xrand(3, 9);
}

int creature::get(skill_s value) const {
	auto result = getbasic(value);
	result += get(skill_data[value].ability[0]) + get(skill_data[value].ability[1]);
	return result;
}

bool creature::use(skill_s value) {
	if(is(Anger)) {
		if(isplayer())
			logs::add("��� ���� ������� ������ � ���� � ����������.");
		return false;
	}
	targets ti;
	auto& e = skill_data[value];
	if(e.type.target == NoTarget) {
		if(isplayer())
			logs::add("����� %1 �� ������������ �������� �������", ::getstr(value));
		return false;
	} else {
		if(!gettarget(ti, e.type))
			return false;
	}
	unsigned stack = 0;
	auto r = d100();
	auto v = get(value);
	if(e.koef[0])
		v = v / e.koef[0];
	if(e.koef[1] && ti.cre)
		v = v - ti.cre->get(value) / e.koef[1];
	switch(value) {
	case PickPockets:
		switch(rand() % 5) {
		case 1:
			say("������ ������� �������. ��� ... ��� �� �� ����������? ... �����. �����, ����� � ������ ��� ��������.");
			break;
		case 2:
			say("� - ������ ��� ���?");
			break;
		case 3:
			say("�� �� ������ ��� ������ � %1? �, �������, � ��� �������� ������.", maprnd(talk_location));
			break;
		default:
			say("������ ��� ������� ��� %1 � %2? ���? � ����...", maprnd(talk_subjects), maprnd(talk_object));
			break;
		}
		break;
	case Gambling:
		stack = 20 * (1 + get(Gambling) / 20);
		if(money < stack) {
			if(isplayer())
				logs::add("� ���� ��� ������������ ���������� �����.");
			return false;
		}
		say("����� ������� � %1?", maprnd(talk_games));
		if(ti.cre->money < stack) {
			ti.cre->say("���. � �� ����. � ������ ���.");
			return false;
		}
		break;
	}
	if(r >= v) {
		if(d100() < 60) {
			if(isplayer())
				logs::add("������� �� ������� � ���� �������� ������.");
			set(Anger, Minute*xrand(2, 5));
		}
		switch(value) {
		case Gambling:
			money -= stack;
			ti.cre->money += stack;
			if(isplayer())
				logs::add("�� �������� [-%1i] �����.", stack);
			if(ti.cre->isplayer())
				logs::add("�� ������� [+%1i] �����.", stack);
			break;
		}
		return false;
	}
	if(e.text_success) {
		if(ti.cre)
			ti.cre->act(e.text_success);
	}
	switch(value) {
	case Diplomacy:
		set(Goodwill, FiveMinutes);
		break;
	case HideInShadow:
		set(Hiding, FiveMinutes*(1 + v / 20));
		break;
	case Lockpicking:
		game::set(ti.pos, Sealed, false);
		break;
	case PickPockets:
		if(true) {
			auto count = (unsigned)xrand(3, 18);
			if(count > ti.cre->money)
				count = ti.cre->money;
			ti.cre->money -= count;
			money += count;
			if(isplayer())
				act("�� �����%� %1i �����.", count);
		}
		break;
	case Gambling:
		money += stack;
		ti.cre->money -= stack;
		if(isplayer())
			act("�� �������%� [+%1i] �����.", stack);
		if(ti.cre->isplayer())
			ti.cre->act("�� ��������%� [-%1i] �����.", stack);
		break;
	}
	if(e.experience)
		addexp(e.experience);
	return true;
}