#include "archive.h"
#include "main.h"

using namespace game;

const unsigned poison_update = Minute * 4;

static unsigned short		exit_index;
static creature*			current_player;
static adat<creature, 1024>	creature_data;
static adat<creature, 6>	player_data;

static int experience_level[] = {0,
1000, 2000, 4000, 8000, 16000, 32000, 64000, 128000
};
static unsigned	experience_cost[] = {5,
10, 20, 30, 50, 80, 100, 150,
};
static char	dex_speed_bonus[] = {-4,
-3, -2, -2, -2, -1, -1, 0, 0, 0,
0, 0, 0, 0, 0, 0, 1, 2, 2, 3,
3, 3, 4, 4, 4
};
static char	int_checks[] = {2,
2, 2, 2, 2, 3, 3, 3, 3, 4,
4, 4, 5, 5, 6, 6, 6, 7, 7, 7,
8, 8, 9, 9, 9
};
static enchantment_s ability_effects[] = {OfStrenght, OfDexterity, OfConstitution, OfIntellegence, OfWisdow, OfCharisma};
static state_s ability_states[] = {Strenghted, Dexterious, Healthy, Intellegenced, Wisdowed, Charismatic};

static struct equipment_info {
	race_s				race;
	class_s				type;
	adat<variant, 16>	features;;
} equipment_data[] = {{Dwarf, Fighter, {AxeBattle, ScaleMail, Shield, BreadDwarven}},
{Elf, Ranger, {SwordLong, SwordShort, LeatherArmour, BowLong}},
{Elf, Fighter, {SwordLong, LeatherArmour, BowLong}},
{Halfling, Fighter, {SwordShort, LeatherArmour, BreadHalflings}},
{Halfling, Theif, {SwordShort, LeatherArmour, RingRed}},
{Animal, Cleric, {Mace}},
{Animal, Fighter, {SwordLong, LeatherArmour, Shield}},
{Animal, Paladin, {SwordLong, ScaleMail}},
{Animal, Ranger, {SwordLong, SwordShort, LeatherArmour}},
{Animal, Theif, {SwordShort, LeatherArmour}},
{Animal, Mage, {Staff, Wand2, Scroll1, Potion1}},
};

static struct race_info {
	const char*			name;
	char				ability_minimum[6];
	char				ability_maximum[6];
	adat<skill_s, 4>	skills;
	skillvalue			skills_pregen[8];
} race_data[] = {{"�����", {6, 6, 6, 2, 12, 2}, {8, 9, 9, 4, 15, 4}},
//
{"�������", {9, 9, 9, 9, 9, 9}, {11, 11, 11, 11, 11, 11}, {Bargaining, Gambling, Swimming}},
{"����", {11, 6, 13, 9, 9, 9}, {14, 9, 15, 11, 11, 11}, {Smithing, Mining, Athletics}, {{ResistPoison, 30}}},
{"����", {8, 12, 6, 11, 10, 11}, {10, 14, 8, 13, 12, 13}, {Survival, WeaponFocusBows, Swimming}, {{ResistCharm, 30}}},
{"����������", {6, 11, 10, 9, 10, 9}, {8, 13, 11, 11, 12, 11}, {HideInShadow, Acrobatics, Swimming}},
//
{"������", {5, 12, 8, 6, 9, 6}, {7, 14, 10, 8, 11, 8}, {HideInShadow, Acrobatics, Swimming}},
{"�������", {5, 13, 7, 6, 9, 6}, {7, 15, 9, 8, 11, 8}, {HideInShadow, Acrobatics, Swimming}},
{"���", {12, 9, 11, 6, 9, 6}, {15, 11, 13, 8, 11, 8}, {Athletics, Mining, Swimming}},
{"�����", {13, 9, 11, 5, 9, 5}, {16, 11, 13, 7, 11, 7}, {Athletics, Survival, Swimming}},
//
{"���������", {4, 5, 4, 1, 1, 1}, {6, 9, 6, 1, 1, 1}},
{"�������", {10, 6, 10, 4, 1, 1}, {12, 8, 13, 5, 1, 1}},
};
assert_enum(race, Undead);
getstr_enum(race);

static struct class_info {
	const char*			name;
	unsigned char		hp;
	unsigned char		attack;
	unsigned char		experience_award;
	char				ability[6];
	adat<skill_s, 8>	skills;
	adat<spell_s, 4>	spells;
} class_data[] = {{"����������", 4, 1, 4},
{"������", 8, 2, 1, {0, 0, 0, 0, 2, 1}, {Diplomacy, History, Healing}, {Bless, HealingSpell}},
{"����", 10, 3, 1, {2, 0, 1, -2, 0, -1}, {Survival, WeaponFocusBlades, WeaponFocusAxes}},
{"���", 4, 1, 1, {-2, 0, 0, 2, 1, 2}, {Alchemy, Concetration, Literacy}, {Armor, MagicMissile, Sleep}},
{"�������", 10, 3, 1, {2, 0, 1, 0, 1, 2}, {Diplomacy, Literacy, WeaponFocusBlades}, {DetectEvil}},
{"��������", 10, 2, 1, {0, 2, 1, 0, 1, -1}, {Survival, WeaponFocusBows}, {}},
{"���", 6, 1, 1, {0, 2, 0, 0, 0, 1}, {PickPockets, Lockpicking, HideInShadow, Acrobatics, DisarmTraps, Bluff, Backstabbing}},
{"�������", 8, 2, 2},
};
assert_enum(class, Monster);
getstr_enum(class);

static struct attack_info {
	const char*		name;
	const char*		damage;
	const char*		killed;
	bool			ignore_armor;
	skill_s			resist;
} attack_data[] = {{"�������", "%����� ���������%� ���� �� %1i �����", "� ����%�", false, NoSkill},
{"�������", "%����� ���%� �����%� �� %1i �����", "� ����%� �� �����", false, NoSkill},
{"�������", "%����� �������%� ���� �� %1i �����", "� ����%� �� �����", false, NoSkill},
//
{"�������", "%����� ������ �������� �� %1i �����", "� %��� ����%�", false, NoSkill},
{"�����", "%����� ������ ������� �� %1i �����", "� %��� �������%� �� ������", true, ResistCold},
{"�������������", "%����� ������� ������������� ������ �� %1i �����", "� �������� � ����������� %��� ����%�", false, ResistElectricity},
{"�����", "%����� ������ ����� �� %1i �����", "� %��� ���������%��� � ���������� ����", false, ResistFire},
{"�����", "%����� ������� ������� ������� �� %1i �����", "� %��� ����%�", true, NoSkill},
{"��", "%����� �������� �� ��� �� %1i �����", "� �������", true, ResistPoison},
{"����", "%����� ��������%��� ���� �� %1i �����", "� ���������%���", false},
};
assert_enum(attack, WaterAttack);
getstr_enum(attack);

static struct material_info {
	const char*		name;
} material_data[] = {{"������"},
{"������"},
{"������"},
{"����"},
{"��������"},
{"������"},
{"������"},
};
assert_enum(material, Wood);
getstr_enum(material);

int mget(int ox, int oy, int mx, int my);

void manual_skill_race(stringbuffer& sc, manual& me) {
	for(auto& e : race_data) {
		if(!e.skills.is(me.value.skill))
			continue;
		sc.header("[����]: ");
		sc.add(e.name);
	}
	sc.trail(".");
}

void manual_skill_class(stringbuffer& sc, manual& me) {
	for(auto& e : class_data) {
		if(!e.skills.is(me.value.skill))
			continue;
		sc.header("[������]: ");
		sc.add(e.name);
	}
	sc.trail(".");
}

static int roll3d6() {
	return (rand() % 6) + (rand() % 6) + (rand() % 6) + 3;
}

static void start_equipment(creature& e) {
	for(auto& ei : equipment_data) {
		if((!ei.race || ei.race == e.getrace()) && ei.type == e.getclass()) {
			e.apply(ei.features);
			break;
		}
	}
	e.equip(Ration);
	e.setmoney(e.getmoney() + xrand(3, 18)*GP);
}

