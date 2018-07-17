#include "main.h"

short unsigned center(const rect& rc);

bool lowint(const creature& player, const dialog& e) {
	return player.get(Intellegence) <= 7;
}

bool noint(const creature& player, const dialog& e) {
	return player.get(Intellegence) <= 2;
}

static bool isguard(const creature& player, const dialog& e) {
	return player.getguard() != Blocked;
}

static bool isnoguard(const creature& player, const dialog& e) {
	return player.getguard() == Blocked;
}

static bool ishightpriest(const creature& player, const dialog& e) {
	auto p = player.getsite();
	return p && p->type == Temple && p->owner == &player;
}

static bool ishenchmen(const creature& player, const dialog& e) {
	return player.isfriend(e.player) && player.getrole() == Character;
}

static void setguard(dialog& e, const speech& sp) {
	e.opponent->setguard(e.opponent->getposition());
}

static void remguard(dialog& e, const speech& sp) {
	e.opponent->setguard(Blocked);
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
	// Remove FOW
	for(auto x = p->x1; x < p->x2; x++)
		for(auto y = p->y1; y < p->y2; y++)
			game::set(game::get(x, y), Explored, true);
	char name[260]; p->getname(name);
	e.opponent->say(maprnd(talk_start), name);
}

static speech hero_history[] = {{Speech, 0, "В основном тем чем придется. Немного собирал мусор, немного играл в карты."},
{}};
static speech party_member[] = {{Action, noint, "%герой дружественно рычить."},
{Action, noint, "%герой мурлыкая трется об ладонь."},
{Action, noint, "%герой с интересом смотрит на тебя.", 0, 0, true},
{Speech, lowint, "Чего твоя хотеть?"},
{Speech, lowint, "Угу?", 0, 0, true},
{Speech, 0, "Какие планы?"},
{Speech, 0, "Что будем делать?"},
{Speech, 0, "Говори."},
{Answer, isguard, "Пошли со мной.", 0, 0, false, {}, remguard},
{Answer, isnoguard, "Охраняй это место.", 0, 0, false, {}, setguard},
{Answer, 0, "Чего ты постоянно молчишь? Будь немного веселей. Трави шутки что-ли."},
{Answer, 0, "Хороший ты парень. ем ты занимался до встречи со мной?", hero_history},
{Answer, 0, "Ничего особенного. Продолжаем движение."},
{}};
static speech priest_talk[] = {{Speech, 0, "Что еще тебя мучит сын мой?"},
{Speech, 0, "Чего еще желаешь?"},
{Answer, 0, "Расскажи мне что это за место?", priest_talk},
{Answer, 0, "Такое огромное здание? И ты здесь главный?", priest_talk},
{Answer, 0, "Пожалуй мне пора идти.", priest_talk},
{}};
static speech smalltalk[] = {{Speech, ishenchmen, "Да, босс, слушаю.", 0, party_member, true},
{Action, noint, "%герой раздраженно рычит."},
{Action, noint, "%герой недоуменно смотрит на %ГЕРОЙ.", 0, 0, true},
{Speech, lowint, "Чего твоя хотеть?"},
{Speech, lowint, "Твоя тут нравиться?"},
{Speech, lowint, "Моя устал%а сегодня."},
{Speech, lowint, "Моя гулять здесь.", 0, 0, true},
{Speech, ishightpriest, "Здраствуй друг. Чего желаешь?", 0, priest_talk, true},
{Speech, 0, "Привет! Как дела?"},
{Speech, 0, "Хороший день, да?"},
{Speech, 0, "Эх! Устал%а я..."},
{Speech, 0, 0, 0, 0, false, {}, knownbuilding},
{}};

void creature::chat(creature* e) {
	dialog dg(this, e);
	dg.start(smalltalk);
}