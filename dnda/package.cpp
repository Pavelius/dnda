#include "crt.h"
#include "io.h"
#include "package.h"

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
void				pack(const char* url, const char* dest);
void				unpack(const char* url, const char* source);
}

void package::pack(const char* url, const char* dest) {
	package::header			source = {};
	package::header::file	elements[8192];
	source.signature[0] = 'P';
	source.signature[1] = 'K';
	source.signature[2] = 'G';
	source.signature[3] = 0;
	source.count = 0;
	unsigned ofs = 0;
	for(auto e = io::file::find(url); e; e.next()) {
		if(e.name()[0] == '.')
			continue;
		char temp[261];
		elements[source.count].offset = ofs;
		zcpy(elements[source.count].name, e.name(), sizeof(elements[source.count].name) - 1);
		auto e1 = io::file(e.fullname(temp), StreamRead);
		elements[source.count].size = e1.getsize();
		ofs += elements[source.count].size;
		source.count++;
	}
	auto e = io::file(dest, StreamWrite);
	e.write(&source, sizeof(source));
	e.write(&elements, sizeof(elements[0])*source.count);
	for(unsigned i = 0; i < source.count; i++) {
		char temp[261]; szprint(temp, "%1%2", url, elements[i].name);
		auto e1 = io::file(temp, StreamRead);
		if(!e1)
			continue;
	}
}