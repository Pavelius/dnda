#pragma once

class anyptr {
	const void*			type;
	void*				data;
public:
	constexpr anyptr() : type(0), data(0) {}
	constexpr anyptr(int v) : type(0), data((void*)v) {}
	template<class T> constexpr anyptr(T* v) : type(&typeid(T)), data(v) {}
	template<class T> constexpr anyptr(const T* v) : type(&typeid(T)), data(v) {}
	template<class T> constexpr operator T*() const { return (&typeid(T) == type) ? static_cast<T*>(data) : 0; }
	constexpr operator int() const { return (type==0) ? int(data) : 0; }
	constexpr explicit operator bool() const { return data!=0; }
	void				clear() { data = 0; }
};
