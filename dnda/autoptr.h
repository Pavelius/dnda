#pragma once

class autoptr {
	const void*			type;
	void*				data;
public:
	constexpr autoptr() : type(0), data(0) {}
	template<class T> constexpr autoptr(T* v) : type(&typeid(T)), data(v) {}
	template<class T> constexpr autoptr(const T* v) : type(&typeid(T)), data(v) {}
	template<class T> constexpr operator T*() const { return (&typeid(T) == type) ? static_cast<T*>(data) : 0; }
	constexpr operator bool() const { return data!=0; }
};
