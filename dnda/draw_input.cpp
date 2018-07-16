#include "crt.h"
#include "draw.h"

static int		current_command;
static void		(*current_execute)();

void draw::execute(int id, int param) {
	current_command = id;
	hot::key = 0;
	hot::param = param;
}

void draw::execute(void(*proc)(), int param) {
	execute(InputExecute, param);
	current_execute = proc;
}

int draw::input(bool redraw) {
	if(current_command) {
		if(current_execute) {
			current_execute();
			hot::key = InputUpdate;
			return hot::key;
		}
		hot::key = current_command;
		return hot::key;
	}
	// After render plugin events
	for(auto p = renderplugin::first; p; p = p->next)
		p->after();
	hot::key = InputUpdate;
	if(redraw)
		draw::sysredraw();
	else
		hot::key = draw::rawinput();
	if(!hot::key)
		exit(0);
	return hot::key;
}