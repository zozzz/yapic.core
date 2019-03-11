#ifndef D57C95BE_1134_1461_11A3_990B2EDF5491
#define D57C95BE_1134_1461_11A3_990B2EDF5491

#include <Python.h>
#include <memory>
#include "./pyptr.hpp"


#define YapicTyping_REPR(o) (((PyObject*)o) == NULL ? "<NULL>" : ((char*) PyUnicode_DATA(PyObject_Repr(((PyObject*)o)))))
#define YapicTyping_DUMP(o) printf(#o " = %s\n", YapicTyping_REPR(o))


#define Yapic_Typing_ImportFromTyping(into, name) \
    into = PyObject_GetAttrString(_typing, name); \
    if (into == NULL) { \
        return false; \
    }

#define Yapic_Typing_StrCache(into, value) \
    into = PyUnicode_InternFromString(value); \
    if (into == NULL) { \
        return false; \
    }


namespace Yapic {
    class Typing {
        public:
            class TypeVars {
                public:
                    friend Typing;

                    TypeVars(): _vars(NULL) {}
                    ~TypeVars() { Py_CLEAR(_vars); }
                    inline bool Init() { return (_vars = PyDict_New()) != NULL; }
                    inline PyObject* Get(const PyObject* typeVar) const { return PyDict_GetItem(_vars, const_cast<PyObject*>(typeVar)); }
                    inline PyObject* Vars() const { return _vars; }
                    inline bool IsForwardRef(PyObject* val) const { return PyTuple_CheckExact(val); }

                    PyObject* ResolveForwardRef(PyObject* fwr) {
                        assert(PyTuple_CheckExact(fwr));
                        assert(PyTuple_GET_SIZE(fwr) == 2);
                        assert(PyCode_Check(PyTuple_GET_ITEM(fwr, 0)));
                        assert(PyModule_CheckExact(PyTuple_GET_ITEM(fwr, 1)));

                        PyObject* code = PyTuple_GET_ITEM(fwr, 0);
                        PyObject* module = PyTuple_GET_ITEM(fwr, 1);
                        return PyEval_EvalCode(code, PyModule_GetDict(module), NULL);
                    }

                private:
                    PyObject* _vars;
            };

            class ResolvedMroEntry {
                public:
                    friend Typing;

                    ResolvedMroEntry(): {}

                private:
                    PyObject* _type;
                    std::unique_ptr<TypeVars> _vars;
            };

            Typing() {
                memset(this, 0, sizeof(this));
            }

            ~Typing() {
                Py_CLEAR(_typing);
                Py_CLEAR(Generic);
                Py_CLEAR(GenericAlias);
                Py_CLEAR(ForwardRef);
                Py_CLEAR(__origin__);
            }

            bool Init(PyObject* typingModule) {
                _typing = typingModule;
                Py_INCREF(typingModule);

                Yapic_Typing_ImportFromTyping(Generic, "Generic");
                Yapic_Typing_ImportFromTyping(GenericAlias, "_GenericAlias");
                Yapic_Typing_ImportFromTyping(ForwardRef, "ForwardRef");

                Yapic_Typing_StrCache(__origin__, "__origin__");
                Yapic_Typing_StrCache(__args__, "__args__");
                Yapic_Typing_StrCache(__parameters__, "__parameters__");
                Yapic_Typing_StrCache(__module__, "__module__");
                Yapic_Typing_StrCache(__forward_code__, "__forward_code__");

                return true;
            }

            inline bool IsGeneric(PyObject* object) {
                assert(object != NULL);
                assert(Generic != NULL);

                if (PyObject_TypeCheck(object, (PyTypeObject*)GenericAlias)) {
                    PyObject* origin = PyObject_GetAttr(object, __origin__);
                    if (origin != NULL) {
                        bool res = origin == Generic;
                        Py_DECREF(origin);
                        return res;
                    } else {
                        PyErr_Clear();
                        return false;
                    }
                } else {
                    return object == Generic;
                }
            }

