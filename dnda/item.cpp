#include "main.h"

const int GP = 100;
const int SP = 10;
const int CP = 1;
static_assert(sizeof(item) == sizeof(int), "Invalid sizeof(item). Must be equal sizeof(int).");

static struct enchantment_info {
	const char*		id;
	const char*		name;
} enchantment_data[] = {{""},
{"armor", "�����"},
{"charisma", "�������"},
{"cold", "������"},
{"constitution", "������������"},
{"defence", "������"},
{"destruction", "����������"},
{"dexterity", "��������"},
{"fire", "����"},
{"intellegene", "����������"},
{"mana", "����"},
{"precision", "��������"},
{"regeneration", "�����������"},
{"sharping", "�������"},
{"smashing", "������������"},
{"speed", "��������"},
{"strenght", "����"},
{"vampirism", "����������"},
{"wisdow", "��������"},
//
{"cold resistance", "������������� ������"},
{"fire resistance", "������������� ����"},
{"electricity resistance", "������������� �������������"},
{"poison resistance", "������������� ���"},
};
assert_enum(enchantment, LastEnchantment);
getstr_enum(enchantment);

static const char* key_names[][2] = {{"simple", "�������"},
// ������������� �����
{"bronze", "���������"},
{"cooper", "������"},
{"steel", "��������"},
{"silver", "�����������"},
{"golden", "�������"},
// ������ ����� ��� ������ ����������
{"bone", "��������"},
{"stone", "��������"},
{"crystal", "�����������"},
};
static enchantment_s swords_effect[] = {OfCold, OfDefence, OfDexterity, OfFire, OfSpeed, OfPrecision, OfSharping, OfVampirism};
static enchantment_s axe_effect[] = {OfStrenght, OfDestruction, OfFire, OfSharping, OfSmashing};
static enchantment_s bludgeon_effect[] = {OfConstitution, OfDestruction, OfFire, OfSmashing, OfStrenght};
static enchantment_s pierce_effect[] = {OfDefence, OfDexterity, OfPrecision, OfSpeed};
static spell_s scroll_spells[] = {CharmPerson, Identify, Armor, ShieldSpell};
static spell_s wand_spells[] = {Fear, MagicMissile, HealingSpell, ShokingGrasp, Sleep, RemovePoisonSpell, RemoveSickSpell};
static spell_s staff_spells[] = {Fear, MagicMissile, ShokingGrasp, Sleep};
static state_s potion_red[] = {Anger, Blessed, Goodwill, Dexterious, Healthy, Intellegenced, Charismatic};
static state_s potion_blue[] = {Blessed, Goodwill, Hiding, Sleeped, HealState, RemovePoison, RemoveSick, Healthy, Wisdowed};
static state_s potion_green[] = {Poisoned, PoisonedWeak, PoisonedStrong, Sleeped, Sick, RemovePoison, RemoveSick, Strenghted, Healthy};
static constexpr struct item_info {
	struct combat_info {
		char			speed;
		damageinfo		damage;
		char			attack; // Melee or ranger attack bonus
		char			armor[2]; // Bonus to hit and damage reduction
	};
	const char*			name;
	int					cost;
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
} item_data[] = {{"�����"},
{"������ �����", 5 * GP, {1, {1, 8, Slashing}}, {Versatile}, {Melee}, WeaponFocusAxes, axe_effect},
{"������", 5 * CP, {2, {1, 6}}, {}, {Melee}, NoSkill, bludgeon_effect},
{"������", 2 * GP, {3, {1, 4, Piercing}, 1}, {}, {Melee, OffHand}, WeaponFocusBlades, swords_effect},
{"�����", 2 * GP, {1, {2, 5}}, {}, {Melee}, WeaponFocusAxes, bludgeon_effect},
{"������", 8 * GP, {1, {1, 7}}, {}, {Melee}, WeaponFocusAxes, bludgeon_effect},
{"�����", 8 * SP, {1, {1, 8, Piercing}}, {Versatile}, {Melee}, NoSkill, pierce_effect},
{"�����", 1 * SP, {2, {1, 6}}, {TwoHanded}, {Melee}, NoSkill, {}, staff_spells, NoItem, 0, 50},
{"������� ���", 15 * GP, {1, {1, 8, Slashing}}, {}, {Melee}, WeaponFocusBlades, swords_effect},
{"�������� ���", 10 * GP, {1, {1, 6, Slashing}}, {}, {Melee, OffHand}, WeaponFocusBlades, swords_effect},
{"��������� ���", 50 * GP, {0, {2, 12, Slashing}}, {TwoHanded}, {Melee}, WeaponFocusBlades, swords_effect},
{"�������", 40 * GP, {0, {2, 7, Piercing}, 1}, {}, {Ranged}, NoSkill, pierce_effect, {}, Bolt},
{"������� �������", 80 * GP, {0, {3, 9, Piercing}, 1}, {}, {Ranged}, NoSkill, pierce_effect, {}, Bolt},
{"������� ���", 60 * GP, {1, {1, 8, Piercing}}, {}, {Ranged}, WeaponFocusBows, pierce_effect, {}, Arrow},
{"���", 30 * GP, {2, {1, 6, Piercing}}, {}, {Ranged}, WeaponFocusBows, pierce_effect, {}, Arrow},
{"������", 1 * SP, {3, {1, 3, Piercing}}, {}, {Ranged}},
{"�����", 1 * SP, {2, {1, 4}}, {}, {Ranged}, NoSkill, pierce_effect, {}, Rock},
//
{"������", 0, {}, {}, {Amunitions}, NoSkill, {}, {}, NoItem, 20},
{"������", 2 * CP, {}, {}, {Amunitions}, NoSkill, {}, {}, NoItem, 20},
{"����", 1 * CP, {}, {}, {Amunitions}, NoSkill, {}, {}, NoItem, 20},
//
{"�������� �����", 5 * GP, {0, {}, 0, {2}}, {}, {Torso}},
{"��������� �����", 20 * GP, {0, {}, 0, {3}}, {}, {Torso}},
{"���������� ������", 30 * GP, {0, {}, 0, {5}}, {}, {Torso}},
{"��������", 50 * GP, {0, {}, 0, {5, 1}}, {}, {Torso}},
{"������", 200 * GP, {0, {}, 0, {6, 2}}, {}, {Torso}},
{"����", 800 * GP, {0, {}, 0, {8, 3}}, {}, {Torso}},
//
{"���", 20 * GP, {0, {}, {}, {2}}, {}, {OffHand}},
{"����", 5 * GP, {0, {}, {}, {1}}, {}, {Head}},
{"������", 3 * GP, {0, {}, {}, {1}}, {}, {Elbows}},
//
{"�������", 3 * SP, {}, {}, {}, NoSkill, {}, {}, NoItem, 0, 0, {}, {5, 0, {1, 0, 1, 0, 0, 0}, 10 * Minute}},
{"������", 1 * SP, {}, {}, {}, NoSkill, {}, {}, NoItem, 0, 0, {}, {1, 0, {2, 0, 0, 0, 0, 0}, 2 * Minute}},
{"���� ��������", 5 * SP, {}, {}, {}, NoSkill, {}, {}, NoItem, 0, 0, {}, {3, 0, {0, 3, 2, 0, 0, 0}, 4 * Minute, 10}},
{"���� ������", 10 * SP, {}, {}, {}, NoSkill, {}, {}, NoItem, 0, 0, {}, {4, 0, {0, 4, 0, 1, 0, 0}, 5 * Minute, 20}},
{"���� ������", 2 * SP, {}, {}, {}, NoSkill, {}, {}, NoItem, 0, 0, {}, {2, 0, {1, 0, 4, 0, 0, 0}, 5 * Minute, 0, 10}},
{"�������", 1 * SP, {}, {}, {}, NoSkill, {}, {}, NoItem, 0, 0, {}, {1, 0, {0, 0, 0, 1, 0, 1}, Minute}},
{"�������", 8 * SP, {}, {}, {}, NoSkill, {}, {}, NoItem, 0, 0, {}, {4, 0, {1, 1, 1, 0, 0, 0}, 5 * Minute}},
{"����", 5 * SP, {}, {}, {}, NoSkill, {}, {}, NoItem, 0, 0, {}, {2, 0, {2, 0, 0, 0, 0, 0}, 3 * Minute}},
//
{"������", 5 * GP, {}, {}, {}, NoSkill, {}, scroll_spells},
{"������", 6 * GP, {}, {}, {}, NoSkill, {}, scroll_spells},
{"������", 7 * GP, {}, {}, {}, NoSkill, {}, scroll_spells},
//
{"�������", 50 * GP, {}, {}, {}, NoSkill, {}, wand_spells, NoItem, 0, 20},
{"�������", 70 * GP, {}, {}, {}, NoSkill, {}, wand_spells, NoItem, 0, 30},
{"�������", 90 * GP, {}, {}, {}, NoSkill, {}, wand_spells, NoItem, 0, 40},
//
{"�����"},
//
{"�����", 20 * GP, {}, {}, {}, NoSkill, {}, {}, NoItem, 0, 0, potion_red},
{"�����", 25 * GP, {}, {}, {}, NoSkill, {}, {}, NoItem, 0, 0, potion_green},
{"�����", 30 * GP, {}, {}, {}, NoSkill, {}, {}, NoItem, 0, 0, potion_blue},
//
{"����"},
{"������", 1, {}, {}, {}, NoSkill, {}, {}, NoItem, 50},
//
{"�����", 0, {4, {1, 3, Slashing}}},
{"����", 0, {0, {2, 7}}},
{"����", 0, {2, {2, 5, Piercing}}},
};
assert_enum(item, Bite);
getstr_enum(item);

