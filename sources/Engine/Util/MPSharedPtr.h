#ifndef MPSharedPtr_H
#define MPSharedPtr_H

#include "MPEffPrerequisites.h"

template <class T>
class MPSharedPtr {
protected:
	T* _pRep;
	unsigned int* _pUseCount;

public:
	MP_AUTO_SHARED_MUTEX; // 

	/** SharedPtr
	@remarks
	<b>!</b> SharedPtrbind() .
	*/
	MPSharedPtr() : _pRep(0), _pUseCount(0) {
	}

	explicit MPSharedPtr(T* rep) : _pRep(rep), _pUseCount(new unsigned int(1)) {
		MP_NEW_AUTO_SHARED_MUTEX
	}

	MPSharedPtr(const MPSharedPtr& r) {
		// mutex
		MP_LOCK_MUTEX(*(r.MP_AUTO_MUTEX_NAME));
		MP_COPY_AUTO_SHARED_MUTEX(r.MP_AUTO_MUTEX_NAME);

		_pRep = r._pRep;
		_pUseCount = r._pUseCount;

		if (_pUseCount) {
			++(*_pUseCount);
		}
	}

	MPSharedPtr& operator=(const MPSharedPtr& r) {
		if (_pRep == r._pRep)
			return *this;
		release();

		// mutex
		MP_LOCK_MUTEX(*(r.MP_AUTO_MUTEX_NAME));
		MP_COPY_AUTO_SHARED_MUTEX(r.MP_AUTO_MUTEX_NAME);

		_pRep = r._pRep;
		_pUseCount = r._pUseCount;
		if (_pUseCount) {
			++(*_pUseCount);
		}
		return *this;
	}

	virtual ~MPSharedPtr() {
		release();
	}

	inline T& operator*() const {
		assert(_pRep);
		return *_pRep;
	}

	inline T* operator->() const {
		assert(_pRep);
		return _pRep;
	}

	inline T* get() const {
		return _pRep;
	}

	/** SharedPtr.
	@remarks
		SharedPtr!
	*/
	void bind(T* rep) {
		assert(!pRep && !pUseCount);
		MP_NEW_AUTO_SHARED_MUTEX;
		MP_LOCK_AUTO_SHARED_MUTEX;

		_pUseCount = new unsigned int(1);
		_pRep = rep;
	}

	inline bool unique() const {
		assert(_pUseCount);
		MP_LOCK_AUTO_SHARED_MUTEX;
		return (*_pUseCount) == 1;
	}

	inline unsigned int useCount() const {
		assert(_pUseCount);
		MP_LOCK_AUTO_SHARED_MUTEX return *_pUseCount;
	}

	inline unsigned int* useCountPointer() const {
		return _pUseCount;
	}

	inline T* getPointer() const {
		return _pRep;
	}

	inline bool isNull(void) const {
		return _pRep == 0;
	}

	inline void setNull(void) {
		if (_pRep) {
			release();
			_pRep = 0;
			_pUseCount = 0;
			MP_COPY_AUTO_SHARED_MUTEX(0)
		}
	}

protected:
	inline void release(void) {
		bool destroyThis = false;
		{
			// mutex()
			MP_LOCK_AUTO_SHARED_MUTEX;

			if (_pUseCount) {
				if (--(*_pUseCount) == 0) {
					destroyThis = true;
				}
			}
			// boost
		}
		if (destroyThis)
			destroy();
	}

	virtual void destroy(void) {
		// setNull(),
		delete _pRep;
		delete _pUseCount;
		MP_DELETE_AUTO_SHARED_MUTEX;
	}
};

template <class T, class U>
inline bool operator==(MPSharedPtr<T> const& a, MPSharedPtr<U> const& b) {
	return a.get() == b.get();
}

template <class T, class U>
inline bool operator!=(MPSharedPtr<T> const& a, MPSharedPtr<U> const& b) {
	return a.get() != b.get();
}

#endif
