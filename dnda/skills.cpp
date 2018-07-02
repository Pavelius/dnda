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
		e.player.act("%герой украл%а [%1i] монет.", count);
}

static void dance(effectparam& e) {}

static void literacy(effectparam& e) {
	if(e.itm)
		e.player.use(*e.itm);
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

static struct skillinfo {
	const char*		name;
	ability_s		ability[2];
	effectinfo		effect;
	unsigned char	koef[2];
} skill_data[] = {{"Нет навыка"},
{"Торговля", {Charisma, Intellegence}},
{"Блеф", {Charisma, Dexterity}},
{"Дипломатия", {Charisma, Wisdow}, {{TargetNoHostileNoSelf, 1}, {}, setstate, Turn / 2, {Goodwill}, "%герой одобрил%а предложение."}},
//
{"Акробатика", {Dexterity, Dexterity}},
{"Внимательность", {Wisdow, Dexterity}},
{"Атлетика", {Strenght, Dexterity}},
{"Концентрация", {Wisdow, Constitution}},
{"Обезвредить ловушки", {Dexterity, Intellegence}, {{TargetTrap, 1}, {}, removetrap, Instant, {}, "%герой обезвредил%а ловушку."}},
{"Слышать звуки", {Wisdow, Intellegence}},
{"Прятаться в тени", {Dexterity, Dexterity}, {{TargetSelf}, {}, setstate, Turn / 2, {Hiding}, "%герой внезапно изчез%ла из поля зрения."}},
{"Открыть замок", {Dexterity, Intellegence}, {{TargetDoor, 1}, {}, removelock, Instant, {}, "%герой вскрыл%а замок."}, 50},
{"Очистить карманы", {Dexterity, Charisma}, {{TargetNoHostileNoSelf, 1}, {}, pickpockets, Instant, {}, 0, 25}},
{"Алхимия", {Intellegence, Intellegence}},
{"Танцы", {Dexterity, Charisma}, {{TargetSelf}, {}, dance, Instant, {}, "%герой станевал%а отличный танец.", 10}},
{"Инженерное дело", {Intellegence, Intellegence}},
{"Азартные игры", {Charisma, Dexterity}, {{TargetNoHostileNoSelf, 1}, {}, gamble, Instant, {}, 0, {}, 25, failgamble}, {0, 2}},
{"История", {Intellegence, Intellegence}},
{"Лечение", {Wisdow, Intellegence}, {{TargetNoHostileNoSelf, 1}, {}, healdamage, Instant, {}, "%герой перевязал%а раны.", 5}},
{"Грамотность", {Intellegence, Intellegence}, {{TargetItemReadable}, {}, literacy, Minute / 2, {}, 0, {}, 25, literacy}},
{"Шахтерское дело", {Strenght, Intellegence}},
{"Кузнечное дело", {Strenght, Intellegence}},
{"Выживание", {Wisdow, Constitution}},
//
{"Владение луком", {Dexterity, Dexterity}},
{"Владение мечом", {Strenght, Dexterity}},
{"Владение топором", {Strenght, Constitution}},
{"Сражение двумя оружиями", {Strenght, Dexterity}},
//
{"Сопротивление холоду", {Constitution, Strenght}},
{"Сопротивление электричеству", {Dexterity, Dexterity}},
{"Сопротивление огню", {Constitution, Dexterity}},
{"Сопротивление яду", {Constitution, Constitution}},
};
assert_enum(skill, ResistPoison);
getstr_enum(skill);

static const char* talk_subjects[] = {"гномов", "хоббитов", "эльфов", "рыцарей"};
static const char* talk_object[] = {"сокровище", "волшебное кольцо", "проклятый артефакт", "гору"};
static const char* talk_location[] = {"библиотеку", "ратушу", "магазин", "таверну", "храм"};
static const char* talk_games[] = {"кубики", "карты", "наперстки"};

void creature::raise(skill_s value) {
	if(value <= LastSkill)
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
			say("Слушай смешной анекдот. Так ... как же он начинается? ... Забыл. Ладно, давай в другой раз расскажу.");
			break;
		case 2:
			say("О - смотри кто это?");
			break;
		case 3:
			say("Вы не знаете как пройти в %1? О, спасибо, я сам вспомнил дорогу.", maprnd(talk_location));
			break;
		default:
			say("Слышал эту историю про %1 и %2? Нет? Я тоже...", maprnd(talk_subjects), maprnd(talk_object));
			break;
		}
		break;
	case Gambling:
		ep.param = 20 * (1 + get(Gambling) / 20);
		if((int)money < ep.param) {
			if(isplayer())
				logs::add("У тебя нет достаточного количества денег.");
			return;
		}
		say("Давай сыграем в %1?", maprnd(talk_games));
		if((int)ep.cre->money < ep.param) {
			ep.cre->say("Нет. Я на мели. В другой раз.");
			return;
		}
		break;
	}
	if(!ep.fail)
		ep.fail = failskill;
	if(r >= v)
		ep.apply(ep.fail);
	else
		ep.apply();
}