static bool linelossv(int x0, int y0, int x1, int y1) {
	int dx = iabs(x1 - x0), sx = x0 < x1 ? 1 : -1;
	int dy = iabs(y1 - y0), sy = y0 < y1 ? 1 : -1;
	int err = (dx > dy ? dx : -dy) / 2, e2;
	for(;;) {
		if(x0 >= 0 && x0 < max_map_x && y0 >= 0 && y0 < max_map_y) {
			auto i = get(x0, y0);
			set(i, Visible, true);
			set(i, Explored, true);
			if(!ispassablelight(i))
				return false;
		}
		if(x0 == x1 && y0 == y1)
			return true;
		e2 = err;
		if(e2 > -dx) {
			err -= dy;
			x0 += sx;
		}
		if(e2 < dy) {
			err += dx;
			y0 += sy;
		}
	}
}

static bool linelos(int x0, int y0, int x1, int y1) {
	int dx = iabs(x1 - x0), sx = x0 < x1 ? 1 : -1;
	int dy = iabs(y1 - y0), sy = y0 < y1 ? 1 : -1;
	int err = (dx > dy ? dx : -dy) / 2, e2;
	for(;;) {
		if(x0 >= 0 && x0 < max_map_x && y0 >= 0 && y0 < max_map_y) {
			if(!ispassablelight(get(x0, y0)))
				return false;
		}
		if(x0 == x1 && y0 == y1)
			return true;
		e2 = err;
		if(e2 > -dx) {
			err -= dy;
			x0 += sx;
		}
		if(e2 < dy) {
			err += dx;
			y0 += sy;
		}
	}
}

void creature::initialize() {
	creature_data.clear();
}

void* creature::operator new(unsigned size) {
	for(auto& e : creature_data) {
		if(!e)
			return &e;
	}
	return creature_data.add();
}

static void turnbegin() {
	// Set fog of war
	auto max_count = max_map_x * max_map_y;
	for(auto i = 0; i < max_count; i++)
		game::set(i, Visible, false);
	for(auto& e : player_data) {
		if(!e)
			continue;
		e.setlos();
	}
}

creature* creature::addplayer() {
	return player_data.add();
}

bool creature::playturn() {
	turnbegin();
	auto tm = game::getseconds();
	for(auto& e : player_data) {
		if(!e)
			continue;
		e.update();
		if(e.recoil > tm)
			continue;
		e.makemove();
		if(e.recoil <= tm)
			e.recoil = tm + 1;
	}
	for(auto& e : creature_data) {
		if(!e)
			continue;
		e.update();
		if(e.recoil > tm)
			continue;
		e.makemove();
		if(e.recoil <= tm)
			e.recoil = tm + 1;
	}
	for(auto& e : getsites())
		e.update();
	return current_player != 0;
}

void creature::select(creature** result, rect rc) {
	for(auto& e : player_data) {
		if(!e)
			continue;
		auto i = mget(rc.x1, rc.y1, game::getx(e.position), game::gety(e.position));
		if(i == -1)
			continue;
		if(!game::is(e.position, Visible))
			continue;
		result[i] = &e;
	}
	for(auto& e : creature_data) {
		if(!e)
			continue;
		auto i = mget(rc.x1, rc.y1, game::getx(e.position), game::gety(e.position));
		if(i == -1)
			continue;
		if(!game::is(e.position, Visible))
			continue;
		result[i] = &e;
	}
}

void creature::setblocks(short unsigned* movements, short unsigned value) {
	for(auto& e : player_data) {
		if(!e)
			continue;
		movements[e.position] = value;
	}
	for(auto& e : creature_data) {
		if(!e)
			continue;
		movements[e.position] = value;
	}
}

creature* creature::getcreature(short unsigned index) {
	if(index == Blocked)
		return 0;
	for(auto& e : player_data) {
		if(!e)
			continue;
		if(e.position == index)
			return &e;
	}
	for(auto& e : creature_data) {
		if(!e)
			continue;
		if(e.position == index)
			return &e;
	}
	return 0;
}

static int isqrt(int num) {
	int res = 0;
	int bit = 1 << 30;
	// "bit" starts at the highest power of four <= the argument.
	while(bit > num)
		bit >>= 2;
	while(bit != 0) {
		if(num >= res + bit) {
			num -= res + bit;
			res = (res >> 1) + bit;
		} else
			res >>= 1;
		bit >>= 2;
	}
	return res;
}

aref<creature*> creature::getcreatures(aref<creature*> result, short unsigned start, int range) {
	auto pb = result.data;
	auto pe = result.data + result.count;
	auto x = game::getx(start);
	auto y = game::gety(start);
	for(auto& e : player_data) {
		if(!e)
			continue;
		auto dx = x - game::getx(e.position);
		auto dy = y - game::gety(e.position);
		auto d = isqrt(dx*dx + dy * dy);
		if(d > range)
			continue;
		if(!linelos(x, y, game::getx(e.position), game::gety(e.position)))
			continue;
		if(pb < pe)
			*pb++ = &e;
		else
			break;
	}
	for(auto& e : creature_data) {
		if(!e)
			continue;
		auto dx = x - game::getx(e.position);
		auto dy = y - game::gety(e.position);
		auto d = isqrt(dx*dx + dy * dy);
		if(d > range)
			continue;
		if(!linelos(x, y, game::getx(e.position), game::gety(e.position)))
			continue;
		if(pb < pe)
			*pb++ = &e;
		else
			break;
	}
	return aref<creature*>(result.data, pb - result.data);
}

creature* creature::add(short unsigned index, role_s role) {
	if(creature_data.count >= (sizeof(creature_data.data) / sizeof(creature_data.data[0]) - 8))
		return 0;
	index = getfree(index);
	if(index == Blocked)
		return 0;
	auto p = new creature(role);
	p->move(index);
	return p;
}

creature* creature::add(short unsigned index, race_s race, gender_s gender, class_s type) {
	if(creature_data.count >= sizeof(creature_data.data) / sizeof(creature_data.data[0]))
		return 0;
	index = getfree(index);
	if(index == Blocked)
		return 0;
	auto p = new creature(race, gender, type);
	p->move(index);
	return p;
}

void creature::hint(const char* format, ...) const {
	if(!isinteractive())
		return;
	logs::driver driver;
	driver.name = getname();
	driver.gender = gender;
	logs::addv(driver, format, xva_start(format));
}

void creature::applyability() {
	for(auto i = Strenght; i <= Charisma; i = (ability_s)(i + 1))
		abilities[i] = xrand(race_data[race].ability_minimum[i], race_data[race].ability_maximum[i]);
	for(auto i = Strenght; i <= Charisma; i = (ability_s)(i + 1))
		abilities[i] += class_data[type].ability[i];
	for(auto& a : abilities) {
		if(a < 1)
			a = 1;
	}
	for(auto e : race_data[race].skills_pregen)
		skills[e.id] += e.value;
	for(auto e : race_data[race].skills)
		raise(e);
	for(auto e : class_data[type].skills)
		raise(e);
}

void creature::raiseskills(int number) {
	skill_s source[sizeof(skills) / sizeof(skills[0])];
	auto pb = source;
	for(auto i = Bargaining; i <= TwoWeaponFighting; i = (skill_s)(i + 1)) {
		if(skills[i])
			*pb++ = i;
	}
	unsigned count = pb - source;
	if(!count)
		return;
	zshuffle(source, count);
	unsigned index = 0;
	while(number >= 0) {
		auto result = NoSkill;
		if(index >= count)
			index = 0;
		result = source[index++];
		raise(result);
		number--;
	}
}

static spell_s choose_spells(creature* p) {
	spell_s source[64];
	auto pb = source;
	auto pe = pb + sizeof(source) / sizeof(source[0]);
	for(auto e = FirstSpell; e <= LastSpell; e = (spell_s)(e + 1)) {
		if(p->get(e))
			continue;
		if(pb < pe)
			*pb++ = e;
	}
	auto count = pb - source;
	if(!count)
		FirstSpell;
	return source[rand() % count];
}

void creature::create(race_s race, gender_s gender, class_s type) {
	clear();
	this->race = race;
	this->gender = gender;
	this->type = type;
	this->level = 1;
	applyability();
	if(abilities[Intellegence] >= 9)
		raise(Literacy);
	for(auto e : class_data[type].spells)
		set(e, 1);
	start_equipment(*this);
	// ������� ������
	auto skill_checks = maptbl(int_checks, abilities[Intellegence]);
	raiseskills(skill_checks);
	// ��������� ����
	mhp = class_data[type].hp; hp = getmaxhits();
	mmp = 0; mp = getmaxmana();
	// ���
	name = game::genname(race, gender);
	updateweight();
}

