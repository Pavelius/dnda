#include "initializer_list.h"

extern "C" int				rand(void);
extern "C" void*			realloc(void *ptr, unsigned size);
extern "C" int				strcmp(const char* p1, const char* p2);

#pragma once

// Abstract collection
struct collection {
	virtual void*			add() = 0; // Add new empthy element to collection
	void					add(const void* element); // Add new element to collection
	virtual void			clear() = 0; // Clear all elements in collection
	int						find(const char* value, unsigned offset = 0); // Find value by name
	virtual void*			get(int index) const = 0; // Get content of element with index
	virtual unsigned		getmaxcount() const = 0; // Get maximum possible elements
	virtual unsigned		getcount() const = 0; // Get count of elements in collection
	virtual unsigned		getsize() const = 0; // Get size of one element in collection
	virtual int				indexof(const void* element) const = 0; // Get index of element (-1 if not in collection)
	bool					read(const char* url, const struct bsreq* fields);
	virtual void			remove(int index, int count = 1) = 0; // Remove element from collection
	void					sort(int i1, int i2, int(*compare)(const void* p1, const void* p2, void* param), void* param);	// Sort collection
	void					swap(int i1, int i2); // Swap elements
	bool					write(const char* url, const struct bsreq* fields);
};
// Storge like vector
template<class T, int count_max = 128>
struct adat {
	T						data[count_max];
	unsigned				count;
	//
	constexpr adat() : data(), count(0) {}
	constexpr adat(std::initializer_list<T> list) : count(0) { for(auto& e : list) *add() = e; }
	constexpr const T& operator[](unsigned index) const { return data[index]; }
	constexpr T& operator[](unsigned index) { return data[index]; }
	explicit operator bool() const { return count != 0; }
	T*						add() { if(count < count_max) return data + (count++); return 0; }
	void					add(const T& e) { if(count < count_max) data[count++] = e; }
	T*						begin() { return data; }
	const T*				begin() const { return data; }
	void					clear() { count = 0; }
	T*						endof() { return data + count_max; }
	const T*				end() const { return data + count; }
	template<class Z> T*	find(Z id) { auto e1 = data + count; for(T* e = data; e < e1; e++) { if(e->id == id) return e; } return 0; }
	inline int				getcount() const { return count; }
	int						indexof(const T* e) const { if(e >= data && e <= data + count) return e - data; return -1; }
	int						indexof(const T t) const { for(unsigned i = 0; i < count; i++) if(data[i] == t) return i; return -1; }
	bool					is(const T t) const { for(unsigned i = 0; i < count; i++) if(data[i] == t) return true; return false; }
	T						random() const { return count ? data[rand() % count] : T(); }
	void					remove(int index, int remove_count = 1) { if(index < 0) return; if(index<int(count - 1)) memcpy(data + index, data + index + 1, sizeof(data[0])*(count - index - 1)); count--; }
};
// Reference to array with dymanic size
template<class T> struct aref {
	T*						data;
	unsigned				count;
	constexpr aref() = default;
	constexpr aref(T* data, unsigned count) : data(data), count(count) {}
	template<unsigned N> constexpr aref(T(&data)[N]) : data(data), count(N) {}
	template<unsigned N> constexpr aref(const adat<T, N>& source) : data(const_cast<T*>(source.data)), count(source.count) {}
	constexpr T& operator[](int index) { return data[index]; }
	constexpr const T& operator[](int index) const { return data[index]; }
	explicit operator bool() const { return count != 0; }
	T*						add() { return &data[count++]; }
	void					add(const T& e) { data[count++] = e; }
	constexpr T*			begin() { return data; }
	constexpr const T*		begin() const { return data; }
	void					clear() { count = 0; }
	constexpr T*			end() { return data + count; }
	constexpr const T*		end() const { return data + count; }
	int						indexof(const T* t) const { if(t<data || t>data + count) return -1; return t - data; }
	int						indexof(const T t) const { for(unsigned i = 0; i < count; i++) if(data[i] == t) return i; return -1; }
	bool					is(const T value) const { return indexof(value) != -1; }
	T						random() const { return count ? data[rand() % count] : T(); }
	void					remove(int index, int elements_count = 1) { if(index < 0 || index >= count) return; count -= elements_count; if(index >= count) return; memmove(data + index, data + index + elements_count, sizeof(data[0])*(count - index)); }
};
//
template<class T> struct avec : aref<T> {
	constexpr avec() : aref<T>(0, 0), count_max() {}
	constexpr avec(avec&& that) : count_max(that.count_max) { data = that.data; count = that.count; that.data = 0; that.count = 0; that.count_max = 0; }
	~avec() { this->clear(); if(this->data) delete[] this->data; this->data = 0; }
	T* add() {
		if(this->count < count_max)
			return aref<T>::add();
		if(!count_max) {
			count_max = 256;
			this->data = new T[count_max];
		} else {
			if(count_max < 256 * 256 * 4)
				count_max = count_max * 2;
			else
				count_max += 256 * 256 * 4;
			this->data = (T*)realloc(this->data, count_max * sizeof(T));
		}
		return aref<T>::add();
	}
	void add(const T& e) { *add() = e; }
private:
	unsigned				count_max;
};
// Abstract flag data bazed on enumerator
template<typename T, typename DT = unsigned> class cflags {
	static constexpr T maximum = (T)(sizeof(DT) * 8);
	struct iter {
		T	current;
		DT	data;
		iter(T current, DT data) : current(getnext(current, data)), data(data) {}
		T operator*() const { return (T)current; }
		bool operator!=(const iter& e) const { return e.current != current; }
		void operator++() { current = getnext((T)(current + 1), data); }
		constexpr T getnext(T current, unsigned data) const {
			while(current < maximum && (data & (1 << current)) == 0)
				current = (T)(current + 1);
			return current;
		}
	};
public:
	DT						data;
	constexpr cflags() : data(0) {}
	constexpr cflags(std::initializer_list<T> list) : data() { for(auto e : list) add(e); }
	constexpr void			add(T id) { data |= 1 << id; }
	iter					begin() const { return iter((T)0, data); }
	void					clear() { data = 0; }
	iter					end() const { return iter(maximum, data); }
	constexpr bool			is(T id) const { return (data & (1 << id)) != 0; }
	constexpr void			remove(T id) { data &= ~(1 << id); }
};