#include "crt.h"
#include "draw.h"

using namespace draw;

static callback_proc	current_execute;

void draw::execute(callback_proc proc, int param) {
	current_execute = proc;
	hot::param = param;
}

void draw::domodal() {
	if(current_execute) {
		current_execute();
		hot::key = InputUpdate;
	} else {
		hot::key = draw::rawinput();
		if(!hot::key)
			exit(0);
	}
}