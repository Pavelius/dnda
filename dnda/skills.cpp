#include "main.h"

static const char* talk_subjects[] = {"������", "��������", "������", "�������", "�������", "�������", "�������"};
static const char* talk_object[] = {"���������", "��������� ������", "��������� ��������", "����", "�������� ������"};
static const char* talk_location[] = {"����������", "������", "�������", "�������", "����"};
static const char* talk_games[] = {"������", "�����", "���������"};

void setstate(effectparam& e);
void healdamage(effectparam& e);

static void removetrap(effectparam& e) {
	game::set(e.pos, NoTileObject);
}

static void removelock(effectparam& e) {
	game::set(e.pos, Sealed, false);
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
	return true;
}

static void gamble(effectparam& e) {
	e.player.money += e.param;
	e.cre->money -= e.param;
	if(e.player.isplayer())
		e.player.act("%����� �������%� [+%1i] �����.", e.param);
	if(e.cre->isplayer())
		e.cre->act("%����� ��������%� [-%1i] �����.", e.param);
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
	if(e.player.isplayer())
		logs::add("�� �������� [-%1i] �����.", e.param);
	if(e.cre->isplayer())
		logs::add("�� ������� [+%1i] �����.", e.param);
}

static struct skill_info {
	const char*		name;
	const char*		nameof;
	ability_s		ability[2];
	effectinfo		effect;
	unsigned char	koef[2];
	enchantment_s	enchant;
	bool			deny_ability;
} skill_data[] = {{"��� ������"},
{"��������", "��������", {Charisma, Intellegence}},
{"����", "������", {Charisma, Dexterity}},
{"����������", "����������", {Charisma, Wisdow}, {{TargetFriendlySelf, 1}, {}, {setstate}, Turn / 2, {Goodwill}, "%����� �������%� �����������."}},
//
{"����������", "����������", {Dexterity, Dexterity}},
{"��������������", "��������������", {Wisdow, Dexterity}},
{"��������", "��������", {Strenght, Dexterity}},
{"������������", "�����������", {Wisdow, Constitution}},
{"����������� �������", "�������", {Dexterity, Intellegence}, {{TargetTrap, 1}, {}, {removetrap}, Instant, {}, "%����� ����������%� �������.", {}, 30}},
{"������� �����", "�����", {Wisdow, Intellegence}},
{"��������� � ����", "����������", {Dexterity, Dexterity}, {{TargetSelf}, {}, {setstate}, Turn / 2, {Hiding}, "%����� �������� �����%�� �� ���� ������."}},
{"������� �����", "������", {Dexterity, Intellegence}, {{TargetDoor, 1}, {}, {removelock}, Instant, {}, "%����� ������%� �����.", {}, 50}},
{"�������� �������", "���������", {Dexterity, Charisma}, {{TargetFriendlySelf, 1}, {}, {pickpockets, 0, test_pickpockets}, Instant, {}, 0, {}, 25}},
{"�������", "�������", {Intellegence, Intellegence}},
{"�����", "������", {Dexterity, Charisma}, {{TargetSelf}, {}, {dance}, Instant, {}, "%����� ��������%� �������� �����.", {}, 10}},
{"���������� ����", "���������", {Intellegence, Intellegence}},
{"�������� ����", "�������� ���", {Charisma, Dexterity}, {{TargetFriendlySelf, 1}, {}, {gamble, failgamble, test_gamble}, Instant, {}, 0, {}, 25}, {0, 2}},
{"�������", "�������", {Intellegence, Intellegence}},
{"�������", "�������", {Wisdow, Intellegence}, {{TargetFriendlySelf, 1}, {}, {healdamage}, Instant, {}, "%����� ���������%� ����.", 5}},
{"�����������", "������ � ������", {Intellegence, Intellegence}, {{TargetItemReadable}, {}, {literacy, literacy}, Minute / 2, {}, 0, {}, 25}},
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
{"������������� �������", "�������", {Dexterity, Constitution}, {}, {}, OfAcidResistance, true},
{"������������� �����", "������� � �����", {Wisdow, Wisdow}, {}, {}, OfCharmResistance},
{"������������� ������", "������", {Constitution, Strenght}, {}, {}, OfColdResistance, true},
{"������������� �������������", "������", {Dexterity, Dexterity}, {}, {}, OfElectricityResistance, true},
{"������������� ����", "����", {Constitution, Dexterity}, {}, {}, OfFireResistance, true},
{"������������� ���", "���", {Constitution, Constitution}, {}, {}, OfPoisonResistance},
{"������� �����", "����", {Strenght, Constitution}, {}, {}, OfWaterproof, true},
};
assert_enum(skill, ResistWater);
getstr_enum(skill);

const char* creature::getname(skill_s id) {
	return skill_data[id].nameof;
}

void creature::raise(skill_s value) {
	skills[value] += xrand(3, 9);
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
		if(isplayer())
			logs::add("��� ���� ������� ������ � ���� � ����������.");
		return;
	}
	auto& e = skill_data[value];
	effectparam ep(e.effect, *this, true);
	if(e.effect.type.target == NoTarget) {
		if(isplayer())
			logs::add("����� %1 �� ������������ �������� �������", getstr(value));
		return;
	} else {
		if(!gettarget(ep, ep.type))
			return;
	}
	if(!ep.proc.fail)
		ep.proc.fail = failskill;
	if(ep.proc.test) {
		if(!ep.proc.test(ep))
			return;
	}
	auto r = d100();
	auto v = get(value);
	if(e.koef[0])
		v = v / e.koef[0];
	if(e.koef[1] && ep.cre)
		v = v - ep.cre->get(value) / e.koef[1];
	if(r >= v)
		ep.apply(ep.proc.fail);
	else
		ep.apply();
}

static bool isvalid(effectparam& ep) {
	if(ep.proc.validate) {
		if(!ep.proc.validate(ep))
			return false;
	}
}

static aref<creature*> filter(aref<creature*> result, aref<creature*> source, creature& player, skill_info& e) {
	effectparam ep(e.effect, player, false);
	auto pb = result.data;
	auto pe = result.data + result.count;
	for(auto p : source) {
		ep.cre = p;
		if(!isvalid(ep))
			continue;
		if(pb < pe)
			*pb++ = p;
	}
	return result;
}

bool creature::aiskill() {
	creature* creature_data[32];
	adat<skill_s, LastSkill+1> recomended;
	auto creatures = getcreatures(creature_data, {TargetAnyCreature});
	for(auto i = (skill_s)1; i <= LastSkill; i = (skill_s)(i + 1)) {
		if(!skills[i])
			continue;
		if(skill_data[i].effect.type.target == NoTarget)
			continue;
	}
	return false;
}