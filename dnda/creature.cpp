#include "archive.h"
#include "main.h"

using namespace game;

const int chance_loot = 40;
const unsigned poison_update = Minute * 5;

static creature*			current_player;
static adat<creature, 1024>	creature_data;
static unsigned	experience_level[] = {0,
1000, 2000, 4000, 8000, 16000, 32000, 64000
};
static unsigned	experience_cost[] = {5,
10, 20, 30, 50, 80, 100, 150,
};
static char	str_tohit_bonus[] = {-5,
-4, -2, -1, -1, -1, 0, 0, 0, 0,
0, 0, 0, 0, 0, 0, 1, 2, 3, 3,
4, 4, 5, 5, 5, 6
};
static char	str_damage_bonus[] = {-5,
-4, -2, -1, -1, 0, 0, 0, 0, 0,
0, 0, 0, 0, 0, 1, 2, 3, 4, 5,
6, 7, 8, 9, 10, 11
};
static char	dex_tohit_bonus[] = {-5,
-4, -2, -1, -1, -1, 0, 0, 0, 0,
0, 0, 0, 0, 0, 1, 2, 3, 4, 4,
5, 5, 6, 6, 7, 7
};
static char	dex_speed_bonus[] = {-4,
-3, -2, -1, -1, 0, 0, 0, 0, 0,
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
	item_s				equipment[8];
} equipment_data[] = {{Dwarf, Fighter, {AxeBattle, ScaleMail, Shield, BreadDwarven}},
{NoRace, Cleric, {Mace}},
{NoRace, Fighter, {SwordLong, LeatherArmour, Shield}},
{NoRace, Paladin, {SwordLong, ScaleMail}},
{NoRace, Ranger, {SwordLong, SwordShort, LeatherArmour}},
{NoRace, Theif, {SwordShort, LeatherArmour}},
{NoRace, Mage, {Staff, WandGreen, ScrollRed, ScrollGreen, ScrollBlue, PotionBlue}},
};

static constexpr struct race_info {
	const char*		name;
	cflags<skill_s>	skills;
	skillvalue		skills_pregen[8];
} race_data[] = {{"Человекооразный"},
{"Человек", {Bargaining, Gambling, Swimming}},
{"Гном", {Smithing, Mining, Athletics}, {{ResistPoison, 30}}},
{"Эльф", {Survival, WeaponFocusBows, Swimming}},
{"Полурослик", {HideInShadow, Acrobatics, Swimming}},
};
assert_enum(race, Halfling);
getstr_enum(race);

static struct class_info {
	const char*			name;
	unsigned char		hp, mp;
	unsigned char		attack;
	ability_s			ability;
	cflags<skill_s>		skills;
	cflags<spell_s>		spells;
} class_data[] = {{"Клерик", 8, 8, 2, Wisdow, {Diplomacy, History, Healing}, {Bless, HealingSpell}},
{"Воин", 10, 4, 1, Strenght, {Survival, WeaponFocusBlades, WeaponFocusAxes}},
{"Маг", 4, 10, 4, Intellegence, {Alchemy, Concetration, Literacy}, {Identify, MagicMissile, Sleep}},
{"Паладин", 10, 4, 1, Strenght, {Diplomacy, Literacy, WeaponFocusBlades}, {DetectEvil}},
{"Следопыт", 10, 6, 2, Strenght, {Survival, WeaponFocusBows}, {}},
{"Вор", 6, 4, 3, Dexterity, {PickPockets, Lockpicking, HideInShadow, Acrobatics, DisarmTraps, Bluff}},
};
assert_enum(class, Theif);
getstr_enum(class);

