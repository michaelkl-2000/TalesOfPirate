#ifndef MPSingleton_H
#define MPSingleton_H

/** 
*/
template <typename T>
class MPSingleton {
protected:
	static T* _sSingleton;

public:
	MPSingleton(void) {
		assert(!_sSingleton);
#if defined( _MSC_VER ) && _MSC_VER < 1200
		int offset = (int)(T*)1 - (int)(MPSingleton<T>*)(T*)1;
		_sSingleton = (T*)((int)this + offset);
#else
		_sSingleton = static_cast<T*>(this);
#endif
	}

	~MPSingleton(void) {
		assert(ms_Singleton);
		_sSingleton = 0;
	}

	static T& getSingleton(void) {
		assert(ms_Singleton);
		return (*_sSingleton);
	}

	static T* getSingletonPtr(void) {
		return _sSingleton;
	}
};


#endif