item::item(item_s type, int level, int chance_curse) : item(type) {
	// Chance item to be magical
	if(d100() < level * 2) {
		if(level > 5 && d100() < (level - 5))
			magic = Artifact; // Random artifact is very rare
		else
			magic = Magical;
	} else if(d100() < chance_curse)
		magic = Cursed;
	else
		magic = Mundane;
	// Quality depend on level
	auto m = imax(20, 70 - level * 2);
	auto r = d100();
	if(r < m)
		quality = 0;
	else if(r < m + m / 2)
		quality = 1;
	else if(r < m + m / 2 + m / 4)
		quality = 2;
	else
		quality = 3;
	// Effect can be or not can be
	if(item_data[type].effects) {
		if(magic == Artifact
			|| (magic != Mundane && d100() < level))
			effect = item_data[type].effects.data[rand() % item_data[type].effects.count];
	}
	// Spell be on mostly any scroll or wand
	if(item_data[type].spells) {
		if(magic == Artifact
			|| (is(Melee) && magic != Mundane && d100() < level)
			|| !is(Melee))
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

unsigned item::getcostsingle() const {
	return item_data[gettype()].cost;
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
	switch(gettype()) {
	case SwordLong: return 200;
	case SwordShort: return 150;
	case Dagger: return 50;
	case Spear: return 250;
	case Staff: return 200;
	case Mace: return 700;
	case AxeBattle: return 850;
	case HammerWar: return 850;
		//
	case LeatherArmour: return 1000;
	case ChainMail: return 2500;
	case ScaleMail: return 2500;
	case PlateMail: return 3500;
	case Helmet: return 250;
		//
	case Arrow: return 3;
	case Coin: return 1;
	case DoorKey: return 10;
	default: return 100;
	}
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
	if(effect && identify >= KnowEffect)
		sc.prints(zend(result), result_maximum, " %1%+2i", getstr(effect), bonus);
	else if(spell && identify >= KnowQuality)
		sc.prints(zend(result), result_maximum, " %1", getname(spell));
	else if(state && identify >= KnowQuality)
		sc.prints(zend(result), result_maximum, " %1", getname(state));
	if(show_info) {
		auto p = zend(result);
		if(identify >= KnowQuality) {
			if(is(Melee) || is(Ranged)) {
				attackinfo e = {0}; get(e);
				szblock(sc, p, result_maximum, "���� %2i-%3i", e.bonus, e.damage.min, e.damage.max);
			} else if(is(Torso) || is(Head) || is(Legs) || is(Elbows) || is(OffHand))
				szblock(sc, p, result_maximum, "������ %1i/%2i", getdefence(), getarmor());
		}
		if(forsale)
			szblock(sc, p, result_maximum, "���� %1i", getcostsingle());
		if(p[0])
			sc.prints(zend(p), result_maximum, ")");
	}
	if(getcount() > 2)
		sc.prints(zend(result), result_maximum, " %1i ��", getcount());
	return result;
}

skill_s item::getfocus() const {
	return item_data[type].focus;
}

unsigned item::getitems(item_s* result, slot_s* slots, unsigned slots_count) {
	auto pd = result;
	auto pe = slots + slots_count;
	for(auto type = NoItem; type < ManyItems; type = (item_s)(type + 1)) {
		item n(type);
		bool valid = false;
		for(auto s = slots; s < pe; s++) {
			if(n.is(*s)) {
				valid = true;
				break;
			}
		}
		if(valid)
			*pd++ = type;
	}
	return pd - result;
}

item_s item::getammo() const {
	return item_data[type].ammunition;
}