#ifndef C2C90D0B_F133_C961_1374_B789D0F0B01B
#define C2C90D0B_F133_C961_1374_B789D0F0B01B

#include <Python.h>

namespace Yapic {

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
		inline _PyPtr(): _var(reinterpret_cast<O*>(Py_None)) { Py_INCREF(Py_None); }
		inline _PyPtr(O* var): _var(var) {  }
		inline _PyPtr(_PyPtr<O>&& other): _var(other._var) {  }
		inline ~_PyPtr() { Py_XDECREF(_var); }
		inline O& operator* () const { return *_var; }
		inline O* operator-> () const { return _var; }
		inline operator bool() const { return _var != NULL; }

		inline _PyPtr<O>& operator= (const _PyPtr<O>& other) {
			if (this != &other) {
				Py_XDECREF(_var);
				Py_XINCREF(other._var);
				_var = other._var;
			}
			return *this;
		}

		inline _PyPtr<O>& operator= (_PyPtr<O>&& other) {
			if (this != &other) {
				Py_XDECREF(_var);
				if (other._var != NULL) {
					_var = other.Steal();
				} else {
					_var = NULL;
				}
			}
			return *this;
		}

		inline _PyPtr<O>& operator= (const O* other) = delete;

		inline operator PyVarObject* () const { return (PyVarObject*) _var; }
		// deprecated
		// inline operator PyTupleObject* () const { return (PyTupleObject*) _var; }
		// inline operator PyDictObject* () const { return (PyDictObject*) _var; }
		// inline operator PyLongObject* () const { return (PyLongObject*) _var; }
		// inline operator PyFloatObject* () const { return (PyFloatObject*) _var; }
		// inline operator PyByteArrayObject* () const { return (PyByteArrayObject*) _var; }
		// inline operator PyBytesObject* () const { return (PyBytesObject*) _var; }
		// inline operator PyUnicodeObject* () const { return (PyUnicodeObject*) _var; }
		// inline operator PyTypeObject* () const { return (PyTypeObject*) _var; }
		// inline operator PyCodeObject* () const { return (PyCodeObject*) _var; }
		// inline operator PyComplexObject* () const { return (PyComplexObject*) _var; }

		template<typename AS>
		inline AS* As() const { return reinterpret_cast<AS*>(_var); }
		inline PyObject* AsObject() const { return reinterpret_cast<PyObject*>(_var); };

		inline bool IsNull() const { return _var == NULL; }
		inline bool IsValid() const { return _var != NULL; }
		inline bool IsNone() const { return _var == Py_None; }
		inline void Incref() { assert(_var != NULL); Py_INCREF(_var); }
		inline void Decref() { assert(_var != NULL); Py_DECREF(_var); }
		inline O* Steal() { assert(_var != NULL); O* tmp = _var; _var = NULL; return tmp; }
		inline void Clear() { Py_CLEAR(_var); }
	protected:
		O* _var = NULL;
};


template<typename O = PyObject>
class PyPtr: public _PyPtr<O> {
public:
	using _PyPtr<O>::_PyPtr;
	inline PyPtr(): _PyPtr<O>::_PyPtr() { }
	inline PyPtr(PyObject* var): _PyPtr<O>::_PyPtr(reinterpret_cast<O*>(var)) {  }


	inline operator O* () const { return (O*) this->_var; }
	inline operator PyObject* () const { return (PyObject*) this->_var; }

	inline _PyPtr<O>& operator= (const PyObject* other) {
		// Py_XINCREF(other);
		Py_XDECREF(this->_var);
		this->_var = (O*) other;
		return *this;
	}
};


template<>
class PyPtr<PyObject>: public _PyPtr<PyObject> {
public:
	using _PyPtr<PyObject>::_PyPtr;
	inline operator PyObject* () const { return (PyObject*) this->_var; }
};


} /* end namespace Yapic */

#endif /* C2C90D0B_F133_C961_1374_B789D0F0B01B */
