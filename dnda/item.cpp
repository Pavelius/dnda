#include "main.h"

const int GP = 100;
const int SP = 10;
const int CP = 1;
static_assert(sizeof(item) == sizeof(int), "Invalid sizeof(item). Must be equal sizeof(int).");

static struct enchantment_info {
	const char*		id;
	const char*		name;
	char			cost;
} enchantment_data[] = {{"", ""},
{"armor", "брони", 10},
{"charisma", "харизмы", 5},
{"cold", "холода", 5},
{"constitution", "телосложения", 5},
{"defence", "защиты", 7},
{"destruction", "разрушения", 7},
{"dexterity", "ловкости", 6},
{"fire", "огня", 6},
{"intellegene", "интеллекта", 6},
{"mana", "маны", 9},
{"precision", "точности", 7},
{"regeneration", "регенерации", 10},
{"sharping", "остроты", 5},
{"smashing", "раскалывания", 5},
{"speed", "скорости", 3},
{"strenght", "силы", 8},
{"sustenance", "питания", 4},
{"vampirism", "вампиризма", 10},
{"wisdow", "мудрости", 6},
//
{"acid resistance", "сопротивления кислоте", 1},
{"cold resistance", "сопротивления холоду", 1},
{"fire resistance", "сопротивления огня", 2},
{"electricity resistance", "сопротивления электричеству", 1},
{"poison resistance", "сопротивления яду", 1},
{"waterproof", "водонепроницаемости", 1},
};
assert_enum(enchantment, LastEnchantment);
getstr_enum(enchantment);