            inline bool IsGenericType(PyObject* object) {
                assert(object != NULL);
                assert(GenericAlias != NULL);

                if (PyObject_TypeCheck(object, (PyTypeObject*)GenericAlias)) {
                    PyObject* origin = PyObject_GetAttr(object, __origin__);
                    if (origin != NULL) {
                        bool res = origin != Generic;
                        Py_DECREF(origin);
                        return res;
                    } else {
                        PyErr_Clear();
                        return false;
                    }
                } else {
                    return false;
                }
            }

            inline bool IsForwardRef(PyObject* object) {
                assert(object != NULL);
                assert(ForwardRef != NULL);
                return PyObject_TypeCheck(object, (PyTypeObject*)ForwardRef);
            }

            inline TypeVars* ResolveType(PyObject* type) {
                std::unique_ptr<TypeVars> vars(new TypeVars());
                if (vars->Init()) {
                    return ResolveType(type, vars.get());
                } else {
                    return NULL;
                }
            }

            TypeVars* ResolveType(PyObject* type, TypeVars* vars) {
                TypeVars* resolved = new TypeVars();

                if (resolved->Init()) {
                    PyPtr<> args = PyObject_GetAttr(type, __args__);
                    if (args.IsValid()) {
                        PyPtr<> origin = PyObject_GetAttr(type, __origin__);
                        if (origin.IsValid()) {
                            PyPtr<> params = PyObject_GetAttr(origin, __parameters__);
                            if (params.IsValid()) {
                                assert(PyTuple_CheckExact(params));
                                assert(PyTuple_CheckExact(args));
                                assert(PyTuple_GET_SIZE(args) == PyTuple_GET_SIZE(params));
                                Py_ssize_t l = PyTuple_GET_SIZE(params);

                                PyObject* argItem;
                                PyObject* argResult;
                                for (Py_ssize_t i = 0; i < l; ++i) {
                                    argItem = PyTuple_GET_ITEM(args, i);

                                    if (IsForwardRef(argItem)) {
                                        argResult = NewForwardRef(type, argItem);
                                        if (argResult == NULL) {
                                            goto error;
                                        }
                                    } else {
                                        argResult = vars->Get(argItem);
                                        if (argResult == NULL) {
                                            argResult = argItem;
                                        }
                                        Py_INCREF(argResult);
                                    }

                                    if (PyDict_SetItem(resolved->_vars, PyTuple_GET_ITEM(params, i), argResult) == -1) {
                                        Py_DECREF(argResult);
                                        goto error;
                                    } else {
                                        Py_DECREF(argResult);
                                    }
                                }

                                return resolved;
                            }
                        }
                    }
                }

                error:
                    delete resolved;
                    if (PyErr_Occurred()) {
                        PyErr_Clear();
                    }
                    return NULL;
            }

            void ResolveMro() {

            }

        private:
            PyObject* NewForwardRef(PyObject* type, PyObject* ref) {
                PyPtr<> result = PyTuple_New(2);
                if (result.IsValid()) {
                    PyPtr<> moduleName = PyObject_GetAttr(type, __module__);
                    if (moduleName.IsValid()) {
                        PyPtr<> module = PyImport_Import(moduleName);
                        if (module.IsValid()) {
                            PyPtr<> code = PyObject_GetAttr(ref, __forward_code__);
                            if (code.IsValid()) {
                                PyTuple_SET_ITEM(result, 0, code.Steal());
                                PyTuple_SET_ITEM(result, 1, module.Steal());
                                return result.Steal();
                            }
                        }
                    }
                }

                if (PyErr_Occurred()) {
                    PyErr_Clear();
                }
                return NULL;
            }

            PyObject* _typing;
            PyObject* Generic;
            PyObject* GenericAlias;
            PyObject* ForwardRef;

            PyObject* __origin__;
            PyObject* __args__;
            PyObject* __parameters__;
            PyObject* __module__;
            PyObject* __forward_code__;
    };


}; /* end namespace Yapic */


#endif /* D57C95BE_1134_1461_11A3_990B2EDF5491 */
