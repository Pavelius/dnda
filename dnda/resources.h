#pragma once

struct sprite;
namespace res {
struct element {
	const char*		name;
	const char*		folder;
	sprite*			data;
	bool			isfolder;
	bool			notfound;
};
void				cleanup();
extern element		elements[];
const char*			getname(int rid);
}
sprite*				gres(int rid);
