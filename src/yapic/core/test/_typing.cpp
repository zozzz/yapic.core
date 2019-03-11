
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
        Yapic::PyPtr<> vars(State(module)->Typing.ResolveTypeVars(o));
        if (vars.IsValid()) {
            Yapic::PyPtr<> result = PyDict_New();
            if (result.IsNull()) {
                return NULL;
            }

            PyObject* key;
            PyObject* value;
            Py_ssize_t i = 0;

            while (PyDict_Next(vars, &i, &key, &value)) {
                if (PyTuple_CheckExact(value)) {

                    Yapic::PyPtr<> rt = PyEval_EvalCode(
                        PyTuple_GET_ITEM(value, 0),
                        PyTuple_GET_ITEM(value, 1),
                        NULL);
                    if (rt.IsNull()) {
                        return NULL;
                    }
                    if (PyDict_SetItem(result, key, rt) == -1) {
                        return NULL;
                    }
                } else {
                    if (PyDict_SetItem(result, key, value) == -1) {
                        return NULL;
                    }
                }
            }

            return result.Steal();
        }
        Py_RETURN_NONE;
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
