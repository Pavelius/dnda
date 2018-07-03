#include "main.h"

static const char* talk_subjects[] = {"гномов", "хоббитов", "эльфов", "рыцарей", "троллей", "дракона", "колдуна"};
static const char* talk_object[] = {"сокровище", "волшебное кольцо", "проклятый артефакт", "гору"};
static const char* talk_location[] = {"библиотеку", "ратушу", "магазин", "таверну", "храм"};
static const char* talk_games[] = {"кубики", "карты", "наперстки"};

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
		"Слушай смешной анекдот. Так ... как же он начинается? ... Забыл. Ладно, давай в другой раз расскажу.",
		"О - смотри кто это?",
		"Вы не знаете как пройти в %3? О, спасибо, я сам вспомнил дорогу.",
		"Слышал эту историю про %1 и %2? Нет? Я тоже..."
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
		e.player.act("%герой украл%а [%1i] монет.", count);
}

static void dance(effectparam& e) {}

static void literacy(effectparam& e) {
	if(e.itm)
		e.player.use(*e.itm);
}

static bool test_gamble(effectparam& e) {
	e.param = 20 * (1 + e.player.get(Gambling) / 20);
	if((int)e.player.money < e.param) {
		e.player.hint("У тебя нет достаточного количества денег.");
		return false;
	}
	e.player.say("Давай сыграем в %1?", maprnd(talk_games));
	if((int)e.cre->money < e.param) {
		e.cre->say("Нет. Я на мели. В другой раз.");
		return false;
	}
	return true;
}

static void gamble(effectparam& e) {
	e.player.money += e.param;
	e.cre->money -= e.param;
	if(e.player.isplayer())
		e.player.act("%герой выиграл%а [+%1i] монет.", e.param);
	if(e.cre->isplayer())
		e.cre->act("%герой проиграл%а [-%1i] монет.", e.param);
}

static void failskill(effectparam& e) {
	if(d100() < 40) {
		e.player.hint("Попытка не удалась и тебя охватила злость.");
		e.player.set(Anger, Minute * xrand(2, 5));
	} else
		e.player.hint("Попытка не удалась.");
}

static void failgamble(effectparam& e) {
	e.player.money -= e.param;
	e.cre->money += e.param;
	if(e.player.isplayer())
		logs::add("Ты проиграл [-%1i] монет.", e.param);
	if(e.cre->isplayer())
		logs::add("Ты выиграл [+%1i] монет.", e.param);
}

static struct skill_info {
	const char*		name;
	ability_s		ability[2];
	effectinfo		effect;
	unsigned char	koef[2];
	enchantment_s	enchant;
	bool			deny_ability;
} skill_data[] = {{"Нет навыка"},
{"Торговля", {Charisma, Intellegence}},
{"Блеф", {Charisma, Dexterity}},
{"Дипломатия", {Charisma, Wisdow}, {{TargetFriendlySelf, 1}, {}, setstate, Turn / 2, {Goodwill}, "%герой одобрил%а предложение."}},
//
{"Акробатика", {Dexterity, Dexterity}},
{"Внимательность", {Wisdow, Dexterity}},
{"Атлетика", {Strenght, Dexterity}},
{"Концентрация", {Wisdow, Constitution}},
{"Обезвредить ловушки", {Dexterity, Intellegence}, {{TargetTrap, 1}, {}, removetrap, Instant, {}, "%герой обезвредил%а ловушку.", {}, 30}},
{"Слышать звуки", {Wisdow, Intellegence}},
{"Прятаться в тени", {Dexterity, Dexterity}, {{TargetSelf}, {}, setstate, Turn / 2, {Hiding}, "%герой внезапно изчез%ла из поля зрения."}},
{"Открыть замок", {Dexterity, Intellegence}, {{TargetDoor, 1}, {}, removelock, Instant, {}, "%герой вскрыл%а замок.", {}, 50}},
{"Очистить карманы", {Dexterity, Charisma}, {{TargetFriendlySelf, 1}, {}, pickpockets, Instant, {}, 0, {}, 25, 0, test_pickpockets}},
{"Алхимия", {Intellegence, Intellegence}},
{"Танцы", {Dexterity, Charisma}, {{TargetSelf}, {}, dance, Instant, {}, "%герой станевал%а отличный танец.", {}, 10}},
{"Инженерное дело", {Intellegence, Intellegence}},
{"Азартные игры", {Charisma, Dexterity}, {{TargetFriendlySelf, 1}, {}, gamble, Instant, {}, 0, {}, 25, failgamble, test_gamble}, {0, 2}},
{"История", {Intellegence, Intellegence}},
{"Лечение", {Wisdow, Intellegence}, {{TargetFriendlySelf, 1}, {}, healdamage, Instant, {}, "%герой перевязал%а раны.", 5}},
{"Грамотность", {Intellegence, Intellegence}, {{TargetItemReadable}, {}, literacy, Minute / 2, {}, 0, {}, 25, literacy}},
{"Шахтерское дело", {Strenght, Intellegence}},
{"Кузнечное дело", {Strenght, Intellegence}},
{"Выживание", {Wisdow, Constitution}},
{"Плавание", {Strenght, Constitution}},
//
{"Владение луком", {Dexterity, Dexterity}},
{"Владение мечом", {Strenght, Dexterity}},
{"Владение топором", {Strenght, Constitution}},
{"Сражение двумя оружиями", {Strenght, Dexterity}},
//
{"Сопротивление кислоте", {Dexterity, Constitution}, {}, {}, OfAcidResistance, true},
{"Сопротивление холоду", {Constitution, Strenght}, {}, {}, OfColdResistance, true},
{"Сопротивление электричеству", {Dexterity, Dexterity}, {}, {}, OfElectricityResistance, true},
{"Сопротивление огню", {Constitution, Dexterity}, {}, {}, OfFireResistance, true},
{"Сопротивление яду", {Constitution, Constitution}, {}, {}, OfPoisonResistance},
{"Дыхание водой", {Strenght, Constitution}, {}, {}, OfWaterproof, true},
};
assert_enum(skill, ResistWater);
getstr_enum(skill);

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
			logs::add("Вам надо немного прийти в себя и успокоится.");
		return;
	}
	auto& e = skill_data[value];
	effectparam ep(e.effect, *this, true);
	if(e.effect.type.target == NoTarget) {
		if(isplayer())
			logs::add("Навык %1 не используется подобным образом", getstr(value));
		return;
	} else {
		if(!gettarget(ep, ep.type))
			return;
	}
	if(!ep.fail)
		ep.fail = failskill;
	if(ep.test) {
		if(!ep.test(ep))
			return;
	}
	auto r = d100();
	auto v = get(value);
	if(e.koef[0])
		v = v / e.koef[0];
	if(e.koef[1] && ep.cre)
		v = v - ep.cre->get(value) / e.koef[1];
	if(r >= v)
		ep.apply(ep.fail);
	else
		ep.apply();
}