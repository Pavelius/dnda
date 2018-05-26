#pragma once

template<typename T>
void csort(T* source, unsigned count) {
	for(auto pe = source + count; pe > source; pe--) {
		for(auto pb = source; pb < pe; pb++) {
			if(pb[0] > pb[1]) {
				auto i = pb[0];
				pb[0] = pb[1];
				pb[1] = i;
			}
		}
	}
}