static struct attack_info {
	const char*	name;
	const char*	damage;
	const char*	killed;
	bool		ignore_armor;
	skill_s		resist;
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
	const char*			name;
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
	e.money += xrand(3, 18);
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

void creature::playturn() {
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
		if(!game::isvisible(e.position))
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

bool creature::isbooming() {
	return creature_data.count >= sizeof(creature_data.data) / sizeof(creature_data.data[0]);
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
	auto count = pb - source;
	if(!count)
		return;
	zshuffle(source, count);
	auto index = 0;
	while(number >= 0) {
		if(index >= count)
			index = 0;
		raise(source[index++]);
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
	// Способности
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
	result += wears[Torso].getdefence() + wears[Torso].getquality();
	result += wears[TorsoBack].getdefence();
	result += wears[Elbows].getdefence();
	result += wears[Legs].getdefence();
	result += wears[Melee].getdefence();
	result += wears[OffHand].getdefence() + (wears[OffHand].isarmor() ? wears[Torso].getquality() : 0);
	result += wears[LeftFinger].getbonus(OfDefence);
	result += wears[RightFinger].getbonus(OfDefence);
	result += get(Acrobatics) / 30; // RULE: Acrobatics raise armor class by +1 for every 30%.
	if(is(Shielded))
		result += 6;
	if(is(Armored))
		result += 2;
	// RULE: Heavy encumbrace level apply to defence
	if(is(HeavilyEncumbered))
		result -= 4;
	return result;
}

creature* creature::getparty() const {
	if(charmer && is(Charmed))
		return charmer;
	return party;
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
	logs::driver driver;
	driver.name = getname();
	driver.gender = gender;
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
	if(role == Character)
		return mhp + get(Constitution);
	return mhp;
}

int	creature::getmaxmana() const {
	auto result = mmp;
	result += (get(Wisdow) + get(Intellegence)) / 2;
	result += getbasic(Concetration) / 5;
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
	// RULE: Навык торговли делает продажу/покупку более привлекательной.
	auto bs = get(Bargaining);
	auto bc = customer->get(Bargaining);
	auto delta = bc - bs;
	return 40 * delta / 100;
}

bool creature::pickup(item value, bool interactive) {
	char temp[260];
	if(value.isforsale()) {
		auto loc = getlocation(position);
		if(loc && loc->owner && loc->owner != this) {
			auto cost = value.getcost();
			cost -= cost * loc->owner->getdiscount(this) / 100;
			if(interactive) {
				if(!loc->owner->askyn(this, "Хотите купить за %1i монет?", value.getcost()))
					return false;
			}
			if(money < cost) {
				static const char* text[] = {
					"Возвращайтесь когда будет достаточно денег.",
					"У вас нету нужного количества денег.",
					"Нет нужного количества денег - нет товара.",
				};
				if(interactive)
					loc->owner->say(maprnd(text));
				return false;
			}
			money -= cost;
			value.setsold();
		}
	}
	if(value.gettype() == Coin) {
		if(interactive)
			act("%герой собрал%а %1.", value.getname(temp, zendof(temp)));
		money += value.getcount();
		updateweight();
		return true;
	}
	for(auto slot = FirstBackpack; slot <= LastBackpack; slot = (slot_s)(slot + 1)) {
		if(wears[slot])
			continue;
		wears[slot] = value;
		if(interactive)
			act("%герой поднял%а %1.", value.getname(temp, zendof(temp)));
		updateweight();
		return true;
	}
	return false;
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
	auto itc = value;
	value.clear();
	drop(position, itc);
	act("%герой положил%а %1.", value.getname(temp, zendof(temp)));
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
	recoil += segments;
}

bool creature::walkaround() {
	if(d100() < 40)
		return false;
	auto d = xrand(Left, RightDown);
	return move(to(position, d));
}

void creature::trapeffect() {
	auto trap = gettrap(position);
	if(!trap)
		return;
	if(isparty(current_player))
		game::set(position, Hidden, false);
	if(get(Alertness)) {
		if(roll(Alertness)) {
			hint("%герой успешно обош%ла ловушку.");
			return;
		}
	}
	auto a = game::getattackinfo(trap);
	damage(a.damage.roll(), a.damage.type, true);
}

bool creature::move(short unsigned i) {
	if(i == Blocked)
		return false;
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
	// RULE: sleeped creature don't move
	if(is(Sleeped))
		return;
	if(getplayer() == this) {
		logs::turn(*this);
		return;
	}
	if(!enemy)
		enemy = getnearest({TargetHostile});
	// Make move depends on conditions
	if(horror)
		moveaway(horror->position);
	else if(enemy)
		moveto(enemy->position);
	else if(guard != Blocked)
		moveto(guard);
	else if(charmer) {
		if(distance(charmer->position, position) <= 2)
			walkaround();
		else
			moveto(charmer->position);
	} else if(party && !isleader()) {
		if(distance(party->position, position) <= 3)
			walkaround();
		else
			moveto(party->position);
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
	return party == value;
}

bool creature::isfriend(const creature* value) const {
	if(!value)
		return false;
	return party == value
		|| charmer == value
		|| value->party == this
		|| value->charmer == this;
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

void creature::manipulate(short unsigned index) {
	switch(getobject(index)) {
	case Door:
		if(!isopen(index)) {
			if(isseal(index))
				say("Здесь заперто.");
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
		if(!isopen(index)) {
			if(isseal(index))
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
			// Монстры и другие персонажи не меняются
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

static void attack(creature* attacker, creature* defender, const attackinfo& ai, int bonus = 0) {
	auto s = d20();
	auto r = s + ai.bonus + bonus - defender->getdefence();
	if(s == 1 || (r < 10)) {
		attacker->act("%герой промазал%а.");
		return;
	}
	bool critical_hit = s >= (20 - ai.critical);
	attacker->act(critical_hit ? "%герой критически попал%а." : "%герой попал%а.");
	auto damage = ai.damage;
	if(critical_hit) {
		auto step = imax(damage.max, damage.min) - damage.min;
		if(step < 2)
			step = 2;
		if(step > 10)
			step = 10;
		for(auto i = ai.multiplier; i > 0; i--)
			damage.max += step;
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
		attacker->heal(-(xrand(1, 4) + ai.quality), false);
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
				act("%герой выдержал%а удар");
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
					act(" и проснул%ась");
				remove(Sleeped);
			}
		}
		if(interactive)
			act(".");
		damagewears(value, type);
		if(hp <= 0) {
			for(auto& e : wears) {
				if(!e)
					continue;
				if(party == current_player || (d100() < chance_loot)) {
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
	return maptbl(experience_cost, level);
}

void creature::rangeattack() {
	auto ammo = wears[Ranged].getammo();
	if(ammo) {
		if(wears[Amunitions].gettype() != ammo) {
			if(isplayer()) {
				logs::add("Для стрельбы необходим %1", getstr(ammo));
				return;
			}
		}
	}
	auto enemy = getnearest({TargetHostile});
	if(!enemy) {
		if(isplayer())
			logs::add("Вокруг нет подходящей цели", getstr(ammo));
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

unsigned creature::getobjects(aref<short unsigned> result, targetdesc ti) const {
	int x1 = game::getx(position);
	int y1 = game::gety(position);
	auto pb = result.data;
	auto pe = pb + result.count;
	for(auto y = y1 - ti.range; y <= y1 + ti.range; y++) {
		if(y < 0 || y >= max_map_y)
			continue;
		for(auto x = x1 - ti.range; x <= x1 + ti.range; x++) {
			if(x < 0 || x >= max_map_x)
				continue;
			auto index = game::get(x, y);
			auto type = game::getobject(index);
			if(type == NoTileObject)
				continue;
			switch(ti.target) {
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
	return pb - result.data;
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

unsigned creature::getcreatures(aref<creature*> result, targetdesc ti, short unsigned position, const creature* player, const creature* exclude) {
	auto pb = result.data;
	auto pe = pb + result.count;
	auto x = game::getx(position);
	auto y = game::gety(position);
	for(auto& e : creature_data) {
		if(!e)
			continue;
		if(&e == exclude)
			continue;
		switch(ti.target) {
		case TargetFriendly:
		case TargetFriendlySelf:
			if(e.isenemy(player))
				continue;
			break;
		case TargetHostile:
			if(!e.isenemy(player))
				continue;
			break;
		}
		if(game::distance(position, e.position) > ti.range)
			continue;
		if(!linelos(x, y, game::getx(e.position), game::gety(e.position)))
			continue;
		if(pb < pe) {
			*pb++ = &e;
			if(pb >= pe)
				break;
		}
	}
	return pb - result.data;
}

unsigned creature::getcreatures(aref<creature*> result, targetdesc ti) const {
	if(!ti.range)
		ti.range = getlos();
	const creature* exclude = 0;
	if(ti.target == TargetFriendlySelf)
		exclude = this;
	return getcreatures(result, ti, position, this, exclude);
}

creature* creature::getnearest(targetdesc ti) const {
	creature* source[128];
	auto count = getcreatures(source, ti);
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
		attack_per_level = 2;
	attackinfo result = {};
	result.bonus = level / attack_per_level;
	auto& weapon = wears[slot];
	if(weapon) {
		wears[slot].get(result);
		auto focus = weapon.getfocus();
		if(focus && getbasic(focus)) {
			auto fs = get(focus);
			result.bonus += fs / 30;
			result.damage.max += fs / 40;
		}
	} else
		result.damage.max = 2;
	switch(slot) {
	case Melee:
	case OffHand:
		result.bonus += maptbl(str_tohit_bonus, get(Strenght));
		result.damage.max += maptbl(str_damage_bonus, get(Strenght));
		break;
	case Ranged:
		result.bonus += maptbl(dex_tohit_bonus, get(Dexterity));
		break;
	}
	if(is(Blessed)) {
		result.bonus += 2;
		result.damage.max++;
	}
	// RULE: Heavy encumbrace level apply to attack
	if(is(HeavilyEncumbered))
		result.bonus -= 4;
	return result;
}

static void weapon_information(item weapon, const attackinfo& ai) {
	char t1[260];
	logs::add("%1 с бонусом [%2i] наносит [%3i-%4i] урона",
		weapon.getname(t1, zendof(t1), false), ai.bonus, ai.damage.min, ai.damage.max);
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
	e.act("%герой поробывал%а свое оружие. ");
	if(e.wears[Melee] && e.wears[OffHand].is(Melee)) {
		e.act("Одновременно ");
		weapon_information(e.wears[Melee], e.getattackinfo(Melee));
		logs::add(" и ");
		weapon_information(e.wears[OffHand], e.getattackinfo(OffHand));
	} else if(e.wears[OffHand].is(Melee))
		weapon_information(e.wears[OffHand], e.getattackinfo(OffHand));
	else if(e.wears[Melee].is(Melee))
		weapon_information(e.wears[Melee], e.getattackinfo(Melee));
	logs::add(".");
	e.act("Это занимает %1i секунд.", e.getattacktime(Melee)*(60 / Minute));
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

void creature::set(state_s value, unsigned segments_count) {
	switch(value) {
	case HealState:
		heal(xrand(10, 20), false);
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
			unsigned stop = segments + segments_count;
			if(states[value] < stop)
				states[value] = stop;
		}
		break;
	}
}

bool creature::gettarget(targetinfo& result, const targetdesc ti) const {
	if(ti.target >= TargetFriendly && ti.target <= TargetHostile) {
		if(!logs::getcreature(*this, &result.cre, ti))
			return false;
	} else if(ti.target >= TargetItem && ti.target <= TargetItemChargeable) {
		if(!logs::getitem(*this, &result.itm, ti))
			return false;
	} else {
		switch(ti.target) {
		case TargetSelf:
			result.cre = (creature*)this;
			break;
		case TargetDoor:
		case TargetDoorSealed:
			if(!logs::getindex(*this, result.pos, ti))
				return false;
			break;
		case TargetInvertory:
			// All invertory of caster
			break;
		default:
			return false;
		}
	}
	return true;
}

static item** select_items(item** pb, item** pe, const item* source, unsigned count, target_s target) {
	return pb;
}

unsigned creature::getitems(aref<item*> result, targetdesc ti) const {
	auto pb = result.data;
	auto pe = result.data + result.count;
	for(auto& it : wears) {
		if(!it)
			continue;
		switch(ti.target) {
		case TargetItemUnidentified:
			if(it.getidentify() >= KnowEffect)
				continue;
			break;
		case TargetItemWeapon:
			if(!(it.is(Melee) || it.is(Ranged)))
				continue;
			break;
		case TargetItemChargeable:
			if(!it.ischargeable())
				continue;
			break;
		case TargetItemReadable:
			if(!it.isreadable())
				continue;
			break;
		case TargetItemDrinkable:
			if(!it.isdrinkable())
				continue;
			break;
		case TargetItemEdible:
			if(!it.isedible())
				continue;
			break;
		}
		if(pb < pe)
			*pb++ = (item*)&it;
	}
	return pb - result.data;
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

void creature::levelup() {
	auto n = maptbl(int_checks, abilities[Intellegence]);
	raiseskills(n);
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
	return spells[value];
}

void creature::set(spell_s value, int level) {
	spells[value] = level;
}

void creature::passturn(unsigned minutes) {
	wait(minutes*Minute);
	while(recoil > segments) {
		playturn();
		segments += Minute;
	}
}

void creature::update() {
	// Remove any links if target is invalid
	if(horror && (!is(Scared) || !(*horror)))
		horror = 0;
	if(charmer && (!is(Charmed) || !(*charmer)))
		charmer = 0;
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
			mp += imax(0, 1 + getbonus(OfMana) + get(Concetration) / 30);
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
	logs::add("Здесь ");
	if(count > 1)
		logs::add("лежат ");
	else
		logs::add("лежит ");
	for(int i = 0; i < count; i++) {
		if(i != 0)
			logs::add(" и ");
		char temp[260];
		logs::add(source[i]->getname(temp, zendof(temp), true));
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
			wait(Minute / 4);
		}
	} else {
		it.set(KnowEffect);
		auto spell = it.getspell();
		if(!spell)
			hint("Этот свиток пустой. Его нельзя прочитать.");
		else {
			use(spell, 1 + it.getquality(), "%герой прочитал%а свиток.");
			it.clear();
			wait(Minute / 2);
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
		return false;
	}
	auto itc = it;
	it.clear();
	if(!pickup(itc, false))
		game::drop(position, itc);
	return true;
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