static const char* key_names[][2] = {{"simple", "простой"},
// Металлические ключи
{"bronze", "бронзовый"},
{"cooper", "медный"},
{"steel", "стальной"},
{"silver", "серебрянный"},
{"golden", "золотой"},
// Редкие ключи для дверей подземелья
{"bone", "костяной"},
{"stone", "каменный"},
{"crystal", "кристальный"},
};
static enchantment_s swords_effect[] = {OfCold, OfDefence, OfDexterity, OfFire, OfSpeed, OfPrecision, OfSharping, OfVampirism};
static enchantment_s axe_effect[] = {OfStrenght, OfDestruction, OfFire, OfSharping, OfSmashing};
static enchantment_s bludgeon_effect[] = {OfConstitution, OfDestruction, OfFire, OfSmashing, OfStrenght};
static enchantment_s pierce_effect[] = {OfDefence, OfDexterity, OfPrecision, OfSpeed};
static enchantment_s armor_effect[] = {OfDefence, OfArmor, OfCharisma, OfAcidResistance, OfColdResistance, OfElectricityResistance, OfFireResistance, OfPoisonResistance, OfWaterproof};
static enchantment_s helm_effect[] = {OfIntellegence, OfWisdow, OfCharisma};
static enchantment_s bracers_effect[] = {OfDefence, OfArmor, OfStrenght, OfDexterity};
static enchantment_s ring_effect[] = {OfStrenght, OfDexterity, OfConstitution, OfIntellegence, OfWisdow, OfCharisma,
OfPrecision, OfDefence, OfArmor, OfRegeneration, OfMana,
OfAcidResistance, OfColdResistance, OfElectricityResistance, OfFireResistance, OfPoisonResistance, OfWaterproof};
static spell_s scroll_spells[] = {CharmPerson, Identify, Armor, ShieldSpell};
static spell_s wand_spells[] = {Fear, MagicMissile, HealingSpell, ShokingGrasp, Sleep, RemovePoisonSpell, RemoveSickSpell};
static spell_s staff_spells[] = {Fear, MagicMissile, ShokingGrasp, Sleep};
static state_s potion_red[] = {Anger, Blessed, Goodwill, Dexterious, Healthy, Intellegenced, Charismatic};
static state_s potion_blue[] = {Blessed, Goodwill, Hiding, Sleeped, HealState, RemovePoison, RemoveSick, Healthy, Wisdowed};
static state_s potion_green[] = {Poisoned, PoisonedWeak, PoisonedStrong, Sleeped, Sick, RemovePoison, RemoveSick, Strenghted, Healthy};
static struct item_info {
	struct combat_info {
		char			speed;
		damageinfo		damage;
		char			attack; // Melee or ranger attack bonus
		char			armor[2]; // Bonus to hit and damage reduction
	};
	const char*			name;
	int					weight;
	int					cost;
	material_s			material;
	combat_info			combat;
	cflags<item_flag_s>	flags;
	cflags<slot_s>		slots;
	skill_s				focus;
	aref<enchantment_s>		effects;
	aref<spell_s>		spells;
	item_s				ammunition;
	unsigned char		count;
	unsigned char		charges;
	aref<state_s>		states;
	foodinfo			food;
} item_data[] = {{"Пусто"},
{"Боевой топор", 850, 5 * GP, Iron, {1, {1, 8, Slashing}}, {Versatile}, {Melee}, WeaponFocusAxes, axe_effect},
{"Дубина", 1000, 5 * CP, Wood, {2, {1, 6}}, {}, {Melee}, NoSkill, bludgeon_effect},
{"Кинжал", 50, 2 * GP, Iron, {3, {1, 4, Piercing}, 1}, {}, {Melee, OffHand}, WeaponFocusBlades, swords_effect},
{"Молот", 800, 2 * GP, Wood, {1, {2, 5}}, {}, {Melee}, WeaponFocusAxes, bludgeon_effect},
{"Булава", 700, 8 * GP, Iron, {1, {1, 7}}, {}, {Melee}, WeaponFocusAxes, bludgeon_effect},
{"Копье", 250, 8 * SP, Wood, {1, {1, 8, Piercing}}, {Versatile}, {Melee}, NoSkill, pierce_effect},
{"Посох", 200, 1 * SP, Wood, {2, {1, 6}}, {TwoHanded}, {Melee}, NoSkill, {}, staff_spells, NoItem, 0, 50},
{"Длинный меч", 200, 15 * GP, Iron, {1, {1, 8, Slashing}}, {}, {Melee}, WeaponFocusBlades, swords_effect},
{"Короткий меч", 150, 10 * GP, Iron, {1, {1, 6, Slashing}}, {}, {Melee, OffHand}, WeaponFocusBlades, swords_effect},
{"Двуручный меч", 1500, 50 * GP, Iron, {0, {2, 12, Slashing}}, {TwoHanded}, {Melee}, WeaponFocusBlades, swords_effect},
{"Арбалет", 700, 40 * GP, Wood, {0, {2, 7, Piercing}, 1}, {}, {Ranged}, NoSkill, pierce_effect, {}, Bolt},
{"Тяжелый арбалет", 1200, 80 * GP, Wood, {0, {3, 9, Piercing}, 1}, {}, {Ranged}, NoSkill, pierce_effect, {}, Bolt},
{"Длинный лук", 500, 60 * GP, Wood, {1, {1, 8, Piercing}}, {}, {Ranged}, WeaponFocusBows, pierce_effect, {}, Arrow},
{"Лук", 300, 30 * GP, Wood, {2, {1, 6, Piercing}}, {}, {Ranged}, WeaponFocusBows, pierce_effect, {}, Arrow},
{"Дротик", 30, 1 * SP, Wood, {3, {1, 3, Piercing}}, {}, {Ranged}},
{"Пращя", 50, 1 * SP, Leather, {2, {1, 4}}, {}, {Ranged}, NoSkill, pierce_effect, {}, Rock},
//
{"Камень", 20, 0, Stone, {}, {}, {Amunitions}, NoSkill, {}, {}, NoItem, 20},
{"Стрела", 2, 2 * CP, Wood, {}, {}, {Amunitions}, NoSkill, {}, {}, NoItem, 20},
{"Болт", 3, 1 * CP, Iron, {}, {}, {Amunitions}, NoSkill, {}, {}, NoItem, 20},
//
{"Кожанная броня", 1000, 5 * GP, Leather, {0, {}, 0, {2}}, {}, {Torso}, NoSkill, armor_effect},
{"Клепанная броня", 1500, 15 * GP, Leather, {0, {}, 0, {3}}, {}, {Torso}, NoSkill, armor_effect},
{"Чешуйчатый доспех", 2500, 30 * GP, Iron, {0, {}, 0, {5}}, {}, {Torso}, NoSkill, armor_effect},
{"Кольчуга", 2600, 50 * GP, Iron, {0, {}, 0, {5, 1}}, {}, {Torso}, NoSkill, armor_effect},
{"Бахрец", 3000, 200 * GP, Iron, {0, {}, 0, {6, 2}}, {}, {Torso}, NoSkill, armor_effect},
{"Латы", 3500, 800 * GP, Iron, {0, {}, 0, {8, 3}}, {}, {Torso}, NoSkill, armor_effect},
//
{"Щит", 1500, 20 * GP, Iron, {0, {}, {}, {2}}, {}, {OffHand}, NoSkill, armor_effect},
{"Шлем", 300, 5 * GP, Iron, {0, {}, {}, {1}}, {}, {Head}, NoSkill, helm_effect},
{"Наручи", 200, 3 * GP, Leather, {0, {}, {}, {1}}, {}, {Elbows}, NoSkill, bracers_effect},
{"Наручи", 400, 8 * GP, Iron, {0, {}, {}, {1, 1}}, {}, {Elbows}, NoSkill, bracers_effect},
//
{"Сухпаек", 100, 3 * SP, Organic, {}, {}, {}, NoSkill, {}, {}, NoItem, 0, 0, {}, {5, 0, {1, 0, 1, 0, 0, 0}, 10 * Minute}},
{"Яблоко", 10, 1 * SP, Organic, {}, {}, {}, NoSkill, {}, {}, NoItem, 0, 0, {}, {1, 0, {2, 0, 0, 0, 0, 0}, 2 * Minute}},
{"Хлеб хоббитов", 20, 5 * SP, Organic, {}, {}, {}, NoSkill, {}, {}, NoItem, 0, 0, {}, {3, 0, {0, 3, 2, 0, 0, 0}, 4 * Minute, 10}},
{"Хлеб эльфов", 20, 10 * SP, Organic, {}, {}, {}, NoSkill, {}, {}, NoItem, 0, 0, {}, {4, 0, {0, 4, 0, 1, 0, 0}, 5 * Minute, 20}},
{"Хлеб гномов", 25, 2 * SP, Organic, {}, {}, {}, NoSkill, {}, {}, NoItem, 0, 0, {}, {2, 0, {1, 0, 4, 0, 0, 0}, 5 * Minute, 0, 10}},
{"Печенье", 10, 1 * SP, Organic, {}, {}, {}, NoSkill, {}, {}, NoItem, 0, 0, {}, {1, 0, {0, 0, 0, 1, 0, 1}, Minute}},
{"Колбаса", 40, 8 * SP, Organic, {}, {}, {}, NoSkill, {}, {}, NoItem, 0, 0, {}, {4, 0, {1, 1, 1, 0, 0, 0}, 5 * Minute}},
{"Мясо", 50, 5 * SP, Organic, {}, {}, {}, NoSkill, {}, {}, NoItem, 0, 0, {}, {2, 0, {2, 0, 0, 0, 0, 0}, 3 * Minute}},
//
{"Свиток", 10, 5 * GP, Paper, {}, {}, {}, NoSkill, {}, scroll_spells},
{"Свиток", 10, 6 * GP, Paper, {}, {}, {}, NoSkill, {}, scroll_spells},
{"Свиток", 10, 7 * GP, Paper, {}, {}, {}, NoSkill, {}, scroll_spells},
//
{"Палочка", 5, 50 * GP, Wood, {}, {}, {}, NoSkill, {}, wand_spells, NoItem, 0, 20},
{"Палочка", 5, 70 * GP, Wood, {}, {}, {}, NoSkill, {}, wand_spells, NoItem, 0, 30},
{"Палочка", 5, 90 * GP, Iron, {}, {}, {}, NoSkill, {}, wand_spells, NoItem, 0, 40},
//
{"Книга", 300, 0, Paper},
//
{"Зелье", 40, 20 * GP, Glass, {}, {}, {}, NoSkill, {}, {}, NoItem, 0, 0, potion_red},
{"Зелье", 40, 25 * GP, Glass, {}, {}, {}, NoSkill, {}, {}, NoItem, 0, 0, potion_green},
{"Зелье", 40, 30 * GP, Glass, {}, {}, {}, NoSkill, {}, {}, NoItem, 0, 0, potion_blue},
//
{"Кольцо", 2, 60 * GP, Iron, {}, {}, {}, NoSkill, ring_effect},
{"Кольцо", 2, 70 * GP, Iron, {}, {}, {}, NoSkill, ring_effect},
{"Кольцо", 2, 80 * GP, Iron, {}, {}, {}, NoSkill, ring_effect},
//
{"Ключ", 10, 0, Iron},
{"Монета", 1, 1, Iron, {}, {}, {}, NoSkill, {}, {}, NoItem, 50},
//
{"Когти", 0, 0, Organic, {4, {1, 3, Slashing}}},
{"Удар", 0, 0, Organic, {0, {2, 7}}},
{"Укус", 0, 0, Organic, {2, {2, 5, Piercing}}},
};
assert_enum(item, Bite);
getstr_enum(item);

