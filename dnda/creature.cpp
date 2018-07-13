#include "archive.h"
#include "main.h"

using namespace game;

const unsigned poison_update = Minute * 4;

static unsigned short		exit_index;
static creature*			current_player;
static adat<creature, 1024>	creature_data;

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
	race_s			race;
	class_s			type;
	item_s			equipment[8];
} equipment_data[] = {{Dwarf, Fighter, {AxeBattle, ScaleMail, Shield, BreadDwarven}},
{NoRace, Cleric, {Mace}},
{NoRace, Fighter, {SwordLong, LeatherArmour, Shield}},
{NoRace, Paladin, {SwordLong, ScaleMail}},
{NoRace, Ranger, {SwordLong, SwordShort, LeatherArmour}},
{NoRace, Theif, {SwordShort, LeatherArmour}},
{NoRace, Mage, {Staff, WandGreen, ScrollRed, ScrollGreen, PotionBlue}},
};

static constexpr struct race_info {
	const char*		name;
	cflags<skill_s>	skills;
	skillvalue		skills_pregen[8];
} race_data[] = {{"���������������"},
{"�������", {Bargaining, Gambling, Swimming}},
{"����", {Smithing, Mining, Athletics}, {{ResistPoison, 30}}},
{"����", {Survival, WeaponFocusBows, Swimming}},
{"����������", {HideInShadow, Acrobatics, Swimming}},
};
assert_enum(race, Halfling);
getstr_enum(race);

static struct class_info {
	const char*		name;
	unsigned char	hp;
	unsigned char	attack;
	ability_s		ability;
	cflags<skill_s>	skills;
	cflags<spell_s>	spells;
} class_data[] = {{"������", 8, 1, Wisdow, {Diplomacy, History, Healing}, {Bless, HealingSpell}},
{"����", 10, 2, Strenght, {Survival, WeaponFocusBlades, WeaponFocusAxes}},
{"���", 4, 1, Intellegence, {Alchemy, Concetration, Literacy}, {Identify, MagicMissile, Sleep}},
{"�������", 10, 2, Strenght, {Diplomacy, Literacy, WeaponFocusBlades}, {DetectEvil}},
{"��������", 10, 2, Strenght, {Survival, WeaponFocusBows}, {}},
{"���", 6, 1, Dexterity, {PickPockets, Lockpicking, HideInShadow, Acrobatics, DisarmTraps, Bluff, Backstabbing}},
};
assert_enum(class, Theif);
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
		if((ei.race == e.race || ei.race == NoRace) && ei.type == e.type) {
			for(auto i : ei.equipment) {
				if(!i)
					break;
				e.equip(i);
			}
			break;
		}
	}
	e.equip(Ration);
	e.money += xrand(3, 18)*GP;
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

void creature::turnbegin() {
	// Set fog of war
	auto max_count = max_map_x * max_map_y;
	for(auto i = 0; i < max_count; i++)
		game::set(i, Visible, false);
	for(auto& e : creature_data) {
		if(!e)
			continue;
		// Remove fog of war from party
		if(e.isparty(current_player))
			e.setlos();
	}
}

bool creature::playturn() {
	turnbegin();
	for(auto& e : creature_data) {
		if(!e)
			continue;
		e.update();
		if(e.recoil > segments)
			continue;
		e.makemove();
		if(e.recoil <= segments)
			e.recoil = segments + 1;
	}
	return current_player != 0;
}

creature* creature::gethenchmen(int index) const {
	for(auto& e : creature_data) {
		if(!e)
			continue;
		if(e.party == this) {
			if(index-- == 0)
				return &e;
		}
	}
	return 0;
}

void creature::select(creature** result, rect rc) {
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
	for(auto& e : creature_data) {
		if(!e)
			continue;
		movements[e.position] = value;
	}
}

