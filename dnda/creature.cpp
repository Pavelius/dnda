#include "main.h"

using namespace game;

adat<creature*, 6>	players;
static creature*	current_player;
static unsigned	experience_level[] = {
	0, 1000, 2000, 4000, 8000, 16000, 32000, 64000
};
static unsigned	experience_cost[] = {
	5, 10, 20, 30, 50, 80, 100, 150,
};
static char	str_tohit_bonus[] = {
	-5, -4, -2, -1, -1, -1, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 1, 2, 3, 3,
	4, 4, 5, 5, 5, 6
};
static char	str_damage_bonus[] = {
	-5, -4, -2, -1, -1, -1, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 1, 2, 3, 4, 5,
	6, 7, 8, 9, 10, 11
};
static char	dex_tohit_bonus[] = {
	-5, -4, -2, -1, -1, -1, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 1, 2, 3, 4, 4,
	5, 5, 6, 6, 7, 7
};
static char	dex_speed_bonus[] = {
	-4, -3, -2, -1, -1, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 1, 2, 2, 3,
	3, 3, 4, 4, 4
};
static char	int_check_bonus[] = {
	-3, -3, -2, -2, -2, -1, -1, -1, 0, 0,
	0, 0, 0, 0, 0, 1, 1, 2, 3, 3,
	4, 4, 5, 5, 5
};
const int chance_loot = 40;

static struct equipment_info {
	race_s				race;
	class_s				type;
	item_s				equipment[8];
} equipment_data[] = {{Dwarf, Fighter, {AxeBattle, ScaleMail, Shield, Ration}},
{NoRace, Cleric, {Mace, Ration}},
{NoRace, Fighter, {SwordLong, LeatherArmour, Shield, Ration}},
{NoRace, Paladin, {SwordLong, ScaleMail}},
{NoRace, Ranger, {SwordLong, SwordShort, LeatherArmour}},
{NoRace, Theif, {SwordShort, LeatherArmour}},
{NoRace, Mage, {Staff}},
};

static struct class_info {
	const char*			name;
	unsigned char		hp, mp;
	unsigned char		attack;
	ability_s			ability;
	cflags<skill_s>		skills;
	cflags<spell_s>		spells;
	item_s				equipment[8];
} class_data[] = {{"������", 8, 8, 2, Wisdow, {Diplomacy, Healing}, {Bless}, {Mace}},
{"����", 10, 4, 1, Strenght, {Survival, WeaponFocusBlades}, {}, {SwordLong, LeatherArmour, Shield}},
{"���", 4, 10, 4, Intellegence, {Literacy, History}, {Identify, MagicMissile}, {Staff}},
{"�������", 10, 4, 1, Strenght, {Diplomacy, WeaponFocusBlades}, {}, {SwordLong, ScaleMail}},
{"��������", 10, 6, 2, Strenght, {Survival, WeaponFocusBows}, {}, {SwordLong, SwordShort, LeatherArmour}},
{"���", 6, 4, 3, Dexterity, {PickPockets, Lockpicking, HideInShadow, Acrobatics, DisarmTraps, Bluff}, {}, {SwordShort, LeatherArmour}},
};
assert_enum(class, Theif);
getstr_enum(class);

static int roll3d6() {
	return (rand() % 6) + (rand() % 6) + (rand() % 6) + 3;
}

static unsigned char getrang(int value) {
	if(value <= 0)
		return 0;
	else if(value <= 50)
		return 1;
	else if(value <= 80)
		return 2;
	return 3;
}

static void start_equipment(creature& e) {
	for(auto& ei : equipment_data) {
		if((ei.race == e.race || ei.race == NoRace) && ei.type == e.type) {
			for(auto i : ei.equipment) {
				if(!i)
					break;
				e.equip(i);
			}
			e.money += xrand(3, 18);
			return;
		}
	}
}

static void correct_best_ability(creature& e) {
	auto a = class_data[e.type].ability;
	auto b = Strenght;
	for(auto i = Strenght; i <= Charisma; i = (ability_s)(i + 1)) {
		if(e.abilities[i] > e.abilities[b])
			b = i;
	}
	if(a != b)
		iswap(e.abilities[a], e.abilities[b]);
}

