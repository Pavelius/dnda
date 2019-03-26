#include "main.h"

static const char* talk_subjects[] = {"������", "��������", "������", "�������", "�������", "�������", "�������", "���� ������"};
static const char* talk_object[] = {"���������", "��������� ������", "��������� ��������", "����", "�������� ������", "���������� ����������"};
static const char* talk_location[] = {"����������", "������", "�������", "�������", "����"};
static const char* talk_games[] = {"������", "�����", "���������", "������"};

bool	setstate(sceneparam& e, creature& v, bool run);
int		compare_skills(const void* p1, const void* p2);

void message(creature& player, const target_info& ti, const char* format) {
	if(!format)
		return;
	char temp[260];
	switch(ti.type) {
	case Creature:
		ti.cre->act(format, player.getname());
		break;
	case ItemPtr:
		player.act(format, ti.itm->getname(temp, zendof(temp)));
		break;
	default:
		player.act(format);
		break;
	}
}

static void skill_success(const sceneparam& sp, const target_info& ti) {
	message(sp.player, ti, sp.messages.success);
	sp.player.addexp(sp.experience);
}

static void skill_fail(const sceneparam& sp, const target_info& ti) {
	if(sp.messages.fail)
		message(sp.player, ti, sp.messages.fail);
	else
		sp.player.hint("������� �� �������.");
	if(sp.roll>90) {
		sp.player.hint("��� �������� ���������������� ������.");
		sp.player.set(Anger, Minute*xrand(3, 7));
	} else if(sp.roll > 70) {
		sp.player.hint("�� ����� ���� �������, �� ��� ���� ������.");
		sp.player.wait(Minute * xrand(1, 3));
	}
}