void creature::release() const {
	for(auto& e : player_data) {
		if(!e)
			continue;
		if(e.horror == this)
			e.horror = 0;
		if(e.charmer == this)
			e.charmer = 0;
	}
	for(auto& e : creature_data) {
		if(!e)
			continue;
		if(e.horror == this)
			e.horror = 0;
		if(e.charmer == this)
			e.charmer = 0;
	}
	game::release(this);
}

void creature::clear() {
	memset(this, 0, sizeof(creature));
	position = Blocked;
	guard = Blocked;
	role = Character;
}

int	creature::getbonus(enchantment_s value) const {
	auto result = 0;
	for(int i = Head; i < Legs; i++)
		result += wears[i].getbonus(value);
	return result;
}

int creature::getarmor() const {
	auto result = wears[Head].getarmor();
	result += wears[Torso].getarmor();
	result += wears[TorsoBack].getarmor();
	result += wears[Elbows].getarmor();
	result += wears[Legs].getarmor();
	result += wears[Melee].getarmor();
	result += wears[OffHand].getarmor();
	result += wears[LeftFinger].getarmor();
	result += wears[RightFinger].getarmor();
	if(is(Armored))
		result += 1;
	return result;
}

int creature::getdefence() const {
	auto result = wears[Head].getdefence();
	result += wears[Torso].getdefence() + wears[Torso].getquality() * 5;
	result += wears[TorsoBack].getdefence();
	result += wears[Elbows].getdefence();
	result += wears[Legs].getdefence();
	result += wears[Melee].getdefence();
	result += wears[OffHand].getdefence() + (wears[OffHand].isarmor() ? wears[Torso].getquality() * 5 : 0);
	result += wears[LeftFinger].getbonus(OfDefence) * 5;
	result += wears[RightFinger].getbonus(OfDefence) * 5;
	result += get(Acrobatics) / 4; // RULE: Acrobatics raise defence by +1% for every 4% or 1% for every 2 points of dexterity
	if(is(Shielded))
		result += 30;
	if(is(Armored))
		result += 10;
	// RULE: Heavy encumbrace level apply to defence
	if(is(HeavilyEncumbered))
		result -= 20;
	return result;
}

bool creature::is(state_s value) const {
	return game::getseconds() <= states[value];
}

bool creature::canhear(short unsigned index) const {
	if(!this)
		return false;
	return distance(position, index) <= getlos();
}

void creature::say(const char* format, ...) {
	sayv(format, xva_start(format), 0);
}

void creature::sayvs(creature& opponent, const char* format, ...) {
	sayv(format, xva_start(format), &opponent);
}

bool creature::sayv(const char* format, const char* param, creature* opponent) {
	if(!format)
		return false;
	if(get(Intellegence) < 6)
		return false;
	if(!getplayer()->canhear(position))
		return false;
	logs::driver driver;
	driver.name = getname();
	driver.gender = gender;
	if(opponent) {
		driver.name_opponent = opponent->getname();
		driver.gender_opponent = opponent->gender;
	}
	logs::addnc("[%1:]\"", getname());
	logs::addvnc(driver, format, param);
	logs::add("\"");
	return true;
}

bool creature::askyn(creature* opponent, const char* format, ...) {
	if(getplayer() != opponent) {
		sayv(format, xva_start(format), opponent);
		return true;
	} else {
		if(!sayv(format, xva_start(format), opponent))
			return false;
		return logs::chooseyn();
	}
}

int	creature::getmaxhits() const {
	if(role == Character)
		return mhp + get(Constitution);
	return mhp + get(Constitution) / 2;
}

int	creature::getmaxmana() const {
	auto result = mmp;
	result += get(Intellegence);
	result += get(Concetration) / 4;
	return result;
}

const char* creature::getname() const {
	auto p = getmonstername();
	if(p)
		return p;
	return game::getnamepart(name);
}

char* creature::getfullname(char* result, const char* result_maximum, bool show_level, bool show_alignment) const {
	zcpy(result, getname());
	if(show_level)
		szprints(zend(result), result_maximum, " %1-%3 %2i ������", getstr(type), level, getstr(race));
	return result;
}

int creature::getdiscount(creature* customer) const {
	// RULE: Bargain skill modify cost of selling or bying items
	auto bs = get(Bargaining);
	auto bc = customer->get(Bargaining);
	auto delta = bc - bs;
	return 40 * delta / 100;
}

void creature::pickup(item& value, bool interactive) {
	char temp[260];
	if(value.isforsale()) {
		auto loc = getlocation(position);
		if(loc && loc->owner && loc->owner != this) {
			auto cost = value.getcost();
			if(interactive) {
				if(!loc->owner->askyn(this, "������ ������ �� %1i �����?", value.getcost()))
					return;
			}
			if(money < cost) {
				static const char* text[] = {
					"����������� ����� ����� ���������� �����.",
					"� ���� ���� ������� ���������� �����.",
					"��� ������� ���������� ����� - ��� ������.",
				};
				if(interactive)
					loc->owner->say(maprnd(text));
				return;
			}
			if(roll(Bargaining, 0, *loc->owner, Bargaining, 0) > 0) {
				cost = cost * 70 / 100;
				if(interactive) {
					static const char* ask_discount[] = {
						"�������? � �� ��������� ������� �� ����� ���������!",
						"���� ����� ������ � ������������. �������� ������?",
						"� �� ���� �������� ������ � ���� ������. ��� ����� ������, ��� ������ �� �����!",
					};
					static const char* accept_discount[] = {
						"��������. ����� ������ [%1i] �����.",
						"��, � ������� ������. ����� �� [%1i] �����.",
					};
					say(maprnd(ask_discount), cost);
					loc->owner->say(maprnd(accept_discount), cost);
				}
			}
			money -= cost;
			value.setsold();
		}
	}
	if(value.gettype() == Coin) {
		if(interactive)
			act("%����� ������%� %1.", value.getname(temp, zendof(temp)));
		money += value.getcount();
		value.clear();
		updateweight();
		return;
	}
	for(auto slot = FirstBackpack; slot <= LastBackpack; slot = (slot_s)(slot + 1)) {
		if(wears[slot])
			continue;
		wears[slot] = value;
		if(interactive)
			act("%����� ������%� %1.", value.getname(temp, zendof(temp)));
		value.clear();
		updateweight();
		return;
	}
	drop(position, value);
	value.clear();
	updateweight();
}

void creature::dropdown(item& value) {
	char temp[260];
	auto loc = getlocation(position);
	if(loc && loc->owner && loc->owner != this) {
		auto cost = value.getsalecost();
		cost += cost * loc->owner->getdiscount(this) / 100;
		if(cost <= 0) {
			static const char* text[] = {
				"��� ������� � ����� �� ����.",
				"������ �� ���� �������� �����.",
				"������ ���, �� ������ �� ��������� ����� ���������.",
			};
			loc->owner->say(maprnd(text));
			return;
		}
		if(!loc->owner->askyn(this, "� ���� ������ ��� �� %1i �����. �� ��������?", cost))
			return;
		money += cost;
		value.setforsale();
	}
	act("%����� �������%� %1.", value.getname(temp, zendof(temp)));
	drop(position, value);
	value.clear();
	updateweight();
}

void creature::equip(slot_s slot, item value) {
	wears[slot] = value;
	updateweight();
}

bool creature::equip(item value) {
	if(!value)
		return true;
	for(auto i = Head; i <= LastBackpack; i = (slot_s)(i + 1)) {
		if(wears[i])
			continue;
		if(i < FirstBackpack && !value.is(i))
			continue;
		equip(i, value);
		return true;
	}
	return false;
}

bool creature::equip(item_s value) {
	item it(value, 0, 0, 0, 0);
	return equip(it);
}

int	creature::getweight() const {
	auto result = 0;
	for(auto& e : wears) {
		if(e)
			result += e.getweight();
	}
	return result;
}

int creature::getweight(encumbrance_s id) const {
	switch(id) {
	case Encumbered: return get(Strenght) * 5 * 50;
	case HeavilyEncumbered: return get(Strenght) * 10 * 50;
	default: return 0;
	}
}

