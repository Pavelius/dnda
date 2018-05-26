#pragma once

// Use this macro and don't have any problem
#define COMMAND(name) static void name();\
extern command* command_##name;\
static command local_##name(&command_##name, name);\
static void name()

struct command
{
	command*		next;
	void			(*proc)();
	command(command** first, void(*proc)());
	void			execute();
};

extern command*		command_app_initialize; // Standart initialization command