item::item(item_s type, int chance_artifact, int chance_magic, int chance_cursed, int chance_quality = 30) :
	type(type), count(0), forsale(0), damaged(0), effect(NoEffect), identify(Unknown) {
	// Chance item to be magical
	if(d100() < chance_artifact)
		magic = Artifact;
	else if(d100() < chance_magic)
		magic = Magical;
	else if(d100() < chance_cursed)
		magic = Cursed;
	else
		magic = Mundane;
	// Quality depend on level
	auto r = d100();
	auto m = 100 - chance_quality;
	if(r < m)
		quality = 0;
	else if(r < m + chance_quality / 2)
		quality = 1;
	else if(r < m + (chance_quality * 3) / 4)
		quality = 2;
	else
		quality = 3;
	// Effect can be or not can be
	if(item_data[type].effects) {
		if(magic != Mundane)
			effect = item_data[type].effects.data[rand() % item_data[type].effects.count];
	}
	// Spell be on mostly any scroll or wand
	if(item_data[type].spells) {
		if((is(Melee) && magic != Mundane) || !is(Melee))
			effect = (enchantment_s)item_data[type].spells.data[rand() % item_data[type].spells.count];
	}
	// Spell be on mostly any scroll or wand
	if(item_data[type].states)
		effect = (enchantment_s)item_data[type].states.data[rand() % item_data[type].states.count];
	// Set maximum item count in set
	if(iscountable())
		setcount(item_data[type].count);
	// Set random charge
	if(ischargeable())
		setcharges(xrand(item_data[type].charges / 2, item_data[type].charges));
}

