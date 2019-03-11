
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

    static PyObject* is_generic(PyObject* module, PyObject* o) {
        if (State(module)->Typing.IsGeneric(o)) {
            Py_RETURN_TRUE;
        } else {
            Py_RETURN_FALSE;
        }
    }

    static PyObject* is_generic_type(PyObject* module, PyObject* o) {
        if (State(module)->Typing.IsGenericType(o)) {
            Py_RETURN_TRUE;
        } else {
            Py_RETURN_FALSE;
        }
    }

    static PyObject* is_forward_ref(PyObject* module, PyObject* o) {
        if (State(module)->Typing.IsForwardRef(o)) {
            Py_RETURN_TRUE;
        } else {
            Py_RETURN_FALSE;
        }
    }

    static PyObject* resolve_type_vars(PyObject* module, PyObject* o) {
        return State(module)->Typing.ResolveTypeVars(o);
    }

    static PyObject* resolve_mro(PyObject* module, PyObject* o) {
        return State(module)->Typing.ResolveMro(o);
    }

    static PyObject* class_hints(PyObject* module, PyObject* o) {
        return State(module)->Typing.ClassHints(o);
    }

	Yapic_METHODS_BEGIN
		Yapic_Method(get_typing, METH_NOARGS, NULL)
		Yapic_Method(is_generic, METH_O, NULL)
		Yapic_Method(is_generic_type, METH_O, NULL)
		Yapic_Method(is_forward_ref, METH_O, NULL)
		Yapic_Method(resolve_type_vars, METH_O, NULL)
		Yapic_Method(resolve_mro, METH_O, NULL)
		Yapic_Method(class_hints, METH_O, NULL)
	Yapic_METHODS_END
};


PyMODINIT_FUNC PyInit__typing(void) {
	return Module::Create();
}