void creature::updateweight() {
	static encumbrance_s levels[] = {HeavilyEncumbered, Encumbered};
	auto w = getweight();
	encumbrance = NoEncumbered;
	for(auto e : levels) {
		auto m = getweight(e);
		if(w >= m) {
			encumbrance = e;
			break;
		}
	}
}

void creature::wait(int value) {
	if(!recoil)
		recoil = game::getseconds();
	recoil += value;
}

static bool use_skills(creature& player, scene& sc) {
	adat<skill_s, LastSkill + 1> recomended;
	for(auto i = (skill_s)1; i <= LastSkill; i = (skill_s)(i + 1)) {
		if(!player.getbasic(i))
			continue;
		recomended.add(i);
	}
	zshuffle(recomended.data, recomended.count);
	for(auto e : recomended) {
		if(player.use(sc, e))
			return true;
	}
	return false;
}

static bool use_spells(creature& player, scene& sc, bool combat) {
	adat<spell_s, LastSpell + 1> recomended;
	for(auto i = (spell_s)1; i <= LastSpell; i = (spell_s)(i + 1)) {
		if(!player.get(i))
			continue;
		if(player.getmana() < player.getcost(i))
			continue;
		if(combat && !player.geteffect(i).iscombat())
			continue;
		recomended.add(i);
	}
	if(!recomended)
		return false;
	zshuffle(recomended.data, recomended.count);
	for(auto e : recomended) {
		if(player.use(sc, e))
			return true;
	}
	return false;
}

static bool move_skills(creature& player, scene& sc) {
	for(auto i = (skill_s)1; i <= LastSkill; i = (skill_s)(i + 1)) {
		if(!player.getbasic(i))
			continue;
		obj_proc pr = creature::geteffect(i).proc;
		if(!pr)
			continue;
		if(player.use(sc, i))
			return true;
	}
	return false;
}

void creature::walkaround(scene& sc) {
	const int chance_act = 40;
	if(d100() < chance_act) {
		// Do nothing
		wait(xrand(Minute / 2, Minute));
		return;
	}
	// When we try to stand and think
	if(d100() < chance_act) {
		if(aiboost())
			return;
		if(use_skills(*this, sc))
			return;
		if(d100() < chance_act) {
			if(use_spells(*this, sc, false))
				return;
		}
		wait(Minute / 3);
	}
	// When we move and traps or close door just before our step
	if(move_skills(*this, sc))
		return;
	auto d = (direction_s)xrand(Left, RightDown);
	move(to(position, d));
}

void creature::trapeffect() {
	auto trap = gettrap(position);
	if(!trap)
		return;
	if(isparty(current_player))
		game::set(position, Hidden, false);
	if(roll(Acrobatics)) {
		hint("%����� ������� ����%�� �������.");
		return;
	}
	auto a = game::getattackinfo(trap);
	damage(a.damage.roll(), a.damage.type, true);
}

bool creature::move(short unsigned i) {
	if(i == Blocked)
		return false;
	if(is(Drunken)) {
		auto n = game::getdirection({(short)getx(position), (short)gety(position)}, {(short)getx(i), (short)gety(i)});
		if(d100() < 45) {
			n = turn(n, (d100() < 50) ? LeftUp : RightUp);
			i = to(position, n);
			if(d100() < 70) {
				static const char* talk[] = {"��������.",
					"��������.",
					"��! �� ����...",
					"�� ���� ����!",
					"���!"
				};
				say(maprnd(talk));
			}
		}
	}
	if(position != Blocked) {
		auto n = game::getdirection({(short)getx(position), (short)gety(position)}, {(short)getx(i), (short)gety(i)});
		switch(n) {
		case Left:
		case LeftUp:
		case LeftDown:
		case Right:
		case RightUp:
		case RightDown:
			direction = n;
			break;
		}
	}
	if(alertness())
		return true;
	if(interact(i))
		return true;
	auto p1 = getlocation(position);
	auto p2 = getlocation(i);
	if(p1 != p2) {
		// �������� �������� �� ����� �������� �������
		if(p1 && p1->owner == this)
			return false;
	}
	if(!ispassable(i))
		return false;
	position = i;
	auto new_current_site = getlocation(position);
	if(current_site != new_current_site) {
		if(new_current_site)
			new_current_site->entering(*this);
	}
	current_site = new_current_site;
	if(isplayer())
		lookfloor();
	trapeffect();
	switch(gettile(position)) {
	case Water:
		if(!roll(Swimming))
			damage(xrand(1, 6), WaterAttack, true);
		else if(!roll(ResistWater))
			damagewears(1, WaterAttack);
		break;
	}
	wait(getmoverecoil());
	return true;
}

bool creature::moveto(short unsigned index) {
	if(index == position)
		return false;
	if(distance(position, index) > 1) {
		makewave(index);
		index = getstepto(position);
		if(index == Blocked) {
			wait(xrand(1, 5));
			return false;
		}
	}
	return move(index);
}

bool creature::moveaway(short unsigned index) {
	makewave(index);
	index = getstepfrom(position);
	if(index == 0xFFFF)
		return false;
	move(index);
	return true;
}

creature* creature::getenemy(aref<creature*> source) const {
	for(auto p : source) {
		if(isenemy(p))
			return p;
	}
	return 0;
}

void creature::makemove() {
	// RULE: sleeped or paralized creature don't move
	if(is(Sleeped) || is(Paralized))
		return;
	// Make scene
	scene sc(getlos(), getposition());
	// Player turn
	if(getplayer() == this) {
		logs::turn(*this, sc);
		return;
	}
	// Make move depends on conditions
	if(horror && distance(horror->position, position) <= getlos() + 1) {
		moveaway(horror->position);
		return;
	}
	if(sc.isenemy(*this)) {
		if(use_spells(*this, sc, true))
			return;
		//	if(aiboost())
		//		return;
		//	if(aiusewand(creatures, TargetHostile))
		//		return;
		//	auto spell = aispell({&enemy, 1}, TargetHostile);
		//	if(spell)
		//		use(spell);
		//	else if(isranged(false))
		//		rangeattack(enemy);
		//	else
		//		moveto(enemy->position);
		return;
	}
	// If creature guard some square move to guard position
	if(guard != Blocked) {
		moveto(guard);
		return;
	}
	// If creature have a leader don't move far away from him
	auto leader = getleader();
	if(leader) {
		if(distance(leader->position, position) > 2) {
			moveto(leader->position);
			return;
		}
	}
	walkaround(sc);
}

creature* creature::getplayer() {
	return current_player;
}

creature* creature::getplayer(int index) {
	if(index >= (int)player_data.count)
		return 0;
	return player_data.data + index;
}

void creature::setplayer() {
	current_player = this;
}

bool creature::isplayer() const {
	return player_data.indexof(this) != -1;
}

bool creature::isinteractive() const {
	return current_player == this;
}

bool creature::isparty(const creature* value) const {
	return value && getleader() == value->getleader();
}

bool creature::isfriend(const creature* value) const {
	if(!value)
		return false;
	return &getai() == &value->getai();
}

bool creature::isenemy(const creature* target) const {
	if(!target || target == this)
		return false;
	if(target->role == Shopkeeper || this->role == Shopkeeper)
		return false;
	if(getleader() == target->getleader())
		return false;
	return isagressive() != target->isagressive();
}

void creature::manipulate(short unsigned index) {
	auto a = getobject(index);
	switch(a) {
	case Door:
		if(!game::is(index, Opened)) {
			if(game::is(index, Sealed))
				say("����� �������.");
			else
				game::set(index, Opened, true);
		} else
			game::set(index, Opened, false);
		wait(Minute / 4);
		break;
	case StairsUp:
	case StairsDown:
		if(isplayer()) {
			logs::add("�� ������������� ������ %1",
				(a == StairsDown) ? "���������� ����" : "��������� ������");
			if(logs::chooseyn())
				exit_index = position;
		}
		break;
	}
}

