#ifndef C2C90D0B_F133_C961_1374_B789D0F0B01B
#define C2C90D0B_F133_C961_1374_B789D0F0B01B

namespace Yapic {

/**
 * Usage:
 * Local injector(Injector::Alloc());
 * // do something...
 * return injector.Steal()
 *
 * Note:
 * 	DONT USE ON BORROWED REFERENCES LIKE PyDict_GetItem
 */
template<typename O = PyObject>
class _Local {

	public:
		inline _Local(O* var): _var(var) {  }
		inline ~_Local() { if (_var != NULL) { Py_DECREF(_var); } }
		inline O& operator* () const { return *_var; }
		inline O* operator-> () const { return _var; }

		inline _Local<O>& operator= (const _Local<O>& other) {
			if (this != &other) {
				Py_DECREF(_var);
				_var = other._var;
			}
			return *this;
		}

		inline _Local<O>& operator= (const O& other) {
			if (_var != &other) {
				Py_DECREF(_var);
				_var = other;
			}
			return *this;
		}

		inline operator PyObject* () const { return (PyObject*) _var; }
		inline operator PyTupleObject* () const { return (PyTupleObject*) _var; }
		inline operator PyDictObject* () const { return (PyDictObject*) _var; }

		inline bool isNull() const { return _var == NULL; }
		inline bool isValid() const { return _var != NULL; }
		inline PyObject* Steal() { O* tmp = _var; _var = NULL; return (PyObject*) tmp; }
	private:
		O* _var;
};

typedef _Local<> Local;

} /* end namespace Yapic */

#endif /* C2C90D0B_F133_C961_1374_B789D0F0B01B */
