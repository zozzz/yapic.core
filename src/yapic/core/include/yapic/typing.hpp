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
            /*
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
            */

            /*
            class ResolvedMro {
                public:
                    friend Typing;

                    ResolvedMro(): _init(NULL), _attrs(NULL) {}
                    ~ResolvedMro() { Py_CLEAR(_init); Py_CLEAR(_attrs); }

                    inline PyObject* GetInit() { return _init; }
                    inline PyObject* GetAttrs() { return _attrs; }

                private:
                    PyObject* _init;
                    PyObject* _attrs;
            };
            */

            Typing() {
                memset(this, 0, sizeof(this));
            }

            ~Typing() {
                Py_CLEAR(_typing);
                Py_CLEAR(Generic);
                Py_CLEAR(GenericAlias);
                Py_CLEAR(ForwardRef);
                Py_CLEAR(TypeVar);
                Py_CLEAR(__origin__);
                Py_CLEAR(__args__);
                Py_CLEAR(__parameters__);
                Py_CLEAR(__module__);
                Py_CLEAR(__forward_code__);
                Py_CLEAR(__orig_bases__);
                Py_CLEAR(__name__);
                Py_CLEAR(__annotations__);
                Py_CLEAR(copy_with);
            }

            bool Init(PyObject* typingModule) {
                _typing = typingModule;
                Py_INCREF(typingModule);

                Yapic_Typing_ImportFromTyping(Generic, "Generic");
                Yapic_Typing_ImportFromTyping(GenericAlias, "_GenericAlias");
                Yapic_Typing_ImportFromTyping(ForwardRef, "ForwardRef");
                Yapic_Typing_ImportFromTyping(TypeVar, "TypeVar");

                Yapic_Typing_StrCache(__origin__, "__origin__");
                Yapic_Typing_StrCache(__args__, "__args__");
                Yapic_Typing_StrCache(__parameters__, "__parameters__");
                Yapic_Typing_StrCache(__module__, "__module__");
                Yapic_Typing_StrCache(__forward_code__, "__forward_code__");
                Yapic_Typing_StrCache(__orig_bases__, "__orig_bases__");
                Yapic_Typing_StrCache(__name__, "__name__");
                Yapic_Typing_StrCache(__annotations__, "__annotations__");
                Yapic_Typing_StrCache(copy_with, "copy_with");

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

            inline bool IsTypeVar(PyObject* object) {
                assert(object != NULL);
                assert(TypeVar != NULL);
                return PyObject_TypeCheck(object, (PyTypeObject*)TypeVar);
            }

            inline PyObject* ResolveTypeVars(PyObject* type) {
                PyPtr<> vars = PyDict_New();
                if (vars.IsValid()) {
                    return ResolveTypeVars(type, vars);
                } else {
                    return NULL;
                }
            }

            PyObject* ResolveTypeVars(PyObject* type, PyObject* vars) {
                PyPtr<> resolved = PyDict_New();

                if (resolved.IsValid()) {
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
                                        argResult = NewForwardRef(argItem, type, Py_None, vars);
                                        // YapicTyping_DUMP(argResult);
                                        if (argResult == NULL) {
                                            goto error;
                                        }
                                    } else {
                                        argResult = PyDict_GetItem(vars, argItem);
                                        if (argResult == NULL) {
                                            argResult = argItem;
                                        }
                                        Py_INCREF(argResult);
                                    }

                                    if (PyDict_SetItem(resolved, PyTuple_GET_ITEM(params, i), argResult) == -1) {
                                        Py_DECREF(argResult);
                                        goto error;
                                    } else {
                                        Py_DECREF(argResult);
                                    }
                                }

                                return resolved.Steal();
                            }
                        }
                    }
                }

                error:
                    if (PyErr_Occurred()) {
                        PyErr_Clear();
                    }
                    return NULL;
            }

            inline PyObject* ResolveMro(PyObject* type) {
                PyPtr<> vars = PyDict_New();
                if (vars.IsValid()) {
                    return ResolveMro(type, vars);
                } else {
                    return NULL;
                }
            }

            PyObject* ResolveMro(PyObject* type, PyObject* vars) {
                PyPtr<> mro(NULL);

                if (IsGenericType(type)) {
                    PyPtr<> origin = PyObject_GetAttr(type, __origin__);
                    if (origin.IsValid()) {
                        if (PyType_Check(origin)) {
                            mro = ((PyTypeObject*)origin)->tp_mro;
                            mro.Incref();
                        } else {
                            PyErr_BadInternalCall();
                            return NULL;
                        }
                    } else {
                        return NULL;
                    }
                } else if (PyType_Check(type)) {
                    mro = reinterpret_cast<PyTypeObject*>(type)->tp_mro;
                    mro.Incref();
                } else {
                    PyErr_BadArgument();
                    return NULL;
                }

                assert(mro.IsValid());
                assert(PyTuple_CheckExact(mro));

                PyPtr<> resolved = PyTuple_New(PyTuple_GET_SIZE(mro));
                if (resolved.IsValid()
                    && ResolveMro(type, mro, resolved, vars)
                    && FillMroHoles(mro, resolved)) {
                    return resolved.Steal();
                } else {
                    return NULL;
                }
            }

            inline PyObject* ClassHints(PyObject* type) {
                PyPtr<> vars = PyDict_New();
                if (vars.IsValid()) {
                    return ClassHints(type, vars);
                } else {
                    return NULL;
                }
            }

            // inline PyObject* ClassHints(PyObject* type, PyObject* vars) {
            //     PyPtr<> locals = VarsToLocals(vars);
            //     if (locals.IsValid()) {
            //         return ClassHints(type, vars, locals);
            //     } else {
            //         return NULL;
            //     }
            // }

            PyObject* ClassHints(PyObject* type, PyObject* vars) {
                PyPtr<> mro = ResolveMro(type, vars);

                if (mro.IsValid()) {
                    PyPtr<> attrs = PyDict_New();
                    if (attrs.IsValid()) {
                        PyObject* mroEntry;
                        PyPtr<> annots(NULL);
                        Py_ssize_t l = PyTuple_GET_SIZE(mro);
                        for (Py_ssize_t i = 0; i < l; ++i) {
                            mroEntry = PyTuple_GET_ITEM(mro, i);
                            assert(mroEntry != NULL);

                            PyObject* currentType = PyTuple_GET_ITEM(mroEntry, 0);
                            PyObject* currentVars = PyTuple_GET_ITEM(mroEntry, 2);
                            annots = PyObject_GetAttr(currentType, __annotations__);
                            if (annots.IsValid()) {
                                if (!ResolveAnnots(currentType, annots, currentVars, attrs)) {
                                    return NULL;
                                }
                            } else {
                                PyErr_Clear();
                            }
                        }

                        return PyTuple_Pack(2, attrs, Py_None);
                    }
                }
                return NULL;
            }

        private:
            PyObject* NewForwardRef(PyCodeObject* code, PyObject* type, PyObject* locals, PyObject* vars) {
                PyPtr<> moduleName = PyObject_GetAttr(type, __module__);
                if (moduleName.IsValid()) {
                    PyPtr<> module = PyImport_Import(moduleName);
                    if (module.IsValid()) {
                        PyPtr<> result = PyTuple_New(4);
                        if (result.IsValid()) {
                            PyObject* moduleDict = PyModule_GetDict(module); // borrowed

                            PyTuple_SET_ITEM(result, 0, (PyObject*)code);
                            PyTuple_SET_ITEM(result, 1, moduleDict);
                            PyTuple_SET_ITEM(result, 2, locals);
                            PyTuple_SET_ITEM(result, 3, vars);

                            Py_INCREF(code);
                            Py_INCREF(moduleDict);
                            Py_INCREF(locals);
                            Py_INCREF(vars);
                            return result.Steal();
                        }
                    }
                }

                if (PyErr_Occurred()) {
                    PyErr_Clear();
                }
                return NULL;
            }

            PyObject* NewForwardRef(PyObject* ref, PyObject* type, PyObject* locals, PyObject* vars) {
                PyPtr<PyCodeObject> code = (PyCodeObject*)PyObject_GetAttr(ref, __forward_code__);
                if (code.IsValid()) {
                    return NewForwardRef((PyCodeObject*)code, type, locals, vars);
                } else {
                    return NULL;
                }
            }

            PyObject* NewForwardRef(const char* ref, PyObject* type, PyObject* locals, PyObject* vars) {
                PyPtr<PyCodeObject> code = (PyCodeObject*)Py_CompileString(ref, "<string>", Py_eval_input);
                if (code.IsValid()) {
                    return NewForwardRef((PyCodeObject*)code, type, locals, vars);
                } else {
                    return NULL;
                }
            }

            PyObject* NewForwardRef(PyUnicodeObject* str, PyObject* type, PyObject* locals, PyObject* vars) {
                PyPtr<> ascii = PyUnicode_AsASCIIString((PyObject*)str);
                if (ascii.IsValid()) {
                    return NewForwardRef(PyBytes_AS_STRING(ascii), type, locals, vars);
                } else {
                    return NULL;
                }
            }

            /**
             * mro_entry = (original_type, generic_type)
             */
            bool ResolveMro(PyObject* type, PyObject *mro, PyObject *resolved, PyObject* vars) {
                PyPtr<> origin = PyObject_GetAttr(type, __origin__);
                if (origin.IsValid()) {
                    PyPtr<> bases = PyObject_GetAttr(origin, __orig_bases__);
                    if (bases.IsValid()) {
                        Py_ssize_t l = PyTuple_GET_SIZE(bases);

                        PyPtr<> selfVars(ResolveTypeVars(type, vars));

                        for (Py_ssize_t i = 0; i < l; ++i) {
                            PyObject* base = PyTuple_GET_ITEM(bases, i);
                            if (ResolveMro(base, mro, resolved, selfVars)) {
                                PyPtr<> entry = PyTuple_Pack(3, origin, type, selfVars);
                                if (entry.IsValid()) {
                                    ReplaceMro(mro, resolved, origin, entry);
                                } else {
                                    return false;
                                }
                            } else {
                                return false;
                            }
                        }
                    } else {
                        PyErr_Clear();
                    }
                } else {
                    PyErr_Clear();
                }
                return true;
            }

            void ReplaceMro(PyObject *mro, PyObject *resolved, PyObject* orig, PyObject* entry) {
                Py_ssize_t idx = 0;
                Py_ssize_t l = PyTuple_GET_SIZE(mro);

                for (Py_ssize_t i = 0; i < l; ++i) {
                    PyObject* cv = PyTuple_GET_ITEM(mro, i);
                    if (cv == orig) {
                        PyObject* item = PyTuple_GET_ITEM(resolved, i);
                        Py_XDECREF(item);
                        Py_INCREF(entry);
                        PyTuple_SET_ITEM(resolved, i, entry);
                    }
                }
            }

            bool FillMroHoles(PyObject *mro, PyObject *resolved) {
                Py_ssize_t l = PyTuple_GET_SIZE(mro);
                for (Py_ssize_t i = 0; i < l; ++i) {
                    PyObject* item = PyTuple_GET_ITEM(resolved, i);
                    if (item == NULL) {
                        PyObject* tvars = PyDict_New();
                        if (tvars == NULL) {
                            return false;
                        }

                        item = PyTuple_Pack(3, PyTuple_GET_ITEM(mro, i), Py_None, tvars);
                        Py_DECREF(tvars);
                        if (item == NULL) {
                            return false;
                        }

                        PyTuple_SET_ITEM(resolved, i, item);
                    }
                }
                return true;
            }

            PyObject* VarsToLocals(PyObject* vars) {
                PyPtr<> locals = PyDict_New();

                if (PyDict_Size(vars) > 0) {
                    Py_ssize_t pos = 0;
                    PyObject* key;
                    PyObject* value;

                    while (PyDict_Next(vars, &pos, &key, &value)) {
                        if (IsTypeVar(key)) {
                            PyPtr<> ln = PyObject_GetAttr(key, __name__);
                            if (ln.IsNull() || PyDict_SetItem(locals, ln, value) == -1) {
                                return NULL;
                            }
                        }
                    }
                }

                return locals.Steal();
            }

            bool ResolveAnnots(PyObject* type, PyObject* annots, PyObject* vars, PyObject* attrs) {
                PyPtr<> locals(VarsToLocals(vars));
                if (locals.IsNull()) {
                    return false;
                }

                Py_ssize_t pos = 0;
                PyObject* key;
                PyObject* value;

                while (PyDict_Next(annots, &pos, &key, &value)) {
                    switch (PyDict_Contains(attrs, key)) {
                        case 0:
                            if (PyUnicode_Check(value)) {
                                PyPtr<> forward = NewForwardRef((PyUnicodeObject*)value, type, locals, vars);
                                if (PyDict_SetItem(attrs, key, forward) == -1) {
                                    return false;
                                }
                            } else {
                                PyPtr<> subst = SubstType(value, vars, locals);
                                if (subst.IsNull() || PyDict_SetItem(attrs, key, subst) == -1) {
                                    return false;
                                }
                            }
                        break;

                        case 1:
                            continue;

                        case -1:
                            return false;
                    }
                }

                return true;
            }

            PyObject* SubstType(PyObject* value, PyObject* vars, PyObject* locals) {
                if (IsTypeVar(value)) {
                    PyObject* exists = PyDict_GetItem(vars, value);
                    if (exists != NULL) {
                        return SubstType(exists, vars, locals);
                    } else {
                        Py_INCREF(value);
                        return value;
                    }
                } else if (IsForwardRef(value)) {
                    PyObject* exists = PyDict_GetItem(vars, value);
                    if (exists == NULL) {
                        return PyTuple_Pack(4, Py_None, Py_None, Py_None, Py_None);
                    } else {
                        return SubstType(exists, vars, locals);
                    }
                } else if (IsGenericType(value)) {
                    PyPtr<> args = PyObject_GetAttr(value, __args__);
                    if (args.IsValid()) {
                        Py_ssize_t l = PyTuple_GET_SIZE(args);
                        if (l > 0) {
                            PyPtr<> newArgs = PyTuple_New(l);
                            if (newArgs.IsValid()) {
                                for (Py_ssize_t i = 0; i < l; ++i) {
                                    PyObject* oldA = PyTuple_GET_ITEM(args, i);
                                    PyObject* newA = PyDict_GetItem(vars, oldA);
                                    YapicTyping_DUMP(oldA);
                                    YapicTyping_DUMP(newA);
                                    if (newA != NULL) {
                                        PyObject* t = SubstType(newA, vars, locals);
                                        if (t == NULL) {
                                            return NULL;
                                        }
                                        PyTuple_SET_ITEM(newArgs, i, t);
                                    } else {
                                        oldA = SubstType(oldA, vars, locals);
                                        if (oldA == NULL) {
                                            return NULL;
                                        }
                                        PyTuple_SET_ITEM(newArgs, i, oldA);
                                    }
                                }

                                PyPtr<> params = PyObject_GetAttr(value, __parameters__);
                                if (params.IsNull() || PyTuple_GET_SIZE(params) == 0) {
                                    PyPtr<> copy = PyObject_GetAttr(value, copy_with);
                                    if (copy.IsValid()) {
                                        PyPtr<> copyArgs = PyTuple_Pack(1, newArgs);
                                        if (copyArgs.IsNull()) {
                                            return NULL;
                                        }
                                        return PyObject_CallObject(copy, copyArgs);
                                    } else {
                                        return NULL;
                                    }
                                } else {
                                    return PyObject_GetItem(value, newArgs);
                                }
                                // YapicTyping_DUMP(newArgs);

                            } else {
                                return NULL;
                            }
                        } else {
                            Py_INCREF(value);
                            return value;
                        }
                    }
                    return NULL;


                    // PyObject* tvars = ResolveTypeVars(value, vars);

                    // YapicTyping_DUMP();
                    // YapicTyping_DUMP(PyObject_GetAttr(value, __parameters__));
                    // YapicTyping_DUMP(vars);

                } else {
                    Py_INCREF(value);
                    return value;
                }
                // } else {

                //     // ResolveTypeVars



                //     // PyPtr<> params = PyObject_GetAttr(value, __parameters__);
                //     // if (params.IsValid()) {
                //     //     Py_ssize_t l = PyTuple_GET_SIZE(params);

                //     //     if (l > 0) {
                //     //         PyPtr<> newParams = PyTuple_New(l);
                //     //         if (newParams.IsValid()) {
                //     //             for (Py_ssize_t i = 0; i < l; ++i) {
                //     //                 PyPtr<> evald = SubstType(PyTuple_GET_ITEM(params, i), vars, skipForward);
                //     //                 if (evald.IsValid()) {
                //     //                     PyTuple_SET_ITEM(newParams, i, evald.Steal());
                //     //                 } else {
                //     //                     return NULL;
                //     //                 }
                //     //             }

                //     //             return PyObject_GetItem(value, newParams);
                //     //         } else {
                //     //             return NULL;
                //     //         }
                //     //     } else {
                //     //         Py_INCREF(value);
                //     //         return value;
                //     //     }
                //     // } else {
                //     //     PyErr_Clear();
                //     //     Py_INCREF(value);
                //     //     return value;
                //     // }
                // }
            }


            PyObject* _typing;
            PyObject* Generic;
            PyObject* GenericAlias;
            PyObject* ForwardRef;
            PyObject* TypeVar;

            PyObject* __origin__;
            PyObject* __args__;
            PyObject* __parameters__;
            PyObject* __module__;
            PyObject* __forward_code__;
            PyObject* __orig_bases__;
            PyObject* __name__;
            PyObject* __annotations__;
            PyObject* copy_with;
    };


}; /* end namespace Yapic */


#endif /* D57C95BE_1134_1461_11A3_990B2EDF5491 */
