#include <array>

template<typename T, size_t size>
class PrivateImp {
public:
	PrivateImp() = default;

	template<typename ... Args>
	PrivateImp( Args... args ) {
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