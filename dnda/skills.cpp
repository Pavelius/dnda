#include "main.h"

static const char* talk_subjects[] = {"������", "��������", "������", "�������", "�������", "�������", "�������"};
static const char* talk_object[] = {"���������", "��������� ������", "��������� ��������", "����", "�������� ������"};
static const char* talk_location[] = {"����������", "������", "�������", "�������", "����"};
static const char* talk_games[] = {"������", "�����", "���������"};

void setstate(effectparam& e);
int compare_skills(const void* p1, const void* p2);

static void healdamage(effectparam& e) {
	auto damage = e.damage.max + e.skill_value / 10;
	e.cre->heal(e.damage.roll(), true);
}

static void removetrap(effectparam& e) {
	game::set(e.pos, NoTileObject);
}

static bool testbash(effectparam& e) {
	e.skill_bonus = -20;
	return true;
}

static void bashdoor(effectparam& e) {
	game::set(e.pos, NoTileObject);
}

static void removelock(effectparam& e) {
	game::set(e.pos, Sealed, false);
}

static bool isundead(effectparam& e) {
	return e.cre->is(Undead);
}

static bool test_pickpockets(effectparam& e) {
	static const char* talk[] = {
		"������ ������� �������. ��� ... ��� �� �� ����������? ... �����. �����, ����� � ������ ��� ��������.",
		"� - ������ ��� ���?",
		"�� �� ������ ��� ������ � %3? �, �������, � ��� �������� ������.",
		"������ ��� ������� ��� %1 � %2? ���? � ����..."
	};
	e.player.say(maprnd(talk), maprnd(talk_subjects), maprnd(talk_object), maprnd(talk_location));
	return true;
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
	if(e.itm)
		e.player.use(*e.itm);
}

static bool test_gamble(effectparam& e) {
	e.param = 20 * (1 + e.player.get(Gambling) / 20);
	if((int)e.player.money < e.param) {
		e.player.hint("� ���� ��� ������������ ���������� �����.");
		return false;
	}
	e.player.say("����� ������� � %1?", maprnd(talk_games));
	if((int)e.cre->money < e.param) {
		e.cre->say("���. � �� ����. � ������ ���.");
		return false;
	}
	e.skill_bonus = -e.cre->get(Gambling) / 2;
	return true;
}

static void gamble(effectparam& e) {
	e.player.money += e.param;
	e.cre->money -= e.param;
	e.cre->act("%����� ��������%� [%1i] �����.", e.param);
}

static void failskill(effectparam& e) {
	if(d100() < 40) {
		e.player.hint("������� �� ������� � ���� �������� ������.");
		e.player.set(Anger, Minute * xrand(2, 5));
	} else
		e.player.hint("������� �� �������.");
}

static void failgamble(effectparam& e) {
	e.player.money -= e.param;
	e.cre->money += e.param;
	e.player.act("%����� ��������%� [%1i] �����.", e.param);
}

static void killing(effectparam& e) {
	auto total = e.skill_value + e.skill_bonus;
	e.player.meleeattack(e.cre, total / 2, total / 30);
}

static void killing_fail(effectparam& e) {
	e.player.meleeattack(e.cre);
}

static struct skill_info {
	const char*		name;
	const char*		nameof;
	ability_s		ability[2];
	effectinfo		effect;
	enchantment_s	enchant;
	bool			deny_ability;
} skill_data[] = {{"��� ������"},
{"��������", "��������", {Charisma, Intellegence}},
{"����", "������", {Charisma, Dexterity}},
{"����������", "����������", {Charisma, Wisdow}, {{TargetFriendly, 1}, {}, {setstate}, {Goodwill, Turn / 2}, "%����� �������%� �����������."}},
//
{"����������", "����������", {Dexterity, Dexterity}},
{"��������������", "��������������", {Wisdow, Dexterity}},
{"��������", "��������", {Strenght, Dexterity}, {{TargetDoor, 1}, {}, {bashdoor, 0, testbash}, {}, "%����� ������%�� ����� � �����.", {}, 20}},
{"��������", "��������", {Dexterity, Dexterity}, {{TargetFriendly, 1}, {}, {killing, killing_fail}, {}, "%����� �����%�� ������ ����.", {}, 50}},
{"������������", "�����������", {Wisdow, Constitution}},
{"����������� �������", "�������", {Dexterity, Intellegence}, {{TargetTrap, 1}, {}, {removetrap}, {}, "%����� ����������%� �������.", {}, 30}},
{"������� �����", "�����", {Wisdow, Intellegence}},
{"��������� � ����", "����������", {Dexterity, Dexterity}, {{TargetSelf}, {}, {setstate}, {Hiding, Turn / 2}, "%����� �������� �����%�� �� ���� ������."}},
{"������� �����", "������", {Dexterity, Intellegence}, {{TargetDoorSealed, 1}, {}, {removelock}, {}, "%����� ������%� �����.", {}, 50}},
{"�������� �������", "���������", {Dexterity, Charisma}, {{TargetFriendly, 1}, {}, {pickpockets, 0, test_pickpockets}, {}, 0, {}, 25}},
{"�������", "�������", {Intellegence, Intellegence}},
{"�����", "������", {Dexterity, Charisma}, {{TargetSelf}, {}, {dance}, {}, "%����� ��������%� �������� �����.", {}, 10}},
{"���������� ����", "���������", {Intellegence, Intellegence}},
{"�������� ����", "�������� ���", {Charisma, Dexterity}, {{TargetFriendly, 1}, {}, {gamble, failgamble, test_gamble}, {}, 0, {}, 25}},
{"�������", "�������", {Intellegence, Intellegence}},
{"�������", "�������", {Wisdow, Intellegence}, {{TargetFriendlyWounded, 1}, {}, {healdamage}, {}, "%����� ���������%� ����.", {1, 3}, 5}},
{"�����������", "������ � ������", {Intellegence, Intellegence}, {{TargetItemReadable}, {}, {literacy, literacy}, {}, 0, {}, 25}},
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
	if(e.effect.type.target == NoTarget) {
		hint("����� %1 �� ������������ �������� �������", getstr(value));
		return;
	}
	if(e.effect.proc.validate) {
		if(!e.effect.proc.validate(*this))
			return;
	}
	apply(e.effect, 0, isplayer(), 0, 0, d100(), get(value), failskill);
	// Use skill is one minute
	wait(Minute);
}

skill_s creature::aiskill(aref<creature*> creatures) {
	if(is(Anger))
		return NoSkill;
	adat<skill_s, LastSkill + 1> recomended;
	for(auto i = (skill_s)1; i <= LastSkill; i = (skill_s)(i + 1)) {
		if(!skills[i])
			continue;
		if(!skill_data[i].effect.type.iscreature())
			continue;
		if(!skill_data[i].effect.type.isallow(*this, creatures))
			continue;
		recomended.add(i);
	}
	if(recomended.count > 0)
		return recomended.data[rand() % recomended.count];
	return NoSkill;
}

skill_s creature::aiskill() {
	if(is(Anger))
		return NoSkill;
	aref<creature*> source = {};
	adat<skill_s, LastSkill + 1> recomended;
	for(auto i = (skill_s)1; i <= LastSkill; i = (skill_s)(i + 1)) {
		if(!skills[i])
			continue;
		if(!skill_data[i].effect.type.isposition())
			continue;
		if(!skill_data[i].effect.type.isallow(*this, source))
			continue;
		recomended.add(i);
	}
	if(recomended.count > 0)
		return recomended.data[rand() % recomended.count];
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