#include "crt.h"
#include "io.h"
#include "package.h"

void package::pack(const char* url, const char* dest, bool remove) {
	package::header			h = {};
	package::header::file	elements[8192];
	char					buffer[1024];
	h.signature[0] = 'P';
	h.signature[1] = 'K';
	h.signature[2] = 'G';
	h.signature[3] = 0;
	h.count = 0;
	for(auto e = io::file::find(url); e; e.next()) {
		if(e.name()[0] == '.')
			continue;
		char temp[261];
		zcpy(elements[h.count].name, e.name(), sizeof(elements[h.count].name) - 1);
		io::file e1(e.fullname(temp), StreamRead);
		elements[h.count].size = e1.getsize();
		h.count++;
	}
	// Update offset
	unsigned ofs = sizeof(h) + sizeof(elements[0]) * h.count;
	for(unsigned i = 0; i < h.count; i++) {
		elements[i].offset = ofs;
		ofs += elements[i].size;
	}
	// Write body
	io::file e(dest, StreamWrite);
	e.write(&h, sizeof(h));
	e.write(&elements, sizeof(elements[0])*h.count);
	for(unsigned i = 0; i < h.count; i++) {
		char temp[261]; szprint(temp, "%1/%2", url, elements[i].name);
		auto e1 = io::file(temp, StreamRead);
		if(!e1)
			continue;
		auto s = elements[i].size;
		while(s > 0) {
			auto n = sizeof(buffer);
			if(s < n)
				n = s;
			e1.read(buffer, n);
			e.write(buffer, n);
			s -= n;
		}
	}
}

bool package::unpack(const char* url, const char* source) {
	io::file e(source, StreamRead);
	if(!e)
		return false;
	header hdr; e.read(&hdr, sizeof(hdr));
	header::file elements[8192];
	char	buffer[1024];
	bool isvalid = hdr.signature[0] == 'P' && hdr.signature[1] == 'K' && hdr.signature[2] == 'G' && hdr.signature[3] == 0;
	if(!isvalid)
		return false;
	e.read(elements, sizeof(elements[0])*hdr.count);
	for(unsigned i = 0; i < hdr.count; i++) {
		char temp[261]; szprint(temp, "%1/%2", url, elements[i].name);
		if(true) {
			auto e1 = io::file(temp, StreamWrite);
			if(!e1)
				continue;
			e.seek(elements[i].offset, SeekSet);
			auto s = elements[i].size;
			while(s > 0) {
				auto n = sizeof(buffer);
				if(s < n)
					n = s;
				e.read(buffer, n);
				e1.write(buffer, n);
				s -= n;
			}
		}
	}
	return true;
}