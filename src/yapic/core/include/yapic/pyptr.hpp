#ifndef C2C90D0B_F133_C961_1374_B789D0F0B01B
#define C2C90D0B_F133_C961_1374_B789D0F0B01B

#include <Python.h>

namespace Yapic {

#define __PyPtr_DECREF(V) \
	if (V != NULL) { Py_DECREF(V); }

/**
 * Usage:
 * PyPtr injector(Injector::Alloc());
 * // do something...
 * return injector.Steal()
 *
 * Note:
 * 	DONT USE ON BORROWED REFERENCES LIKE PyDict_GetItem
 */
template<typename O>
class _PyPtr {

	public:
		inline _PyPtr(O* var): _var(var) {  }
		inline ~_PyPtr() { __PyPtr_DECREF(_var); }
		inline O& operator* () const { return *_var; }
		inline O* operator-> () const { return _var; }

		inline _PyPtr<O>& operator= (const _PyPtr<O>& other) {
			if (this != &other) {
				__PyPtr_DECREF(_var);
				_var = other._var;
				if (_var != NULL) {
					Py_INCREF(_var);
				}
			}
			return *this;
		}

		inline _PyPtr<O>& operator= (_PyPtr<O>&& other) {
			if (this != &other) {
				__PyPtr_DECREF(_var);
				_var = other.Steal();
			}
			return *this;
		}

		inline _PyPtr<O>& operator= (const O* other) {
			if (_var != other) {
				__PyPtr_DECREF(_var);
				_var = other;
			}
			return *this;
		}

		inline operator PyTupleObject* () const { return (PyTupleObject*) _var; } \
		inline operator PyDictObject* () const { return (PyDictObject*) _var; }

		inline bool IsNull() const { return _var == NULL; }
		inline bool IsValid() const { return _var != NULL; }
		inline O* Steal() { O* tmp = _var; _var = NULL; return tmp; }
		inline void Clear() {
			if (_var != NULL) {
				Py_DECREF(_var);
				_var = NULL;
			}
		}
	protected:
		O* _var;
};


template<typename O = PyObject>
class PyPtr: public _PyPtr<O> {
public:
	using _PyPtr<O>::_PyPtr;

	inline operator O* () const { return (O*) _var; }
	inline operator PyObject* () const { return (PyObject*) _var; }

	inline _PyPtr<O>& operator= (const PyObject* other) {
		if (_var != ((O*) other)) {
			__PyPtr_DECREF(_var);
			_var = (O*) other;
		}
		return *this;
	}
};


template<>
class PyPtr<PyObject>: public _PyPtr<PyObject> {
public:
	using _PyPtr<PyObject>::_PyPtr;
	inline operator PyObject* () const { return (PyObject*) _var; }
};


} /* end namespace Yapic */

#endif /* C2C90D0B_F133_C961_1374_B789D0F0B01B */
