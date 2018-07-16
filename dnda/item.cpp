#include "main.h"

static_assert(sizeof(item) == sizeof(int), "Invalid sizeof(item). Must be equal sizeof(int).");

enum item_flag_s : unsigned char {
	TwoHanded, Versatile, Readable, Tome, Natural,
};

struct effectlist {
	enum variant_s : unsigned char {
		NoVariant,
		Actions, Skills, States, Echantments, Spells,
	};
	variant_s				type;
	union {
		aref<skill_s>		skills;
		aref<enchantment_s>	effects;
		aref<state_s>		states;
		aref<spell_s>		spells;
		const action*		actions;
	};
	constexpr effectlist() : type(NoVariant), skills() {}
	template<unsigned N> constexpr effectlist(spell_s(&data)[N]) : type(Spells), spells({data, N}) {}
	template<unsigned N> constexpr effectlist(state_s(&data)[N]) : type(States), states({data, N}) {}
	template<unsigned N> constexpr effectlist(skill_s(&data)[N]) : type(Skills), skills({data, N}) {}
	template<unsigned N> constexpr effectlist(enchantment_s(&data)[N]) : type(Echantments), effects({data, N}) {}
	constexpr effectlist(const action* data) : type(Echantments), actions(data) {}
};

static struct enchantment_info {
	const char*	id;
	const char*	name;
	const char*	name_cursed;
	char		cost;
} enchantment_data[] = {{"", ""},
{"armor", "брони", "мягкости", 10},
{"charisma", "харизмы", "уродства", 5},
{"cold", "холода", "холода", 5},
{"constitution", "телосложения", "болезненности", 5},
{"defence", "защиты", "притягивания ударов", 7},
{"dexterity", "ловкости", "неуклюжести", 6},
{"holiness", "святости", "скверны", 8},
{"fire", "огня", "огня", 6},
{"intellegene", "интеллекта", "глупости", 6},
{"mana", "маны", "проклятия маны", 9},
{"arrow deflection", "отражения стрел", "притяжения стрел", 6},
{"orc slying", "убийства орков", "любви к оркам", 6},
{"paralize", "паралича", "паралича", 8},
{"poison", "отравления", "отравления", 6},
{"precision", "точности", "косоглазия", 7},
{"regeneration", "регенерации", "увядания", 10},
{"sickness", "болезни", "тошноты", 5},
{"sharping", "остроты", "тупости", 5},
{"smashing", "раскалывания", "заботы", 5},
{"speed", "скорости", "замедления", 3},
{"strenght", "силы", "слабости", 8},
{"sustenance", "питания", "пьявки", 4},
{"vampirism", "вампиризма", "лечения врага", 10},
{"weakness", "ослабления", "ослабления", 6},
{"wisdow", "мудрости", "беспечности", 6},
//
{"acid resistance", "сопротивления кислоте", "уязвимости к кислоте", 1},
{"charm resistance", "сопротивления шарму", "уязвимости к шарму", 1},
{"cold resistance", "сопротивления холоду", "уязвимости к холоду", 1},
{"fire resistance", "сопротивления огня", "уязвимости к огню", 2},
{"paralize resistance", "сопротивления параличу", "уязвимости к параличу", 1},
{"electricity resistance", "сопротивления молнии", "уязвимости к молнии", 1},
{"poison resistance", "сопротивления яду", "уязвимости к яду", 1},
{"waterproof", "водонепроницаемости", "уязвимости к воде", 1},
};
assert_enum(enchantment, LastEnchantment);
getstr_enum(enchantment);

