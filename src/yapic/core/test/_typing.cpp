
#include <yapic/module.hpp>
#include <yapic/typing.hpp>


class TypingModule : public Yapic::Module<TypingModule> {
public:
    using ModuleRef = Yapic::ModuleRef<TypingModule>;

	static constexpr const char* __name__ = "yapic.core.test._typing";

    ModuleRef typing;
    Yapic::Typing Typing;

	static inline int __init__(PyObject* module, TypingModule* state) {
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

    static PyObject* is_forward_decl(PyObject* module, PyObject* o) {
        if (State(module)->Typing.IsForwardDecl(o)) {
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

    static PyObject* type_hints(PyObject* module, PyObject* o) {
        return State(module)->Typing.TypeHints(o);
    }

    static PyObject* callable_hints(PyObject* module, PyObject* o) {
        return State(module)->Typing.CallableHints(o);
    }

    static PyObject* callable_hints_with_type(PyObject* module, PyObject* args) {
        PyObject* callable;
        PyObject* type;
        if (PyArg_UnpackTuple(args, "OO", 2, 2, &callable, &type)) {
            return State(module)->Typing.CallableHints(callable, type);
        }
        return NULL;
    }

    static PyObject* unpack_forward_decl(PyObject* module, PyObject* o) {
        return State(module)->Typing.UnpackForwardDecl(o);
    }

	Yapic_METHODS_BEGIN
		Yapic_Method(get_typing, METH_NOARGS, NULL)
		Yapic_Method(is_generic, METH_O, NULL)
		Yapic_Method(is_generic_type, METH_O, NULL)
		Yapic_Method(is_forward_ref, METH_O, NULL)
        Yapic_Method(is_forward_decl, METH_O, NULL)
		Yapic_Method(resolve_type_vars, METH_O, NULL)
		Yapic_Method(resolve_mro, METH_O, NULL)
		Yapic_Method(type_hints, METH_O, NULL)
		Yapic_Method(callable_hints, METH_O, NULL)
		Yapic_Method(callable_hints_with_type, METH_VARARGS, NULL)
        Yapic_Method(unpack_forward_decl, METH_O, NULL)
	Yapic_METHODS_END
};


PyMODINIT_FUNC PyInit__typing(void) {
	return TypingModule::Create();
}
