#include "main.h"

static const char* talk_subjects[] = {"гномов", "хоббитов", "эльфов", "рыцарей", "троллей", "дракона", "колдуна"};
static const char* talk_object[] = {"сокровище", "волшебное кольцо", "проклятый артефакт", "гору", "истинную любовь"};
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
	const char*		nameof;
	ability_s		ability[2];
	effectinfo		effect;
	unsigned char	koef[2];
	enchantment_s	enchant;
	bool			deny_ability;
} skill_data[] = {{"Нет навыка"},
{"Торговля", "торговли", {Charisma, Intellegence}},
{"Блеф", "обмана", {Charisma, Dexterity}},
{"Дипломатия", "дипломатии", {Charisma, Wisdow}, {{TargetFriendlySelf, 1}, {}, {setstate}, Turn / 2, {Goodwill}, "%герой одобрил%а предложение."}},
//
{"Акробатика", "акробатики", {Dexterity, Dexterity}},
{"Внимательность", "внимательности", {Wisdow, Dexterity}},
{"Атлетика", "атлетики", {Strenght, Dexterity}},
{"Концентрация", "концетрации", {Wisdow, Constitution}},
{"Обезвредить ловушки", "ловушек", {Dexterity, Intellegence}, {{TargetTrap, 1}, {}, {removetrap}, Instant, {}, "%герой обезвредил%а ловушку.", {}, 30}},
{"Слышать звуки", "слуха", {Wisdow, Intellegence}},
{"Прятаться в тени", "скрытности", {Dexterity, Dexterity}, {{TargetSelf}, {}, {setstate}, Turn / 2, {Hiding}, "%герой внезапно изчез%ла из поля зрения."}},
{"Открыть замок", "взлома", {Dexterity, Intellegence}, {{TargetDoor, 1}, {}, {removelock}, Instant, {}, "%герой вскрыл%а замок.", {}, 50}},
{"Очистить карманы", "воровства", {Dexterity, Charisma}, {{TargetFriendlySelf, 1}, {}, {pickpockets, 0, test_pickpockets}, Instant, {}, 0, {}, 25}},
{"Алхимия", "алхимии", {Intellegence, Intellegence}},
{"Танцы", "танцев", {Dexterity, Charisma}, {{TargetSelf}, {}, {dance}, Instant, {}, "%герой станевал%а отличный танец.", {}, 10}},
{"Инженерное дело", "инженерии", {Intellegence, Intellegence}},
{"Азартные игры", "азартных игр", {Charisma, Dexterity}, {{TargetFriendlySelf, 1}, {}, {gamble, failgamble, test_gamble}, Instant, {}, 0, {}, 25}, {0, 2}},
{"История", "истории", {Intellegence, Intellegence}},
{"Лечение", "лечения", {Wisdow, Intellegence}, {{TargetFriendlySelf, 1}, {}, {healdamage}, Instant, {}, "%герой перевязал%а раны.", 5}},
{"Грамотность", "письма и чтения", {Intellegence, Intellegence}, {{TargetItemReadable}, {}, {literacy, literacy}, Minute / 2, {}, 0, {}, 25}},
{"Шахтерское дело", "шахтерского дела", {Strenght, Intellegence}},
{"Кузнечное дело", "кузнечного дела", {Strenght, Intellegence}},
{"Выживание", "выживания", {Wisdow, Constitution}},
{"Плавание", "плавания", {Strenght, Constitution}},
//
{"Владение луком", "стрельбы из лука", {Dexterity, Dexterity}},
{"Владение мечом", "сражения на мечах", {Strenght, Dexterity}},
{"Владение топором", "сражения на топорах", {Strenght, Constitution}},
{"Сражение двумя оружиями", "ужасного оружия", {Strenght, Dexterity}},
//
{"Сопротивление кислоте", "кислоты", {Dexterity, Constitution}, {}, {}, OfAcidResistance, true},
{"Сопротивление шарму", "красоты и любви", {Wisdow, Wisdow}, {}, {}, OfCharmResistance},
{"Сопротивление холоду", "холода", {Constitution, Strenght}, {}, {}, OfColdResistance, true},
{"Сопротивление электричеству", "молнии", {Dexterity, Dexterity}, {}, {}, OfElectricityResistance, true},
{"Сопротивление огню", "огня", {Constitution, Dexterity}, {}, {}, OfFireResistance, true},
{"Сопротивление яду", "яда", {Constitution, Constitution}, {}, {}, OfPoisonResistance},
{"Дыхание водой", "воды", {Strenght, Constitution}, {}, {}, OfWaterproof, true},
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