static const char* item_adv[][4] = {{"", "", "", ""},
{"проклятое", "проклятый", "проклятая", "проклятые"},
{"благословенное", "благословенный", "благословенная", "благословенные"},
{"реликт", "реликт", "реликт", "реликт"},
};
static enchantment_s bite_effect[] = {OfPoison, OfWeakness};
static enchantment_s swords_effect[] = {OfCold, OfDefence, OfDexterity, OfFire, OfSpeed, OfPrecision, OfSharping, OfVampirism, OfWeakness};
static enchantment_s axe_effect[] = {OfCold, OfFire, OfStrenght, OfSharping, OfSmashing};
static enchantment_s bludgeon_effect[] = {OfConstitution, OfFire, OfSmashing, OfStrenght};
static enchantment_s pierce_effect[] = {OfDefence, OfDexterity, OfPoison, OfPrecision, OfSickness, OfSpeed, OfWeakness};
static enchantment_s armor_effect[] = {OfDefence, OfArmor, OfCharisma, OfAcidResistance, OfColdResistance, OfElectricityResistance, OfFireResistance, OfPoisonResistance, OfWaterproof};
static enchantment_s shield_effect[] = {OfDefence, OfArmor, OfAcidResistance, OfColdResistance, OfElectricityResistance, OfFireResistance, OfPoisonResistance, OfWaterproof, OfMissileDeflection, OfMissileDeflection};
static enchantment_s helm_effect[] = {OfIntellegence, OfWisdow, OfCharisma};
static enchantment_s bracers_effect[] = {OfDefence, OfArmor, OfStrenght, OfDexterity, OfMissileDeflection};
static enchantment_s cloack_effect[] = {OfDefence, OfArmor, OfIntellegence, OfFireResistance, OfColdResistance, OfMissileDeflection};
static enchantment_s ring_effect[] = {OfStrenght, OfDexterity, OfConstitution, OfIntellegence, OfWisdow, OfCharisma,
OfPrecision, OfDefence, OfArmor, OfRegeneration, OfMana,
OfAcidResistance, OfColdResistance, OfElectricityResistance, OfFireResistance, OfPoisonResistance, OfWaterproof};
static spell_s scroll_spells[] = {CharmPerson, Identify, Armor, ShieldSpell};
static spell_s book_spells[] = {Bless, Armor, Fear, MagicMissile, HealingSpell, ShokingGrasp, Sleep, RemovePoisonSpell, RemoveSickSpell};
static spell_s wand_spells[] = {Fear, MagicMissile, HealingSpell, ShokingGrasp, Sleep, RemovePoisonSpell, RemoveSickSpell};
static spell_s staff_spells[] = {Fear, MagicMissile, ShokingGrasp, Sleep};
static state_s potion_state[] = {Anger, Armored, Blessed, Charmed, Hiding, Goodwill,
Lighted, PoisonedWeak, Poisoned, PoisonedStrong,
Shielded, Sick, Scared, Sleeped, Weaken,
Strenghted, Dexterious, Healthy, Intellegenced, Wisdowed, Charismatic,
Experience,
RestoreHits, RestoreHits, RestoreHits,
RestoreMana, RestoreMana,
RemoveSick, RemovePoison};
static skill_s manual_skills[] = {Athletics, Acrobatics, Bargaining, Bluff, Diplomacy,
Acrobatics, Alertness, Athletics, Concetration, DisarmTraps, HearNoises, HideInShadow, Lockpicking, PickPockets,
Alchemy, Dancing, Engineering, Gambling, History, Healing, Literacy, Mining, Smithing, Survival, Swimming,
WeaponFocusBows, WeaponFocusBlades, WeaponFocusAxes, TwoWeaponFighting};
static struct item_info {
	struct combatinfo {
		char			speed;
		damageinfo		damage;
		char			bonus; // Melee or ranger attack percent bonus
		char			armor[2]; // Bonus to hit and damage reduction
		char			armor_magic_bonus;
	};
	const char*			name;
	int					weight;
	int					cost;
	gender_s			gender;
	material_s			material;
	combatinfo			combat;
	specialinfo			special;
	cflags<item_flag_s>	flags;
	cflags<slot_s>		slots;
	skill_s				focus;
	effectlist			magic;
	item_s				ammunition;
	unsigned char		count;
	unsigned char		charges;
	foodinfo			food;
} item_data[] = {{"Пусто"},
{"Боевой топор", 850, 5 * GP, Male, Iron, {1, {1, 8, Slashing}}, {5}, {Versatile}, {Melee}, WeaponFocusAxes, axe_effect},
{"Дубина", 1000, 5 * CP, Female, Wood, {2, {1, 6}}, {4}, {}, {Melee}, NoSkill, bludgeon_effect},
{"Кинжал", 50, 2 * GP, Male, Iron, {3, {1, 4, Piercing}, 5}, {5}, {}, {Melee, OffHand}, WeaponFocusBlades, swords_effect},
{"Молот", 800, 2 * GP, Male, Wood, {1, {2, 5}}, {5}, {}, {Melee}, WeaponFocusAxes, bludgeon_effect},
{"Булава", 700, 8 * GP, Female, Iron, {1, {1, 7}}, {5}, {}, {Melee}, WeaponFocusAxes, bludgeon_effect},
{"Копье", 250, 8 * SP, NoGender, Wood, {1, {1, 8, Piercing}}, {5}, {Versatile}, {Melee}, NoSkill, pierce_effect},
{"Посох", 200, 1 * SP, Male, Wood, {2, {1, 6}}, {6}, {TwoHanded}, {Melee}, NoSkill, staff_spells, NoItem, 0, 50},
{"Длинный меч", 200, 15 * GP, Male, Iron, {1, {1, 8, Slashing}}, {6}, {}, {Melee}, WeaponFocusBlades, swords_effect},
{"Короткий меч", 150, 10 * GP, Male, Iron, {1, {1, 6, Slashing}}, {6}, {}, {Melee, OffHand}, WeaponFocusBlades, swords_effect},
{"Двуручный меч", 1500, 50 * GP, Male, Iron, {0, {2, 12, Slashing}}, {6}, {TwoHanded}, {Melee}, WeaponFocusBlades, swords_effect},
{"Арбалет", 700, 40 * GP, Male, Wood, {0, {2, 7, Piercing}, 1}, {5}, {}, {Ranged}, NoSkill, pierce_effect, Bolt},
{"Тяжелый арбалет", 1200, 80 * GP, Male, Wood, {0, {3, 9, Piercing}, 5}, {5}, {}, {Ranged}, NoSkill, pierce_effect, Bolt},
{"Длинный лук", 500, 60 * GP, Male, Wood, {1, {1, 8, Piercing}}, {5}, {}, {Ranged}, WeaponFocusBows, pierce_effect, Arrow},
{"Лук", 300, 30 * GP, Male, Wood, {2, {1, 6, Piercing}}, {5}, {}, {Ranged}, WeaponFocusBows, pierce_effect, Arrow},
{"Дротик", 30, 1 * SP, Male, Wood, {3, {1, 3, Piercing}}, {5}, {}, {Ranged}},
{"Пращя", 50, 1 * SP, Female, Leather, {2, {1, 4}}, {5}, {}, {Ranged}, NoSkill, pierce_effect, Rock},
//
{"Камень", 20, 0, Male, Stone, {1, {1, 3}}, {}, {}, {Amunitions, Ranged}, NoSkill, {}, NoItem, 20},
{"Стрела", 2, 2 * CP, Female, Wood, {}, {}, {}, {Amunitions}, NoSkill, {}, NoItem, 20},
{"Болт", 3, 1 * CP, Male, Iron, {}, {}, {}, {Amunitions}, NoSkill, {}, NoItem, 20},
//
{"Кожанная броня", 1000, 5 * GP, Female, Leather, {0, {}, 0, {10}, 4}, {}, {}, {Torso}, NoSkill, armor_effect},
{"Клепанная броня", 1500, 15 * GP, Female, Leather, {0, {}, 0, {15}, 4}, {}, {}, {Torso}, NoSkill, armor_effect},
{"Чешуйчатый доспех", 2500, 30 * GP, Male, Iron, {0, {}, 0, {25}, 5}, {}, {}, {Torso}, NoSkill, armor_effect},
{"Кольчуга", 2600, 50 * GP, Female, Iron, {0, {}, 0, {25, 1}, 5}, {}, {}, {Torso}, NoSkill, armor_effect},
{"Бахрец", 3000, 200 * GP, Male, Iron, {0, {}, 0, {30, 2}, 5}, {}, {}, {Torso}, NoSkill, armor_effect},
{"Латы", 3500, 800 * GP, Female, Iron, {0, {}, 0, {40, 3}, 5}, {}, {}, {Torso}, NoSkill, armor_effect},
//
{"Щит", 1500, 20 * GP, Male, Iron, {0, {}, {}, {12}, 5}, {}, {}, {OffHand}, NoSkill, shield_effect},
{"Шлем", 300, 5 * GP, Male, Iron, {0, {}, {}, {3}, 2}, {}, {}, {Head}, NoSkill, helm_effect},
{"Наручи", 200, 3 * GP, They, Leather, {0, {}, {}, {3}, 2}, {}, {}, {Elbows}, NoSkill, bracers_effect},
{"Наручи", 400, 8 * GP, They, Iron, {0, {}, {}, {5, 1}, 2}, {}, {}, {Elbows}, NoSkill, bracers_effect},
//
{"Плащ", 200, 5 * GP, Male, Leather, {0, {}, {}, {1, 0}, 1}, {}, {}, {TorsoBack}, NoSkill, cloack_effect},
{"Плащ", 200, 6 * GP, Male, Leather, {0, {}, {}, {2, 0}, 1}, {}, {}, {TorsoBack}, NoSkill, cloack_effect},
{"Плащ", 200, 5 * GP, Male, Leather, {0, {}, {}, {1, 0}, 2}, {}, {}, {TorsoBack}, NoSkill, cloack_effect},
{"Плащ", 200, 5 * GP, Male, Leather, {0, {}, {}, {1, 0}, 1}, {}, {}, {TorsoBack}, NoSkill, cloack_effect},
{"Плащ", 200, 6 * GP, Male, Leather, {0, {}, {}, {2, 0}, 1}, {}, {}, {TorsoBack}, NoSkill, cloack_effect},
//
{"Сапоги", 300, 6 * GP, They, Leather, {0, {}, {}, {1, 0}, 1}, {}, {}, {Legs}, NoSkill},
{"Сапоги", 300, 6 * GP, They, Leather, {0, {}, {}, {2, 0}, 1}, {}, {}, {Legs}, NoSkill},
{"Сапоги", 350, 6 * GP, They, Leather, {0, {}, {}, {3, 0}, 1}, {}, {}, {Legs}, NoSkill},
{"Сапоги", 400, 6 * GP, They, Leather, {0, {}, {}, {4, 0}, 1}, {}, {}, {Legs}, NoSkill},
{"Сапоги", 450, 6 * GP, They, Leather, {0, {}, {}, {4, 0}, 2}, {}, {}, {Legs}, NoSkill},
//
{"Сухпаек", 100, 3 * SP, Male, Organic, {}, {}, {}, {}, NoSkill, {}, NoItem, 0, 0, {5, 0, {1, 0, 1, 0, 0, 0}, 10 * Minute}},
{"Яблоко", 10, 1 * SP, NoGender, Organic, {}, {}, {}, {}, NoSkill, {}, NoItem, 0, 0, {1, 0, {2, 0, 0, 0, 0, 0}, 2 * Minute}},
{"Хлеб хоббитов", 20, 5 * SP, Male, Organic, {}, {}, {}, {}, NoSkill, {}, NoItem, 0, 0, {3, 0, {0, 3, 2, 0, 0, 0}, 4 * Minute, 10}},
{"Хлеб эльфов", 20, 10 * SP, Male, Organic, {}, {}, {}, {}, NoSkill, {}, NoItem, 0, 0, {4, 0, {0, 4, 0, 1, 0, 0}, 5 * Minute, 20}},
{"Хлеб гномов", 25, 2 * SP, Male, Organic, {}, {}, {}, {}, NoSkill, {}, NoItem, 0, 0, {2, 0, {1, 0, 4, 0, 0, 0}, 5 * Minute, 0, 10}},
{"Печенье", 10, 1 * SP, Male, Organic, {}, {}, {}, {}, NoSkill, {}, NoItem, 0, 0, {1, 0, {0, 0, 0, 1, 0, 1}, Minute}},
{"Колбаса", 40, 8 * SP, Female, Organic, {}, {}, {}, {}, NoSkill, {}, NoItem, 0, 0, {4, 0, {1, 1, 1, 0, 0, 0}, 5 * Minute}},
{"Мясо", 50, 5 * SP, NoGender, Organic, {}, {}, {}, {}, NoSkill, {}, NoItem, 0, 0, {2, 0, {2, 0, 0, 0, 0, 0}, 3 * Minute}},
//
{"Свиток", 10, 5 * GP, Male, Paper, {}, {15, 5}, {Readable}, {}, NoSkill, scroll_spells},
{"Свиток", 10, 6 * GP, Male, Paper, {}, {10, 10}, {Readable}, {}, NoSkill, scroll_spells},
{"Свиток", 10, 7 * GP, Male, Paper, {}, {5, 10}, {Readable}, {}, NoSkill, scroll_spells},
//
{"Палочка", 5, 50 * GP, Female, Wood, {}, {}, {}, {}, NoSkill, wand_spells, NoItem, 0, 20},
{"Палочка", 5, 70 * GP, Female, Wood, {}, {}, {}, {}, NoSkill, wand_spells, NoItem, 0, 30},
{"Палочка", 5, 90 * GP, Female, Iron, {}, {}, {}, {}, NoSkill, wand_spells, NoItem, 0, 40},
//
{"Книга", 300, 80 * GP, Female, Paper, {}, {80, -20, 65}, {Readable, Tome}, {}, NoSkill, wand_spells},
{"Мануал", 350, 100 * GP, Male, Paper, {}, {55, -15, 50}, {Readable, Tome}, {}, NoSkill, manual_skills},
{"Книга", 300, 110 * GP, Female, Paper, {}, {70, -10, 55}, {Readable, Tome}, {}, NoSkill, wand_spells},
{"Книга", 300, 90 * GP, Female, Paper, {}, {75, -15, 50}, {Readable, Tome}, {}, NoSkill, wand_spells},
{"Том", 300, 130 * GP, Male, Paper, {}, {60, -15, 35}, {Readable, Tome}, {}, NoSkill, wand_spells},
//
{"Зелье", 40, 20 * GP, NoGender, Glass, {}, {}, {}, {}, NoSkill, potion_state, NoItem},
{"Зелье", 40, 25 * GP, NoGender, Glass, {}, {}, {}, {}, NoSkill, potion_state, NoItem},
{"Зелье", 40, 30 * GP, NoGender, Glass, {}, {}, {}, {}, NoSkill, potion_state, NoItem},
//
{"Кольцо", 2, 60 * GP, NoGender, Iron, {}, {}, {}, {}, NoSkill, ring_effect},
{"Кольцо", 2, 70 * GP, NoGender, Iron, {}, {}, {}, {}, NoSkill, ring_effect},
{"Кольцо", 2, 80 * GP, NoGender, Iron, {}, {}, {}, {}, NoSkill, ring_effect},
//
{"Амулет", 2, 50 * GP, Male, Iron, {}, {}, {}, {}, NoSkill, ring_effect},
{"Амулет", 2, 50 * GP, Male, Iron, {}, {}, {}, {}, NoSkill, ring_effect},
{"Амулет", 2, 50 * GP, Male, Iron, {}, {}, {}, {}, NoSkill, ring_effect},
{"Амулет", 2, 50 * GP, Male, Iron, {}, {}, {}, {}, NoSkill, ring_effect},
{"Амулет", 2, 50 * GP, Male, Iron, {}, {}, {}, {}, NoSkill, ring_effect},
//
{"Ключ", 10, 0, Male, Iron},
{"Монета", 1, 1 * CP, Female, Iron, {}, {}, {}, {}, NoSkill, {}, NoItem, 50},
//
{"Когти", 0, 0, They, Organic, {4, {1, 3, Slashing}}, {}, {Natural}, {Melee}, NoSkill, bite_effect},
{"Удар", 0, 0, Male, Organic, {0, {2, 7}}, {}, {Natural}, {Melee}, NoSkill, bite_effect},
{"Укус", 0, 0, Male, Organic, {2, {2, 5, Piercing}}, {}, {Natural}, {Melee}, NoSkill, bite_effect},
{"Хитин", 0, 0, Male, Organic, {0, {}, 0, {20}, 4}, {}, {Natural}, {Torso}, NoSkill, armor_effect},
{"Мех", 0, 0, Male, Organic, {0, {}, 0, {12}, 4}, {}, {Natural}, {Torso}, NoSkill, armor_effect},
};
assert_enum(item, ManyItems-1);
getstr_enum(item);

