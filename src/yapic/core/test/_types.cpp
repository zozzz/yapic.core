
#include <yapic/module.hpp>
#include <yapic/pyptr.hpp>

class Module;

using ModuleVar = Yapic::ModuleVar<Module>;
using ModuleExc = Yapic::ModuleExc<Module>;


class A: public Yapic::Type<A, Yapic::Object> {

};


class B: public Yapic::Type<B, A> {
public:
	PyObject* d;

	static int __init__(Self* self, PyObject* args, PyObject* kwargs) {
		self->d = PyDict_New();
		if (self->d == NULL) {
			return -1;
		}
		Yapic::PyPtr<> key = PyUnicode_FromString("hello");
		if (key.IsNull()) {
			return -1;
		}
		Yapic::PyPtr<> item = PyUnicode_FromString("world");
		if (item.IsNull()) {
			return -1;
		}
		if (PyDict_SetItem(self->d, key, item) < 0) {
			return -1;
		}
		return 0;
	}

	static void __dealloc__(Self* self) {
		Py_CLEAR(self->d);
		Super::__dealloc__(self);
	}

	static Py_ssize_t __mp_len__(Self* self) {
		return 42;
	}

	static PyObject* __mp_getitem__(Self* self, PyObject* key) {
		PyObject* item = PyDict_GetItemWithError(self->d, key);
		Py_XINCREF(item);
		return item;
	}

	static int __mp_setitem__(Self* self, PyObject* key, PyObject* value) {
		return PyDict_SetItem(self->d, key, value);
	}
};


class S: public Yapic::Type<S, Yapic::Object> {
public:
	static Py_ssize_t __sq_len__(Self* self) {
		return 33;
	}
};


class Module : public Yapic::Module<Module> {
public:
	static constexpr const char* __name__ = "yapic.core.test._types";

	static inline int __init__(PyObject* module, Module* state) {
		if (!A::Register(module) ||
			!B::Register(module) ||
			!S::Register(module)) {
			return -1;
		}
		return 0;
	}
};


PyMODINIT_FUNC PyInit__types(void) {
	return Module::Create();
}