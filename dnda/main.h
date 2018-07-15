#include "collection.h"
#include "crt.h"
#include "grammar.h"
#include "point.h"
#include "rect.h"
#include "stringcreator.h"

#pragma once

const int GP = 100;
const int SP = 10;
const int CP = 1;
const int max_map_x = 84;
const int max_map_y = 84;
const int chance_to_hit = 40; // Dexterity add to this value. Mediaval dexterity is 10, so medium chance to hit is 50%.
const unsigned short Blocked = 0xFFFF;
const unsigned short BlockedCreature = Blocked - 1;

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
	Boot1, Boot2, IronBoot1, IronBoot2, IronBoot3,
	Ration, Apple, BreadHalflings, BreadEvlen, BreadDwarven, Cake, Sausage, Meat,
	ScrollRed, ScrollGreen, ScrollBlue,
	WandRed, WandGreen, WandBlue,
	Book1, Book2, Book3, Book4, Book5,
	PotionRed, PotionGreen, PotionBlue,
	RingRed, RingBlue, RingGreen,
	Amulet1, Amulet2, Amulet3, Amulet4, Amulet5,
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
	OfDefence, OfDexterity,
	OfHoliness,
	OfFire,
	OfIntellegence,
	OfMana, OfMissileDeflection,
	OfOrcSlying,
	OfPoison, OfPrecision,
	OfRegeneration,
	OfSickness, OfSharping, OfSmashing, OfSpeed, OfStrenght, OfSustenance,
	OfVampirism, OfWeakness, OfWisdow,
	// Resistances
	OfAcidResistance, OfCharmResistance, OfColdResistance, OfFireResistance, OfElectricityResistance, OfPoisonResistance, OfWaterproof,
	LastEnchantment = OfWaterproof,
};
enum race_s : unsigned char {
	Animal,
	Human, Dwarf, Elf, Halfling,
	Goblin, Kobold, Orc,
	Undead
};
enum class_s : unsigned char {
	Commoner,
	Cleric, Fighter, Mage, Paladin, Ranger, Theif,
};
enum gender_s : unsigned char {
	NoGender, Male, Female, They,
};
enum role_s : unsigned char {
	GoblinWarrior, GoblinRockthrower, OrcWarrior, LargeBat, GiantRat,
	HumanMale, HumanGuard, HumanChild, HumanFemale,
	Shopkeeper, DwarvenSmith, Bartender, Skeleton, Zombie,
	KobolWarrior,
	GreatDog, Lynx,
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
	Acrobatics, Alertness, Athletics, Backstabbing, Concetration, DisarmTraps, HearNoises, HideInShadow, Lockpicking, PickPockets,
	Alchemy, Dancing, Engineering, Gambling, History, Healing, Literacy, Mining, Smithing, Survival, Swimming,
	WeaponFocusBows, WeaponFocusBlades, WeaponFocusAxes, TwoWeaponFighting,
	LastSkill = TwoWeaponFighting,
	ResistAcid, ResistCharm, ResistCold, ResistElectricity, ResistFire, ResistPoison, ResistWater,
	LastResist = ResistWater,
};
enum state_s : unsigned char {
	NoState,
	Anger, Armored, Blessed, Charmed, Drunken, Hiding, Goodwill,
	Lighted, PoisonedWeak, Poisoned, PoisonedStrong,
	Shielded, Sick, Scared, Sleeped, Weaken,
	// Ability boost effect
	Strenghted, Dexterious, Healthy, Intellegenced, Wisdowed, Charismatic,
	LastState = Charismatic,
	// Instant effects
	Experience, RestoreHits, RestoreMana, RemoveSick, RemovePoison,
	LastEffectState = RemovePoison,
};
enum tile_s : unsigned char {
	NoTile,
	Plain, Water, Floor, Wall, Road,
	Swamp, Hill,
	Sea, Foothills, Mountains, CloudPeaks, Forest,
	City,
};
enum site_s : unsigned char {
	EmpthyRoom, TreasureRoom,
	StairsDownRoom, StairsUpRoom, House,
	Temple, Tavern,
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
	Center,
	Left, Right, Up, Down, LeftUp, LeftDown, RightUp, RightDown
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
	ResSea, ResPlains, ResFoothills, ResMountains, ResCloudPeaks, ResDecals,
	ResUI,
	ResPCmar, ResPCmbd, ResPCmac
};
enum target_s : unsigned char {
	NoTarget,
	TargetSelf, TargetFriendly, TargetFriendlyWounded, TargetNeutral, TargetHostile,
	TargetItemMundane, TargetItemUnidentified, TargetItemDamaged, TargetItemEdible, TargetItemDrinkable, TargetItemReadable, TargetItemWeapon, TargetItemChargeable, TargetInvertory,
	TargetObject,
	TargetDoor, TargetDoorSealed, TargetHiddenObject,
	TargetTrap,
};
enum spell_s : unsigned char {
	NoSpell,
	Armor, Bless, BlessItem, CharmPerson, DetectEvil, DetectMagic, Fear, HealingSpell,
	Identify, Invisibility, LightSpell, MagicMissile,
	Repair, RemovePoisonSpell, RemoveSickSpell,
	ShieldSpell, ShokingGrasp, Sleep,
	FirstSpell = Armor, LastSpell = Sleep
};
enum map_flag_s : unsigned char {
	Visible, Hidden, Opened, Sealed, Explored,
};
enum duration_s : unsigned {
	Instant = 0,
	Minute = 12, Turn = 10 * Minute,
	Halfhour = 30 * Minute, Hour = 60 * Minute, HalfDay = 12 * Hour, Day = 24 * Hour,
	Week = 7 * Day, Month = 30 * Day, Season = 3 * Month, Year = 4 * Season,
	Permanent = 100 * Year
};
enum identify_s : unsigned char {
	Unknown, KnowQuality, KnowColor, KnowEffect
};
enum item_type_s : unsigned char {
	Mundane, Cursed, BlessedItem, Artifact,
};
enum attack_s : unsigned char {
	Bludgeon, Slashing, Piercing,
	Acid, Cold, Electricity, Fire, Magic, Poison, WaterAttack
};
enum material_s : unsigned char {
	Glass, Iron, Leather, Organic, Paper, Stone, Wood,
};
enum encumbrance_s : unsigned char {
	NoEncumbered,
	Encumbered, HeavilyEncumbered,
};
enum variant_s : unsigned char {
	NoVariant,
	Ability, Item, Skill, State, Enchantment,
	Text
};
enum speech_s : unsigned char {
	NoTalking,
	Answer, Action, Speech,
};
enum manual_s : unsigned char {
	Element, Header
};
struct attackinfo;
struct creature;
struct dialog;
struct effectparam;
struct site;
struct targetdesc;
class item;
struct skillvalue {
	skill_s				id;
	char				value;
};
struct targetdesc {
	target_s			target;
	unsigned char		range;
	unsigned char		area;
	bool				isallow(const creature& player, aref<creature*> creatures) const;
	bool				iscreature() const;
	bool				isposition() const;
};
struct damageinfo {
	char				min;
	char				max;
	attack_s			type;
	explicit operator bool() const { return max != 0; }
	int					roll() const;
};
struct foodinfo {
	char				hits;
	char				mana;
	char				abilities[Charisma + 1];
	unsigned			duration;
	char				sickness;
	char				poision;
	explicit operator bool() const { return hits != 0; }
	int					get(int value) const { return value * 50; }
};
struct specialinfo {
	char				chance_broke;
	char				bonus;
	char				chance_side;
};
struct effectinfo {
	struct callback {
		void(*success)(effectparam& e);
		void(*fail)(effectparam& e);
		bool(*test)(effectparam& e);
		bool(*validate)(const creature& e);
	};
	struct stateinfo {
		state_s			type;
		unsigned		duration;
	};
	targetdesc			type;
	callback			proc;
	stateinfo			state;
	const char*			text;
	damageinfo			damage;
	unsigned			experience;
};
struct effectparam : effectinfo {
	creature&			player;
	bool				interactive;
	creature*			cre;
	item*				itm;
	short unsigned		pos;
	int					param;
	int					level;
	int					skill_roll;
	int					skill_value;
	int					skill_bonus;
	aref<creature*>	creatures;
	constexpr effectparam(const effectinfo& effect_param, creature& player, aref<creature*>	p_creatures, bool interactive) :
		effectinfo(effect_param), player(player), interactive(interactive),
		cre(0), itm(0), pos(Blocked),
		param(0), level(1), creatures(p_creatures),
		skill_roll(0), skill_value(0), skill_bonus(0) {
	}
	int					apply(const char* format, const char* format_param);
	bool				applyfull();
};
class item {
	item_s				type;
	//
	enchantment_s		effect;
	//
	unsigned char		count : 6;
	unsigned char		damaged : 2;
	//
	item_type_s			magic : 2;
	unsigned char		quality : 2;
	identify_s			identify : 2;
	unsigned char		forsale : 1;
public:
	constexpr item() : type(NoItem), effect(NoEffect), count(0), magic(Mundane), quality(0), identify(Unknown), forsale(0), damaged(0) {}
	constexpr item(spell_s spell) : type(ScrollRed), effect((enchantment_s)spell), count(0), magic(), quality(0), identify(KnowEffect), forsale(0), damaged(0) {}
	constexpr item(item_s type) : type(type), effect(NoEffect), count(0), magic(Mundane), quality(0), identify(Unknown), forsale(0), damaged(0) {}
	constexpr item(item_s type, item_type_s magic, enchantment_s effect) : type(type), effect(effect), count(0), magic(magic), quality(0), identify(KnowEffect), forsale(0), damaged(0) {}
	item(item_s type, int chance_artifact, int chance_magic, int chance_cursed, int chance_quality);
	operator bool() const { return type != NoItem; }
	void				act(const char* format, ...) const;
	void				clear();
	bool				damageb();
	void				damage();
	void				get(attackinfo& e) const;
	item_s				getammo() const;
	int					getarmor() const;
	int					getbonus(enchantment_s type) const;
	int					getcharges() const;
	unsigned			getcost() const;
	int					getcount() const;
	int					getdefence() const;
	enchantment_s		geteffect() const;
	skill_s				getfocus() const;
	const foodinfo&		getfood() const;
	identify_s			getidentify() const { return identify; }
	static aref<item_s>	getitems(aref<item_s> result, aref<slot_s> source);
	gender_s			getgender() const;
	item_type_s			getmagic() const { return magic; }
	material_s			getmaterial() const;
	static const char*	getname(spell_s value);
	static const char*	getname(state_s value);
	char*				getname(char* result, const char* result_maximum, bool show_info = true) const;
	int					getquality() const;
	int					getqualityraw() const { return quality; }
	int					getsalecost() const;
	skill_s				getskill() const;
	spell_s				getspell() const;
	const specialinfo&	getspecial() const;
	state_s				getstate() const;
	item_s				gettype() const { return type; }
	int					getweight() const;
	int					getweightsingle() const;
	bool				is(slot_s value) const;
	bool				isarmor() const;
	bool				isartifact() const { return magic == Artifact; }
	bool				ischargeable() const;
	bool				iscountable() const;
	bool				iscursed() const { return magic == Cursed; }
	bool				isdamaged() const { return damaged != 0; }
	bool				isdrinkable() const;
	bool				isedible() const;
	bool				isforsale() const { return forsale != 0; }
	bool				isnatural() const;
	bool				ismagical() const { return magic != Mundane; }
	bool				isreadable() const;
	bool				istome() const;
	bool				isthrown() const;
	bool				istwohanded() const;
	bool				isversatile() const;
	bool				isunbreakable() const;
	void				loot();
	void				repair(int level);
	void				setcharges(int count);
	void				setcount(int count);
	void				set(identify_s value);
	void				set(item_type_s value) { magic = value; }
	void				set(enchantment_s value) { effect = value; }
	void				setforsale() { forsale = 1; }
	void				setsold() { forsale = 0; }
};
struct variant {
	variant_s			type;
	union {
		ability_s		ability;
		enchantment_s	enchantment;
		skill_s			skill;
		state_s			state;
		item_s			item;
		const char*		text;
	};
	constexpr variant() : type(NoVariant), skill(NoSkill) {}
	constexpr variant(skill_s v) : type(Skill), skill(v) {}
	constexpr variant(state_s v) : type(State), state(v) {}
	constexpr variant(enchantment_s v) : type(Enchantment), enchantment(v) {}
	constexpr variant(ability_s v) : type(Ability), ability(v) {}
	constexpr variant(item_s v) : type(Item), item(v) {}
	constexpr variant(const char* v) : type(Text), text(v) {}
	explicit operator bool() const { return type != NoVariant; }
	const char*			getname() const;
};
struct action {
	char				chance; // Usually from 1 to 10
	const char*			text;
	void(*proc)(creature& player, item& it, const action& e);
	damageinfo			damage;
	variant				value;
	unsigned			duration;
	explicit operator bool() const { return chance != 0; }
	void				apply(creature& player, item& it) const;
	void				applyrnd(creature & player, item & it) const;
	const action*		random() const;
};
struct attackinfo {
	char				speed;
	damageinfo			damage;
	char				bonus; // Percent bonus to hit
	char				critical;
	char				multiplier;
	enchantment_s		effect;
	char				quality;
};
struct command {
	unsigned short		move;
	skill_s				skill;
	spell_s				spell;
	constexpr command() : move(Blocked), skill(NoSkill), spell(NoSpell) {}
};
struct creature {
	race_s				race;
	class_s				type;
	gender_s			gender;
	role_s				role;
	direction_s			direction;
	unsigned short		name;
	unsigned char		level;
	int					experience;
	unsigned			money;
	unsigned short		position, guard;
	item				wears[LastBackpack + 1];
	//
	creature() = default;
	creature(role_s value);
	creature(race_s race, gender_s gender, class_s type);
	explicit operator bool() const { return hp > 0; }
	void* operator new(unsigned size);
	//
	void				act(const char* format, ...) const { actv(format, xva_start(format)); }
	void				actv(const char* format, const char* param) const;
	void				actv(creature& opponent, const char* format, const char* param) const;
	void				actnc(const char* format, ...) const { actvnc(format, xva_start(format)); }
	void				actvnc(const char* format, const char* param) const;
	void				actvs(creature& opponent, const char* format, ...) const { actv(opponent, format, xva_start(format)); }
	void				addexp(int count);
	skill_s				aiskill();
	skill_s				aiskill(aref<creature*> creatures);
	spell_s				aispell(aref<creature*> creatures, target_s target = NoTarget);
	bool				alertness();
	void				apply(state_s state, item_type_s magic, int quality, unsigned duration, bool interactive);
	bool				apply(const effectinfo& effect, int level, bool interactive, const char* format, const char* format_param, int skill_roll, int skill_value, void(*fail_proc)(effectparam& e) = 0);
	bool				askyn(creature* opponent, const char* format, ...);
	void				athletics(bool interactive);
	void				attack(creature* defender, slot_s slot, int bonus = 0, int multiplier = 0);
	bool				canhear(short unsigned index) const;
	void				chat(creature* opponent);
	item*				choose(aref<item*> source, bool interactive) const;
	creature*			choose(aref<creature*> source, bool interactive) const;
	short unsigned		choose(aref<short unsigned> source, bool interactive) const;
	void				applyability();
	void				clear();
	void				clear(state_s value) { states[value] = 0; }
	void				consume(int value, bool interactive);
	void				damage(int count, attack_s type, bool interactive);
	void				damagewears(int count, attack_s type);
	void				drink(item& it, bool interactive);
	void				dropdown(item& value);
	bool				equip(item_s value);
	bool				equip(item value);
	void				equip(slot_s slot, item value);
	int					get(ability_s value) const;
	int					get(spell_s value) const;
	int					get(skill_s value) const;
	int					getarmor() const;
	attackinfo			getattackinfo(slot_s slot) const;
	int					getattacktime(slot_s slot) const;
	int					getbasic(ability_s value) const;
	int					getbasic(skill_s value) const;
	int					getbonus(enchantment_s value) const;
	int					getcost(spell_s value) const;
	unsigned			getcostexp() const;
	static creature*	getcreature(short unsigned index);
	static aref<creature*> getcreatures(aref<creature*> result, short unsigned start, int range);
	int					getdefence() const;
	int					getdiscount(creature* customer) const;
	creature*			getenemy(aref<creature*> source) const;
	encumbrance_s		getencumbrance() const { return encumbrance; }
	char*				getfullname(char* result, const char* result_maximum, bool show_level, bool show_alignment) const;
	creature*			gethenchmen(int index) const;
	int					gethits() const { return hp; }
	creature*			gethorror() const { return horror; }
	creature*			getleader() const;
	int					getlos() const;
	int					getmana() const { return mp; }
	int					getmaxhits() const;
	int					getmaxmana() const;
	const char*			getmonstername() const;
	int					getmoverecoil() const;
	const char*			getname() const; // Name used predefined names array
	static const char*	getname(tile_s id);
	static const char*	getname(state_s id, bool cursed);
	static const char*	getname(skill_s id);
	creature*			getnearest(aref<creature*> source, targetdesc ti) const;
	static creature*	getplayer();
	short unsigned		getposition() const { return position; }
	damageinfo			getraise(skill_s id) const;
	site*				getsite() const { return current_site; }
	int					getweight() const;
	int					getweight(encumbrance_s id) const;
	void				heal(int value, bool interactive) { damage(-value, Magic, interactive); }
	void				hint(const char* format, ...) const;
	static void			initialize();
	bool				interact(short unsigned index);
	bool				is(state_s value) const;
	bool				is(encumbrance_s value) const { return encumbrance == value; }
	bool				isagressive() const;
	bool				isallow(spell_s id) const;
	static bool			isbooming();
	bool				ischaracter() const { return role == Character; }
	bool				isenemy(const creature* target) const;
	bool				isfriend(const creature* target) const;
	bool				isguard() const { return guard != 0xFFFF; }
	bool				ishenchman(const creature* target) const { return target == party; }
	bool				isleader() const { return party == this; }
	bool				isparty(const creature* target) const;
	bool				isplayer() const;
	bool				isranged(bool interactive) const;
	void				join(creature* party);
	void				levelup();
	void				lookfloor();
	void				makemove();
	void				manipulate(short unsigned index);
	void				meleeattack(creature* target, int bonus = 0, int multiplier = 0);
	bool				move(short unsigned index);
	bool				moveto(short unsigned index);
	bool				moveaway(short unsigned index);
	static void			play();
	void				pickup(item& value, bool interactive = true);
	void				raise(skill_s value);
	void				raiseskills(int number);
	void				rangeattack(creature* enemy);
	void				readbook(item& it);
	void				release(unsigned exeperience_cost) const;
	void				remove(state_s value);
	void				remove(adat<creature, 16>& source) const;
	bool				roll(skill_s skill, int bonus = 0);
	void				say(const char* format, ...);
	bool				sayv(const char* format, const char* param, creature* opponent);
	void				sayvs(creature& opponent, const char* format, ...);
	bool				saving(bool interactive, skill_s save, int bonus) const;
	static void			select(creature** result, rect rc);
	aref<item*>			select(aref<item*> result, target_s target) const;
	aref<creature*>		select(aref<creature*> result, aref<creature*> creatures, target_s target, char range, short unsigned start, const creature* exclude) const;
	aref<short unsigned> select(aref<short unsigned> result, target_s target, char range, short unsigned start, bool los = true) const;
	void				set(state_s value, unsigned segments, bool after_recoil = false, bool can_save = false);
	void				set(spell_s value, int level);
	void				setcharmer(creature* value) { charmer = value; }
	static void			setblocks(short unsigned* movements, short unsigned value);
	void				sethorror(creature* value) { horror = value; }
	void				setplayer();
	static void			setleader(const creature* party, creature* leader);
	void				setlos();
	void				trapeffect();
	static void			turnbegin();
	void				update();
	void				use(skill_s value);
	bool				use(spell_s value);
	bool				use(spell_s value, int level, const char* format, ...);
	void				use(item& it);
	bool				use(short unsigned index);
	bool				unequip(item& it);
	void				wait(int segments = 0);
private:
	friend struct archive;
	char				abilities[Charisma + 1];
	short				abilities_raise[Charisma + 1];
	short				hp, mhp, mp, mmp;
	unsigned			restore_hits, restore_mana;
	unsigned char		skills[LastResist + 1];
	unsigned char		spells[LastSpell + 1];
	unsigned			states[LastState + 1];
	unsigned			recoil;
	creature*			party;
	creature*			charmer;
	creature*			horror;
	site*				current_site;
	encumbrance_s		encumbrance;
	command				order;
	//
	static bool			playturn();
	void				updateweight();
	bool				walkaround(aref<creature*> creatures);
};
struct site : rect {
	site_s		type;
	diety_s			diety;
	unsigned char	name[2];
	creature*		owner;
	constexpr site() : rect({0, 0, 0, 0}), type(EmpthyRoom), diety(NoGod), name(), owner() {}
	operator bool() const { return x1 != x2; }
	char*			getname(char* result) const;
};
struct speech {
	speech_s		type;
	bool(*test)(const creature& player, const dialog& e);
	const char*		text;
	speech*			success;
	speech*			fail;
	bool			stop_analize;
	skillvalue		skill;
	void(*proc)(dialog& e, const speech& sp);
	explicit operator bool() const { return type != NoTalking; }
};
struct dialog {
	creature*		player;
	creature*		opponent;
	speech*			result;
	constexpr dialog(creature* player, creature* opponent) : player(player), opponent(opponent), result(0) {}
	void			start(const speech* p);
private:
	void			apply(const speech& e);
	aref<const speech*>	select(aref<const speech*> source, const speech* p, speech_s type) const;
	const speech*	say(const speech* pb, speech_s type);
	const speech*	phase(const speech* p);
};
struct groundinfo {
	item			object;
	short unsigned	index;
};
struct areainfo {
	short unsigned	index; // Позиция на карте мира
	short unsigned	positions[8]; // Several positions
	unsigned char	level; // Уровень поздземелья
	unsigned char	rooms; // Количество комнат
	bool			isdungeon; // Underground dungeons has 'true'
	constexpr areainfo() : index(Blocked), level(1), rooms(0), isdungeon(false),
		positions{Blocked, Blocked, Blocked, Blocked, Blocked, Blocked, Blocked, Blocked} {}
};
struct manual {
	typedef void(*proc)(stringbuffer& sc, manual& e);
	manual_s		type;
	variant			value;
	const char*		text;
	manual*			child;
	aref<proc>		procs;
	explicit operator bool() const { return value.type != 0; }
};
namespace game {
site*				add(site_s type, rect rc);
creature*			add(short unsigned index, creature* element);
bool				create(const char* id, short unsigned index, int level, bool explored = false, bool visualize = false);
int					distance(short unsigned i1, short unsigned i2);
void				drop(short unsigned i, item object);
unsigned short		genname(race_s race, gender_s gender);
inline short unsigned get(int x, int y) { return y * max_map_x + x; }
const attackinfo&	getattackinfo(trap_s slot);
direction_s			getdirection(point s, point d);
short unsigned		getfree(short unsigned i);
int					getindex(short unsigned i, tile_s value);
site*				getlocation(short unsigned i);
int					getitems(item** result, unsigned maximum_count, short unsigned index);
short unsigned		getmovement(short unsigned i);
const char*			getnamepart(unsigned short value);
creature*			getnearest(aref<creature*> source, short unsigned position);
creature*			getnearest(aref<creature*> source, short unsigned position, targetdesc ti);
int					getnight();
int					getrand(short unsigned i);
aref<site>			getsites();
short unsigned		getstepto(short unsigned index);
short unsigned		getstepfrom(short unsigned index);
const char*			getdate(char* result, const char* result_maximum, unsigned segments, bool show_time);
trap_s				gettrap(short unsigned i);
tile_s				gettile(short unsigned i);
map_object_s		getobject(short unsigned i);
inline unsigned char getx(short unsigned i) { return i % max_map_x; }
inline unsigned char gety(short unsigned i) { return i / max_map_x; }
void				initialize(short unsigned index, int level, tile_s tile);
bool				is(short unsigned i, map_flag_s v);
bool				isdungeon();
bool				ispassable(short unsigned i);
bool				ispassabledoor(short unsigned i);
bool				ispassablelight(short unsigned i);
void				looktarget(short unsigned index);
void				lookhere(short unsigned index);
void				makewave(short unsigned index, bool(*proc)(short unsigned) = ispassabledoor);
bool				serialize(bool writemode);
bool				serializep(short unsigned index, bool writemode);
bool				serializew(bool writemode);
void				set(short unsigned i, tile_s value);
void				set(short unsigned i, tile_s value, int w, int h);
void				set(short unsigned i, map_object_s value);
void				set(short unsigned i, map_flag_s type, bool value = true);
void				set(short unsigned i, unsigned char value);
extern areainfo		statistic;
short unsigned		to(short unsigned index, direction_s side);
direction_s			turn(direction_s from, direction_s side);
};
namespace logs {
struct driver : stringcreator {
	gender_s		gender;
	gender_s		gender_opponent;
	const char*		name;
	const char*		name_opponent;
	constexpr driver() : gender(Male), gender_opponent(Male), name(""), name_opponent("") {}
	constexpr driver(const char* name_param, gender_s gender_param) : name(name_param), gender(gender_param), name_opponent(""), gender_opponent(Male) {}
	void			parseidentifier(char* result, const char* result_max, const char* identifier) override;
};
void				add(const char* format, ...);
void				addnc(const char* format, ...);
void				add(int id, const char* format, ...);
void				addv(const char* format, const char* param);
void				addv(stringcreator& driver, const char* format, const char* param);
void				addvnc(stringcreator& driver, const char* format, const char* param);
short unsigned		choose(const creature& e, short unsigned* source, int count);
item*				choose(const creature& e, item** source, unsigned count, const char* title);
bool				choose(creature& e, skill_s& result, aref<skill_s> source, bool can_escape = true);
bool				choose(creature& e, skill_s& result, bool can_escape = true);
bool				choose(creature& e, spell_s& result, aref<spell_s> source);
bool				choose(creature& e, spell_s& result);
bool				chooseyn();
void				focusing(short unsigned index);
bool				getindex(const creature& e, short unsigned& result, targetdesc ti);
void				initialize();
int					input();
void				minimap(short unsigned position);
void				next();
void				raise(creature& e, int left);
void				turn(creature& e);
void				worldedit();
}
extern adat<groundinfo, 2048>	grounditems;
unsigned			getday();
unsigned			gethour();
unsigned			getminute();
unsigned			getmonth();
unsigned			getturn();
unsigned			getyear();
extern unsigned		segments;