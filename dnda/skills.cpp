#include "main.h"

void setstate(effectparam& e);
void healdamage(effectparam& e);

static void removetrap(effectparam& e) {
	game::set(e.pos, NoTileObject);
}

static void removelock(effectparam& e) {
	game::set(e.pos, Sealed, false);
}

static void pickpockets(effectparam& e) {
	auto count = (unsigned)xrand(3, 18);
	if(count > e.cre->money)
		count = e.cre->money;
	e.cre->money -= count;
	e.player.money += count;
	if(e.player.isplayer())
		e.player.act("%����� �����%� [%1i] �����.", count);
}

static void dance(effectparam& e) {}

static void literacy(effectparam& e) {
}

static void gamble(effectparam& e) {
	e.player.money += e.param;
	e.cre->money -= e.param;
	if(e.player.isplayer())
		e.player.act("%����� �������%� [+%1i] �����.", e.param);
	if(e.cre->isplayer())
		e.cre->act("%����� ��������%� [-%1i] �����.", e.param);
}

static struct skillinfo {
	const char*		name;
	ability_s		ability[2];
	effectinfo		effect;
	unsigned char	koef[2];
} skill_data[] = {{"��� ������"},
{"��������", {Charisma, Intellegence}},
{"����", {Charisma, Dexterity}},
{"����������", {Charisma, Wisdow}, {{TargetCreature, 1}, {}, setstate, Turn / 2, {Goodwill}, "%����� �������%� �����������."}},
//
{"����������", {Dexterity, Dexterity}},
{"��������������", {Wisdow, Dexterity}},
{"��������", {Strenght, Dexterity}},
{"����������� �������", {Dexterity, Intellegence}, {{TargetTrap, 1}, {}, removetrap, Instant, {}, "%����� ����������%� �������."}},
{"������� �����", {Wisdow, Intellegence}},
{"��������� � ����", {Dexterity, Dexterity}, {{TargetSelf}, {}, setstate, Turn / 2, {Hiding}, "%����� �������� �����%�� �� ���� ������."}},
{"������� �����", {Dexterity, Intellegence}, {{TargetDoor, 1}, {}, removelock, Instant, {}, "%����� ������%� �����."}, 50},
{"�������� �������", {Dexterity, Charisma}, {{TargetCreature, 1}, {}, pickpockets, Instant, {}, 0, 25}},
{"�������", {Intellegence, Intellegence}},
{"�����", {Dexterity, Charisma}, {{TargetSelf}, {}, dance, Instant, {}, "%����� ��������%� �������� �����.", 10}},
{"���������� ����", {Intellegence, Intellegence}},
{"�������� ����", {Charisma, Dexterity}, {{TargetCreature, 1}, {}, gamble, Instant, {}, 0, 25}, {0, 2}},
{"�������", {Intellegence, Intellegence}},
{"�������", {Wisdow, Intellegence}, {{TargetCreature, 1}, {}, healdamage, Instant, {}, "%����� ���������%� ����.", 5}},
{"�����������", {Intellegence, Intellegence}, {{TargetItemReadable}, {}, literacy}},
{"���������� ����", {Strenght, Intellegence}},
{"��������� ����", {Strenght, Intellegence}},
{"���������", {Wisdow, Constitution}},
//
{"�������� �����", {Dexterity, Dexterity}},
{"�������� �����", {Strenght, Dexterity}},
{"�������� �������", {Strenght, Constitution}},
{"�������� ����� ��������", {Strenght, Dexterity}},
//
{"������������� ������", {Constitution, Strenght}},
{"������������� �������������", {Dexterity, Dexterity}},
{"������������� ����", {Constitution, Dexterity}},
{"������������� ���", {Constitution, Constitution}},
};
assert_enum(skill, ResistPoison);
getstr_enum(skill);

static const char* talk_subjects[] = {"������", "��������", "������", "�������"};
static const char* talk_object[] = {"���������", "��������� ������", "��������� ��������", "����"};
static const char* talk_location[] = {"����������", "������", "�������", "�������", "����"};
static const char* talk_games[] = {"������", "�����", "���������"};

void creature::raise(skill_s value) {
	if(value<=LastSkill)
		skills[value] += xrand(3, 9);
}

int creature::get(skill_s value) const {
	auto result = getbasic(value);
	result += get(skill_data[value].ability[0]) + get(skill_data[value].ability[1]);
	switch(value) {
	case ResistPoison:
		if(race == Dwarf)
			result += 30;
		break;
	}
	return result;
}

bool creature::use(skill_s value) {
	if(is(Anger)) {
		if(isplayer())
			logs::add("��� ���� ������� ������ � ���� � ����������.");
		return false;
	}
	auto& e = skill_data[value];
	effectparam ep(e.effect, *this, true);
	if(e.effect.type.target == NoTarget) {
		if(isplayer())
			logs::add("����� %1 �� ������������ �������� �������", getstr(value));
		return false;
	} else {
		if(!gettarget(ep, ep.type))
			return false;
	}
	auto r = d100();
	auto v = get(value);
	if(e.koef[0])
		v = v / e.koef[0];
	if(e.koef[1] && ep.cre)
		v = v - ep.cre->get(value) / e.koef[1];
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
		ep.param = 20 * (1 + get(Gambling) / 20);
		if((int)money < ep.param) {
			if(isplayer())
				logs::add("� ���� ��� ������������ ���������� �����.");
			return false;
		}
		say("����� ������� � %1?", maprnd(talk_games));
		if((int)ep.cre->money < ep.param) {
			ep.cre->say("���. � �� ����. � ������ ���.");
			return false;
		}
		break;
	}
	if(r >= v) {
		if(d100() < 40) {
			if(isplayer())
				logs::add("������� �� ������� � ���� �������� ������.");
			set(Anger, Minute * xrand(2, 5));
		} else {
			if(isplayer())
				logs::add("������� �� �������.");
		}
		switch(value) {
		case Gambling:
			money -= ep.param;
			ep.cre->money += ep.param;
			if(isplayer())
				logs::add("�� �������� [-%1i] �����.", ep.param);
			if(ep.cre->isplayer())
				logs::add("�� ������� [+%1i] �����.", ep.param);
			break;
		}
		return false;
	}
	ep.apply();
	return true;
}