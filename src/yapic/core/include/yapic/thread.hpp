#ifndef FE0AF659_D133_C9C2_126F_EC30482C41AD
#define FE0AF659_D133_C9C2_126F_EC30482C41AD

#include <Python.h>
#include <pythread.h>


namespace Yapic {


template<typename L>
class _AcquireLock {
public:
	inline _AcquireLock(L* lock, _PyTime_t timeout=0): _lock(lock) {
		assert(_lock != NULL);
		_lock->Acquire(timeout);
	}

	inline ~_AcquireLock() {
		assert(_lock != NULL);
		_lock->Release();
	}

private:
	L* _lock;
};


// copied from _threadmodule.c
static inline PyLockStatus AcquireTimed(PyThread_type_lock lock, _PyTime_t timeout) {
	PyLockStatus r;
    _PyTime_t endtime = 0;
    _PyTime_t microseconds;

    if (timeout > 0)
        endtime = _PyTime_GetMonotonicClock() + timeout;

    do {
        microseconds = _PyTime_AsMicroseconds(timeout, _PyTime_ROUND_CEILING);

        /* first a simple non-blocking try without releasing the GIL */
        r = PyThread_acquire_lock_timed(lock, 0, 0);
        if (r == PY_LOCK_FAILURE && microseconds != 0) {
            Py_BEGIN_ALLOW_THREADS
            r = PyThread_acquire_lock_timed(lock, microseconds, 1);
            Py_END_ALLOW_THREADS
        }

        if (r == PY_LOCK_INTR) {
            /* Run signal handlers if we were interrupted.  Propagate
             * exceptions from signal handlers, such as KeyboardInterrupt, by
             * passing up PY_LOCK_INTR.  */
            if (Py_MakePendingCalls() < 0) {
                return PY_LOCK_INTR;
            }

            /* If we're using a timeout, recompute the timeout after processing
             * signals, since those can take time.  */
            if (timeout > 0) {
                timeout = endtime - _PyTime_GetMonotonicClock();

                /* Check for negative values, since those mean block forever.
                 */
                if (timeout < 0) {
                    r = PY_LOCK_FAILURE;
                }
            }
        }
    } while (r == PY_LOCK_INTR);  /* Retry if we were interrupted. */

    return r;
}


class Lock {
public:
	using Auto = _AcquireLock<Lock>;

	inline Lock() {
		_pylock = PyThread_allocate_lock();
	}

	inline ~Lock() {
		if (_pylock) {
			PyThread_free_lock(_pylock);
			_pylock = NULL;
		}
	}

	inline bool IsNull() const { return _pylock == NULL; }
	inline bool IsValid() const { return _pylock != NULL; }

	inline void Acquire(_PyTime_t timeout) {
		PyLockStatus lockStatus = AcquireTimed(_pylock, timeout);
		assert(lockStatus != PY_LOCK_FAILURE);
	}

	inline void Release() {
		if (_pylock) {
			PyThread_release_lock(_pylock);
		}
	}

private:
	PyThread_type_lock _pylock;
};


class RLock {
public:
	using Auto = _AcquireLock<RLock>;

	inline RLock(): _count(0), _tid(0) {
		_pylock = PyThread_allocate_lock();
	}

	inline ~RLock() {
		if (_pylock) {
			PyThread_free_lock(_pylock);
			_pylock = NULL;
		}
	}

	inline bool IsNull() const { return _pylock == NULL; }
	inline bool IsValid() const { return _pylock != NULL; }

	inline void Acquire(_PyTime_t timeout) {
		long current_tid = PyThread_get_thread_ident();
		if (_count > 0 && _tid == current_tid) {
			++_count;
		} else {
			PyLockStatus lockStatus = AcquireTimed(_pylock, timeout);
			if (lockStatus == PY_LOCK_ACQUIRED) {
				assert(_count == 0);
				_count = 1;
				_tid = current_tid;
			}
			assert(lockStatus != PY_LOCK_FAILURE);
		}
	}

	inline void Release() {
		if (_pylock) {
			long current_tid = PyThread_get_thread_ident();

			if (_count > 0 && _tid == current_tid) {
				if (--_count == 0) {
					_tid = 0;
					PyThread_release_lock(_pylock);
				}
			}
		}
	}

private:
	PyThread_type_lock _pylock;
	size_t _count;
	long _tid;
};


} /* end namespace Yapic */

#endif /* FE0AF659_D133_C9C2_126F_EC30482C41AD */