void item::clear() {
	*((int*)this) = 0;
}

void item::set(identify_s level) {
	if(identify < level)
		identify = level;
}

void item::loot() {
	identify = Unknown;
	forsale = 0;
}

void item::get(attackinfo& e) const {
	auto b = getquality();
	e.bonus += item_data[type].combat.attack + b + getbonus(OfPrecision) - damaged;
	e.speed += item_data[type].combat.speed + getbonus(OfSpeed);
	e.damage = item_data[type].combat.damage;
	e.critical += getbonus(OfSmashing);
	e.multiplier += getbonus(OfSmashing);
	if(e.damage.type = Bludgeon) {
		e.damage.min += b / 2;
		e.damage.max += b / 2;
	} else
		e.damage.max += b / 2;
	if(e.damage.type = Slashing)
		e.critical++;
	else if(e.damage.type = Piercing)
		e.multiplier++;
	if(e.damage.max < e.damage.min)
		e.damage.max = e.damage.min;
}

int item::getquality() const {
	switch(magic) {
	case Cursed: return -(quality + 1);
	case Magical: return quality + 1;
	case Artifact: return quality + 2;
	default: return quality;
	}
}

spell_s item::getspell() const {
	if(item_data[type].spells.count)
		return (spell_s)effect;
	return NoSpell;
}

state_s item::getstate() const {
	if(item_data[type].states.count)
		return (state_s)effect;
	return NoState;
}

enchantment_s item::geteffect() const {
	if(item_data[type].effects.count)
		return effect;
	return NoEffect;
}

int item::getbonus(enchantment_s value) const {
	return (geteffect() == value) ? getquality() : 0;
}

unsigned item::getcost() const {
	auto result = item_data[gettype()].cost;
	auto effect = geteffect();
	if(effect)
		result += enchantment_data[effect].cost * 10 * GP;
	if(result < 0)
		result = 1;
	return result;
}

int item::getsalecost() const {
	return getcost() / 3;
}

bool item::iscountable() const {
	return item_data[type].count != 0;
}

int item::getcount() const {
	if(iscountable())
		return 1 + count;
	return 1;
}

