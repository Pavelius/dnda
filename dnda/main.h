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
	Shield, Helmet, BracersLeather, BracersIron,
	Cloack1, Cloack2, Cloack3, Cloack4, Cloack5,
	Ration, Apple, BreadHalflings, BreadEvlen, BreadDwarven, Cake, Sausage, Meat,
	ScrollRed, ScrollGreen, ScrollBlue,
	WandRed, WandGreen, WandBlue,
	Book1, Book2, Book3,
	PotionRed, PotionGreen, PotionBlue,
	RingRed, RingBlue, RingGreen,
	DoorKey, Coin,
	Claws, Slam, Bite,
	ManyItems
};
enum diety_s : unsigned char {
	NoGod,
	GodBane, GodBhaal, GodGruumsh, GodHelm, GodMistra, GodTempus, GodTyr
};
enum slot_s : unsigned char {
	Head, Neck, Melee, OffHand, TorsoBack, Torso, RightFinger, LeftFinger, Elbows, Legs, Ranged, Amunitions,
	FirstBackpack, LastBackpack = FirstBackpack + 19,
};
enum enchantment_s : unsigned char {
	NoEffect,
	OfArmor,
	OfCharisma, OfCold, OfConstitution,
	OfDefence, OfDestruction, OfDexterity,
	OfFire,
	OfIntellegence,
	OfMana,
	OfPrecision,
	OfRegeneration,
	OfSharping, OfSmashing, OfSpeed, OfStrenght, OfSustenance,
	OfVampirism, OfWisdow,
	// Resistances
	OfAcidResistance, OfColdResistance, OfFireResistance, OfElectricityResistance, OfPoisonResistance, OfWaterproof,
	LastEnchantment = OfWaterproof,
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
	Acrobatics, Alertness, Athletics, Concetration, DisarmTraps, HearNoises, HideInShadow, Lockpicking, PickPockets,
	Alchemy, Dancing, Engineering, Gambling, History, Healing, Literacy, Mining, Smithing, Survival, Swimming,
	WeaponFocusBows, WeaponFocusBlades, WeaponFocusAxes, TwoWeaponFighting,
	LastSkill = TwoWeaponFighting,
	ResistAcid, ResistCold, ResistElectricity, ResistFire, ResistPoison, ResistWater,
	LastResist = ResistWater,
};
enum state_s : unsigned char {
	NoState,
	Anger, Armored, Blessed, Charmed, Hiding, Goodwill,
	Lighted, PoisonedWeak, Poisoned, PoisonedStrong,
	Shielded, Sick, Scared, Sleeped, Weaken,
	// Ability boost effect
	Strenghted, Dexterious, Healthy, Intellegenced, Wisdowed, Charismatic,
	LastState = Charismatic,
	// Instant effects
	HealState, RemoveSick, RemovePoison,
	LastEffectState = RemovePoison,
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
	TargetSelf, TargetFriendly, TargetFriendlySelf, TargetHostile,
	TargetItem, TargetItemUnidentified, TargetItemEdible, TargetItemDrinkable, TargetItemReadable, TargetItemWeapon, TargetItemChargeable, TargetInvertory,
	TargetDoor, TargetDoorSealed,
	TargetTrap,
};
enum spell_s : unsigned char {
	NoSpell,
	Armor, Bless, CharmPerson, DetectEvil, Fear, HealingSpell, Identify, Invisibility, LightSpell, MagicMissile,
	RemovePoisonSpell, RemoveSickSpell,
	ShieldSpell, ShokingGrasp, Sleep,
	FirstSpell = Armor, LastSpell = Sleep
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
enum save_s : unsigned char {
	NoSave, SaveAbility, SaveSkill,
};
enum material_s : unsigned char {
	Glass, Iron, Leather, Organic, Paper, Stone, Wood,
};
enum encumbrance_s : unsigned char {
	NoEncumbered,
	Encumbered, HeavilyEncumbered,
};
struct attackinfo;
struct creature;
struct effectparam;
struct location;
struct targetdesc;
class item;
struct skillvalue {
	skill_s			id;
	char			value;
};
struct targetinfo {
	constexpr targetinfo() : cre(0), itm(0), pos(Blocked) {}
	creature*		cre;
	item*			itm;
	short unsigned	pos;
};
struct targetdesc {
	target_s		target;
	unsigned char	range;
	unsigned char	area;
};
struct damageinfo {
	char			min;
	char			max;
	attack_s		type;
	explicit operator bool() const { return max != 0; }
	int				roll() const;
};
struct foodinfo {
	char			hits;
	char			mana;
	char			abilities[Charisma + 1];
	unsigned		duration;
	char			sickness;
	char			poision;
	explicit operator bool() const { return hits != 0; }
	int				get(int value) const { return value * 50; }
};
struct effectinfo {
	struct savec {
		save_s		type;
		ability_s	ability;
		skill_s		skill;
	};
	targetdesc		type;
	savec			save;
	void(*success)(effectparam& e);
	unsigned		duration;
	cflags<state_s>	state;
	const char*		text;
	damageinfo		damage;
	unsigned		experience;
	void(*fail)(effectparam& e);
	bool(*test)(effectparam& e);
};
struct effectparam : targetinfo, effectinfo {
	creature&		player;
	bool			interactive;
	int				param;
	int				level;
	constexpr effectparam(const effectinfo& effect_param, creature& player, bool interactive) :
		effectinfo(effect_param), player(player), interactive(interactive), param(0), level(1) {}
	void			apply();
	void			apply(void(*proc)(effectparam& e));
	bool			saving() const;
};
class item {
	item_s			type;
	//
	enchantment_s	effect;
	//
	unsigned char	count : 6;
	unsigned char	damaged : 2;
	//
	item_type_s		magic : 2;
	unsigned char	quality : 2;
	identify_s		identify : 2;
	unsigned char	forsale : 1;
public:
	constexpr item() : type(NoItem), effect(NoEffect), count(0), magic(Mundane), quality(0), identify(Unknown), forsale(0), damaged(0) {}
	constexpr item(spell_s spell) : type(ScrollRed), effect((enchantment_s)spell), count(0), magic(), quality(0), identify(KnowEffect), forsale(0), damaged(0) {}
	constexpr item(item_s type) : type(type), effect(NoEffect), count(0), magic(Mundane), quality(0), identify(Unknown), forsale(0), damaged(0) {}
	constexpr item(item_s type, item_type_s magic, enchantment_s effect) : type(type), effect(effect), count(0), magic(magic), quality(0), identify(KnowEffect), forsale(0), damaged(0) {}
	item(item_s type, int chance_artifact, int chance_magic, int chance_cursed, int chance_quality);
	operator bool() const { return type != NoItem; }
	void			act(const char* format, ...) const;
	void			clear();
	void			damage();
	void			get(attackinfo& e) const;
	item_s			getammo() const;
	int				getarmor() const;
	int				getbonus(enchantment_s type) const;
	int				getcharges() const;
	unsigned		getcost() const;
	int				getcount() const;
	int				getdefence() const;
	enchantment_s			geteffect() const;
	skill_s			getfocus() const;
	const foodinfo& getfood() const;
	identify_s		getidentify() const { return identify; }
	static aref<item_s>	getitems(aref<item_s> result, aref<slot_s> source);
	item_type_s		getmagic() const { return magic; }
	material_s		getmaterial() const;
	static const char* getname(spell_s value);
	static const char* getname(state_s value);
	char*			getname(char* result, const char* result_maximum, bool show_info = true) const;
	int				getquality() const;
	int				getqualityraw() const { return quality; }
	int				getsalecost() const;
	spell_s			getspell() const;
	state_s			getstate() const;
	item_s			gettype() const { return type; }
	int				getweight() const;
	int				getweightsingle() const;
	bool			is(slot_s value) const;
	bool			is(item_flag_s value) const;
	bool			isarmor() const;
	bool			isartifact() const { return magic == Artifact; }
	bool			ischargeable() const;
	bool			iscountable() const;
	bool			iscursed() const { return magic == Cursed; }
	bool			isdrinkable() const;
	bool			isedible() const;
	bool			isforsale() const { return forsale != 0; }
	bool			isreadable() const;
	bool			istwohanded() const;
	void			loot();
	void			setcharges(int count);
	void			setcount(int count);
	void			set(identify_s value);
	void			set(item_type_s value) { magic = value; }
	void			set(enchantment_s value) { effect = value; }
	void			setforsale() { forsale = 1; }
	void			setsold() { forsale = 0; }
};
struct attackinfo {
	char			speed;
	damageinfo		damage;
	char			bonus;
	char			critical;
	char			multiplier;
	enchantment_s			effect;
	char			quality;
};
struct creature {
	race_s			race;
	class_s			type;
	gender_s		gender;
	role_s			role;
	direction_s		direction;
	unsigned short	name;
	unsigned char	level;
	unsigned		experience;
	unsigned		money;
	creature*		charmer;
	creature*		enemy;
	creature*		horror;
	unsigned short	position, guard;
	item			wears[LastBackpack + 1];
	//
	constexpr creature() : race(NoRace), type(Cleric), gender(NoGender), role(Character), direction(Left),
		abilities(), abilities_raise(), skills(), spells(), name(), level(),
		wears(), states(),
		hp(), mhp(), mp(), mmp(),
		recoil(), restore_hits(), restore_mana(), experience(), money(),
		charmer(0), enemy(0), horror(), party(),
		position(), guard(), encumbrance(NoEncumbered) {}
	creature(role_s value);
	creature(race_s race, gender_s gender, class_s type);
	explicit operator bool() const { return hp > 0; }
	void* operator new(unsigned size);
	//
	void			act(const char* format, ...) const { actv(format, xva_start(format)); }
	void			actv(const char* format, const char* param) const;
	void			actv(creature& opponent, const char* format, const char* param) const;
	void			actvs(creature& opponent, const char* format, ...) const { actv(opponent, format, xva_start(format)); }
	void			addexp(unsigned count);
	bool			askyn(creature* opponent, const char* format, ...);
	bool			canhear(short unsigned index) const;
	void			chat(creature* opponent);
	void			choosebestability();
	void			clear();
	void			clear(state_s value) { states[value] = 0; }
	void			consume(int value, bool interactive);
	void			damage(int count, attack_s type, bool interactive);
	void			damagewears(int count, attack_s type);
	void			drink(item& it, bool interactive);
	void			dropdown(item& value);
	bool			equip(item_s value);
	bool			equip(item value);
	void			equip(slot_s slot, item value);
	int				get(ability_s value) const;
	int				get(spell_s value) const;
	int				get(skill_s value) const;
	int				getarmor() const;
	attackinfo		getattackinfo(slot_s slot) const;
	int				getattacktime(slot_s slot) const;
	int				getbasic(ability_s value) const;
	int				getbasic(skill_s value) const;
	int				getbonus(enchantment_s value) const;
	int				getcost(spell_s value) const;
	unsigned		getcostexp() const;
	static creature* getcreature(short unsigned index);
	unsigned		getcreatures(aref<creature*> result, targetdesc ti) const;
	static unsigned	getcreatures(aref<creature*> result, targetdesc ti, short unsigned position, const creature* player, const creature* exclude);
	int				getdefence() const;
	int				getdiscount(creature* customer) const;
	encumbrance_s	getencumbrance() const { return encumbrance; }
	char*			getfullname(char* result, const char* result_maximum, bool show_level, bool show_alignment) const;
	creature*		gethenchmen(int index) const;
	int				gethits() const { return hp; }
	unsigned		getitems(aref<item*> result, targetdesc ti) const;
	int				getlos() const;
	int				getmana() const { return mp; }
	int				getmaxhits() const;
	int				getmaxmana() const;
	const char*		getmonstername() const;
	int				getmoverecoil() const;
	const char*		getname() const; // Name used predefined names array
	static const char* getname(tile_s id);
	static const char* getname(state_s id, bool cursed);
	creature*		getnearest(targetdesc ti) const;
	unsigned		getobjects(aref<short unsigned> result, targetdesc ti) const;
	creature*		getparty() const;
	static creature* getplayer();
	short unsigned	getposition() const { return position; }
	bool			gettarget(targetinfo& result, const targetdesc td) const;
	int				getweight() const;
	int				getweight(encumbrance_s id) const;
	void			heal(int value, bool interactive) { damage(-value, Magic, interactive); }
	void			hint(const char* format, ...) const;
	static void		initialize();
	bool			interact(short unsigned index);
	bool			is(state_s value) const;
	bool			is(encumbrance_s value) const { return encumbrance == value; }
	bool			isagressive() const;
	static bool		isbooming();
	bool			ischaracter() const { return role == Character; }
	bool			isenemy(const creature* target) const;
	bool			isfriend(const creature* target) const;
	bool			isguard() const { return guard != 0xFFFF; }
	bool			isleader() const { return party == this; }
	bool			isparty(const creature* target) const;
	bool			isplayer() const;
	void			join(creature* party);
	void			levelup();
	void			lookfloor();
	void			makemove();
	void			manipulate(short unsigned index);
	void			meleeattack(creature* target);
	bool			move(short unsigned index);
	bool			moveto(short unsigned index);
	bool			moveaway(short unsigned index);
	void			passturn(unsigned minutes);
	static void		playturn();
	bool			pickup(item value, bool interactive = true);
	void			raise(skill_s value);
	void			raiseskills(int number);
	void			rangeattack();
	void			release(unsigned exeperience_cost) const;
	void			remove(state_s value);
	bool			roll(skill_s skill, int bonus = 0);
	void			say(const char* format, ...);
	bool			sayv(const char* format, const char* param);
	static void		select(creature** result, rect rc);
	void			set(state_s value, unsigned segments);
	void			set(spell_s value, int level);
	static void		setblocks(short unsigned* movements, short unsigned value);
	void			setplayer();
	static void		setleader(const creature* party, creature* leader);
	void			setlos();
	void			trapeffect();
	static void		turnbegin();
	void			update();
	void			use(skill_s value);
	bool			use(spell_s value);
	bool			use(spell_s value, int level, const char* format, ...);
	void			use(item& it);
	bool			use(short unsigned index);
	bool			unequip(item& it);
	void			wait(int segments = 0);
	bool			walkaround();
private:
	char			abilities[Charisma + 1];
	short			abilities_raise[Charisma + 1];
	short			hp, mhp, mp, mmp;
	unsigned		restore_hits, restore_mana;
	unsigned char	skills[LastResist + 1];
	unsigned char	spells[LastSpell + 1];
	unsigned		states[LastState + 1];
	unsigned		recoil;
	creature*		party;
	encumbrance_s	encumbrance;
	friend struct archive;
	void			updateweight();
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
void				create(tile_s type, short unsigned index, int level, bool explored = false, bool visualize = false);
int					distance(short unsigned i1, short unsigned i2);
void				drop(short unsigned i, item object);
unsigned short		genname(race_s race, gender_s gender);
inline short unsigned get(int x, int y) { return y * max_map_x + x; }
const attackinfo&	getattackinfo(trap_s slot);
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
bool				serialize(bool writemode);
void				set(short unsigned i, tile_s value);
void				set(short unsigned i, tile_s value, int w, int h);
void				set(short unsigned i, map_object_s value);
void				set(short unsigned i, map_flag_s type, bool value);
void				set(short unsigned i, unsigned char value);
extern areainfo		statistic;
short unsigned		to(short unsigned index, int id);
direction_s			turn(direction_s from, direction_s side);
};
namespace logs {
struct driver : stringcreator {
	gender_s		gender;
	gender_s		gender_opponent;
	const char*		name;
	const char*		name_opponent;
	constexpr driver() : gender(Male), gender_opponent(Male), name(""), name_opponent("") {}
	void			parseidentifier(char* result, const char* result_max, const char* identifier) override;
};
void				add(const char* format, ...);
void				add(int id, const char* format, ...);
void				addu(const char* format, ...);
void				addv(const char* format, const char* param, int letter = 0);
void				addv(stringcreator& driver, const char* format, const char* param, int letter = 0);
short unsigned		choose(const creature& e, short unsigned* source, int count);
item*				choose(const creature& e, item** source, unsigned count, const char* title);
bool				choose(creature& e, skill_s& result, skill_s* source, unsigned count, bool can_escape = true);
bool				choose(creature& e, skill_s& result, bool can_escape = true);
bool				choose(creature& e, spell_s& result, spell_s* source, unsigned count);
bool				choose(creature& e, spell_s& result);
bool				chooseyn();
void				focusing(short unsigned index);
bool				getcreature(const creature& e, creature** result, targetdesc ti);
bool				getindex(const creature& e, short unsigned& result, targetdesc ti);
bool				getitem(const creature& e, item** result, targetdesc ti, const char* title = 0);
void				initialize();
int					input();
void				minimap(creature& e);
void				next();
void				turn(creature& e);
}
extern adat<location, 128>		locations;
extern adat<groundinfo, 2048>	grounditems;
unsigned			getday();
unsigned			gethour();
unsigned			getminute();
unsigned			getmonth();
unsigned			getturn();
unsigned			getyear();
extern unsigned		segments;