static void raise_skills(creature* p, int number) {
	skill_s source[sizeof(p->skills) / sizeof(p->skills[0])];
	auto count = p->get(source);
	if(!count)
		return;
	zshuffle(source, count);
	while(number >= 0) {
		if(count == 0) {
			count = p->get(source);
			zshuffle(source, count);
		}
		if(count > 0)
			p->raise(source[--count]);
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
	// �����������
	for(auto& e : abilities)
		e = roll3d6();
	correct_best_ability(*this);
	for(auto e : game::getskills(race))
		raise(e);
	for(auto e : class_data[type].skills)
		raise(e);
	for(auto e : class_data[type].spells)
		set(e, 1);
	if(class_data[type].spells) {
		for(int i = 0; i < 2; i++)
			set(choose_spells(this), 1);
	}
	// ������� ������
	auto skill_checks = imax(1, (int)maptbl(int_check_bonus, abilities[Intellegence]));
	raise_skills(this, skill_checks);
	// ��������� ����
	mhp = class_data[type].hp; hp = getmaxhits();
	mmp = 0; mp = getmaxmana();
	// ���
	name = game::genname(race, gender);
	//
	start_equipment(*this);
}

void creature::clear() {
	memset(this, 0, sizeof(creature));
	position = 0xFFFF;
	guard = 0xFFFF;
	role = Character;
}

int	creature::getbonus(magic_s value) const {
	auto result = 0;
	for(int i = Head; i < Legs; i++)
		result += wears[i].getbonus(value);
	return result;
}

int	creature::get(skill_s value) const {
	return skills[value];
}

int creature::getarmor() const {
	auto result = wears[Head].getarmor();
	result += wears[Torso].getarmor();
	result += wears[TorsoBack].getarmor();
	result += wears[Elbows].getarmor();
	result += wears[Legs].getarmor();
	result += wears[Melee].getarmor();
	result += wears[OffHand].getarmor();
	result += getbonus(OfDeflection);
	return result;
}

int creature::getdefence() const {
	auto result = wears[Head].getdefence();
	result += wears[Torso].getdefence();
	result += wears[TorsoBack].getdefence();
	result += wears[Elbows].getdefence();
	result += wears[Legs].getdefence();
	result += wears[Melee].getdefence();
	result += wears[OffHand].getdefence();
	result += getbonus(OfDeflection);
	result += get(Acrobatics) / 30; // RULE: Acrobatics raise armor class by +1 for every 30%.
	if(is(Shielded))
		result += 4;
	return result;
}

creature* creature::getleader() const {
	if(charmer)
		return charmer;
	return leader;
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
	sayv(format, xva_start(format));
}

bool creature::sayv(const char* format, const char* param) {
	if(!getplayer()->canhear(position))
		return false;
	logs::driver driver(getname(), gender, 0);
	logs::add("[%1:]\"", getname());
	logs::addv(driver, format, param);
	logs::add("\"");
	return true;
}

bool creature::askyn(creature* opponent, const char* format, ...) {
	if(getplayer() != opponent) {
		sayv(format, xva_start(format));
		return true;
	} else {
		if(!sayv(format, xva_start(format)))
			return false;
		return logs::chooseyn();
	}
}

int	creature::getmaxhits() const {
	if(role==Character)
		return mhp + get(Constitution);
	return mhp;
}

int	creature::getmaxmana() const {
	return mmp + (get(Wisdow) + get(Intellegence)) / 2;
}

int	creature::getmoverecoil() const {
	return 3; // Default speed if 4 hexes in minute.
}

const char* creature::getname() const {
	auto p = getmonstername();
	if(p)
		return p;
	return game::getnamepart(name);
}

char* creature::getfullname(char* temp, bool show_level, bool show_alignment) const {
	zcpy(temp, getname());
	if(show_level)
		szprint(zend(temp), " %1-%3 %2i ������", getstr(type), level, getstr(race));
	return temp;
}

int creature::getdiscount(creature* customer) const {
	// RULE: ����� �������� ������ �������/������� ����� ���������������.
	auto bs = getminimal(Bargaining);
	auto bc = customer->getminimal(Bargaining);
	auto delta = bc - bs;
	return 40 * delta / 100;
}

bool creature::pickup(item value) {
	char temp[260];
	if(value.isforsale()) {
		auto loc = getlocation(position);
		if(loc && loc->owner && loc->owner != this) {
			auto cost = value.getcost();
			cost -= cost * loc->owner->getdiscount(this) / 100;
			if(!loc->owner->askyn(this, "������ ������ �� %1i �����?", value.getcost()))
				return false;
			if(money < cost) {
				static const char* text[] = {
					"������������� ����� ����� ���������� �����.",
					"� ��� ���� ������� ���������� �����.",
					"��� ������� ���������� ����� - ��� ������.",
				};
				loc->owner->say(maprnd(text));
				return false;
			}
			money -= cost;
			value.setsold();
		}
	}
	if(value.gettype() == Coin) {
		act("%����� ������%� %1.", value.getname(temp));
		money += value.getcount();
		return true;
	}
	for(auto& e : backpack) {
		if(e)
			continue;
		e = value;
		act("%����� ������%� %1.", value.getname(temp));
		return true;
	}
	return false;
}

bool creature::dropdown(item& value) {
	char temp[260];
	auto loc = getlocation(position);
	if(loc && loc->owner && loc->owner != this) {
		auto cost = value.getsalecost();
		cost += cost * loc->owner->getdiscount(this) / 100;
		if(cost <= 0) {
			static const char* text[] = {
				"��� ������� � ����� �� ����.",
				"������ �� ���� �������� �����.",
				"������ ��� �� ��������� ����� ���������.",
			};
			loc->owner->say(maprnd(text));
			return false;
		}
		if(!loc->owner->askyn(this, "� ���� ������ ��� �� %1i �����. �� ��������?", cost))
			return false;
		money += cost;
		value.setforsale();
	}
	drop(position, value);
	act("%����� �������%� %1.", value.getname(temp));
	return true;
}

bool creature::equip(item value) {
	if(!value)
		return true;
	for(auto i = Head; i <= Amunitions; i = (slot_s)(i + 1)) {
		if(wears[i])
			continue;
		if(!value.is(i))
			continue;
		wears[i] = value;
		wears[i].set(KnowQuality);
		return true;
	}
	for(auto& e : backpack) {
		if(e)
			continue;
		e = value;
		return true;
	}
	return false;
}

int	creature::getweight() const {
	auto result = 0;
	for(auto& e : wears)
		result += e.getweight();
	for(auto& e : backpack)
		result += e.getweight();
	return result;
}

void creature::wait(int segments) {
	if(!segments)
		segments = getmoverecoil();
	recoil += segments;
}

bool creature::walkaround() {
	if(d100() < 50)
		return false;
	auto d = xrand(Left, RightDown);
	return move(to(position, d));
}

void creature::trapeffect() {
	auto trap = gettrap(position);
	if(!trap)
		return;
	if(isparty())
		game::set(position, Hidden, false);
	if(get(Alertness)) {
		if(roll(Alertness)) {
			if(isparty())
				act("%����� ������� ����%�� �������.");
			return;
		}
	}
	auto ai = ::getattackinfo(trap);
	damage(ai.roll());
}

bool creature::move(short unsigned i) {
	if(i == 0xFFFF)
		return false;
	if(position != 0xFFFF) {
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
	if(isplayer())
		lookfloor();
	trapeffect();
	wait();
	return true;
}

bool creature::moveto(short unsigned index) {
	if(distance(position, index) > 1) {
		makewave(index);
		index = getstepto(position);
		if(index == 0xFFFF)
			return false;
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
	if(enemy && !(*enemy))
		enemy = 0;
	if(horror && !(*horror))
		horror = 0;
	if(getplayer() == this) {
		logs::turn(*this);
		return;
	}
	if(!enemy)
		enemy = getnearest(TargetHostileCreature);
	// ������� ��� � ����������� �� �������
	if(enemy)
		moveto(enemy->position);
	else if(horror)
		moveaway(horror->position);
	else if(guard != 0xFFFF)
		moveto(guard);
	else if(leader) {
		if(distance(leader->position, position) <= 3)
			walkaround();
		else
			moveto(leader->position);
	} else
		walkaround();
}

void creature::setplayer() {
	if(!joinparty())
		return;
	current_player = this;
	for(auto p : players) {
		if(!p)
			continue;
		if(p != current_player)
			p->leader = current_player;
		else
			p->leader = 0;
	}
}

creature* creature::getplayer() {
	return current_player;
}

bool creature::joinparty() {
	if(players.indexof(this) != -1)
		return true;
	if(players.count >= sizeof(players.data) / sizeof(players.data[0]))
		return false;
	players.add(this);
	return true;
}

bool creature::isfriend(const creature* target) const {
	if(!target)
		return false;
	return (getleader() == target || target->getleader() == this);
}

static bool isenemystate(const creature* p1, const creature* p2) {
	if(p1->enemy == p2)
		return true;
	if(p1->role == Shopkeeper)
		return false;
	if(p1->isagressive() && !p2->isagressive())
		return true;
	return false;
}

bool creature::isenemy(const creature* target) const {
	if(!target || target == this)
		return false;
	return isenemystate(this, target) || isenemystate(target, this);
}

bool creature::isplayer() const {
	return this == getplayer();
}

bool creature::isparty() const {
	return players.indexof((creature*)this) != -1;
}

bool creature::manipulate(short unsigned index) {
	switch(getobject(index)) {
	case Door:
		if(!isopen(index)) {
			if(isseal(index))
				say("����� �������.");
			else {
				game::set(index, Opened, true);
				return true;
			}
		} else {
			game::set(index, Opened, false);
			return true;
		}
		break;
	}
	return false;
}

bool creature::interact(short unsigned index) {
	switch(getobject(index)) {
	case Door:
		if(!isopen(index)) {
			if(isseal(index))
				say("����� �������.");
			else
				game::set(index, Opened, true);
			wait();
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
			// ������� � ������ ��������� �� ��������
			wait();
			return true;
		} else if(p->isguard()) {
			static const char* talk[] = {
				"� ������� ��� �����.",
				"����� �� ������.",
				"�� ��������, � �� ������.",
			};
			p->say(maprnd(talk));
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
			p->wait();
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

static void attack(creature* attacker, creature* defender, const attackinfo& ai, int bonus = 0) {
	auto s = d20();
	auto r = s + ai.bonus + bonus - defender->getdefence();
	if(s == 1 || (r < 10)) {
		attacker->act("%����� ��������%�.");
		return;
	}
	bool critical_hit = s >= ai.critical;
	attacker->act(critical_hit ? "%����� ���������� �����%�." : "%����� �����%�.");
	auto damage = ai.roll();
	if(critical_hit)
		damage *= ai.multiplier;
	defender->damage(damage);
}

void creature::damage(int value) {
	hp -= value;
	if(value <= 0)
		act("%����� ������%� ���� �� �����");
	else
		act("%����� �������%� %1i �����", value);
	if(hp <= 0)
		act(" � ����%�");
	act(".");
	if(hp <= 0) {
		for(auto& e : wears) {
			if(!e)
				continue;
			if(isparty() || (d100() < chance_loot)) {
				e.loot();
				drop(position, e);
			}
		}
		for(auto& e : backpack) {
			if(!e)
				continue;
			e.loot();
			drop(position, e);
		}
		game::release(this, getcostexp());
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
				logs::add("��� �������� ��������� %1", getstr(ammo));
				return;
			}
		}
	}
	auto enemy = getnearest(TargetHostileCreature);
	if(!enemy) {
		if(isplayer())
			logs::add("������ ��� ���������� ����", getstr(ammo));
		return;
	}
	if(!enemy->enemy)
		enemy->enemy = this;
	if(wears[Ranged])
		attack(this, enemy, getattackinfo(Ranged));
	wait(getattacktime(Ranged));
	if(ammo)
		wears[Amunitions].setcount(wears[Amunitions].getcount() - 1);
}

void creature::meleeattack(creature* enemy) {
	if(!(*enemy))
		return;
	if(!enemy->enemy)
		enemy->enemy = this;
	if(wears[Melee].is(Melee) && wears[OffHand].is(Melee)) {
		attack(this, enemy, getattackinfo(Melee), -4);
		attack(this, enemy, getattackinfo(OffHand), -6);
	} else if(wears[OffHand].is(Melee))
		attack(this, enemy, getattackinfo(OffHand));
	else
		attack(this, enemy, getattackinfo(Melee));
	wait(getattacktime(Melee));
}

int creature::getobjects(short unsigned* result, unsigned count, target_s target, int range) const {
	int x1 = game::getx(position);
	int y1 = game::gety(position);
	auto pb = result;
	auto pe = result + count;
	for(auto y = y1 - range; y <= y1 + range; y++) {
		if(y < 0 || y >= max_map_y)
			continue;
		for(auto x = x1 - range; x <= x1 + range; x++) {
			if(x < 0 || x >= max_map_x)
				continue;
			auto index = game::get(x, y);
			auto type = game::getobject(index);
			if(type == NoTileObject)
				continue;
			switch(target) {
			case TargetDoor:
				if(type != Door)
					continue;
				break;
			case TargetDoorSealed:
				if(type != Door)
					continue;
				if(!isseal(index))
					continue;
				break;
			}
			if(pb < pe)
				*pb++ = index;
		}
	}
	return pb - result;
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

int creature::getcreatures(creature** result, unsigned count, target_s target, int range) const {
	auto pb = result;
	auto pe = pb + count - 1;
	if(range == -1)
		range = getlos();
	auto x = game::getx(position);
	auto y = game::gety(position);
	for(auto e : creatures) {
		if(e == this)
			continue;
		switch(target) {
		case TargetFriendlyCreature:
			if(!isfriend(e))
				continue;
			break;
		case TargetNotHostileCreature:
			if(isenemy(e))
				continue;
			break;
		case TargetHostileCreature:
			if(!isenemy(e))
				continue;
			if(!linelos(x, y, game::getx(e->position), game::gety(e->position)))
				continue;
			break;
		}
		if(range && game::distance(position, e->position) > range)
			continue;
		if(pb < pe)
			*pb++ = e;
	}
	*pb = 0;
	return pb - result;
}

creature* creature::getnearest(target_s target) const {
	creature* source[64];
	auto count = getcreatures(source, sizeof(source) / sizeof(source[0]), target);
	if(!count)
		return 0;
	return game::getnearest(source, count, position);
}

int	creature::getlos() const {
	if(game::isdungeon())
		return 4;
	return 6;
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

int creature::get(ability_s value) const {
	static magic_s effects[] = {OfStrenght, OfDexterity, OfConstitution, OfIntellegence, OfWisdow, OfCharisma};
	return abilities[value]
		+ getbonus(effects[value]);
}

attackinfo creature::getattackinfo(slot_s slot) const {
	auto attack_per_level = class_data[type].attack;
	if(!attack_per_level)
		attack_per_level = 2;
	attackinfo result;
	result.clear();
	result.bonus = level / attack_per_level;
	auto& weapon = wears[slot];
	if(weapon) {
		if(weapon.is(Slashing))
			result.critical--;
		if(weapon.is(Piercing))
			result.multiplier++;
		result.speed = wears[slot].getspeed() + wears[slot].getbonus(OfSpeed);
		result.damage[0] = wears[slot].getdamagemin();
		result.damage[1] = wears[slot].getdamagemax();
		auto focus = weapon.getfocus();
		if(focus)
			result.damage[1] += get(focus) / 20;
	}
	switch(slot) {
	case Melee:
	case OffHand:
		result.bonus += maptbl(str_tohit_bonus, get(Strenght));
		result.damage[1] += maptbl(str_damage_bonus, get(Strenght));
		break;
	case Ranged:
		result.bonus += maptbl(dex_tohit_bonus, get(Dexterity));
		break;
	}
	return result;
}

static void weapon_information(item weapon, const attackinfo& ai) {
	char t1[260];
	logs::add("%1 � ������� [%2i] ������� [%3i-%4i] �����",
		weapon.getname(t1, false), ai.bonus, ai.damage[0], ai.damage[1]);
}

int creature::getattacktime(slot_s slot) const {
	auto tm = Minute - maptbl(dex_speed_bonus, get(Dexterity)) - getattackinfo(slot).speed;
	if(slot == Melee && wears[Melee].is(Melee) && wears[OffHand].is(Melee))
		tm += Minute / 2 - getattackinfo(OffHand).speed;
	if(tm < Minute / 4)
		tm = Minute / 4;
	return tm;
}

void testweapon(creature& e) {
	e.act("%����� ���������%� ���� ������. ");
	if(e.wears[Melee] && e.wears[OffHand].is(Melee)) {
		e.act("������������ ");
		weapon_information(e.wears[Melee], e.getattackinfo(Melee));
		logs::add(" � ");
		weapon_information(e.wears[OffHand], e.getattackinfo(OffHand));
	} else if(e.wears[OffHand].is(Melee))
		weapon_information(e.wears[OffHand], e.getattackinfo(OffHand));
	else if(e.wears[Melee].is(Melee))
		weapon_information(e.wears[Melee], e.getattackinfo(Melee));
	logs::add(".");
	e.act("��� �������� %1i ������.", e.getattacktime(Melee)*(60 / Minute));
	e.wait(Minute);
}

bool creature::use(short unsigned index) {
	if(index == 0xFFFF)
		return false;
	switch(game::gettile(index)) {
	case Door:
		break;
	}
	return true;
}

void creature::set(state_s value, unsigned segments_count) {
	unsigned stop = segments + segments_count;
	if(states[value] < stop)
		states[value] = stop;
}

bool creature::gettarget(targetinfo& result, const targetdesc td) const {
	if(td.target >= TargetCreature && td.target <= TargetHostileCreature) {
		if(!logs::getcreature(*this, &result.creature, td.target, td.range))
			return false;
	} else if(td.target >= TargetItem && td.target <= TargetItemWeapon) {
		if(!logs::getitem(*this, &result.item, td.target))
			return false;
	} else {
		switch(td.target) {
		case TargetSelf:
			result.creature = (creature*)this;
			break;
		case TargetDoor:
			if(!logs::getindex(*this, result.index, td.target, td.range))
				return false;
			break;
		default:
			return false;
		}
	}
	return true;
}

static item** select_items(item** pb, item** pe, const item* source, unsigned count, target_s target) {
	for(unsigned i = 0; i < count; i++) {
		if(!source[i])
			continue;
		switch(target) {
		case TargetItemUnidentified:
			if(source[i].getidentify() >= KnowEffect)
				continue;
			break;
		case TargetItemWeapon:
			if(!(source[i].is(Melee) || source[i].is(Ranged)))
				continue;
			break;
		}
		if(pb < pe)
			*pb++ = (item*)&source[i];
	}
	return pb;
}

int	creature::getitems(item** result, unsigned maximum_count, target_s target) const {
	auto pb = result;
	auto pe = result + maximum_count;
	pb = select_items(pb, pe, wears, sizeof(wears) / sizeof(wears[0]), target);
	pb = select_items(pb, pe, backpack, sizeof(backpack) / sizeof(backpack[0]), target);
	return pb - result;
}

unsigned getexperiencelevel(unsigned value) {
	if(!value)
		return 1;
	for(int i = 0; i < sizeof(experience_level) / sizeof(experience_level[0]); i++) {
		if(value < experience_level[i])
			return i;
	}
	return sizeof(experience_level) / sizeof(experience_level[0]);
}

int	creature::get(skill_s* source) const {
	auto pb = source;
	for(auto i = Bargaining; i <= TwoWeaponFighting; i = (skill_s)(i + 1)) {
		if(skills[i])
			*pb++ = i;
	}
	return pb - source;
}

void creature::levelup() {
	// RULE: ������� 2-5 ������� ������� � ����������� �� ���������� (� ������� 3)
	auto n = imax(1, (int)maptbl(int_check_bonus, abilities[Intellegence]));
	raise_skills(this, n);
	level++;
}

void creature::addexp(unsigned count) {
	if(!count)
		return;
	if(level == 0)
		return;
	experience += count;
	while(true) {
		if(getexperiencelevel(experience) <= level)
			break;
		levelup();
	}
}

int creature::get(spell_s value) const {
	return (spells[value / 32] & (1 << (value % 32))) ? 1 : 0;
}

void creature::set(spell_s value, int level) {
	if(level)
		spells[value / 32] |= 1 << (value % 32);
	else
		spells[value / 32] &= ~(1 << (value % 32));
}

void creature::passturn(unsigned minutes) {
	wait(minutes*Minute);
	while(recoil > segments) {
		game::turn();
		segments += Minute;
	}
}

void creature::update() {
	if(!restore_hits)
		restore_hits = segments;
	if(!restore_mana)
		restore_mana = segments;
	// RULE: � ������� ��������������� 1 ��� �� 30 �����
	if(segments >= restore_hits) {
		if(hp < getmaxhits())
			hp++;
		restore_hits += imax(5, 40 - get(Constitution))*Minute;
	}
	// RULE: � ������� ��������������� 1 ����� �� 10 �����
	if(segments >= restore_mana) {
		if(mp < getmaxmana())
			mp++;
		restore_mana += imax(5, 50 - get(Intellegence) * 2)*Minute / 3;
	}
}

void creature::lookfloor() {
	item* source[64];
	auto count = game::getitems(source, sizeof(source) / sizeof(source[0]), position);
	if(!count)
		return;
	logs::add("����� ");
	if(count > 1)
		logs::add("����� ");
	else
		logs::add("����� ");
	for(int i = 0; i < count; i++) {
		if(i != 0)
			logs::add(" � ");
		char temp[260];
		logs::add(source[i]->getname(temp, true));
	}
	logs::add(".");
}

bool creature::roll(skill_s skill, int bonus) {
	auto result = getminimal(skill) + bonus;
	if(bonus <= 0)
		return false;
	return d100() < result;
}

void creature::act(const char* format, ...) const {
	auto player = getplayer();
	if(!player)
		return;
	if(!player->canhear(position))
		return;
	logs::driver driver(getname(), gender, 0);
	if(wears[Melee])
		driver.weapon = getstr(wears[Melee].gettype());
	logs::addv(driver, format, xva_start(format));
}