bool creature::interact(short unsigned index) {
	switch(getobject(index)) {
	case Door:
		if(!game::is(index, Opened)) {
			if(game::is(index, Sealed))
				say("����� �������.");
			else
				game::set(index, Opened, true);
			wait(Minute / 4);
			return true;
		}
		break;
	}
	if(!ispassable(index))
		return false;
	auto p = getcreature(index);
	if(p) {
		if(isenemy(p)) {
			meleeattack(p);
			return true;
		} else if(this != getplayer()) {
			// ������I � ������ ��������� �� ��������
			wait(xrand(2, 8));
			return true;
		} else if(p->isguard()) {
			static const char* talk[] = {
				"� ������� ��� �����.",
				"����� �� ������.",
				"�� ��������, � �� ������.",
			};
			p->say(maprnd(talk));
			wait(xrand(1, 4));
			return true;
		} else {
			// ����� �������� �������� � ���������
			auto loc = getlocation(p->position);
			if(loc && loc->owner == p) {
				// � ����������� ������� �� ���������� �������
				static const char* talk[] = {
					"�� ������ ���� � ���� ���������.",
					"����� ���� �� ����.",
					"� �� ����� �����",
				};
				p->say(maprnd(talk));
				return true;
			}
			p->position = position;
			p->wait(xrand(1, 4));
			if(d100() < 50) {
				static const char* talk[] = {
					"��! �� ��������.",
					"�����, �������.",
					"���� �� ��� �������?",
				};
				p->say(maprnd(talk));
			}
			return false;
		}
	}
	return false;
}

void creature::attack(creature* defender, slot_s slot, int bonus, int multiplier) {
	if(!(*defender))
		return;
	auto ai = getattackinfo(slot);
	switch(ai.effect) {
	case OfHoliness:
		if(defender->race == Undead) {
			ai.bonus += ai.quality;
			if(ai.quality > 0)
				multiplier++;
		}
		break;
	case OfOrcSlying:
		if(defender->race == Orc || defender->race == Goblin) {
			ai.bonus += ai.quality;
			if(ai.quality > 0)
				multiplier++;
		}
		break;
	}
	auto s = rand() % 100;
	auto r = s + ai.bonus + bonus - defender->getdefence();
	if(slot == Ranged) {
		// RULE: missile deflection very effective on ranged attack
		r -= defender->getbonus(OfMissileDeflection) * 10;
	}
	if(slot == Ranged) {
		if(wears[slot].isthrown())
			actnc("%����� �����%� %1 �", getstr(wears[slot].gettype()));
		else
			actnc("%����� ���������%� �� %1 �", getstr(wears[slot].gettype()));
	} else
		actnc("%�����");
	if(s < 5 || (r < chance_to_hit)) {
		act("��������%�.");
		return;
	}
	auto damage = ai.damage;
	// RULE: basic chance critical hit is 4% per point
	bool critical_hit = s >= (95 - ai.critical * 4);
	act(critical_hit ? "���������� �����%�." : "�����%�.");
	if(critical_hit)
		multiplier += ai.multiplier;
	// RULE: Each multiplier step add one maximum value (minimum 2 and maximum 10)
	auto step = imax(2, imin(12, imax(damage.max, damage.min) - damage.min));
	for(auto i = ai.multiplier; i > 0; i--)
		damage.max += step;
	if(wears[slot]) {
		// RULE: artifact is unbreakable
		if(!wears[slot].isunbreakable()) {
			if(d100() < wears[slot].getspecial().chance_broke) {
				if(wears[slot].damageb()) {
					wears[slot].act("%����� ������%���.");
					wears[slot].clear();
				} else
					wears[slot].act("%����� �������%�.");
			}
		}
	}
	auto value = damage.roll();
	defender->damage(value, damage.type, true);
	switch(ai.effect) {
	case OfFire:
		defender->damage(xrand(1, 6) + ai.quality * 2, Fire, true);
		break;
	case OfCold:
		defender->damage(xrand(1, 4) + ai.quality, Cold, true);
		break;
	case OfVampirism:
		// RULE: weapon of vampirism heal owner
		heal(xrand(1, 4) + ai.quality, false);
		break;
	case OfSickness:
		// RULE: sickness effect are long
		if(ai.quality > 0) {
			if(!defender->roll(ResistPoison, 10 - value))
				defender->set(Sick, Hour*ai.quality);
		} else {
			// Cursed sickness item affect attacker
			if(!roll(ResistPoison))
				set(Sick, (Hour / 2)*(-ai.quality));
		}
		break;
	case OfPoison:
		if(ai.quality < 0) {
			// RULE: cused poison weapon wound owner instead
			if(!roll(ResistPoison, 10 - value))
				set(PoisonedWeak, Minute*poison_update * 2 * (-ai.quality));
		} else {
			// RULE: power of poison depends on magical bonus
			static state_s quality_state[] = {PoisonedWeak, PoisonedWeak, PoisonedWeak, Poisoned, Poisoned, PoisonedStrong};
			// RULE: chance be poisoned depends on damage deal
			if(!defender->roll(ResistPoison, 10 - value))
				defender->set(maptbl(quality_state, ai.quality), Minute*poison_update * 5);
		}
		break;
	case OfParalize:
		// RULE: chance be paralized depends on quality
		if(!defender->roll(ResistParalize, 8 - ai.quality * 4))
			defender->set(Paralized, Minute);
		break;
	case OfSlowing:
		// RULE: chance be paralized depends on quality
		if(!defender->roll(ResistParalize, -ai.quality * 5))
			defender->set(Slowed, Minute);
		break;
	case OfWeakness:
		if(ai.quality < 0) {
			if(!roll(ResistPoison, 10 - value))
				set(Weaken, Minute * 3);
		} else {
			// RULE: chance be poisoned depends on damage deal
			if(!defender->roll(ResistPoison, 10 - value))
				defender->set(Weaken, Minute + Minute * value);
		}
		break;
	}
}

void creature::damage(int value, attack_s type, bool interactive) {
	if(value >= 0) {
		const auto& a = attack_data[type];
		if(value >= 0 && a.resist) {
			// RULE: reistance to damage remove percent of damage
			auto skill_value = get(a.resist);
			if(skill_value > 100)
				skill_value = 100;
			value = (value * (100 - skill_value)) / 100;
		}
		if(!a.ignore_armor)
			value -= getarmor();
		if(value < 0)
			value = 0;
		hp -= value;
		if(value <= 0) {
			if(interactive)
				actnc("%����� ��������%� ����");
		} else {
			if(interactive)
				actnc(a.damage, value);
		}
		if(hp <= 0) {
			if(interactive)
				actnc(a.killed);
		} else {
			if(is(Sleeped)) {
				if(interactive)
					actnc(" � �������%���");
				remove(Sleeped);
			}
		}
		if(interactive)
			act(".");
		damagewears(value, type);
		if(hp <= 0) {
			addexp(getcostexp(), position, 8, this, this);
			const int chance_loot = 40;
			for(auto& e : wears) {
				if(!e)
					continue;
				if(e.isnatural())
					continue;
				if(isplayer()
					|| (&e >= &wears[FirstBackpack])
					|| (d100() < chance_loot)) {
					e.loot();
					drop(position, e);
				}
			}
			release();
			clear();
		}
	} else {
		auto mhp = getmaxhits();
		value = -value;
		if(hp + value > mhp)
			value = mhp - hp;
		if(value > 0) {
			hp += value;
			if(interactive)
				act("%����� �����������%� %1i �����.", value);
		}
	}
}

void creature::damagewears(int value, attack_s type) {
	static struct attack_damage {
		attack_s	type;
		material_s	material;
		char		damage_chance;
		const char*	damage_text;
		void		(item::*destroy)();
	} attack_damage_data[] = {{Fire, Paper, 55, "%����� ������%� �����.", &item::clear},
	{Fire, Iron, 15, "%����� ���������%��� �����.", &item::clear},
	{Fire, Wood, 40, "%����� ������%� � ����� �������.", &item::clear},
	{Fire, Leather, 30, "%����� �������� ������%�.", &item::damage},
	{Acid, Iron, 80, "%����� ������� ��������.", &item::damage},
	{Acid, Leather, 70, "%����� ������� ��������.", &item::damage},
	{Acid, Wood, 60, "%����� ������� ��������.", &item::damage},
	{WaterAttack, Iron, 50, "%����� ��������%�.", &item::damage},
	{WaterAttack, Paper, 60, "%����� ������%�� ��������.", &item::clear},
	};
	if(!(*this))
		return;
	if(value <= 0)
		return;
	for(auto& e : attack_damage_data) {
		if(e.type != type)
			continue;
		for(auto& it : wears) {
			if(!it)
				continue;
			if(it.isartifact())
				continue;
			if(e.material != it.getmaterial())
				continue;
			auto chance = e.damage_chance;
			auto result = d100();
			chance -= iabs(it.getquality()) * 5;
			if((chance <= 0) || (result >= chance))
				continue;
			if(getplayer()->canhear(position))
				it.act(e.damage_text);
			(it.*e.destroy)();
		}
	}
}

