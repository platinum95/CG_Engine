#ifndef INTERNAL_OBJ_H
#define INTERNAL_OBJ_H

#include <array>

template<typename T, size_t size>
class InternalObj {
public:
	InternalObj() = default;

	template<typename ... Args>
	InternalObj( Args... args ) {
		T *member = GetMember();
		*member = T( args... );
	}

	inline T *operator->() {
		return GetMember();
	}
private:
	T *GetMember() {
		return reinterpret_cast<T *>( m_storage.data() );
	}
	std::array<uint8_t, size> m_storage;
};

#endif // INTERNAL_OBJ_H