#include "main.h"

short unsigned center(const rect& rc);

static bool lowint(const creature& player, const dialog& e) {
	return player.get(Intellegence) <= 7;
}

static bool isanimal(const creature& player, const dialog& e) {
	return player.race == Animal;
}

static bool isguard(const creature& player, const dialog& e) {
	return player.guard != Blocked;
}

static bool isnoguard(const creature& player, const dialog& e) {
	return player.guard == Blocked;
}

static void setguard(dialog& e, const speech& sp) {
	e.opponent->guard = e.opponent->position;
}

static void remguard(dialog& e, const speech& sp) {
	e.opponent->guard = Blocked;
}

static site* get_non_explored_location() {
	site* source[128];
	auto pb = source;
	auto pe = source + sizeof(source) / sizeof(source[0]);
	for(auto& e : game::getsites()) {
		if(e.type == House)
			continue;
		if(game::is(center(e), Explored))
			continue;
		*pb++ = &e;
	}
	auto count = pb - source;
	if(!count)
		return 0;
	return source[rand() % count];
}

static void knownbuilding(dialog& e, const speech& sp) {
	auto p = get_non_explored_location();
	if(!p) {
		static const char* talk[] = {
			"Я бы тебе рассказал, что и как тут у нас, но похоже ты и так все знаешь.",
			"Да ты и так все знаешь - мне нечего тебе рассказать.",
			"К сожелению, мне нечего тебе расказать.",
		};
		e.opponent->say(maprnd(talk));
		return;
	}
	static const char* talk_start[] = {
		"Не так далеко отсюда находится %1.",
		"В той стороне находится %1.",
		"Если пойдешь в том направлении найдешь %1.",
	};
	// Исследуем область
	for(auto x = p->x1; x < p->x2; x++)
		for(auto y = p->y1; y < p->y2; y++)
			game::set(game::get(x, y), Explored, true);
	char name[260]; p->getname(name);
	e.opponent->say(maprnd(talk_start), name);
}

static speech old_house[] = {{Speech, 0, "А что рассказывать? Он был разрушен еще со времен войны орков. Сейчас зарос бурьяном и там никто не живет."},
{Speech, 0, "Много разных слухов ходят об этом старом доме. Говрят там когда-то повесился какой-то знаменитый дворянин."},
{Speech, 0, "У нас ходит легенда, что по ночам что-то появляется у того дома в полнолуние. Но это лишь легенда - никто никогда ничего не видел."},
{Speech, 0, "Обычный такой дом. Со странностями, правда."},
{}};
static speech test_dialog[] = {{Speech, 0, "Привет %ГЕРОЙ. Чем могу быть полезен?"},
{Answer, 0, "Что вы можете рассказать про старый дом у обочины?", old_house},
{Answer, 0, "Где мы могли бы найти достойное оружие или броню?"},
{Answer, 0, "Возможно нам стоит обсудить переспективы дальнейшего сотрудничества?", 0, 0, false, {Diplomacy, 20}},
{Answer, 0, "Пока ничего не надо."},
{}};
static speech party_member[] = {{Speech, isanimal, "Дружественно рычить."},
{Speech, isanimal, "Мурлыкая трется об ладонь.", 0, 0, true},
{Speech, lowint, "Чего твоя хететь?"},
{Speech, lowint, "Угу?", 0, 0, true},
{Speech, 0, "Какие планы?"},
{Speech, 0, "Что будем делать?"},
{Speech, 0, "Говори."},
{Answer, isguard, "Пошли со мной.", 0, 0, false, {}, remguard},
{Answer, isnoguard, "Охраняй это место.", 0, 0, false, {}, setguard},
{Answer, 0, "Ничего особенного. Продолжаем движение."},
{}};
static speech smalltalk[] = {{Action, isanimal, "%герой раздраженно рычит."},
{Action, isanimal, "%герой недоуменно смотрит на %ГЕРОЙ.", 0, 0, true},
{Speech, lowint, "Чего твоя хотеть?"},
{Speech, lowint, "Твоя тут нравиться?"},
{Speech, lowint, "Моя устал%а сегодня."},
{Speech, lowint, "Моя гулять здесь.", 0, 0, true},
{Speech, 0, "Привет! Как дела?"},
{Speech, 0, "Хороший день, да?"},
{Speech, 0, "Эх! Устал%а я..."},
{Speech, 0, 0, 0, 0, false, {}, knownbuilding},
{}};

void creature::chat(creature* e) {
	dialog dg(this, e);
	if(isfriend(e) && e->getleader() == this && e->role == Character)
		dg.start(party_member);
	else
		dg.start(smalltalk);
}