static bool heal(sceneparam& e, creature& v, bool run) {
	if(run) {
		auto damage = e.damage.max + e.param / 10;
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
	if(!game::is(index, Sealed))
		return false;
	if(run) {
		if(e.player.roll(Athletics, -20)) {
			game::set(index, NoTileObject);
			skill_success(e, index);
		} else
			skill_fail(e, index);
	}
	return true;
}

static bool openlock(sceneparam& e, short unsigned index, bool run) {
	auto t = game::getobject(index);
	if(t != Door)
		return false;
	if(!game::is(index, Sealed))
		return false;
	if(run)
		game::set(index, Sealed, false);
	return true;
}

static bool pickpockets(sceneparam& e, creature& v, bool run) {
	if(v.isfriend(&e.player))
		return false;
	if(!v.getmoney())
		return false;
	if(run) {
		static const char* talk[] = {
			"������ ������� �������. ��� ... ��� �� �� ����������? ... �����. �����, ����� � ������ ��� ��������.",
			"� - ������ ��� ���?",
			"�� �� ������ ��� ������ � %3? �, �������, � ��� �������� ������.",
			"������ ��� ������� ��� %1 � %2? ���? � ����..."
		};
		e.player.say(maprnd(talk), maprnd(talk_subjects), maprnd(talk_object), maprnd(talk_location));
		if(e.player.roll(PickPockets)) {
			auto count = xrand(3, 18);
			if(count > v.getmoney())
				count = v.getmoney();
			v.setmoney(v.getmoney() - count);
			e.player.setmoney(e.player.getmoney() + count);
			e.player.hint("%����� �����%� [%1i] �����.", count);
			e.player.addexp(e.experience);
		} else
			skill_fail(e, &v);
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
	if(!v.isreadable())
		return false;
	if(run) {
		if(v.istome())
			e.player.readbook(v);
		else {
			v.setidentify(true);
			auto spell = v.getspell();
			if(spell) {
				//e.player.use(sc, spell, 1 + v.getquality(), "%����� ��������%� ������.");
				v.clear();
				e.player.wait(Minute / 2);
			}
		}
	}
	return true;
}

static bool dance(sceneparam& e, creature& v, bool run) {
	return true;
}

static bool gamble(sceneparam& e, creature& v, bool run) {
	auto stack = 20 * (1 + e.param / 20);
	if(!v.getbasic(Gambling))
		return false;
	if(run) {
		if(e.player.getmoney() < stack) {
			e.player.say("��� [%2i] �����, ������� � %1.", maprnd(talk_games), stack);
			return false;
		}
		e.player.say("����� ������� � %1?", maprnd(talk_games));
		if(v.getmoney() < stack) {
			v.say("���. � �� ����. � ������ ���.");
			return false;
		}
		v.say("���� ��������. � ������ ��.");
		auto result = e.player.roll(Gambling, 0, v, Gambling, 0);
		if(result > 0) {
			e.player.setmoney(e.player.getmoney() + stack);
			v.setmoney(v.getmoney() - stack);
			e.player.act("%����� �������%� [%1i] �����.", stack);
			e.player.addexp(e.experience);
		} else if(result < 0) {
			e.player.setmoney(e.player.getmoney() - stack);
			v.setmoney(v.getmoney() + stack);
			e.player.act("%����� ��������%� [%1i] �����.", stack);
		} else
			e.player.act("���� �� �����. ��� �������� ��� �����.");
	}
	return true;
}

static bool backstabbing(sceneparam& e, creature& v, bool run) {
	if(run) {
		auto total = 0;
		if(e.player.roll(Backstabbing))
			total = e.player.get(Backstabbing);
		e.player.meleeattack(&v, total / 2, total / 30);
	}
	return true;
}

static struct skill_info {
	const char*		name;
	const char*		nameof;
	ability_s		ability[2];
	effect_info		effect;
	enchantment_s	enchant;
	bool			deny_ability;
} skill_data[] = {{"��� ������"},
{"��������", "��������", {Charisma, Intellegence}},
{"����", "������", {Charisma, Dexterity}},
{"����������", "����������", {Charisma, Wisdow}, {setstate, Friendly | Close, {}, {Goodwill, Turn / 2}, 0, {0, "%����� �������%� �����������."}}},
//
{"����������", "����������", {Dexterity, Dexterity}},
{"��������������", "��������������", {Wisdow, Dexterity}},
{"��������", "��������", {Strenght, Dexterity}, {bash, Close, {}, {}, 20, {0, "%����� ������%�� ����� � �����."}}},
{"��������", "��������", {Dexterity, Dexterity}, {backstabbing, Special | Close | Friendly, {}, {}, 50, {"%����� �����%�� ������ ����."}}},
{"������������", "�����������", {Wisdow, Wisdow}},
{"����������� �������", "�������", {Dexterity, Intellegence}, {removetrap, Close | Identified, {}, {}, 30, {0, "%����� ����������%� �������."}}},
{"������� �����", "�����", {Wisdow, Intellegence}},
{"��������� � ����", "����������", {Dexterity, Dexterity}, {setstate, You, {}, {Hiding, Turn / 2}, 0, {0, "%����� �������� �����%�� �� ���� ������."}}},
{"������� �����", "������", {Dexterity, Intellegence}, {openlock, Close, {}, {}, 50, {0, "%����� ������%� �����."}}},
{"�������� �������", "���������", {Dexterity, Charisma}, {pickpockets, Special | Close, {}, {}, 25}},
{"�������", "�������", {Intellegence, Intellegence}},
{"�����", "������", {Dexterity, Charisma}, {dance, All | Near, {}, {}, 10, {0, "%����� ��������%� �������� �����."}}},
{"���������� ����", "���������", {Intellegence, Intellegence}},
{"�������� ����", "�������� ���", {Charisma, Dexterity}, {gamble, Special | Close, {}, {}, 25}},
{"�������", "�������", {Intellegence, Intellegence}},
{"�������", "�������", {Wisdow, Intellegence}, {heal, Close | Friendly | Damaged, {1, 3}, {}, 5, {0, "%����� ���������%� ����."}}},
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
	else if(value < 50)
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

bool creature::use(scene& sc, skill_s value) {
	if(is(Anger)) {
		hint("��� ���� ������� ������ � ���� � ����������.");
		return false;
	}
	auto& e = skill_data[value];
	if(!e.effect) {
		hint("����� %1 �� ������������ �������� �������", getstr(value));
		return false;
	}
	target_info ti;
	if(!choose(e.effect, sc, ti, isinteractive())) {
		hint("��� ������ %1 ��� ���������� ����", getstr(value));
		return false;
	}
	sceneparam sp(e.effect, *this, true);
	sp.param = get(value);
	sp.roll = d100();
	if(sp.flags&Special)
		sp.apply(sc, ti);
	else {
		if(sp.roll < sp.param) {
			sp.apply(sc, ti);
			skill_success(sp, ti);
		} else {
			if(sp.messages.action)
				act(sp.messages.action);
			skill_fail(sp, ti);
		}
	}
	// Use skill is one minute
	wait(Minute);
	return true;
}

const effect_info& creature::geteffect(skill_s v) {
	return skill_data[v].effect;
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