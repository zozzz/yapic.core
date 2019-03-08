
#include <yapic/module.hpp>
#include <yapic/typing.hpp>

class Module;


class Module : public Yapic::Module<Module> {
public:
    using ModuleRef = Yapic::ModuleRef<Module>;

	static constexpr const char* __name__ = "yapic.core.test._typing";

    ModuleRef typing;
    Yapic::Typing Typing;

	static inline int __init__(PyObject* module, Module* state) {
        state->typing.Import("typing");

        if (!state->Typing.Init(state->typing)) {
            return 1;
        }

		return 0;
	}

    static PyObject* get_typing(PyObject* module) {
        PyObject* v = State(module)->typing;
        Py_INCREF(v);
        return v;
    }

	Yapic_METHODS_BEGIN
		Yapic_Method(get_typing, METH_NOARGS, NULL)
	Yapic_METHODS_END
};


PyMODINIT_FUNC PyInit__module(void) {
	return Module::Create();
}