unsigned creature::getcostexp() const {
	auto result = maptbl(experience_cost, level);
	for(auto slot = Head; slot <= Ranged; slot = (slot_s)(slot + 1)) {
		if(!wears[slot].isnatural())
			continue;
		result += wears[slot].getenchantcost() * 10 + wears[slot].getquality() * 5;
	}
	return result;
}

void creature::addexp(int value, short unsigned position, int range, const creature* exclude, const creature* enemies) {
	auto x0 = getx(position);
	auto y0 = gety(position);
	for(auto& e : creature_data) {
		if(!e)
			continue;
		if(&e == exclude)
			continue;
		if(enemies && !e.isenemy(enemies))
			continue;
		if(distance(e.position, position) > range)
			continue;
		if(!linelos(x0, y0, getx(e.position), gety(e.position)))
			continue;
		e.addexp(value);
	}
}

bool creature::isranged(bool interactive) const {
	if(!wears[Ranged])
		return false;
	auto ammo = wears[Ranged].getammo();
	if(ammo) {
		if(wears[Amunitions].gettype() != ammo) {
			if(interactive)
				hint("��� �������� ���������� %1", getstr(ammo));
			return false;
		}
	}
	return true;
}

void creature::rangeattack(creature* enemy) {
	if(!isranged(true))
		return;
	auto ammo = wears[Ranged].getammo();
	attack(enemy, Ranged);
	wait(getattacktime(Ranged));
	// Ranged weapon usually use ammunition
	if(ammo)
		wears[Amunitions].setcount(wears[Amunitions].getcount() - 1);
	// Throwing weapon remove self
	if(wears[Ranged].iscountable())
		wears[Ranged].setcount(wears[Ranged].getcount() - 1);
}

void creature::meleeattack(creature* enemy, int bonus, int multiplier) {
	if(!(*enemy))
		return;
	if(wears[Melee].is(Melee) && wears[OffHand].is(Melee)) {
		attack(enemy, Melee, bonus - 4, multiplier);
		attack(enemy, OffHand, bonus - 6, multiplier);
	} else if(wears[OffHand].is(Melee))
		attack(enemy, OffHand, bonus, multiplier);
	else
		attack(enemy, Melee, bonus, multiplier);
	wait(getattacktime(Melee));
}

int	creature::getlos() const {
	if(game::isdungeon())
		return 4;
	return 6;
}

creature* creature::getleader() const {
	if(charmer)
		return charmer;
	if(isplayer()) {
		if(current_player == this)
			return 0;
		return current_player;
	}
	return 0;
}

const creature& creature::getai() const {
	auto p = getleader();
	if(p)
		return *p;
	return *this;
}

void creature::setlos() {
	auto x0 = getx(position);
	auto y0 = gety(position);
	auto r = getlos();
	for(auto x = x0 - r; x <= x0 + r; x++) {
		linelossv(x0, y0, x, y0 - r);
		linelossv(x0, y0, x, y0 + r);
	}
	for(auto y = y0 - r; y <= y0 + r; y++) {
		linelossv(x0, y0, x0 - r, y);
		linelossv(x0, y0, x0 + r, y);
	}
}

int creature::getbasic(ability_s value) const {
	return abilities[value];
}

int creature::get(ability_s value) const {
	auto result = abilities[value];
	if(is(ability_states[value])) {
		if(result < 16)
			result = 16;
		else
			result++;
	}
	result += getbonus(ability_effects[value]);
	switch(value) {
	case Strenght:
		if(is(Weaken))
			result -= 4;
		if(is(Sick))
			result -= 2;
		if(is(Poisoned))
			result--;
		break;
	case Dexterity:
		if(is(Sick))
			result -= 3;
		if(is(PoisonedWeak))
			result--;
		if(is(Paralized))
			result /= 2;
		break;
	}
	if(result < 0)
		result = 0;
	return result;
}

attackinfo creature::getattackinfo(slot_s slot) const {
	auto attack_per_level = class_data[type].attack;
	if(!attack_per_level)
		attack_per_level = 1;
	attackinfo result = {};
	result.bonus = (level - 1) * attack_per_level;
	auto& weapon = wears[slot];
	if(weapon) {
		wears[slot].get(result);
		auto focus = weapon.getfocus();
		if(focus && getbasic(focus)) {
			auto fs = get(focus);
			result.bonus += fs / 5;
			result.damage.max += fs / 40;
		}
		// RULE: Versatile weapon if used two-handed made more damage.
		if(slot == Melee && weapon.isversatile() && !wears[OffHand]) {
			result.damage.min += 1;
			result.damage.max += 1;
		}
	} else
		result.damage.max = 2;
	switch(slot) {
	case Melee:
	case OffHand:
		result.bonus += get(Strenght);
		result.damage.max += (get(Strenght) - 10) / 2;
		break;
	case Ranged:
		result.bonus += get(Dexterity);
		break;
	}
	if(is(Blessed)) {
		// RULE: bless add Wisdow ability to attack bonus
		result.bonus += get(Wisdow);
		result.damage.max++;
	}
	// RULE: Heavy encumbrace level apply to attack
	if(is(HeavilyEncumbered))
		result.bonus -= 20;
	return result;
}

static void weapon_information(creature& player, item weapon, const attackinfo& ai) {
	char t1[260];
	player.act("%1 � ������ [%2i%%] ������� [%3i-%4i] �����",
		weapon.getname(t1, zendof(t1), false), chance_to_hit + ai.bonus, ai.damage.min, ai.damage.max);
}

int creature::getattacktime(slot_s slot) const {
	auto tm = Minute - maptbl(dex_speed_bonus, get(Dexterity)) - getattackinfo(slot).speed;
	if(slot == Melee && wears[Melee].is(Melee) && wears[OffHand].is(Melee))
		tm += Minute / 2 - getattackinfo(OffHand).speed;
	// RULE: Encumbrace level apply to attack time
	tm += 4 * getencumbrance();
	if(tm < Minute / 4)
		tm = Minute / 4;
	if(is(Slowed))
		tm += 6;
	return tm;
}

void testweapon(creature& e, scene& sc) {
	e.act("%����� ���������%� ���� ������.");
	if(e.wears[Melee] && e.wears[OffHand].is(Melee)) {
		e.act("������������ ");
		weapon_information(e, e.wears[Melee], e.getattackinfo(Melee));
		e.act(" � ");
		weapon_information(e, e.wears[OffHand], e.getattackinfo(OffHand));
	} else if(e.wears[OffHand].is(Melee))
		weapon_information(e, e.wears[OffHand], e.getattackinfo(OffHand));
	else if(e.wears[Melee].is(Melee))
		weapon_information(e, e.wears[Melee], e.getattackinfo(Melee));
	logs::add(".");
	e.act("��� �������� %1i ������.", e.getattacktime(Melee)*(60 / Minute));
	if(e.wears[Ranged]) {
		weapon_information(e, e.wears[Ranged], e.getattackinfo(Ranged));
		e.act("��� �������� %1i ������.", e.getattacktime(Ranged)*(60 / Minute));
	}
	e.wait(Minute);
}

bool creature::use(short unsigned index) {
	if(index == Blocked)
		return false;
	switch(game::gettile(index)) {
	case Door:
		break;
	}
	return true;
}

void creature::remove(state_s value) {
	if(value <= LastState)
		states[value] = game::getseconds();
}

void creature::set(state_s value, unsigned segments_count) {
	switch(value) {
	case RestoreHits:
		heal(xrand(10, 20), false);
		break;
	case RestoreMana:
		consume(xrand(8, 16), false);
		break;
	case RemovePoison:
		remove(Poisoned);
		remove(PoisonedWeak);
		remove(PoisonedStrong);
		break;
	case RemoveSick:
		remove(Sick);
		remove(Weaken);
		break;
	default:
		if(value <= LastState) {
			unsigned stop = recoil + segments_count;
			if(states[value] < stop)
				states[value] = stop;
		}
		break;
	}
}