item::item(item_s type, int chance_artifact, int chance_magic, int chance_cursed, int chance_quality = 30) :
	type(type), count(0), forsale(0), damaged(0), effect(NoEffect), identify(Unknown) {
	// Chance item to be magical
	if(d100() < chance_artifact)
		magic = Artifact;
	else if(d100() < chance_cursed) {
		if(d100() < 50)
			magic = BlessedItem;
		else
			magic = Cursed;
	} else
		magic = Mundane;
	// Quality depend on level
	auto r = d100();
	if(r < chance_quality / 8)
		quality = 3;
	else if(r < chance_quality / 3)
		quality = 2;
	else if(r < chance_quality)
		quality = 1;
	else
		quality = 0;
	// Several effect types
	if(!isnatural()) {
		switch(item_data[type].magic.type) {
		case effectlist::Echantments:
			if(d100() < chance_magic)
				effect = item_data[type].magic.effects.data[rand() % item_data[type].magic.effects.count];
			break;
		case effectlist::Spells:
			if((is(Melee) && (d100() < chance_magic)) || !is(Melee))
				effect = (enchantment_s)item_data[type].magic.spells.data[rand() % item_data[type].magic.spells.count];
			break;
		case effectlist::States:
			effect = (enchantment_s)item_data[type].magic.states.data[rand() % item_data[type].magic.states.count];
			break;
		case effectlist::Skills:
			effect = (enchantment_s)item_data[type].magic.skills.data[rand() % item_data[type].magic.skills.count];
			break;
		}
	}
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

item& item::set(identify_s level) {
	if(identify < level)
		identify = level;
	return *this;
}

void item::loot() {
	identify = Unknown;
	forsale = 0;
}

void item::get(attackinfo& e) const {
	auto b = getquality();
	e.bonus += item_data[type].combat.bonus + (b + getbonus(OfPrecision)) * 5;
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
	auto e = effect ? 1 : 0;
	switch(magic) {
	case Cursed: return -(quality + 0 + e) - damaged;
	case BlessedItem: return (quality + 1 + e) - damaged;
	case Artifact: return (quality + 2 + e) - damaged;
	default: return (quality + 0 + e) - damaged;
	}
}

spell_s item::getspell() const {
	if(item_data[type].magic.type == effectlist::Spells)
		return (spell_s)effect;
	return NoSpell;
}

skill_s	item::getskill() const {
	if(item_data[type].magic.type == effectlist::Skills)
		return (skill_s)effect;
	return NoSkill;
}

state_s item::getstate() const {
	if(item_data[type].magic.type == effectlist::States)
		return (state_s)effect;
	return NoState;
}

enchantment_s item::geteffect() const {
	if(item_data[type].magic.type == effectlist::Echantments)
		return effect;
	return NoEffect;
}

const specialinfo& item::getspecial() const {
	return item_data[type].special;
}

int item::getbonus(enchantment_s value) const {
	return (geteffect() == value) ? getquality() : 0;
}

char item::getenchantcost() const {
	return enchantment_data[geteffect()].cost;
}

unsigned item::getcost() const {
	auto result = item_data[gettype()].cost;
	auto effect = geteffect();
	result += imin(5000, (result * quality) / 5);
	if(effect)
		result += getenchantcost() * 10 * GP;
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

item& item::setcount(int value) {
	if(!value)
		clear();
	else if(iscountable())
		count = value - 1;
	return *this;
}

bool item::ischargeable() const {
	return item_data[type].charges != 0;
}

int item::getcharges() const {
	if(ischargeable())
		return count;
	return 0;
}

item& item::setcharges(int value) {
	if(ischargeable())
		count = value;
	return *this;
}

int item::getweight() const {
	return getweightsingle() * getcount();
}

bool item::is(slot_s value) const {
	return item_data[type].slots.is(value);
}

bool item::isthrown() const {
	return is(Ranged) && iscountable();
}

bool item::istwohanded() const {
	return item_data[type].flags.is(TwoHanded);
}

bool item::isversatile() const {
	return item_data[type].flags.is(Versatile);
}

bool item::istome() const {
	return item_data[type].flags.is(Tome);
}

bool item::isnatural() const {
	return item_data[type].flags.is(Natural);
}

bool item::isunbreakable() const {
	return magic==Artifact || isnatural();
}

gender_s item::getgender() const {
	return item_data[type].gender;
}

bool item::isarmor() const {
	return item_data[type].combat.armor[0] != 0;
}

bool item::isdrinkable() const {
	return item_data[type].magic.type == effectlist::States;
}

bool item::isreadable() const {
	return item_data[type].flags.is(Readable);
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
	result += getquality() * item_data[type].combat.armor_magic_bonus;
	return result + getbonus(OfDefence) * 5;
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
	auto skill = getskill();
	sc.prints(result, result_maximum, item_data[type].name);
	if(effect && identify >= KnowEffect) {
		sc.prints(zend(result), result_maximum, " %1%", iscursed() ? enchantment_data[effect].name_cursed : enchantment_data[effect].name);
		if(forsale)
			sc.prints(zend(result), result_maximum, "%+1i", bonus);
	} else if(spell && identify >= KnowQuality)
		sc.prints(zend(result), result_maximum, " %1", getname(spell));
	else if(state && identify >= KnowQuality)
		sc.prints(zend(result), result_maximum, " %1", getname(state));
	else if(skill && identify >= KnowQuality)
		sc.prints(zend(result), result_maximum, " %1", creature::getname(skill));
	if(show_info) {
		auto p = zend(result);
		if(identify >= KnowQuality) {
			if(is(Melee) || is(Ranged)) {
				attackinfo e = {0}; get(e);
				szblock(sc, p, result_maximum, "урон %2i-%3i", e.bonus, e.damage.min, e.damage.max);
			} else if(isarmor()) {
				szblock(sc, p, result_maximum, "защита %+1i%%", getdefence());
				if(getarmor())
					sc.prints(zend(p), result_maximum, " и %1i", getarmor());
			}
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

bool item::damageb() {
	if(magic == Artifact)
		return false;
	if(damaged < 3)
		damaged++;
	else if(magic==Mundane)
		return true;
	return false;
}

void item::damage() {
	if(damageb())
		clear();
}

void item::act(const char* format, ...) const {
	char temp[260];
	logs::driver driver;
	driver.name = getname(temp, zendof(temp), false);
	driver.gender = getgender();
	logs::addv(driver, format, xva_start(format));
}

void item::repair(int level) {
	auto new_count = damaged - level;
	if(new_count <= 0)
		damaged = 0;
	else if(new_count > 3)
		clear();
	else
		damaged = new_count;
}

void manual_skill_focus_item(stringbuffer& sc, manual& me) {
	for(auto& e : item_data) {
		if(me.value.skill != e.focus)
			continue;
		sc.header("[Оружие]: ");
		sc.add(e.name);
	}
	sc.trail(".");
}