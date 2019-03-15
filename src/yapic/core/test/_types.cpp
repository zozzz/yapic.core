
#include <yapic/module.hpp>
#include <yapic/pyptr.hpp>

class TypesModule;

using ModuleVar = Yapic::ModuleVar<TypesModule>;
using ModuleExc = Yapic::ModuleExc<TypesModule>;


class A: public Yapic::Type<A, Yapic::Object> {
public:
	int member;

	Yapic_MEMBERS_BEGIN
		Yapic_Member(member, T_INT, READONLY, "")
	Yapic_MEMBERS_END

	static A* __new__(PyTypeObject* type, PyObject* args, PyObject* kwargs) {
		A* self = Self::Alloc(type);
		if (self == NULL) {
			return NULL;
		}
		self->member = 42;
		return self;
	}
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


class Base: public Yapic::Type<Base, Yapic::Object> {
public:
	static inline int Flags() {
		return Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE;
	}
};

#define NumberP_M(n, i) \
	static PyObject* n(PyObject* self, PyObject* other) { \
		return PyLong_FromLong(i); \
	}

class NumberP : public Yapic::Type<NumberP, Yapic::Object> {
public:
	static int __bool__(PyObject* self) {
		return 0;
	}

	NumberP_M(__add__, 1)
	NumberP_M(__sub__, 2)
	NumberP_M(__mul__, 3)
	NumberP_M(__mod__, 4)
	NumberP_M(__divmod__, 5)
	NumberP_M(__pow__, 6)
	NumberP_M(__neg__, 7)
	NumberP_M(__pos__, 8)
	NumberP_M(__abs__, 9)
	NumberP_M(__invert__, 10)
	NumberP_M(__lshift__, 11)
	NumberP_M(__rshift__, 12)
	NumberP_M(__and__, 13)
	NumberP_M(__xor__, 14)
	NumberP_M(__or__, 15)
	NumberP_M(__int__, 16)
	static PyObject* __float__(PyObject* self) {
		return PyFloat_FromDouble(17.0);
	}
	NumberP_M(__iadd__, 18)
	NumberP_M(__isub__, 19)
	NumberP_M(__imul__, 20)
	NumberP_M(__imod__, 21)
	NumberP_M(__ipow__, 22)
	NumberP_M(__ilshift__, 23)
	NumberP_M(__irshift__, 24)
	NumberP_M(__iand__, 25)
	NumberP_M(__ixor__, 26)
	NumberP_M(__ior__, 27)
	NumberP_M(__floordiv__, 28)
	NumberP_M(__truediv__, 29)
	NumberP_M(__ifloordiv__, 30)
	NumberP_M(__itruediv__, 31)
	NumberP_M(__index__, 32)
	NumberP_M(__matmul__, 33)
	NumberP_M(__imatmul__, 34)
};


class FR: public Yapic::Type<FR, Yapic::Object, Yapic::FreeList<FR, 10>> {
public:
	static PyObject* __new__(PyTypeObject* type, PyObject* args, PyObject* kwargs) {
		return (PyObject*) Self::Alloc(type);
	}
};


class TypesModule : public Yapic::Module<TypesModule> {
public:
	static constexpr const char* __name__ = "yapic.core.test._types";

	static inline int __init__(PyObject* module, TypesModule* state) {
		if (!A::Register(module) ||
			!B::Register(module) ||
			!S::Register(module) ||
			!Base::Register(module) ||
			!NumberP::Register(module) ||
			!FR::Register(module)) {
			return -1;
		}
		return 0;
	}
};


PyMODINIT_FUNC PyInit__types(void) {
	return TypesModule::Create();
}
