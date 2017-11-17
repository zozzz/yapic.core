
#include <yapic/module.hpp>

class Module;

using ModuleVar = Yapic::ModuleVar<Module>;
using ModuleExc = Yapic::ModuleExc<Module>;


class Module : public Yapic::Module<Module> {
public:
	static constexpr const char* __name__ = "yapic.core.test._module";
	ModuleVar Str_code;
	ModuleVar ErrCode_XYZ;
	ModuleExc Exc_SomeError;

	static inline int __init__(PyObject* module, Module* state) {
		state->Str_code = "__code__";
		state->ErrCode_XYZ.Value(42).Export("XYZ");
		state->Exc_SomeError.Define("SomeError");

		return 0;
	}

	static PyObject* do_something(PyObject* module) {
		return PyLong_FromLong(111);
	}

	static PyObject* get_state_var(PyObject* module) {
		PyObject* v = State(module)->Str_code;
		Py_INCREF(v);
		return v;
	}

	static PyObject* raise_error(PyObject* module) {
		return State(module)->Exc_SomeError.Raise("raised...");
	}

	static PyObject* raise_error_formatted(PyObject* module) {
		return State(module)->Exc_SomeError.Raise("raised... %d\n", 4);
	}

	Yapic_METHODS_BEGIN
		Yapic_Method(do_something, METH_NOARGS, NULL)
		Yapic_Method(get_state_var, METH_NOARGS, NULL)
		Yapic_Method(raise_error, METH_NOARGS, NULL)
		Yapic_Method(raise_error_formatted, METH_NOARGS, NULL)
	Yapic_METHODS_END
};


PyMODINIT_FUNC PyInit__module(void) {
	return Module::Create();
}