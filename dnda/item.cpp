#include "main.h"

const int GP = 100;
const int SP = 10;
const int CP = 1;
static_assert(sizeof(item) == sizeof(int), "Invalid sizeof(item). Must be equal sizeof(int).");

static struct magic_info {
	const char*		id;
	const char*		name;
} magic_data[] = {{""},
{"armor", "�����"},
{"charisma", "�������"},
{"constitution", "������������"},
{"defence", "������"},
{"destruction", "����������"},
{"dexterity", "��������"},
{"intellegene", "����������"},
{"precision", "��������"},
{"regeneration", "�����������"},
{"sharping", "�������"},
{"smashing", "������������"},
{"speed", "��������"},
{"strenght", "����"},
{"wisdow", "��������"},
};
assert_enum(magic, OfWisdow);
getstr_enum(magic);

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
static magic_s swords_effect[] = {OfDefence, OfDexterity, OfSpeed, OfPrecision, OfSharping};
static magic_s axe_effect[] = {OfStrenght, OfDestruction, OfSharping, OfSmashing};
static magic_s bludgeon_effect[] = {OfStrenght, OfDestruction, OfSmashing, OfConstitution};
static magic_s pierce_effect[] = {OfDefence, OfDexterity, OfPrecision, OfSpeed};
static spell_s scroll_spells[] = {Identify, Armor, ShieldSpell};
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
	aref<magic_s>		effects;
	aref<spell_s>		spells;
	item_s				ammunition;
	unsigned char		count;
} item_data[] = {{"�����"},
{"������ �����", 5 * GP, {1, {1, 8, Slashing}}, {Versatile}, {Melee}, WeaponFocusAxes, axe_effect},
{"������", 5 * CP, {2, {1, 6}}, {}, {Melee}, NoSkill, bludgeon_effect},
{"������", 2 * GP, {3, {1, 4, Piercing}, 1}, {}, {Melee, OffHand}, WeaponFocusBlades, swords_effect},
{"�����", 2 * GP, {1, {2, 5}}, {}, {Melee}, WeaponFocusAxes, bludgeon_effect},
{"������", 8 * GP, {1, {1, 7}}, {}, {Melee}, WeaponFocusAxes, bludgeon_effect},
{"�����", 8 * SP, {1, {1, 8, Piercing}}, {Versatile}, {Melee}, NoSkill, pierce_effect},
{"�����", 1 * SP, {2, {1, 6}}, {TwoHanded}, {Melee}, NoSkill, bludgeon_effect},
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
{"�������", 3 * SP},
{"������", 1 * SP},
{"���� ��������", 5 * SP},
{"���� ������", 10 * SP},
{"���� ������", 2 * SP},
{"�������", 1 * SP},
{"�������", 8 * SP},
{"����", 5 * SP},
//
{"������", 5 * GP, {}, {}, {}, NoSkill, {}, scroll_spells},
{"������", 6 * GP, {}, {}, {}, NoSkill, {}, scroll_spells},
{"������", 7 * GP, {}, {}, {}, NoSkill, {}, scroll_spells},
//
{"�����"},
//
{"�����", 20 * GP},
{"�����", 25 * GP},
{"�����", 30 * GP},
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
	// Scrolls and magical wands mostly be magical
	if(magic == Mundane) {
		switch(type) {
		case ScrollBlue:
		case ScrollGreen:
		case ScrollRed:
			if(d100()<70)
				magic = Magical;
			break;
		}
	}
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
	if(item_data[type].effects
		&& (magic == Artifact || (magic != Mundane && d100() < level)))
		effect = item_data[type].effects.data[rand() % item_data[type].effects.count];
	// Set maximum item count in set
	if(item_data[type].count)
		count = item_data[type].count - 1;
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

magic_s item::geteffect() const {
	if(item_data[type].effects.count)
		return effect;
	return NoEffect;
}

int item::getbonus(magic_s value) const {
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

void item::setcount(int count) {
	if(!count) {
		clear();
		return;
	}
	if(iscountable())
		this->count = count - 1;
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

bool item::isedible() const {
	switch(type) {
	case Meat:
	case BreadEvlen:
	case BreadDwarven:
	case BreadHalflings:
	case Ration:
	case Apple:
	case Cake:
	case Sausage:
		return true;
	default:
		return false;
	}
}

int item::getarmor() const {
	return item_data[type].combat.armor[1] + getbonus(OfArmor);
}

int item::getdefence() const {
	return item_data[type].combat.armor[0] - damaged + getbonus(OfDefence);
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
	sc.prints(result, result_maximum, item_data[type].name);
	if(getidentify() >= KnowEffect) {
		if(effect)
			sc.prints(zend(result), result_maximum, " %1%+2i", getstr(effect), bonus);
		else if(spell)
			sc.prints(zend(result), result_maximum, " '%1'", getstr(spell));
	}
	if(show_info) {
		auto p = zend(result);
		if(getidentify() >= KnowQuality) {
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