creature* creature::getcreature(short unsigned index) {
	if(index == Blocked)
		return 0;
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

bool creature::isbooming() {
	return creature_data.count >= sizeof(creature_data.data) / sizeof(creature_data.data[0]) - 4;
}

void creature::hint(const char* format, ...) const {
	if(!isplayer())
		return;
	logs::driver driver;
	driver.name = getname();
	driver.gender = gender;
	logs::addv(driver, format, xva_start(format));
}

void creature::choosebestability() {
	auto a = class_data[type].ability;
	auto b = Strenght;
	for(auto i = Strenght; i <= Charisma; i = (ability_s)(i + 1)) {
		if(abilities[i] > abilities[b])
			b = i;
	}
	if(a != b)
		iswap(abilities[a], abilities[b]);
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

creature::creature(race_s race, gender_s gender, class_s type) {
	clear();
	this->race = race;
	this->gender = gender;
	this->type = type;
	this->level = 1;
	// �����������
	for(auto& e : abilities)
		e = roll3d6();
	choosebestability();
	for(auto e : race_data[race].skills_pregen)
		skills[e.id] += e.value;
	for(auto e : race_data[race].skills)
		raise(e);
	for(auto e : class_data[type].skills)
		raise(e);
	if(abilities[Intellegence] >= 9)
		raise(Literacy);
	for(auto e : class_data[type].spells)
		set(e, 1);
	// ������� ������
	auto skill_checks = maptbl(int_checks, abilities[Intellegence]);
	raiseskills(skill_checks);
	// ��������� ����
	mhp = class_data[type].hp; hp = getmaxhits();
	mmp = 0; mp = getmaxmana();
	// ���
	name = game::genname(race, gender);
	//
	start_equipment(*this);
	updateweight();
}

void creature::join(creature* value) {
	party = value;
}

void creature::release(unsigned exeperience_cost) const {
	for(auto& e : creature_data) {
		if(!e)
			continue;
		if(e.enemy == this) {
			// RULE: when killed get experience to creature, who want enemy this player
			e.addexp(exeperience_cost);
			e.enemy = 0;
		}
		if(e.horror == this)
			e.horror = 0;
		if(e.charmer == this)
			e.charmer = 0;
	}
	if(isleader()) {
		auto new_leader = gethenchmen(0);
		setleader(party, new_leader);
		if(current_player == this)
			current_player = new_leader;
	}
}

void creature::clear() {
	memset(this, 0, sizeof(creature));
	position = Blocked;
	guard = Blocked;
	order.move = Blocked;
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

creature* creature::getenemy() const {
	if(enemy)
		return enemy;
	auto p = getleader();
	if(p && p->enemy)
		return p->enemy;
	return 0;
}

bool creature::is(state_s value) const {
	return segments <= states[value];
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
	return mhp;
}

int	creature::getmaxmana() const {
	auto result = mmp;
	result += get(Intellegence);
	if(skills[Concetration])
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
			cost -= cost * loc->owner->getdiscount(this) / 100;
			if(interactive) {
				if(!loc->owner->askyn(this, "������ ������ �� %1i �����?", value.getcost()))
					return;
			}
			if(money < cost) {
				static const char* text[] = {
					"������������� ����� ����� ���������� �����.",
					"� ��� ���� ������� ���������� �����.",
					"��� ������� ���������� ����� - ��� ������.",
				};
				if(interactive)
					loc->owner->say(maprnd(text));
				return;
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
	if(slot < FirstBackpack)
		wears[slot].set(KnowQuality);
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
	it.set(KnowEffect);
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

void creature::wait(int segments) {
	if(!recoil)
		recoil = segments;
	recoil += segments;
}

bool creature::walkaround() {
	if(d100() < 40) {
		wait(xrand(1, Minute / 2));
		return false;
	}
	// When we try to stand and think
	if(d100() < 40) {
		creature* creature_data[256];
		auto creatures = getcreatures(creature_data, position, getlos());
		auto skill = aiskill(creatures);
		auto spell = aispell(creatures);
		if(skill && spell) {
			if(d100() < 50)
				skill = NoSkill;
		}
		if(skill) {
			use(skill);
			return false;
		}
		if(spell) {
			use(spell);
			return false;
		}
	}
	// When we try move
	auto skill = aiskill();
	if(skill) {
		use(skill);
		return false;
	}
	auto d = (direction_s)xrand(Left, RightDown);
	return move(to(position, d));
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
		auto n = getdirection({(short)getx(position), (short)gety(position)}, {(short)getx(i), (short)gety(i)});
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
		auto n = getdirection({(short)getx(position), (short)gety(position)}, {(short)getx(i), (short)gety(i)});
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
	current_site = getlocation(position);
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

void creature::makemove() {
	if(order.move == position)
		order.move = Blocked;
	// RULE: sleeped creature don't move
	if(is(Sleeped))
		return;
	if(getplayer() == this) {
		logs::turn(*this);
		return;
	}
	// Make move depends on conditions
	if(horror && distance(horror->position, position) <= (getlos() + 1))
		moveaway(horror->position);
	else if(getenemy())
		moveto(getenemy()->position);
	else if(guard != Blocked)
		moveto(guard);
	else if(!isleader() && getleader()) {
		auto target = getleader();
		if(distance(target->position, position) <= 2)
			walkaround();
		else
			moveto(target->position);
	} else if(order.move != Blocked)
		moveto(order.move);
	else if(order.skill) {
		use(order.skill);
		order.skill = NoSkill;
	} else if(order.spell) {
		use(order.spell);
		order.spell = NoSpell;
	} else
		walkaround();
}

void creature::setleader(const creature* party, creature* leader) {
	if(party == leader)
		return;
	if(party) {
		for(auto& e : creature_data) {
			if(!e)
				continue;
			if(e.party == party)
				e.party = leader;
		}
	}
	if(leader)
		leader->party = leader;
}

creature* creature::getplayer() {
	return current_player;
}

void creature::setplayer() {
	setleader(party, this);
	current_player = this;
}

bool creature::isplayer() const {
	return this == current_player;
}

bool creature::isparty(const creature* value) const {
	return value && getleader()==value->getleader();
}

bool creature::isfriend(const creature* value) const {
	if(!value)
		return false;
	return party == value
		|| charmer == value
		|| value->party == this
		|| value->charmer == this;
}

bool creature::isenemy(const creature* target) const {
	if(!target || target == this)
		return false;
	return getenemy() == target
		|| target->getenemy() == this;
}

void creature::manipulate(short unsigned index) {
	switch(getobject(index)) {
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
	if(!enemy)
		enemy = defender;
	if(!defender->enemy)
		defender->enemy = this;
	auto ai = getattackinfo(slot);
	auto s = d100();
	auto r = s + ai.bonus + bonus - defender->getdefence();
	auto damage = ai.damage;
	if(s == 1 || (r < chance_to_hit)) {
		act("%����� ��������%�.");
		return;
	}
	// RULE: basic chance critical hit is 4% per point
	bool critical_hit = s >= (95 - ai.critical * 4);
	act(critical_hit ? "%����� ���������� �����%�." : "%����� �����%�.");
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
				act("%����� ��������%� ����");
		} else {
			if(interactive)
				act(a.damage, value);
		}
		if(hp <= 0) {
			if(interactive)
				act(a.killed);
		} else {
			if(is(Sleeped)) {
				if(interactive)
					act(" � �������%���");
				remove(Sleeped);
			}
		}
		if(interactive)
			act(".");
		damagewears(value, type);
		if(hp <= 0) {
			const int chance_loot = 40;
			for(auto& e : wears) {
				if(!e)
					continue;
				if(e.isnatural())
					continue;
				if(party == current_player
					|| (&e >= &wears[FirstBackpack])
					|| (d100() < chance_loot)) {
					e.loot();
					drop(position, e);
				}
			}
			release(getcostexp());
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
	return maptbl(experience_cost, level);
}

void creature::rangeattack() {
	auto ammo = wears[Ranged].getammo();
	if(ammo) {
		if(wears[Amunitions].gettype() != ammo) {
			if(isplayer()) {
				logs::add("��� �������� ���������� %1", getstr(ammo));
				return;
			}
		}
	}
	if(!enemy) {
		hint("������ ��� ���������� ����", getstr(ammo));
		return;
	}
	if(wears[Ranged])
		attack(enemy, Ranged);
	wait(getattacktime(Ranged));
	if(ammo)
		wears[Amunitions].setcount(wears[Amunitions].getcount() - 1);
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
	return party;
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
	return tm;
}

void testweapon(creature& e) {
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
		states[value] = segments;
}

void creature::set(state_s value, unsigned segments_count, bool after_recoil, bool can_save) {
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
			unsigned stop = ((after_recoil && recoil) ? recoil : segments) + segments_count;
			if(states[value] < stop) {
				if(can_save) {
				}
				states[value] = stop;
			}
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

void creature::levelup() {
	bool interactive = isplayer();
	if(interactive) {
		logs::add("%1 ������ %2 ������ %3i.", getname(), getstr(type), level + 1);
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
		levelup();
	}
}

int creature::get(spell_s value) const {
	return spells[value];
}

void creature::set(spell_s value, int level) {
	spells[value] = level;
}

void creature::update() {
	// Remove any links if target is invalid
	if(horror && (!is(Scared) || !(*horror)))
		horror = 0;
	if(charmer && (!is(Charmed) || !(*charmer))) {
		remove(Charmed);
		charmer = 0;
	}
	if(enemy && !(*enemy))
		enemy = 0;
	// RULE: Poison affect creature every short time interval.
	if((segments % poison_update) == 0) {
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
	if(segments >= restore_hits) {
		if(hp < getmaxhits()) {
			// RULE: cursed ring of regeneration disable hit point natural recovering
			hp += imax(0, 1 + getbonus(OfRegeneration));
		}
		if(!restore_hits)
			restore_hits = segments;
		// RULE: ring of regeneration increase regeneration time
		restore_hits += imax(5, 40 - (get(Constitution) + getbonus(OfRegeneration) * 2))*Minute;
	}
	// RULE: � ������� ��������������� 1 ����� �� 10 �����
	if(segments >= restore_mana) {
		if(mp < getmaxmana()) {
			// RULE: cursed ring of mana disable mana points natural recovering
			mp += imax(0, 1 + getbonus(OfMana) + get(Concetration) / 50);
		}
		if(!restore_mana)
			restore_mana = segments;
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
		logs::addnc(source[i]->getname(temp, zendof(temp), true));
	}
	logs::add(".");
}

bool creature::roll(skill_s skill, int bonus) {
	auto result = get(skill) + bonus;
	if(result <= 0)
		return false;
	return d100() < result;
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

void creature::consume(int value, bool interactive) {
	auto mmp = getmaxmana();
	mp -= value;
	if(mp > mmp)
		mp = mmp;
	if(mp < 0)
		mp = 0;
}

bool creature::alertness() {
	if(getplayer() != getleader())
		return false;
	// RULE: for party only alertness check secrect doors and traps
	short unsigned source_data[5 * 5];
	auto result = select(source_data, TargetHiddenObject, 2, position);
	auto found = false;
	for(auto index : result) {
		if(roll(Alertness)) {
			found = true;
			game::set(index, Hidden, false);
			auto object = game::getobject(index);
			if(isplayer()) {
				act("�� ���������� %1.", getstr(object));
				logs::next();
			}
			addexp(25);
		}
	}
	return found;
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

bool creature::apply(const effectinfo& effect, int level, bool interactive, const char* format, const char* format_param, int skill_roll, int skill_value, void(*fail_proc)(effectparam& e)) {
	creature* source_data[256];
	auto creatures = getcreatures(source_data, position, getlos());
	effectparam ep(effect, *this, creatures, isplayer());
	ep.level = level;
	ep.skill_roll = skill_roll;
	ep.skill_value = skill_value;
	if(fail_proc) {
		if(!ep.proc.fail)
			ep.proc.fail = fail_proc;
	}
	return ep.apply(format, format_param);
}

void creature::use(item& it) {
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
		it.set(KnowEffect);
		auto spell = it.getspell();
		if(!spell)
			hint("%1 �� ��������.", temp);
		else if(!it.getcharges())
			hint("%1 ��������, �� ����� ��������.", temp);
		else {
			grammar::what(temp, getstr(it.gettype()));
			use(spell, 1 + it.getquality(), "%����� ��������%� ������ %L1.", temp);
			it.setcharges(it.getcharges() - 1);
			if(it && it.getcharges() == 0) {
				hint("%1 ����������� � ����.", it.getname(temp, zendof(temp), false));
				it.clear();
			}
			wait(Minute / 4);
		}
	} else if(it.isreadable()) {
		if(it.istome())
			readbook(it);
		else {
			it.set(KnowEffect);
			auto spell = it.getspell();
			if(spell) {
				use(spell, 1 + it.getquality(), "%����� ��������%� ������.");
				it.clear();
				wait(Minute / 2);
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
		it.set(KnowEffect);
		return false;
	}
	pickup(it, false);
	return true;
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

void creature::play() {
	bool party_killed = false;
	while(true) {
		exit_index = Blocked;
		if(!playturn())
			return;
		segments++;
		// Speed up time if wait too match
		if(current_player->recoil > segments && (current_player->recoil - segments) >= poison_update * 2) {
			unsigned new_segment = ((segments + poison_update - 1) / poison_update) * poison_update;
			while(segments < new_segment) {
				if(!playturn())
					return;
				segments++;
			}
			new_segment = (current_player->recoil / poison_update) * poison_update;
			while(segments < new_segment) {
				if(!playturn())
					return;
				segments += poison_update;
			}
		}
		if(exit_index != Blocked) {
			serialize(true);
			// ���-�� ����������� ������� �� ������ �������
			// �������� ����� ���������� ������ � �������� ��������� �����
		}
	}
}

void creature::remove(adat<creature, 16>& source) const {
	auto p = source.add();
	if(!p)
		return;
	for(auto& e : creature_data) {
		if(!e)
			continue;
		if(e.enemy == this)
			e.enemy = 0;
		if(e.party == this)
			e.party = 0;
		if(e.horror == this)
			e.horror = 0;
		if(e.charmer == this)
			e.charmer = 0;
	}
	*p = *this;
	p->current_site = 0;
}

template<> void archive::set<creature>(creature& e) {
	set(e.race);
	set(e.type);
	set(e.gender);
	set(e.role);
	set(e.direction);
	set(e.abilities);
	set(e.abilities_raise);
	set(e.skills);
	set(e.spells);
	set(e.name);
	set(e.level);
	set(e.hp);
	set(e.mhp);
	set(e.mp);
	set(e.mmp);
	set(e.recoil);
	set(e.restore_hits);
	set(e.restore_mana);
	set(e.experience);
	set(e.money);
	set(e.position);
	set(e.guard);
	set(e.states);
	set(e.wears);
	set(e.encumbrance);
	set(e.charmer);
	set(e.enemy);
	set(e.horror);
	set(e.party);
}

archive::dataset creature_dataset() {
	return creature_data;
};

void creature_serialize(archive& e) {
	e.set(creature_data);
}