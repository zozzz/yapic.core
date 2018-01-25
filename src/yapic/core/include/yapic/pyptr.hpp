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
		inline _PyPtr(O* value): ref(value) {  }
		inline ~_PyPtr() { Py_XDECREF(this->ref); }
		inline O& operator* () const { assert(this->ref != NULL); return *this->ref; }
		inline O* operator-> () const { assert(this->ref != NULL); return this->ref; }
		inline operator bool() const { return this->ref != NULL; }

		inline _PyPtr<O>& operator= (const _PyPtr<O>& other) {
			if (this != &other) {
				Py_XDECREF(this->ref);
				Py_XINCREF(other.ref);
				this->ref = other.ref;
			}
			return *this;
		}

		inline _PyPtr<O>& operator= (_PyPtr<O>&& other) {
			if (this != &other) {
				Py_XDECREF(this->ref);
				this->ref = other.Steal();
			}
			return *this;
		}

		inline _PyPtr<O>& operator= (const O* other) = delete;

		template<typename AS>
		inline AS* As() const { return (AS*) this->ref; }
		inline PyObject* Ref() const { return (PyObject*) this->ref; };

		template<typename V>
		inline bool Is(V* value) const { return this->ref == value; }
		inline bool IsNull() const { return this->ref == NULL; }
		inline bool IsNone() const { return this->ref == Py_None; }
		inline bool IsTrue() const { return this->ref == Py_True; }
		inline bool IsFalse() const { return this->ref == Py_False; }
		inline bool IsExactly(PyTypeObject* type) const { assert(type != NULL && PyType_Check(type)); return Py_TYPE((PyObject*) self->ref) == type; }
		inline bool IsInstance(PyTypeObject* type) const { assert(type != NULL && PyType_Check(type)); return PyObject_TypeCheck((PyObject*) this->ref, type); }

		inline void Incref() { assert(this->ref != NULL); Py_INCREF(this->ref); }
		inline void Decref() { assert(this->ref != NULL); Py_DECREF(this->ref); }
		inline O* Steal() { O* tmp = this->ref; this->ref = NULL; return tmp; }
		inline void Clear() { Py_CLEAR(this->ref); }
		inline Py_ssize_t RefCount() const { return Py_REFCNT((PyObject*) this->ref); }
	protected:
		O* ref = NULL;
};


template<typename O = PyObject>
class PyPtr: public _PyPtr<O> {
public:
	using _PyPtr<O>::_PyPtr;

	inline operator O* () const { return (O*) this->ref; }
	inline operator PyObject* () const { return (PyObject*) this->ref; }

	inline _PyPtr<O>& operator= (const PyObject* other) {
		if (this->ref != ((O*) other)) {
			Py_XDECREF(this->ref);
			this->ref = (O*) other;
		}
		return *this;
	}
};


template<>
class PyPtr<PyObject>: public _PyPtr<PyObject> {
public:
	using _PyPtr<PyObject>::_PyPtr;
	inline operator PyObject* () const { return (PyObject*) this->ref; }
};


} /* end namespace Yapic */

#endif /* C2C90D0B_F133_C961_1374_B789D0F0B01B */
