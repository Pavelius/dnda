#include "crt.h"
#include "command.h"
#include "draw.h"

command*	command_after_render;
command*	command_clear_render;
static int	current_command;
extern rect	sys_static_area;

void draw::execute(int id, int param) {
	hot::key = 0;
	current_command = id;
	hot::param = param;
}

int draw::input(bool redraw) {
	auto temp_hotkey = hot::key;
	auto temp_command = current_command;
	// Очистим данные
	current_command = 0;
	hot::key = 0;
	command_clear_render->execute();
	// Если была команда, надо ее выполнить
	if(temp_command) {
		hot::key = temp_command;
		return hot::key;
	}
	if(hot::key)
		return hot::key;
	// Нарисуем функционал расширения после выполнения всех комманд.
	// Таким образм скриншот, если он делается по команде не будет иметь
	// Такие вещи как строка сообщения и подсказка.
	command_after_render->execute();
	int id = InputUpdate;
	if(redraw)
		draw::sysredraw();
	else
		id = draw::rawinput();
	if(!id)
		exit(0);
	if(hot::mouse.x < 0 || hot::mouse.y < 0)
		sys_static_area.clear();
	else
		sys_static_area = {0, 0, draw::getwidth(), draw::getheight()};
	hot::cursor = CursorArrow;
	return id;
}