void item::setcount(int value) {
	if(!value)
		clear();
	else if(iscountable())
		count = value - 1;
}

bool item::ischargeable() const {
	return item_data[type].charges != 0;
}

int item::getcharges() const {
	if(ischargeable())
		return count;
	return 0;
}

void item::setcharges(int value) {
	if(ischargeable())
		count = value;
}

int item::getweight() const {
	return getweightsingle() * getcount();
}

bool item::is(slot_s value) const {
	return item_data[type].slots.is(value);
}

bool item::is(item_flag_s value) const {
	return item_data[type].flags.is(value);
}

bool item::istwohanded() const {
	return item_data[type].flags.is(TwoHanded);
}

bool item::isarmor() const {
	return item_data[type].combat.armor[0] != 0;
}

bool item::isdrinkable() const {
	return item_data[type].states.count != 0;
}

bool item::isreadable() const {
	switch(type) {
	case ScrollRed:
	case ScrollBlue:
	case ScrollGreen:
	case Book:
		return true;
	default:
		return false;
	}
}

const foodinfo& item::getfood() const {
	return item_data[type].food;
}

bool item::isedible() const {
	return item_data[type].food.hits != 0;
}

int item::getarmor() const {
	return item_data[type].combat.armor[1] + getbonus(OfArmor);
}

int item::getdefence() const {
	auto result = item_data[type].combat.armor[0];
	if(result)
		result += getquality();
	return result - damaged + getbonus(OfDefence);
}

int	item::getweightsingle() const {
	return item_data[type].weight;
}

static void szblock(stringcreator& sc, char* p, const char* pm, const char* format, ...) {
	sc.prints(zend(p), pm, p[0] ? " " : " (");
	sc.printv(zend(p), pm, format, xva_start(format));
}

char* item::getname(char* result, const char* result_maximum, bool show_info) const {
	stringcreator sc;
	auto bonus = getquality();
	auto effect = geteffect();
	auto spell = getspell();
	auto state = getstate();
	auto identify = getidentify();
	sc.prints(result, result_maximum, item_data[type].name);
	if(effect && identify >= KnowEffect) {
		sc.prints(zend(result), result_maximum, " %1%", getstr(effect));
		if(forsale)
			sc.prints(zend(result), result_maximum, "%+1i", bonus);
	}
	else if(spell && identify >= KnowQuality)
		sc.prints(zend(result), result_maximum, " %1", getname(spell));
	else if(state && identify >= KnowQuality)
		sc.prints(zend(result), result_maximum, " %1", getname(state));
	if(show_info) {
		auto p = zend(result);
		if(identify >= KnowQuality) {
			if(is(Melee) || is(Ranged)) {
				attackinfo e = {0}; get(e);
				szblock(sc, p, result_maximum, "урон %2i-%3i", e.bonus, e.damage.min, e.damage.max);
			} else if(isarmor())
				szblock(sc, p, result_maximum, "защита %1i/%2i", getdefence(), getarmor());
		}
		if(forsale)
			szblock(sc, p, result_maximum, "цена %1i", getcost());
		if(p[0])
			sc.prints(zend(p), result_maximum, ")");
	}
	if(getcount() > 2)
		sc.prints(zend(result), result_maximum, " %1i шт", getcount());
	return result;
}

skill_s item::getfocus() const {
	return item_data[type].focus;
}

aref<item_s> item::getitems(aref<item_s> result, aref<slot_s> source) {
	auto pb = result.begin();
	auto pe = result.end();
	for(auto type = NoItem; type < ManyItems; type = (item_s)(type + 1)) {
		item it(type, Mundane, NoEffect, 0);
		bool valid = false;
		for(auto slot : source) {
			if(it.is(slot)) {
				valid = true;
				break;
			}
		}
		if(valid) {
			if(pb < pe)
				*pb++ = type;
		}
	}
	return aref<item_s>(result.data, pb - result.data);
}

item_s item::getammo() const {
	return item_data[type].ammunition;
}

material_s item::getmaterial() const {
	return item_data[type].material;
}

void item::damage() {
	if(damaged < 3)
		damaged++;
}

void item::act(const char* format, ...) const {
	char temp[260];
	logs::driver driver;
	driver.name = getname(temp, zendof(temp), false);
	logs::addv(driver, format, xva_start(format));
}