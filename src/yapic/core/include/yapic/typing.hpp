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
    class ForwardDecl {
        public:
            static const PyTypeObject Type;

            inline static bool IsForwardTuple(PyObject* o) {
                assert(o != NULL);
                return PyTuple_CheckExact(o)
                    && PyTuple_GET_SIZE(o) == 3
                    && PyCode_Check(PyTuple_GET_ITEM(o, 0));
            }

            inline static bool Check(PyObject* o) {
                assert(o != NULL);
                return PyObject_TypeCheck(o, const_cast<PyTypeObject*>(&Type));
            }

            PyObject_HEAD
            PyObject* decl;

            static PyObject* New(PyObject* decl, PyObject* __args__, PyObject* copy_with) {
                assert((Type.tp_flags & Py_TPFLAGS_READY) == Py_TPFLAGS_READY);
                ForwardDecl* self = (ForwardDecl*) Type.tp_alloc(const_cast<PyTypeObject*>(&Type), Type.tp_basicsize);
                if (self != NULL) {
                    self->decl = decl;
                    self->__args__ = __args__;
                    self->copy_with = copy_with;
                    Py_INCREF(decl);
                    Py_INCREF(__args__);
                    Py_INCREF(copy_with);
                }
                return (PyObject*) self;
            }

            static PyObject* __call__(ForwardDecl* self, PyObject* args, PyObject* kwargs) {
                return self->Resolve();
            }

            static void __dealloc__(ForwardDecl* self) {
                Py_CLEAR(self->decl);
                Py_CLEAR(self->__args__);
                Py_CLEAR(self->copy_with);
            }

            static PyObject* __repr__(ForwardDecl* self) {
                return PyUnicode_FromFormat("<ForwardDecl>");
            }

            PyObject* Resolve() {
                if (IsForwardTuple(decl)) {
                    PyObject* locals = PyTuple_GET_ITEM(decl, 2);
                    PyObject* ev = PyEval_EvalCode(
                        PyTuple_GET_ITEM(decl, 0),
                        PyTuple_GET_ITEM(decl, 1),
                        locals == Py_None ? NULL : locals);

                    if (ev != NULL) {
                        PyObject* res = Resolve(ev);
                        Py_DECREF(ev);
                        return res;
                    } else {
                        return NULL;
                    }
                } else {
                    return Resolve(decl);
                }
            }

        private:
            PyObject* Resolve(PyObject* obj) {
                if (Check(obj)) {
                    return reinterpret_cast<ForwardDecl*>(obj)->Resolve();
                } else {
                    PyPtr<> args = PyObject_GetAttr(obj, __args__);
                    if (args.IsValid()) {
                        Py_ssize_t l = PyTuple_GET_SIZE(args);
                        PyPtr<> newArgs = PyTuple_New(l);

                        for (Py_ssize_t i = 0; i < l; ++i) {
                            PyPtr<> arg = Resolve(PyTuple_GET_ITEM(args, i));
                            if (arg.IsNull()) {
                                return NULL;
                            } else {
                                PyTuple_SET_ITEM(newArgs, 0, arg.Steal());
                            }
                        }

                        PyPtr<> copy = PyObject_GetAttr(obj, copy_with);
                        if (copy.IsValid()) {
                            PyPtr<> copyArgs = PyTuple_Pack(1, newArgs.AsObject());
                            if (copyArgs.IsNull()) {
                                return NULL;
                            }
                            return PyObject_CallObject(copy, copyArgs);
                        } else {
                            return NULL;
                        }
                    } else {
                        PyErr_Clear();
                        Py_INCREF(obj);
                        return obj;
                    }
                }
            }

            PyObject* __args__;
            PyObject* copy_with;
    };


    class Typing {
        public:
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
                Py_DECREF(const_cast<PyTypeObject*>(&ForwardDecl::Type));
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

                if ((ForwardDecl::Type.tp_flags & Py_TPFLAGS_READY) != Py_TPFLAGS_READY) {
                    if (PyType_Ready(const_cast<PyTypeObject*>(&ForwardDecl::Type)) < 0) {
                        return false;
                    }
                }
                Py_INCREF(const_cast<PyTypeObject*>(&ForwardDecl::Type));

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

            inline bool IsForwardDecl(PyObject* object) {
                return ForwardDecl::Check(object);
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
                                        // TODO: locals...
                                        argResult = NewForwardRef(argItem, type, Py_None);
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

                        PyObject* res = PyTuple_New(2);
                        if (res != NULL) {
                            PyTuple_SET_ITEM(res, 0, attrs.Steal());
                            PyTuple_SET_ITEM(res, 1, Py_None);
                            Py_INCREF(Py_None);
                            return res;
                        }
                    }
                }
                return NULL;
            }

        private:
            PyObject* NewForwardRef(PyCodeObject* code, PyObject* type, PyObject* locals) {
                PyPtr<> moduleName = PyObject_GetAttr(type, __module__);
                if (moduleName.IsValid()) {
                    PyPtr<> module = PyImport_Import(moduleName);
                    if (module.IsValid()) {
                        PyPtr<> result = PyTuple_New(3);
                        if (result.IsValid()) {
                            PyObject* moduleDict = PyModule_GetDict(module); // borrowed

                            PyTuple_SET_ITEM(result, 0, (PyObject*)code);
                            PyTuple_SET_ITEM(result, 1, moduleDict);
                            PyTuple_SET_ITEM(result, 2, locals);

                            Py_INCREF(code);
                            Py_INCREF(moduleDict);
                            Py_INCREF(locals);

                            return ForwardDecl::New(result, __args__, copy_with);
                        }
                    }
                }

                if (PyErr_Occurred()) {
                    PyErr_Clear();
                }
                return NULL;
            }

            PyObject* NewForwardRef(PyObject* ref, PyObject* type, PyObject* locals) {
                PyPtr<PyCodeObject> code = (PyCodeObject*)PyObject_GetAttr(ref, __forward_code__);
                if (code.IsValid()) {
                    return NewForwardRef((PyCodeObject*)code, type, locals);
                } else {
                    return NULL;
                }
            }

            PyObject* NewForwardRef(const char* ref, PyObject* type, PyObject* locals) {
                PyPtr<PyCodeObject> code = (PyCodeObject*)Py_CompileString(ref, "<string>", Py_eval_input);
                if (code.IsValid()) {
                    return NewForwardRef((PyCodeObject*)code, type, locals);
                } else {
                    return NULL;
                }
            }

            PyObject* NewForwardRef(PyUnicodeObject* str, PyObject* type, PyObject* locals) {
                PyPtr<> ascii = PyUnicode_AsASCIIString((PyObject*)str);
                if (ascii.IsValid()) {
                    return NewForwardRef(PyBytes_AS_STRING(ascii), type, locals);
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
                                PyObject* entry = PyTuple_New(3);

                                if (entry != NULL) {
                                    Py_INCREF(origin);
                                    Py_INCREF(type);
                                    Py_INCREF(selfVars);

                                    PyTuple_SET_ITEM(entry, 0, origin.AsObject());
                                    PyTuple_SET_ITEM(entry, 1, type);
                                    PyTuple_SET_ITEM(entry, 2, selfVars.AsObject());

                                    ReplaceMro(mro, resolved, origin, entry);
                                    Py_DECREF(entry);
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
                                PyPtr<> forward = NewForwardRef((PyUnicodeObject*)value, type, locals);
                                if (forward.IsValid()) {
                                    if (PyDict_SetItem(attrs, key, forward) == -1) {
                                        return false;
                                    }
                                } else {
                                    return false;
                                }
                            } else {
                                PyPtr<> subst = SubstType(value, type, vars, locals);
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

            PyObject* SubstType(PyObject* value, PyObject* type, PyObject* vars, PyObject* locals) {
                bool hasFwd = false;
                PyObject* res = _SubstType(value, type, vars, locals, &hasFwd);
                if (res != NULL && hasFwd && !ForwardDecl::Check(res)) {
                    return ForwardDecl::New(res, __args__, copy_with);
                } else {
                    return res;
                }
            }

            PyObject* _SubstType(PyObject* value, PyObject* type, PyObject* vars, PyObject* locals, bool *hasFwd) {
                if (IsTypeVar(value)) {
                    PyObject* exists = PyDict_GetItem(vars, value);
                    if (exists != NULL) {
                        return _SubstType(exists, type, vars, locals, hasFwd);
                    }
                } else if (IsForwardRef(value)) {
                    PyObject* exists = PyDict_GetItem(vars, value);
                    if (exists == NULL) {
                        *hasFwd = true;
                        return NewForwardRef(value, type, locals);
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
                                    if (newA == NULL) {
                                        newA = oldA;
                                    }

                                    newA = _SubstType(newA, type, vars, locals, hasFwd);
                                    if (newA == NULL) {
                                        return NULL;
                                    }
                                    PyTuple_SET_ITEM(newArgs, i, newA);
                                }

                                PyPtr<> params = PyObject_GetAttr(value, __parameters__);
                                if (params.IsNull() || PyTuple_GET_SIZE(params) == 0) {
                                    PyPtr<> copy = PyObject_GetAttr(value, copy_with);
                                    if (copy.IsValid()) {
                                        PyPtr<> copyArgs = PyTuple_Pack(1, newArgs.AsObject());
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
                            } else {
                                return NULL;
                            }
                        }
                    }
                    return NULL;
                }

                if (!*hasFwd && ForwardDecl::Check(value)) {
                    *hasFwd = true;
                }

                Py_INCREF(value);
                return value;
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


    const PyTypeObject ForwardDecl::Type = {
        PyVarObject_HEAD_INIT(NULL, 0)
        /* tp_name */ 			"yapic.core.ForwardDecl",
        /* tp_basicsize */ 		sizeof(ForwardDecl),
        /* tp_itemsize */ 		0,
        /* tp_dealloc */ 		(destructor) ForwardDecl::__dealloc__,
        /* tp_print */ 			0,
        /* tp_getattr */ 		0,
        /* tp_setattr */ 		0,
        /* tp_as_async */ 		0,
        /* tp_repr */ 			(reprfunc) ForwardDecl::__repr__,
        /* tp_as_number */ 		0,
        /* tp_as_sequence */ 	0,
        /* tp_as_mapping */ 	0,
        /* tp_hash  */ 			0,
        /* tp_call */ 			(ternaryfunc) ForwardDecl::__call__,
        /* tp_str */ 			0,
        /* tp_getattro */ 		0,
        /* tp_setattro */ 		0,
        /* tp_as_buffer */ 		0,
        /* tp_flags */ 			0,
        /* tp_doc */ 			0,
        /* tp_traverse */ 		0,
        /* tp_clear */ 			0,
        /* tp_richcompare */ 	0,
        /* tp_weaklistoffset */ 0,
        /* tp_iter */ 			0,
        /* tp_iternext */ 		0,
        /* tp_methods */ 		0,
        /* tp_members */ 		0,
        /* tp_getset */ 		0,
        /* tp_base */ 			0,
        /* tp_dict */ 			0,
        /* tp_descr_get */ 		0,
        /* tp_descr_set */ 		0,
        /* tp_dictoffset */ 	0,
        /* tp_init */ 			0,
        /* tp_alloc */ 			0,
        /* tp_new */ 			PyType_GenericNew
    };


}; /* end namespace Yapic */


#endif /* D57C95BE_1134_1461_11A3_990B2EDF5491 */
