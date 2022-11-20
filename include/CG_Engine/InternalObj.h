#ifndef INTERNAL_OBJ_H
#define INTERNAL_OBJ_H

#include <array>

template<typename T, size_t size>
class InternalObj {
public:
	template<typename ... Args>
	InternalObj( Args... args ) {
		new ( m_storage.data() ) T( std::forward<Args>( args )... );
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