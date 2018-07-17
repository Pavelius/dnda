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
{Animal, Cleric, {Mace}},
{Animal, Fighter, {SwordLong, LeatherArmour, Shield}},
{Animal, Paladin, {SwordLong, ScaleMail}},
{Animal, Ranger, {SwordLong, SwordShort, LeatherArmour}},
{Animal, Theif, {SwordShort, LeatherArmour}},
{Animal, Mage, {Staff, Wand2, Scroll1, Scroll2, Potion1}},
};

static struct race_info {
	const char*			name;
	char				ability_minimum[6];
	char				ability_maximum[6];
	adat<skill_s, 4>	skills;
	skillvalue			skills_pregen[8];
} race_data[] = {{"Зверь", {6, 6, 6, 2, 12, 2}, {8, 9, 9, 4, 15, 4}},
//
{"Человек", {9, 9, 9, 9, 9, 9}, {11, 11, 11, 11, 11, 11}, {Bargaining, Gambling, Swimming}},
{"Гном", {11, 6, 13, 9, 9, 9}, {14, 9, 15, 11, 11, 11}, {Smithing, Mining, Athletics}, {{ResistPoison, 30}}},
{"Эльф", {8, 12, 6, 11, 10, 11}, {10, 14, 8, 13, 12, 13}, {Survival, WeaponFocusBows, Swimming}, {{ResistCharm, 40}}},
{"Полурослик", {6, 10, 10, 9, 10, 9}, {8, 13, 11, 11, 12, 11}, {HideInShadow, Acrobatics, Swimming}},
//
{"Гоблин", {5, 12, 8, 6, 9, 6}, {7, 14, 10, 8, 11, 8}, {HideInShadow, Acrobatics, Swimming}},
{"Кобольд", {5, 13, 7, 6, 9, 6}, {7, 15, 9, 8, 11, 8}, {HideInShadow, Acrobatics, Swimming}},
{"Орк", {12, 9, 11, 6, 9, 6}, {15, 11, 13, 8, 11, 8}, {Athletics, Mining, Swimming}},
{"Гнолл", {13, 9, 11, 5, 9, 5}, {16, 11, 13, 7, 11, 7}, {Athletics, Survival, Swimming}},
//
{"Насекомое", {4, 5, 4, 1, 1, 1}, {6, 9, 6, 1, 1, 1}},
{"Мертвец", {10, 6, 10, 4, 1, 1}, {12, 8, 13, 5, 1, 1}},
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
} class_data[] = {{"Крестьянин", 4, 1, 4},
{"Клерик", 8, 2, 1, {0, 0, 0, 0, 2, 1}, {Diplomacy, History, Healing}, {Bless, HealingSpell}},
{"Воин", 10, 3, 1, {2, 0, 1, -2, 0, -1}, {Survival, WeaponFocusBlades, WeaponFocusAxes}},
{"Маг", 4, 1, 1, {-2, 0, 0, 2, 1, 2}, {Alchemy, Concetration, Literacy}, {Armor, MagicMissile, Sleep}},
{"Паладин", 10, 3, 1, {2, 0, 1, 0, 1, 2}, {Diplomacy, Literacy, WeaponFocusBlades}, {DetectEvil}},
{"Следопыт", 10, 2, 1, {0, 2, 1, 0, 1, -1}, {Survival, WeaponFocusBows}, {}},
{"Вор", 6, 1, 1, {0, 2, 0, 0, 0, 1}, {PickPockets, Lockpicking, HideInShadow, Acrobatics, DisarmTraps, Bluff, Backstabbing}},
{"Монстер", 8, 2, 2},
};
assert_enum(class, Monster);
getstr_enum(class);

static struct attack_info {
	const char*		name;
	const char*		damage;
	const char*		killed;
	bool			ignore_armor;
	skill_s			resist;
} attack_data[] = {{"Ударное", "%герой пропустил%а удар на %1i хитов", "и упал%а", false, NoSkill},
{"Режущее", "%герой был%а ранен%а на %1i хитов", "и упал%а на землю", false, NoSkill},
{"Колющее", "%герой получил%а рану на %1i хитов", "и упал%а на землю", false, NoSkill},
//
{"Кислота", "%героя обдало кислотой на %1i хитов", "и %она упал%а", false, NoSkill},
{"Холод", "%героя обдало холодом на %1i хитов", "и %она замерзл%а до смерти", true, ResistCold},
{"Электричество", "%героя поразил электрический разряд на %1i хитов", "и дергаясь в конвульсиях %она упал%а", false, ResistElectricity},
{"Огонь", "%героя обдало огнем на %1i хитов", "и %она превратил%ась в обугленный труп", false, ResistFire},
{"Магия", "%героя поразил сгусток энергии на %1i хитов", "и %она упал%а", true, NoSkill},
{"Яд", "%герой страдает от яда на %1i хитов", "и умирает", true, ResistPoison},
{"Вода", "%герой нахлебал%ась воды на %1i хитов", "и захлебнул%ась", false},
};
assert_enum(attack, WaterAttack);
getstr_enum(attack);

