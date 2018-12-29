#pragma once

namespace package {
struct header {
	struct file {
		char		name[32];
		unsigned	offset;
		unsigned	size;
	};
	char			signature[4];
	unsigned		count;
};
void				pack(const char* url, const char* dest, bool remove = false);
bool				unpack(const char* url, const char* source);
}