int getexperiencelevel(int value) {
	if(!value)
		return 1;
	for(int i = 0; i < sizeof(experience_level) / sizeof(experience_level[0]); i++) {
		if(value < experience_level[i])
			return i;
	}
	return sizeof(experience_level) / sizeof(experience_level[0]);
}

void creature::athletics(bool interactive) {
	static ability_s list[] = {Strenght, Dexterity, Constitution};
	for(auto i : list) {
		auto b = (8 - abilities[i]) * 7;
		if(roll(Athletics, b)) {
			abilities[i]++;
			if(interactive) {
				logs::add("��� ������� %1 ����� �� %2i.", getstr(i), abilities[i]);
				logs::next();
			}
		}
	}
}

void creature::levelup(bool interactive) {
	if(interactive) {
		logs::add("%1 ������ %2 ������ [%3i].", getname(), getstr(type), level + 1);
		logs::next();
	}
	athletics(interactive);
	mhp += xrand(1, class_data[type].hp);
	auto n = maptbl(int_checks, abilities[Intellegence]);
	if(interactive)
		logs::raise(*this, n);
	else
		raiseskills(n);
	level++;
}

void creature::addexp(int count) {
	if(!count)
		return;
	if(level == 0)
		return;
	experience += count;
	if(experience < 0)
		experience = 0;
	while(true) {
		if(getexperiencelevel(experience) <= level)
			break;
		levelup(isplayer());
	}
}

int creature::get(spell_s value) const {
	return spells[value];
}

void creature::set(spell_s value, int level) {
	spells[value] = level;
}

void creature::update() {
	auto tm = game::getseconds();
	// Remove any links if target is invalid
	if(horror && (!is(Scared) || !(*horror))) {
		remove(Scared);
		horror = 0;
	}
	if(charmer && (!is(Charmed) || !(*charmer))) {
		remove(Charmed);
		charmer = 0;
	}
	// RULE: Poison affect creature every short time interval.
	if((tm % poison_update) == 0) {
		static struct poison_info {
			damageinfo	damage;
			state_s		state;
		} poison_effect[] = {{{0, 3, Poison}, PoisonedWeak},
		{{1, 8, Poison}, Poisoned},
		{{2, 16, Poison}, PoisonedStrong}
		};
		auto damage_hits = 0;
		for(auto& e : poison_effect) {
			if(!is(e.state))
				continue;
			damage_hits += e.damage.roll();
		}
		if(damage_hits > 0)
			damage(damage_hits, Poison, true);
	}
	// RULE: � ������� ��������������� 1 ��� �� 30 �����
	if(tm >= restore_hits) {
		if(hp < getmaxhits()) {
			// RULE: cursed ring of regeneration disable hit point natural recovering
			hp += imax(0, 1 + getbonus(OfRegeneration));
		}
		if(!restore_hits)
			restore_hits = tm;
		// RULE: ring of regeneration increase regeneration time
		restore_hits += imax(5, 40 - (get(Constitution) + getbonus(OfRegeneration) * 2))*Minute;
	}
	// RULE: � ������� ��������������� 1 ����� �� 10 �����
	if(tm >= restore_mana) {
		if(mp < getmaxmana()) {
			// RULE: cursed ring of mana disable mana points natural recovering
			mp += imax(0, 1 + getbonus(OfMana) + get(Concetration) / 50);
		}
		if(!restore_mana)
			restore_mana = tm;
		restore_mana += imax(5, 50 - get(Intellegence) * 2)*Minute / 3;
	}
}

void creature::lookfloor() {
	item* source[64];
	auto count = game::getitems(source, sizeof(source) / sizeof(source[0]), position);
	if(!count)
		return;
	logs::addnc("����� ");
	if(count > 1)
		logs::addnc("����� ");
	else
		logs::addnc("����� ");
	for(int i = 0; i < count; i++) {
		if(i != 0)
			logs::addnc(" � ");
		char temp[260];
		stringcreator sc;
		logs::addvnc(sc, source[i]->getname(temp, zendof(temp), true), 0);
	}
	logs::add(".");
}

bool creature::roll(skill_s skill, int bonus) const {
	auto result = get(skill) + bonus;
	if(result <= 0)
		return false;
	return d100() < result;
}

int creature::roll(skill_s skill, int bonus, const creature& opponent, skill_s opponent_skill, int opponent_bonus) const {
	auto s1 = get(skill) + bonus;
	auto s2 = opponent.get(opponent_skill) + opponent_bonus;
	auto d1 = d100();
	auto d2 = d100();
	if(d1 >= s1 && d2 >= s2)
		return 0; // Both parcipant loose
	else if(d1 < s1 && d2 < s2)
		return (d1 > d2) ? 1 : -1; // Win parcipant with greater total result
	return (d1 < s1) ? 1 : -1; // Win those who make roll
}

void creature::actv(creature& opponent, const char* format, const char* param) const {
	auto player = getplayer();
	if(!player)
		return;
	if(!player->canhear(position))
		return;
	logs::driver driver;
	driver.name = getname();
	driver.gender = gender;
	driver.name_opponent = opponent.getname();
	driver.gender_opponent = opponent.gender;
	logs::addv(driver, format, param);
}

void creature::actv(const char* format, const char* param) const {
	if(!getplayer()->canhear(position))
		return;
	logs::driver driver;
	driver.name = getname();
	driver.gender = gender;
	logs::addv(driver, format, param);
}

void creature::actvnc(const char* format, const char* param) const {
	if(!getplayer()->canhear(position))
		return;
	logs::driver driver;
	driver.name = getname();
	driver.gender = gender;
	logs::addvnc(driver, format, param);
}

void creature::consume(int value, bool interactive) {
	auto mmp = getmaxmana();
	mp -= value;
	if(mp > mmp)
		mp = mmp;
	if(mp < 0)
		mp = 0;
}

bool creature::alertness() {
	//if(getplayer() != getleader())
	return false;
	// RULE: for party only alertness check secrect doors and traps
	//short unsigned source_data[5 * 5];
	//auto result = select(source_data, TargetHiddenObject, 2, position);
	//auto found = false;
	//for(auto index : result) {
	//	if(roll(Alertness)) {
	//		found = true;
	//		game::set(index, Hidden, false);
	//		auto object = game::getobject(index);
	//		if(isplayer()) {
	//			act("�� ���������� %1.", getstr(object));
	//			logs::next();
	//		}
	//		addexp(25);
	//	}
	//}
	//return found;
}

static state_s get_ability_state(ability_s id) {
	switch(id) {
	case Dexterity: return Dexterious;
	case Constitution: return Healthy;
	case Intellegence: return Intellegenced;
	case Wisdow: return Wisdowed;
	case Charisma: return Charismatic;
	default: return Strenghted;
	}
}

void creature::apply(aref<variant> features) {
	item it;
	for(auto i : features) {
		switch(i.type) {
		case Items:
		case ItemObject:
			if(i.type == Items)
				it = item(i.itemtype, 0, 15, 10, 35);
			else
				it = i.itemobject;
			equip(it);
			if(it.getammo())
				equip(item(it.getammo(), 0, 15, 10, 35));
			break;
		case Skill:
			raise(i.skill);
			break;
		case Ability:
			abilities[i.ability] += 2;
			break;
		default: break;
		}
	}
}

bool creature::use(scene& sc, spell_s value) {
	auto cost = getcost(value);
	if(getmana() < cost) {
		hint("�� ������� ����.");
		return false;
	}
	if(!use(sc, value, 1, "%����� ���������%� ����������� �������."))
		return false;
	mp -= cost;
	wait(Minute);
	return true;
}