static struct material_info {
	const char*		name;
} material_data[] = {{"Бумага"},
{"Стекло"},
{"Железо"},
{"Кожа"},
{"Органика"},
{"Камень"},
{"Дерево"},
};
assert_enum(material, Wood);
getstr_enum(material);

int mget(int ox, int oy, int mx, int my);

void manual_skill_race(stringbuffer& sc, manual& me) {
	for(auto& e : race_data) {
		if(!e.skills.is(me.value.skill))
			continue;
		sc.header("[Расы]: ");
		sc.add(e.name);
	}
	sc.trail(".");
}

void manual_skill_class(stringbuffer& sc, manual& me) {
	for(auto& e : class_data) {
		if(!e.skills.is(me.value.skill))
			continue;
		sc.header("[Классы]: ");
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
			for(auto i : ei.equipment) {
				if(!i)
					break;
				e.equip(i);
			}
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

creature::creature(race_s race, gender_s gender, class_s type) {
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
	// Повысим навыки
	auto skill_checks = maptbl(int_checks, abilities[Intellegence]);
	raiseskills(skill_checks);
	// Восполним хиты
	mhp = class_data[type].hp; hp = getmaxhits();
	mmp = 0; mp = getmaxmana();
	// Имя
	name = game::genname(race, gender);
	//
	start_equipment(*this);
	updateweight();
}

void creature::join(creature* value) {
	party = value;
}

void creature::release() const {
	for(auto& e : creature_data) {
		if(!e)
			continue;
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
		szprints(zend(result), result_maximum, " %1-%3 %2i уровня", getstr(type), level, getstr(race));
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
				if(!loc->owner->askyn(this, "Хотите купить за %1i монет?", value.getcost()))
					return;
			}
			if(money < cost) {
				static const char* text[] = {
					"Возвращайтесь когда будет достаточно денег.",
					"У вас нету нужного количества денег.",
					"Нет нужного количества денег - нет товара.",
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
			act("%герой собрал%а %1.", value.getname(temp, zendof(temp)));
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
			act("%герой поднял%а %1.", value.getname(temp, zendof(temp)));
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
				"Эту дешевку я брать не буду.",
				"Только не надо мусорить здесь.",
				"Выкинь это, но только за пределами моего заведения.",
			};
			loc->owner->say(maprnd(text));
			return;
		}
		if(!loc->owner->askyn(this, "Я могу купить это за %1i монет. Ты согласен?", cost))
			return;
		money += cost;
		value.setforsale();
	}
	act("%герой положил%а %1.", value.getname(temp, zendof(temp)));
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

creature* creature::getnearest(aref<creature*> source, targetdesc ti) const {
	if(!source)
		return 0;
	creature* creature_result[260];
	auto result = select(creature_result, source, ti.target, ti.range, position, this);
	return game::getnearest(result, position);
}

bool creature::walkaround(aref<creature*> creatures) {
	if(d100() < 40) {
		wait(xrand(1, Minute / 2));
		return false;
	}
	// When we try to stand and think
	if(d100() < 40) {
		auto skill = aiskill(creatures);
		if(skill && d100() < 60) {
			use(skill);
			return false;
		}
		auto spell = aispell(creatures);
		if(spell && d100() < 60) {
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
		hint("%герой успешно обош%ла ловушку.");
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
				static const char* talk[] = {"Простите.",
					"Извините.",
					"Ой! Не туда...",
					"Да чтоб тебя!",
					"Хик!"
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
		// Продавец магазина не может покидать магазин
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

creature* creature::getenemy(aref<creature*> source) const {
	return getnearest(source, {TargetHostile});
}

void creature::makemove() {
	// RULE: sleeped or paralized creature don't move
	if(is(Sleeped) || is(Paralized))
		return;
	// Player turn
	if(getplayer() == this) {
		logs::turn(*this);
		return;
	}
	// Make move depends on conditions
	if(horror && distance(horror->position, position) <= (getlos() + 1)) {
		moveaway(horror->position);
		return;
	}
	// Test any enemy
	creature* creature_data[256];
	auto creatures = getcreatures(creature_data, position, getlos());
	auto enemy = getenemy(creatures);
	if(enemy) {
		if(aiboost())
			return;
		if(aiusewand(creatures, TargetHostile))
			return;
		auto spell = aispell({&enemy, 1}, TargetHostile);
		if(spell)
			use(spell);
		else if(isranged(false))
			rangeattack(enemy);
		else
			moveto(enemy->position);
	} else if(guard != Blocked)
		moveto(guard);
	else if(!isleader() && getleader()) {
		auto target = getleader();
		if(distance(target->position, position) <= 2)
			walkaround(creatures);
		else
			moveto(target->position);
	} else
		walkaround(creatures);
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
	return value && getleader() == value->getleader();
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
				say("Здесь заперто.");
			else
				game::set(index, Opened, true);
		} else
			game::set(index, Opened, false);
		wait(Minute / 4);
		break;
	case StairsUp:
	case StairsDown:
		if(isplayer()) {
			logs::add("Вы действительно хотите %1",
				(a == StairsDown) ? "спуститься вниз" : "подняться наверх");
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
				say("Здесь заперто.");
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
			// МонстрI и другие персонажи не меняются
			wait(xrand(2, 8));
			return true;
		} else if(p->isguard()) {
			static const char* talk[] = {
				"Я охраняю это место.",
				"Здесь не пройти.",
				"Не толкайся, я не отойду.",
			};
			p->say(maprnd(talk));
			wait(xrand(1, 4));
			return true;
		} else {
			// Игрок меняется позицией с неигроком
			auto loc = getlocation(p->position);
			if(loc && loc->owner == p) {
				// С владельцами области не поменяешся местами
				static const char* talk[] = {
					"Не толкай меня в моем заведении.",
					"Держи руки на виду.",
					"Я за тобой слежу",
				};
				p->say(maprnd(talk));
				return true;
			}
			p->position = position;
			p->wait(xrand(1, 4));
			if(d100() < 50) {
				static const char* talk[] = {
					"Эй! Не толкайся.",
					"Давай, проходи.",
					"Куда ты так спешишь?",
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
			actnc("%герой кинул%а %1 и", getstr(wears[slot].gettype()));
		else
			actnc("%герой стрельнул%а из %1 и", getstr(wears[slot].gettype()));
	} else
		actnc("%герой");
	if(s < 5 || (r < chance_to_hit)) {
		act("промазал%а.");
		return;
	}
	auto damage = ai.damage;
	// RULE: basic chance critical hit is 4% per point
	bool critical_hit = s >= (95 - ai.critical * 4);
	act(critical_hit ? "критически попал%а." : "попал%а.");
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
					wears[slot].act("%герой сломал%ась.");
					wears[slot].clear();
				} else
					wears[slot].act("%герой треснул%а.");
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
			defender->set(Paralized, Minute, true);
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
				actnc("%герой выдержал%а удар");
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
					actnc(" и проснул%ась");
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
				if(party == current_player
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
				act("%герой восстановил%а %1i хитов.", value);
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
	} attack_damage_data[] = {{Fire, Paper, 55, "%герой сгорел%а дотла.", &item::clear},
	{Fire, Iron, 15, "%герой расплавил%ась дотла.", &item::clear},
	{Fire, Wood, 40, "%герой сгорел%а в ярком пламени.", &item::clear},
	{Fire, Leather, 30, "%герой частично сгорел%а.", &item::damage},
	{Acid, Iron, 80, "%герой разъело кислотой.", &item::damage},
	{Acid, Leather, 70, "%герой разъело кислотой.", &item::damage},
	{Acid, Wood, 60, "%герой разъело кислотой.", &item::damage},
	{WaterAttack, Iron, 50, "%герой заржавел%а.", &item::damage},
	{WaterAttack, Paper, 60, "%герой промок%ла насквозь.", &item::clear},
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
				hint("Для стрельбы необходимо %1", getstr(ammo));
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
	return party;
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
	player.act("%1 с шансом [%2i%%] наносит [%3i-%4i] урона",
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
	e.act("%герой поробывал%а свое оружие.");
	if(e.wears[Melee] && e.wears[OffHand].is(Melee)) {
		e.act("Одновременно ");
		weapon_information(e, e.wears[Melee], e.getattackinfo(Melee));
		e.act(" и ");
		weapon_information(e, e.wears[OffHand], e.getattackinfo(OffHand));
	} else if(e.wears[OffHand].is(Melee))
		weapon_information(e, e.wears[OffHand], e.getattackinfo(OffHand));
	else if(e.wears[Melee].is(Melee))
		weapon_information(e, e.wears[Melee], e.getattackinfo(Melee));
	logs::add(".");
	e.act("Это занимает %1i секунд.", e.getattacktime(Melee)*(60 / Minute));
	if(e.wears[Ranged]) {
		weapon_information(e, e.wears[Ranged], e.getattackinfo(Ranged));
		e.act("Это занимает %1i секунд.", e.getattacktime(Ranged)*(60 / Minute));
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

void creature::set(state_s value, unsigned segments_count, bool after_recoil) {
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
				logs::add("Ваш атрибут %1 вырос до %2i.", getstr(i), abilities[i]);
				logs::next();
			}
		}
	}
}

void creature::levelup() {
	bool interactive = isplayer();
	if(interactive) {
		logs::add("%1 теперь %2 уровня [%3i].", getname(), getstr(type), level + 1);
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
	if(horror && (!is(Scared) || !(*horror))) {
		remove(Scared);
		horror = 0;
	}
	if(charmer && (!is(Charmed) || !(*charmer))) {
		remove(Charmed);
		charmer = 0;
	}
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
	// RULE: В среднем восстанавливаем 1 хит за 30 минут
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
	// RULE: В среднем восстанавливаем 1 манну за 10 минут
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
	logs::addnc("Здесь ");
	if(count > 1)
		logs::addnc("лежат ");
	else
		logs::addnc("лежит ");
	for(int i = 0; i < count; i++) {
		if(i != 0)
			logs::addnc(" и ");
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
				act("Вы обнаружили %1.", getstr(object));
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
		act("%герой съел%а %L1.", temp);
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
				hint("Вы почувствовали себя %1", getname(get_ability_state(i), false));
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
			hint("%1 не работает.", temp);
		else if(!it.getcharges())
			hint("%1 раряжена, ее нужно зарядить.", temp);
		else {
			grammar::what(temp, getstr(it.gettype()));
			use(spell, 1 + it.getquality(), "%герой выставил%а вперед %L1.", temp);
			it.setcharges(it.getcharges() - 1);
			if(it && it.getcharges() == 0) {
				hint("%1 рассыпалась в прах.", it.getname(temp, zendof(temp), false));
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
				use(spell, 1 + it.getquality(), "%герой прочитал%а свиток.");
				it.clear();
				wait(Minute / 2);
			}
		}
	}
}

bool creature::unequip(item& it) {
	if(it.iscursed()) {
		static const char* text[] = {
			"Уберите руки! Это мое!",
			"Нет, я это не отдам.",
			"Мое сокровище! Моя прелесть!",
		};
		say(maprnd(text));
		it.set(KnowEffect);
		return false;
	} else if(it.isnatural()) {
		static const char* text[] = {
			"Это часть меня!",
			"Как я это сниму? Отрежу?",
			"Ты в своем уме?",
			"Это приросло ко мне намертво.",
		};
		say(maprnd(text));
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
				act("%герой перенес%ла эффект без последствий.");
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
		if(wears[slot].getidentify() < KnowEffect)
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
			serializep(statistic.positions[0], true);
			serialize(true);
			auto object = getobject(exit_index);
			if(object == StairsDown) {
				create("dungeon", statistic.index, statistic.level + 1);
				serializep(statistic.positions[1], false);
			} else {
				create("dungeon", statistic.index, statistic.level - 1);
				serializep(statistic.positions[0], false);
			}
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
	set(e.party);
	set(e.current_site);
	set(e.encumbrance);
}

archive::dataset creature_dataset() {
	return creature_data;
};

void creature_serialize(archive& e) {
	e.set(creature_data);
}

bool game::serializep(short unsigned index, bool writemode) {
	io::file file("maps/party.dat", writemode ? StreamWrite : StreamRead);
	if(!file)
		return false;
	archive a(file, writemode);
	if(!a.signature("PAR"))
		return false;
	if(!a.version(0, 1))
		return false;
	adat<creature, 16> party;
	adat<creature*, 16> party_reference;
	if(writemode) {
		auto p = creature::getplayer();
		if(!p)
			return false;
		for(auto& e : creature_data) {
			if(!e.ishenchman(p))
				continue;
			party_reference.add(&e);
		}
		for(auto p : party_reference) {
			p->remove(party);
			p->clear();
		}
		a.set(party);
	} else {
		a.set(party);
		for(auto& e : party) {
			auto p = new creature();
			*p = e;
			party_reference.add(p);
		}
		for(auto p : party_reference)
			p->join(party_reference[0]);
		party_reference[0]->setplayer();
		for(auto p : party_reference)
			p->setposition(getfree(index));
	}
	return true;
}