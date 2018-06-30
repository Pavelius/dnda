#include "adat.h"
#include "aref.h"
#include "cflags.h"
#include "crt.h"
#include "grammar.h"
#include "point.h"
#include "rect.h"
#include "stringcreator.h"

#pragma once

const int max_map_x = 96;
const int max_map_y = 96;
const unsigned short Blocked = 0xFFFF;

enum item_s : unsigned char {
	NoItem,
	AxeBattle, Club, Dagger, HammerWar, Mace,
	Spear, Staff,
	SwordLong, SwordShort, SwordTwoHanded,
	CrossbowLight, CrossbowHeavy, BowLong, BowShort, Dart, Sling,
	Rock, Arrow, Bolt,
	LeatherArmour, StuddedLeatherArmour, ScaleMail, ChainMail, SplintMail, PlateMail,
	Shield, Helmet, Bracers,
	Ration, Apple, BreadHalflings, BreadEvlen, BreadDwarven, Cake, Sausage, Meat,
	Scroll, Book,
	PotionRed, PotionGreen, PotionBlue,
	DoorKey, Coin,
	Claws, Slam, Bite,
	ManyItems
};
enum diety_s : unsigned char {
	NoGod,
	GodBane, GodBhaal, GodGruumsh, GodHelm, GodMistra, GodTempus, GodTyr
};
enum slot_s : unsigned char {
	Head, Neck, Melee, OffHand, TorsoBack, Torso, RightFinger, LeftFinger, Elbows, Legs, Ranged, Amunitions
};
enum magic_s : unsigned char {
	NoEffect,
	OfArmor,
	OfCharisma, OfConstitution,
	OfDefence, OfDestruction, OfDexterity,
	OfIntellegence,
	OfPrecision,
	OfRegeneration,
	OfSharping, OfSmashing, OfSpeed, OfStrenght,
	OfWisdow,
};
enum race_s : unsigned char {
	NoRace,
	Human, Dwarf, Elf, Halfling,
	Animal, Goblinoid,
};
enum class_s : unsigned char {
	Cleric, Fighter, Mage, Paladin, Ranger, Theif,
};
enum gender_s : unsigned char {
	NoGender, Male, Female
};
enum role_s : unsigned char {
	GoblinWarrior, OrcWarrior, LargeBat, GiantRat,
	HumanMale, HumanGuard, HumanChild, HumanFemale,
	Shopkeeper, DwarvenSmith, Bartender,
	Character
};
enum alignment_s : unsigned char {
	Lawfull, Neutral, Chaotic
};
enum ability_s : unsigned char {
	Strenght, Dexterity, Constitution, Intellegence, Wisdow, Charisma
};
enum skill_s : unsigned char {
	NoSkill,
	Bargaining, Bluff, Diplomacy,
	Acrobatics, Alertness, Athletics, DisarmTraps, HearNoises, HideInShadow, Lockpicking, PickPockets,
	Alchemy, Dancing, Engineering, Gambling, History, Healing, Literacy, Mining, Smithing, Survival,
	WeaponFocusBows, WeaponFocusBlades, WeaponFocusAxes, TwoWeaponFighting,
	LastSkill = TwoWeaponFighting,
	ResistPoison,
};
enum state_s : unsigned char {
	Anger, Blessed, Charmed, Hiding, Goodwill, Lighted,
	PoisonedWeak, Poisoned, PoisonedStrong,
	Shielded, Scared, Sleeped,
	LastState = Sleeped
};
enum tile_s : unsigned char {
	NoTile,
	Plain, Water, Floor, Wall, Road,
	Swamp, Hill,
	City,
};
enum location_s : unsigned char {
	EmpthyRoom, TreasureRoom,
	StairsDownRoom, StairsUpRoom,
	House, Temple, Tavern,
	ShopWeaponAndArmor, ShopPotionAndScrolls, ShopFood,
};
enum map_object_s : unsigned char {
	NoTileObject,
	Door, Tree, Altar, Statue, Trap, StairsUp, StairsDown
};
enum trap_s : unsigned char {
	NoTrap,
	TrapAnimal, TrapAcid, TrapArrow, TrapCorrupt, TrapElectricity, TrapFire,
	TrapLight, TrapPit, TrapSpikedPit, TrapSpear, TrapBleed, TrapCorrosion, TrapWater
};
enum direction_s : unsigned char {
	Left, Right, Up, Down, LeftUp, LeftDown, RightUp, RightDown, Center,
};
enum img_s : unsigned char {
	ResNone,
	ResGrass, ResGrassW,
	ResDungeon, ResDungeonW,
	ResShadow,
	ResRoad,
	ResWater,
	ResMonsters,
	ResItems,
	ResDoors,
	ResFog,
	ResFeature,
	ResUI,
	ResPCmar, ResPCmbd, ResPCmac
};
enum target_s : unsigned char {
	NoTarget,
	TargetSelf, TargetCreature, TargetNotHostileCreature, TargetFriendlyCreature, TargetHostileCreature,
	TargetItem, TargetItemUnidentified, TargetItemEdible, TargetItemDrinkable, TargetItemReadable, TargetItemWeapon, TargetInvertory,
	TargetDoor, TargetDoorSealed,
	TargetTrap,
};
enum spell_s : unsigned char {
	Bless, CharmPerson, DetectEvil, HealingSpell, Identify, Invisibility, MagicMissile, Sleep,
	FirstSpell = Bless, LastSpell = Sleep
};
enum map_flag_s : unsigned char {
	Visible, Hidden, Opened, Sealed, Explored, Experience,
};
enum duration_s : unsigned {
	Instant = 0,
	Minute = 12, Turn = 10 * Minute,
	Halfhour = 30 * Minute, Hour = 60 * Minute, HalfDay = 12 * Hour, Day = 24 * Hour,
	Week = 7 * Day, Month = 30 * Day, Season = 3 * Month, Year = 4 * Season,
	Permanent = 100 * Year
};
enum identify_s : unsigned char {
	Unknown, KnowQuality, KnowMagic, KnowEffect,
};
enum item_type_s : unsigned char {
	Mundane, Cursed, Magical, Artifact,
};
enum item_flag_s : unsigned char {
	TwoHanded, Versatile,
};
enum attack_s : unsigned char {
	Bludgeon, Slashing, Piercing,
	Acid, Cold, Electricity, Fire, Magic, Poison, WaterAttack
};
enum save_s : char {
	NoSave, SaveAbility, SaveSkill,
};
struct attackinfo;
struct creature;
struct effectinfo;
struct location;
struct targetdesc;
class item;
struct targetinfo {
	constexpr targetinfo() : cre(0), itm(0), pos(Blocked) {}
	creature*		cre;
	item*			itm;
	short unsigned	pos;
};
struct targetdesc {
	target_s		target;
	short			range;
};
struct damageinfo {
	char			min;
	char			max;
	attack_s		type;
	explicit operator bool() const { return max != 0; }
	int				roll() const;
};
struct effectparam : targetinfo {
	const effectinfo& effect;
	creature&		player;
	bool			interactive;
	int				param;
	int				level;
	int				count;
	constexpr effectparam(const effectinfo& effect_param, creature& player, bool interactive) :
		effect(effect_param), player(player), interactive(interactive), param(0), level(1), count(0) {}
	void			apply();
	bool			saving() const;
};
struct effectinfo {
	struct savec {
		save_s		type;
		ability_s	ability;
		skill_s		skill;
	};
	targetdesc		type;
	savec			save;
	void(*proc)(effectparam& e);
	unsigned		duration;
	cflags<state_s>	state;
	const char*		text;
	damageinfo		number;
	unsigned		experience;
};
class item {
	item_s			type;
	//
	magic_s			effect;
	//
	unsigned char	count : 6;
	unsigned char	damaged : 2;
	//
	item_type_s		magic : 2;
	unsigned char	quality : 2;
	identify_s		identify : 2;
	unsigned char	forsale : 1;
public:
	constexpr item(item_s type = NoItem) : type(type), effect(NoEffect), count(0), magic(Mundane), quality(0), identify(Unknown), forsale(0), damaged(0) {}
	constexpr item(item_s type, item_type_s magic, magic_s effect, unsigned char quality, identify_s identify = Unknown) : type(type), effect(effect), count(0), magic(magic), quality(quality), identify(identify), forsale(0), damaged(0) {}
	item(item_s type, int level, int chance_curse = 10);
	operator bool() const { return type != NoItem; }
	void			clear();
	void			get(attackinfo& e) const;
	item_s			getammo() const;
	int				getarmor() const;
	int				getbonus(magic_s type) const;
	unsigned		getcost() const { return getcostsingle()*getcount(); }
	unsigned		getcostsingle() const;
	int				getcount() const;
	int				getdefence() const;
	magic_s			geteffect() const;
	skill_s			getfocus() const;
	identify_s		getidentify() const { return identify; }
	static unsigned	getitems(item_s* result, slot_s* slots, unsigned slots_count);
	item_type_s		getmagic() const { return magic; }
	char*			getname(char* result, const char* result_maximum, bool show_info = true) const;
	int				getquality() const;
	int				getsalecost() const;
	item_s			gettype() const { return type; }
	int				getweight() const;
	int				getweightsingle() const;
	bool			is(slot_s value) const;
	bool			is(item_flag_s value) const;
	bool			isarmor() const { return type >= LeatherArmour && type <= Bracers; }
	bool			isartifact() const { return magic == Artifact; }
	bool			iscountable() const;
	bool			iscursed() const { return magic == Cursed; }
	bool			isedible() const;
	bool			isforsale() const { return forsale != 0; }
	bool			isreadable() const;
	bool			istwohanded() const;
	void			loot();
	void			setcount(int count);
	void			set(identify_s value);
	void			set(item_type_s value) { magic = value; }
	void			set(magic_s value) { effect = value; }
	void			setforsale() { forsale = 1; }
	void			setsold() { forsale = 0; }
};
struct attackinfo {
	char			bonus;
	char			critical;
	damageinfo		damage;
	char			multiplier;
	char			speed;
};
struct creature {
	race_s			race;
	class_s			type;
	gender_s		gender;
	role_s			role;
	direction_s		direction;
	unsigned short	name;
	unsigned char	level;
	unsigned		recoil;
	unsigned		experience;
	unsigned		money;
	creature*		charmer;
	creature*		enemy;
	creature*		horror;
	creature*		leader;
	unsigned short	position, guard;
	item			wears[Amunitions + 1];
	item			backpack[16];
	//
	constexpr creature() : race(NoRace), type(Cleric), gender(NoGender), role(Character), direction(Left),
		abilities(), skills(), spells(), name(), level(),
		wears(), backpack(), states(),
		hp(), mhp(), mp(), mmp(),
		recoil(), restore_hits(), restore_mana(), experience(), money(),
		charmer(0), enemy(0), horror(), leader(),
		position(), guard() {}
	creature(role_s value);
	creature(race_s race, gender_s gender, class_s type);
	operator bool() const { return hp > 0; }
	void* operator new(unsigned size);
	//
	void			act(const char* format, ...) const;
	void			addexp(unsigned count);
	bool			askyn(creature* opponent, const char* format, ...);
	bool			canhear(short unsigned index) const;
	void			chat(creature* opponent);
	void			choosebestability();
	void			clear();
	void			clear(state_s value) { states[value] = 0; }
	void			damage(int count, bool interactive = true);
	void			damage(damageinfo dice, bool interactive = true);
	bool			dropdown(item& value);
	bool			equip(item value);
	int				get(ability_s value) const;
	int				get(spell_s value) const;
	int				get(skill_s value) const;
	int				getarmor() const;
	attackinfo		getattackinfo(slot_s slot) const;
	int				getattacktime(slot_s slot) const;
	int				getbasic(skill_s value) const;
	int				getbonus(magic_s value) const;
	int				getcreatures(creature** result, unsigned count, target_s target, int range = -1) const;
	int				getcost(spell_s value) const;
	unsigned		getcostexp() const;
	int				getdefence() const;
	int				getdiscount(creature* customer) const;
	char*			getfullname(char* result, const char* result_maximum, bool show_level, bool show_alignment) const;
	int				gethits() const { return hp; }
	int				getitems(item** result, unsigned maximum_count, target_s target) const;
	creature*		getleader() const;
	int				getlos() const;
	int				getmana() const { return mp; }
	int				getmaxhits() const;
	int				getmaxmana() const;
	const char*		getmonstername() const;
	int				getmoverecoil() const;
	const char*		getname() const; // Name used predefined names array
	creature*		getnearest(target_s target) const;
	int				getobjects(short unsigned* result, unsigned count, target_s target, int range) const;
	static creature* getplayer();
	short unsigned	getposition() const { return position; }
	bool			gettarget(targetinfo& result, const targetdesc td) const;
	int				getweight() const;
	bool			interact(short unsigned index);
	bool			is(state_s value) const;
	bool			isagressive() const;
	bool			ischaracter() const { return role == Character; }
	bool			isenemy(const creature* target) const;
	bool			isfriend(const creature* target) const;
	bool			isguard() const { return guard != 0xFFFF; }
	bool			isparty() const;
	bool			isplayer() const;
	bool			joinparty();
	void			levelup();
	void			lookfloor();
	void			makemove();
	bool			manipulate(short unsigned index);
	void			meleeattack(creature* target);
	bool			move(short unsigned index);
	bool			moveto(short unsigned index);
	bool			moveaway(short unsigned index);
	void			passturn(unsigned minutes);
	bool			pickup(item value);
	void			raise(skill_s value);
	void			raiseskills(int number);
	void			rangeattack();
	void			remove(state_s value);
	bool			roll(skill_s skill, int bonus = 0);
	void			say(const char* format, ...);
	bool			sayv(const char* format, const char* param);
	void			set(state_s value, unsigned segments);
	void			set(spell_s value, int level);
	void			setlos();
	void			setplayer();
	void			trapeffect();
	void			update();
	bool			use(skill_s value);
	bool			use(spell_s value);
	bool			use(short unsigned index);
	void			wait(int segments = 0);
	bool			walkaround();
private:
	unsigned char	abilities[Charisma + 1];
	short			hp, mhp, mp, mmp;
	unsigned		restore_hits, restore_mana;
	unsigned char	skills[LastSkill + 1];
	unsigned char	spells[LastSpell + 1];
	unsigned		states[LastState + 1];
	friend struct archive;
};
struct location : rect {
	location_s		type;
	diety_s			diety;
	unsigned char	name[2];
	creature*		owner;
	operator bool() const { return x1 != x2; }
	char*			getname(char* result) const;
};
struct groundinfo {
	item			object;
	short unsigned	index;
};
struct areainfo {
	short unsigned	index; // Позиция на карте мира
	unsigned char	level; // Уровень поздземелья
	unsigned char	rooms; // Количество комнат
	void			clear();
};
namespace game {
creature*			add(short unsigned index, creature* element);
creature*			add(short unsigned index, role_s type);
creature*			add(short unsigned index, race_s race, gender_s gender, class_s type, bool is_player = false);
void				create(tile_s type, short unsigned index, int level, bool explored = false, bool visualize = false);
int					distance(short unsigned i1, short unsigned i2);
void				drop(short unsigned i, item object);
unsigned short		genname(race_s race, gender_s gender);
inline short unsigned get(int x, int y) { return y * max_map_x + x; }
const attackinfo&	getattackinfo(trap_s slot);
creature*			getcreature(short unsigned index);
direction_s			getdirection(point s, point d);
short unsigned		getfree(short unsigned i);
int					getindex(short unsigned i, tile_s value);
location*			getlocation(short unsigned i);
int					getitems(item** result, unsigned maximum_count, short unsigned index);
short unsigned		getmovement(short unsigned i);
const char*			getnamepart(unsigned short value);
creature*			getnearest(creature** source, unsigned count, short unsigned position);
int					getnight();
int					getrand(short unsigned i);
cflags<skill_s>		getskills(race_s value);
short unsigned		getstepto(short unsigned index);
short unsigned		getstepfrom(short unsigned index);
const char*			getdate(char* result, const char* result_maximum, unsigned segments, bool show_time);
trap_s				gettrap(short unsigned i);
tile_s				gettile();
tile_s				gettile(short unsigned i);
map_object_s		getobject(short unsigned i);
inline unsigned char getx(short unsigned i) { return i % max_map_x; }
inline unsigned char gety(short unsigned i) { return i / max_map_x; }
void				initialize();
bool				isbooming();
bool				isdungeon();
bool				isexplore(short unsigned i);
bool				isexperience(short unsigned i);
bool				ishidden(short unsigned i);
bool				isopen(short unsigned i);
bool				ispassable(short unsigned i);
bool				ispassabledoor(short unsigned i);
bool				ispassablelight(short unsigned i);
bool				isseal(short unsigned i);
bool				isvisible(short unsigned i);
void				looktarget(short unsigned index);
void				lookhere(short unsigned index);
void				makewave(short unsigned index, bool(*proc)(short unsigned) = ispassabledoor);
void				play();
void				release(const creature* target, unsigned experience_cost);
bool				serialize(bool writemode);
void				set(short unsigned i, tile_s value);
void				set(short unsigned i, tile_s value, int w, int h);
void				set(short unsigned i, map_object_s value);
void				set(short unsigned i, map_flag_s type, bool value);
void				set(short unsigned i, unsigned char value);
extern areainfo		statistic;
short unsigned		to(short unsigned index, int id);
void				turn();
direction_s			turn(direction_s from, direction_s side);
};
namespace logs {
struct driver : stringcreator {
	gender_s		gender;
	const char*		name;
	const char*		weapon;
	driver(const char* name, gender_s gender, const char* weapon);
	void			parseidentifier(char* result, const char* result_max, const char* identifier) override;
};
void				add(const char* format, ...);
void				add(int id, const char* format, ...);
void				addv(const char* format, const char* param);
void				addv(stringcreator& driver, const char* format, const char* param);
short unsigned		choose(const creature& e, short unsigned* source, int count);
item*				choose(const creature& e, item** source, unsigned count, const char* title);
bool				choose(creature& e, skill_s& result, skill_s* source, unsigned count, bool can_escape = true);
bool				choose(creature& e, skill_s& result, bool can_escape = true);
bool				choose(creature& e, spell_s& result, spell_s* source, unsigned count);
bool				choose(creature& e, spell_s& result);
bool				chooseyn();
void				focusing(short unsigned index);
bool				getcreature(const creature& e, creature** result, target_s target, int range);
bool				getindex(const creature& e, short unsigned& result, target_s target, int range);
bool				getitem(const creature& e, item** result, target_s target);
void				initialize();
int					input();
void				minimap(creature& e);
void				next();
void				turn(creature& e);
}
extern adat<location, 128>		locations;
extern adat<creature*, 512>		creatures;
extern adat<groundinfo, 2048>	grounditems;
extern adat<creature*, 6>		players;
unsigned			getday();
unsigned			gethour();
unsigned			getminute();
unsigned			getmonth();
unsigned			getturn();
unsigned			getyear();
extern unsigned		segments;