void creature::use(scene& sc, item& it) {
	if(it.isedible()) {
		char temp[260]; grammar::what(temp, getstr(it.gettype()));
		act("%����� ����%� %L1.", temp);
		auto& e = it.getfood();
		auto q = it.getquality();
		auto calory = e.hits + q + getbonus(OfSustenance) * 2;
		heal(calory, true);
		consume(-(e.mana + q), false);
		if(d100() < e.sickness) {
			remove(Sick);
			remove(Weaken);
		}
		if(d100() < e.poision) {
			remove(PoisonedWeak);
			remove(Poisoned);
			remove(PoisonedStrong);
		}
		for(auto i = Strenght; i <= Charisma; i = (ability_s)(i + 1)) {
			auto m = e.get(abilities[i]);
			abilities_raise[i] += e.abilities[i] * (1 + q);
			if(abilities_raise[i] >= m) {
				abilities[i]++;
				abilities_raise[i] -= m;
				hint("�� ������������� ���� %1", getname(get_ability_state(i), false));
			}
		}
		it.clear();
		wait(e.duration + xrand(1, 6));
	} else if(it.isdrinkable())
		drink(it, true);
	else if(it.ischargeable()) {
		char temp[260]; it.getname(temp, zendof(temp), false);
		auto spell = it.getspell();
		if(!spell)
			hint("%1 �� ��������.", temp);
		else if(!it.getcharges())
			hint("%1 ��������, �� ����� ��������.", temp);
		else {
			grammar::what(temp, getstr(it.gettype()));
			if(use(sc, spell, 1 + it.getquality(), "%����� ��������%� ������ %L1.", temp)) {
				it.setcharges(it.getcharges() - 1);
				if(it && it.getcharges() == 0) {
					hint("%1 ����������� � ����.", it.getname(temp, zendof(temp), false));
					it.clear();
				}
				wait(Minute / 4);
			}
		}
	}
}

bool creature::unequip(item& it) {
	if(it.iscursed()) {
		static const char* text[] = {
			"������� ����! ��� ���!",
			"���, � ��� �� �����.",
			"��� ���������! ��� ��������!",
		};
		say(maprnd(text));
		it.setidentify(true);
		return false;
	} else if(it.isnatural()) {
		static const char* text[] = {
			"��� ����� ����!",
			"��� � ��� �����? ������?",
			"�� � ����� ���?",
			"��� �������� �� ��� ��������.",
		};
		say(maprnd(text));
		return false;
	}
	pickup(it, false);
	return true;
}

int creature::getlos(unsigned flags) const {
	switch(flags&RangeMask) {
	case You: return 0;
	case Close: return 1;
	case Reach: return 2;
	default: return getlos();
	}
}

bool creature::saving(bool interactive, skill_s save, int bonus) const {
	if(save == NoSkill)
		return false;
	auto chance_save = get(save) + bonus;
	if(chance_save > 0) {
		auto r = d100();
		if(r < chance_save) {
			if(interactive)
				act("%����� �������%�� ������ ��� �����������.");
			return true;
		}
	}
	return false;
}

bool creature::aiboost() {
	auto php = getmaxhits() ? (hp * 100) / getmaxhits() : 0;
	auto pmp = getmaxmana() ? (mp * 100) / getmaxmana() : 0;
	for(auto slot = FirstBackpack; slot <= LastBackpack; slot = (slot_s)(slot + 1)) {
		if(!wears[slot])
			continue;
		if(!wears[slot].isidentified())
			continue;
		switch(wears[slot].getstate()) {
		case RestoreHits:
			if(php < 50) {
				use(wears[slot]);
				return true;
			}
			break;
		case RestoreMana:
			if(pmp < 30) {
				use(wears[slot]);
				return true;
			}
			break;
		}
	}
	return false;
}

bool serialize_party(bool writemode) {
	io::file file("maps/party.dat", writemode ? StreamWrite : StreamRead);
	if(!file)
		return false;
	archive a(file, writemode);
	if(!a.signature("PAR"))
		return false;
	if(!a.version(0, 1))
		return false;
	a.set(current_player);
	a.set(player_data);
	return true;
}

void creature::play() {
	while(true) {
		exit_index = Blocked;
		if(!playturn())
			return;
		addseconds(1);
		if(exit_index != Blocked) {
			serialize_party(true);
			serialize(true);
			auto object = getobject(exit_index);
			if(object == StairsDown) {
				game::create(AreaDungeon, statistic.index, statistic.level + 1);
				serialize_party(false);
			} else {
				game::create(AreaDungeon, statistic.index, statistic.level - 1);
				serialize_party(false);
			}
		}
	}
}

static unsigned source_select(short unsigned* ps, short unsigned* pe, short unsigned index, int los) {
	auto pb = ps;
	auto x0 = game::getx(index);
	auto x2 = x0 + los;
	auto y0 = game::gety(index);
	auto y2 = y0 + los;
	for(auto y = y0 - los; y < y2; y++) {
		if(y < 0 || y >= max_map_y)
			continue;
		for(auto x = x0 - los; x < x2; x++) {
			if(x < 0 || x >= max_map_x)
				continue;
			auto index = game::get(x, y);
			auto obj = game::getobject(index);
			if(!obj)
				continue;
			if(ps < pe)
				*ps++ = index;
		}
	}
	return ps - pb;
}

scene::scene(int los, short unsigned index) {
	// ������� �������
	for(auto& e : player_data) {
		if(!e)
			continue;
		if(distance(e.getposition(), index) > los)
			continue;
		creatures.add(&e);
	}
	// ������� �������
	for(auto& e : creature_data) {
		if(!e)
			continue;
		if(distance(e.getposition(), index) > los)
			continue;
		creatures.add(&e);
	}
	// ������� ������� ������� �����������
	indecies.count = source_select(indecies.data, indecies.endof(), index, los);
}

speech thank_you[];
speech dont_need_this[];

bool creature::give(creature& opponent, item& value, bool interactive) {
	for(auto slot = FirstBackpack; slot <= LastBackpack; slot = (slot_s)(slot + 1)) {
		if(opponent.wears[slot])
			continue;
		opponent.wears[slot] = value;
		value.clear();
		updateweight();
		opponent.updateweight();
		if(interactive) {
			dialog dg(this, &opponent);
			dg.start(thank_you);
		}
		return true;
	}
	if(interactive) {
		dialog dg(this, &opponent);
		dg.start(dont_need_this);
	}
	return false;
}

template<> void archive::set<creature>(creature& e) {
	set(e.race);
	set(e.type);
	set(e.gender);
	set(e.role);
	set(e.direction);
	set(e.name);
	set(e.level);
	set(e.experience);
	set(e.money);
	set(e.position);
	set(e.guard);
	set(e.wears);
	set(e.charmer);
	set(e.horror);
	set(e.abilities);
	set(e.abilities_raise);
	set(e.hp); set(e.mhp); set(e.mp); set(e.mmp);
	set(e.restore_hits);
	set(e.restore_mana);
	set(e.skills);
	set(e.spells);
	set(e.states);
	set(e.recoil);
	set(e.current_site);
	set(e.encumbrance);
}

archive::dataset creature_dataset() {
	return creature_data;
};

archive::dataset player_dataset() {
	return player_data;
};

void creature_serialize(archive& e) {
	e.set(creature_data);
}

static int compare_spells(const void* p1, const void* p2) {
	auto e1 = *((spell_s*)p1);
	auto e2 = *((spell_s*)p2);
	return strcmp(getstr(e1), getstr(e2));
}

item* creature::choose(aref<item*> source, bool interactive, const char* name) const {
	if(!name)
		name = "��������� �������";
	if(interactive)
		return logs::choose(*this, source.data, source.count, name);
	return source.data[rand() % source.count];
}

short unsigned creature::choose(aref<short unsigned> source, bool interactive) const {
	if(source.count == 1)
		return source.data[0];
	if(interactive)
		return logs::choose(*this, source.data, source.count);
	return source.data[rand() % source.count];
}

creature* creature::choose(aref<creature*> source, bool interactive) const {
	if(source.count == 1)
		return source.data[0];
	if(interactive) {
		short unsigned index_data[256];
		for(unsigned i = 0; i < source.count; i++)
			index_data[i] = source.data[i]->position;
		auto pn = logs::choose(*this, index_data, source.count);
		for(unsigned i = 0; i < source.count; i++) {
			if(index_data[i] == pn) {
				return source.data[i];
			}
		}
	}
	return source.data[rand() % source.count];
}

bool logs::choose(creature& e, spell_s& result) {
	unsigned count = 0;
	spell_s source[LastSpell + 1];
	for(auto i = FirstSpell; i <= LastSpell; i = (spell_s)(i + 1)) {
		if(e.get(i) <= 0)
			continue;
		source[count++] = i;
	}
	qsort(source, count, sizeof(source[0]), compare_spells);
	return logs::choose(e, result, {source, count});
}