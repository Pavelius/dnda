#include "main.h"

static void chat_smalltalk(creature* player, creature* opponent) {
	static const char* talk[] = {
		"Привет! Как дела?",
		"Хороший день, да?",
		"Эх! Устал%а я...",
	};
	player->say(maprnd(talk), opponent->getname());
}

static void chat_boss(creature* player, creature* opponent) {
	static const char* talk[] = {
		"Какие планы?",
		"Что будем делать?",
		"Говори.",
	};
	player->say(maprnd(talk), opponent->getname());
}

static location* get_non_explored_location() {
	location* source[128];
	auto pb = source;
	auto pe = source + sizeof(source) / sizeof(source[0]);
	for(auto& e : locations) {
		if(e.type == House)
			continue;
		rect rc = e;
		//if(game::isexplore(center(rc)))
		//	continue;
		*pb++ = &e;
	}
	auto count = pb - source;
	if(!count)
		return 0;
	return source[rand() % count];
}

static bool chat_location(creature* player, creature* opponent) {
	auto p = get_non_explored_location();
	if(!p)
		return false;
	static const char* talk_start[] = {
		"Не так далеко отсюда находится %1.",
		"В той стороне находится %1.",
		"Если пойдешь в том направлении найдешь %1.",
	};
	// Исследуем область
	for(auto x = p->x1; x <= p->x2; x++)
		for(auto y = p->y1; y <= p->y2; y++)
			game::set(game::get(x, y), Explored, true);
	char temp[4096];
	char name[260]; p->getname(name);
	szprints(temp, zendof(temp), maprnd(talk_start), name);
	player->say(temp);
	return true;
}

void creature::chat(creature* e) {
	enum boss_commands {
		NoBossCommand,
		GuardPlace, FollowMe,
	};
	if(isfriend(e) && e->getparty() == this && e->role == Character) {
		chat_boss(e, this);
		if(e->isguard())
			logs::add(FollowMe, "Пошли со мной.");
		else
			logs::add(GuardPlace, "Охраняй это место.");
		logs::add(NoBossCommand, "Ничего. Продолжаем движение.");
		switch(logs::input()) {
		case GuardPlace:
			e->guard = e->position;
			break;
		case FollowMe:
			e->guard = 0xFFFF;
			e->party = this;
			break;
		}
		return;
	}
	if(d100() < 30 && chat_location(e, this)) {
		e->wait(3);
		return;
	}
	chat_smalltalk(e, this);
	e->wait(2);
}