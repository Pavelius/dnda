#include "initializer_list.h"

#pragma once

// Abstract flag data bazed on enumerator
template<typename T, typename DT = unsigned> class cflags {
	static constexpr T maximum = (T)(sizeof(DT) * 8);
	struct iter {
		T	current;
		DT	data;
		iter(T current, DT data) : current(getnext(current, data)), data(data) {}
		T operator*() const { return (T)current; }
		bool operator!=(const iter& e) const { return e.current != current; }
		void operator++() { current = getnext((T)(current + 1), data);}
		constexpr T getnext(T current, unsigned data) const {
			while(current < maximum && (data & (1 << current)) == 0)
				current = (T)(current + 1);
			return current;
		}
	};
public:
	DT				data;
	constexpr cflags() : data(0) {}
	constexpr cflags(std::initializer_list<T> list) : data() { for(auto e : list) add(e); }
	constexpr void	add(T id) { data |= 1 << id; }
	constexpr operator DT() const { return data; }
	iter			begin() const { return iter((T)0, data); }
	void			clear() { data = 0; }
	iter			end() const { return iter(maximum, data); }
	constexpr bool	is(T id) const { return (data & (1 << id)) != 0; }
	constexpr void	remove(T id) { data &= ~(1 << id); }
};