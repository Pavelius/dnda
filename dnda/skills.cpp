#include "main.h"

static struct skillinfo {
	const char*		name;
	ability_s		ability[2];
	targetdesc		type;
	unsigned char	koef[2];
	unsigned		experience;
	const char*		text_success;
} skill_data[] = {{"Нет навыка"},
{"Торговля", {Charisma, Intellegence}},
{"Блеф", {Charisma, Dexterity}},
{"Дипломатия", {Charisma, Wisdow}, {TargetCreature, 1}, {}, 0, "%герой одобрил%а предложение."},
//
{"Акробатика", {Dexterity, Dexterity}},
{"Внимательность", {Wisdow, Dexterity}},
{"Атлетика", {Strenght, Dexterity}},
{"Обезвредить ловушки", {Dexterity, Intellegence}, {TargetTrap, 1}},
{"Слышать звуки", {Wisdow, Intellegence}},
{"Прятаться в тени", {Dexterity, Dexterity}, {TargetSelf}, {}, 0, "%герой внезапно изчез%ла из поля зрения."},
{"Открыть замок", {Dexterity, Dexterity}, {TargetDoor, 1}, {0}, 50, "%герой вскрыл%а замок."},
{"Очистить карманы", {Dexterity, Dexterity}, {TargetCreature, 1}, {0}, 25},
{"Алхимия", {Intellegence, Intellegence}},
{"Танцы", {Dexterity, Charisma}, {}, {0}, 10, "%герой станевал%а отличный танец."},
{"Инженерное дело", {Intellegence, Intellegence}},
{"Азартные игры", {Charisma, Dexterity}, {TargetCreature, 1}, {0, 2}, 25},
{"История", {Intellegence, Intellegence}},
{"Лечение", {Wisdow, Intellegence}, {TargetCreature, 1}, {}, 10, "%герой перевязала раны."},
{"Грамотность", {Intellegence, Intellegence}},
{"Шахтерское дело", {Strenght, Intellegence}},
{"Кузнечное дело", {Strenght, Intellegence}},
{"Выживание", {Wisdow, Constitution}},
//
{"Владение луком", {Dexterity, Dexterity}},
{"Владение мечом", {Strenght, Dexterity}},
{"Владение топором", {Strenght, Constitution}},
{"Сражение двумя оружиями", {Strenght, Dexterity}},
};
assert_enum(skill, TwoWeaponFighting);
getstr_enum(skill);

static const char* talk_subjects[] = {"гномов", "хоббитов", "эльфов", "рыцарей"};
static const char* talk_object[] = {"сокровище", "волшебное кольцо", "проклятый артефакт", "гору"};
static const char* talk_location[] = {"библиотеку", "ратушу", "магазин", "таверну", "храм"};
static const char* talk_games[] = {"кубики", "карты", "наперстки"};

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
			logs::add("Вам надо немного прийти в себя и успокоится.");
		return false;
	}
	targets ti;
	auto& e = skill_data[value];
	if(e.type.target == NoTarget) {
		if(isplayer())
			logs::add("Навык %1 не используется подобным образом", ::getstr(value));
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
		stack = 20 * (1 + get(Gambling) / 20);
		if(money < stack) {
			if(isplayer())
				logs::add("У тебя нет достаточного количества денег.");
			return false;
		}
		say("Давай сыграем в %1?", maprnd(talk_games));
		if(ti.cre->money < stack) {
			ti.cre->say("Нет. Я на мели. В другой раз.");
			return false;
		}
		break;
	}
	if(r >= v) {
		if(d100() < 60) {
			if(isplayer())
				logs::add("Попытка не удалась и тебя охватила злость.");
			set(Anger, Minute*xrand(2, 5));
		}
		switch(value) {
		case Gambling:
			money -= stack;
			ti.cre->money += stack;
			if(isplayer())
				logs::add("Ты проиграл [-%1i] монет.", stack);
			if(ti.cre->isplayer())
				logs::add("Ты выиграл [+%1i] монет.", stack);
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
				act("Ты украл%а %1i монет.", count);
		}
		break;
	case Gambling:
		money += stack;
		ti.cre->money -= stack;
		if(isplayer())
			act("Ты выиграл%а [+%1i] монет.", stack);
		if(ti.cre->isplayer())
			ti.cre->act("Ты проиграл%а [-%1i] монет.", stack);
		break;
	}
	if(e.experience)
		addexp(e.experience);
	return true;
}