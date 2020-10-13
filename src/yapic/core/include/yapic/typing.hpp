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
    class Typing;

    class ForwardDecl {
        public:
            friend Typing;

            inline static PyHeapTypeObject* NewType() {
                static PyType_Slot slots[] = {
                    {Py_tp_finalize, reinterpret_cast<void*>(__finalize__)},
                    {Py_tp_repr, reinterpret_cast<void*>(__repr__)},
                    {Py_tp_call, reinterpret_cast<void*>(__call__)},
                    {0, 0}
                };

                static PyType_Spec spec = {
                    "yapic.core.ForwardDecl",
                    sizeof(ForwardDecl),
                    0,
                    Py_TPFLAGS_DEFAULT,
                    slots
                };

                auto type = (PyHeapTypeObject*)PyType_FromSpec(&spec);
                if (type) {
                    type->ht_type.tp_new = NULL;
                }
                return type;
            }

            // static const PyTypeObject Type;

            inline static bool IsForwardTuple(PyObject* o) {
                assert(o != NULL);
                return PyTuple_CheckExact(o)
                    && PyTuple_GET_SIZE(o) == 3
                    && PyCode_Check(PyTuple_GET_ITEM(o, 0));
            }

        public:
            PyObject_HEAD
            PyObject* expr;
            PyObject* decl;

            static PyObject* __call__(ForwardDecl* self, PyObject* args, PyObject* kwargs) {
                return self->Resolve();
            }

            static void __finalize__(ForwardDecl* self) {
                Py_CLEAR(self->expr);
                Py_CLEAR(self->decl);
                Py_CLEAR(self->__args__);
                Py_CLEAR(self->copy_with);
            }

            static PyObject* __repr__(ForwardDecl* self) {
                return PyUnicode_FromFormat("<ForwardDecl %R>", self->expr);
            }

            inline bool IsGeneric() const {
                return !IsForwardTuple(decl);
            }

            PyObject* Value() {
                Py_INCREF(decl);
                return decl;
            }

            PyObject* Expr() {
                Py_INCREF(expr);
                return expr;
            }

            PyObject* Resolve() {
                return Resolve(NULL);
            }

            PyObject* Resolve(PyObject* extraLocals) {
                if (IsForwardTuple(decl)) {
                    // YapicTyping_DUMP(decl);
                    // PyObject* locals = PyTuple_GET_ITEM(decl, 2);
                    PyPtr<> locals(PyTuple_GET_ITEM(decl, 2));
                    if (locals.IsValid()) {
                        locals.Incref();
                    }

                    // YapicTyping_DUMP(locals);
                    // YapicTyping_DUMP(extraLocals);

                    if (extraLocals && PyDict_Check(extraLocals)) {
                        if (locals.IsNone()) {
                            locals = PyDict_New();
                            if (!locals.IsValid()) {
                                return NULL;
                            }
                        } else {
                            locals = PyDict_Copy(locals);
                            if (!locals.IsValid()) {
                                return NULL;
                            }
                        }

                        if (PyDict_Update(locals, extraLocals) != 0) {
                            return NULL;
                        }
                    } else {
                        if (locals.IsNone()) {
                            locals.Clear();
                        }
                    }

                    // YapicTyping_DUMP(locals);

                    PyObject* ev = PyEval_EvalCode(
                        PyTuple_GET_ITEM(decl, 0),
                        PyTuple_GET_ITEM(decl, 1),
                        locals);

                    if (ev != NULL) {
                        PyObject* res = Resolve(ev, extraLocals);
                        Py_DECREF(ev);
                        return res;
                    } else {
                        return NULL;
                    }
                } else {
                    return Resolve(decl, extraLocals);
                }
            }

            // PyObject* UnpackGeneric() {
            //     assert(IsGeneric());
            //     PyPtr<> args = PyObject_GetAttr(decl, __args__);
            //     if (args) {
            //         if (PyTuple_GET_SIZE(args) == 1) {
            //             return New(PyTuple_GET_ITEM(args, 0), PyTuple_GET_ITEM(args, 0), __args__, copy_with);
            //         } else {
            //             PyErr_SetString(PyExc_ValueError, "Try to unpack generic type with more than one arguments");
            //             return NULL;
            //         }
            //     } else {
            //         return NULL;
            //     }
            // }

        private:
            PyObject* Resolve(PyObject* obj, PyObject* extraLocals) {
                if (Py_TYPE(this) == Py_TYPE(obj)) {
                    return reinterpret_cast<ForwardDecl*>(obj)->Resolve(extraLocals);
                } else {
                    PyPtr<PyTupleObject> args = PyObject_GetAttr(obj, __args__);
                    if (args.IsValid()) {
                        Py_ssize_t l = PyTuple_GET_SIZE(args);
                        PyPtr<PyTupleObject> newArgs = PyTuple_New(l);

                        for (Py_ssize_t i = 0; i < l; ++i) {
                            PyPtr<> arg = Resolve(PyTuple_GET_ITEM(args, i), extraLocals);
                            if (arg) {
                                PyTuple_SET_ITEM(newArgs, i, arg.Steal());
                            } else {
                                return NULL;
                            }
                        }

                        PyPtr<> copy = PyObject_GetAttr(obj, copy_with);
                        if (copy) {
                            PyPtr<> copyArgs = PyTuple_Pack(1, newArgs.AsObject());
                            if (copyArgs) {
                                return PyObject_CallObject(copy, copyArgs);
                            } else {
                                return NULL;
                            }
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
                memset(this, 0, sizeof(Typing));
            }

            ~Typing() {
                Py_CLEAR(_typing);
                Py_CLEAR(Generic);
                Py_CLEAR(GenericAlias);
                Py_CLEAR(ForwardRef);
                Py_CLEAR(TypeVar);
                Py_CLEAR(MethodWrapperType);
                Py_CLEAR(ForwardDeclType);
                Py_CLEAR(__origin__);
                Py_CLEAR(__args__);
                Py_CLEAR(__parameters__);
                Py_CLEAR(__module__);
                Py_CLEAR(__forward_code__);
                Py_CLEAR(__forward_arg__);
                Py_CLEAR(__orig_bases__);
                Py_CLEAR(__orig_class__);
                Py_CLEAR(__name__);
                Py_CLEAR(__annotations__);
                Py_CLEAR(__dict__);
                Py_CLEAR(__init__);
                Py_CLEAR(__call__);
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
                Yapic_Typing_StrCache(__forward_arg__, "__forward_arg__");
                Yapic_Typing_StrCache(__orig_bases__, "__orig_bases__");
                Yapic_Typing_StrCache(__orig_class__, "__orig_class__");
                Yapic_Typing_StrCache(__name__, "__name__");
                Yapic_Typing_StrCache(__annotations__, "__annotations__");
                Yapic_Typing_StrCache(__dict__, "__dict__");
                Yapic_Typing_StrCache(__init__, "__init__");
                Yapic_Typing_StrCache(__call__, "__call__");
                Yapic_Typing_StrCache(copy_with, "copy_with");

                ForwardDeclType = ForwardDecl::NewType();
                if (ForwardDeclType == NULL) {
                    return false;
                }

                // PyType_Spec

                // printf("const_cast<PyTypeObject*>(&ForwardDecl::Type) = %p\n", const_cast<PyTypeObject*>(&ForwardDecl::Type));
                // YapicTyping_DUMP(const_cast<PyTypeObject*>(&ForwardDecl::Type));
                // if (PyType_Ready(const_cast<PyTypeObject*>(&ForwardDecl::Type)) < 0) {
                //     return false;
                // }
                // Py_INCREF(const_cast<PyTypeObject*>(&ForwardDecl::Type));
                // ForwardDeclType = const_cast<PyTypeObject*>(&ForwardDecl::Type);

                PyObject* wrapper = PyObject_GetAttr(reinterpret_cast<PyObject*>(&PyUnicode_Type), __init__);
                if (wrapper == NULL) {
                    return false;
                } else {
                    MethodWrapperType = (PyObject*)Py_TYPE(wrapper);
                    assert(MethodWrapperType != NULL);
                    Py_INCREF(MethodWrapperType);
                    Py_DECREF(wrapper);
                }

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
                assert(object != NULL);
                return Py_TYPE(object) == (PyTypeObject*) ForwardDeclType;
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
                    PyPtr<PyTupleObject> args = PyObject_GetAttr(type, __args__);
                    if (args.IsValid()) {
                        PyPtr<> origin = PyObject_GetAttr(type, __origin__);
                        if (origin.IsValid()) {
                            PyPtr<PyTupleObject> params = PyObject_GetAttr(origin, __parameters__);
                            if (params.IsValid()) {
                                assert(PyTuple_CheckExact(params));
                                assert(PyTuple_CheckExact(args));
                                assert(PyTuple_GET_SIZE(args) == PyTuple_GET_SIZE(params));
                                Py_ssize_t l = PyTuple_GET_SIZE(params);

                                PyObject* argItem;
                                PyObject* argResult;
                                for (Py_ssize_t i = 0; i < l; ++i) {
                                    argItem = PyTuple_GET_ITEM(args, i);
                                    argResult = PyDict_GetItem(vars, argItem);
                                    if (argResult == NULL) {
                                        argResult = argItem;
                                    }

                                    argResult = SubstType(argResult, type, vars, Py_None);
                                    if (argResult == NULL) {
                                        goto error;
                                    }

                                    if (PyDict_SetItem(resolved, PyTuple_GET_ITEM(params, i), argResult) == -1) {
                                        Py_DECREF(argResult);
                                        goto error;
                                    } else {
                                        Py_DECREF(argResult);
                                    }
                                }
                            }
                        }
                    }

                    if (PyErr_Occurred()) {
                        PyErr_Clear();
                    }
                    return resolved.Steal();
                }

                error:
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

                PyPtr<PyTypeObject> origin = PyObject_GetAttr(type, __origin__);
                if (origin) {
                    if (PyType_Check(origin)) {
                        mro = origin->tp_mro;
                        mro.Incref();
                    } else {
                        PyErr_BadInternalCall();
                        return NULL;
                    }
                } else {
                    PyErr_Clear();

                    if (PyType_Check(type)) {
                        mro = reinterpret_cast<PyTypeObject*>(type)->tp_mro;
                        mro.Incref();
                    } else {
                        PyErr_BadArgument();
                        return NULL;
                    }
                }

                // if (IsGenericType(type)) {
                //     PyPtr<PyTypeObject> origin = PyObject_GetAttr(type, __origin__);
                //     if (origin.IsValid()) {
                //         if (PyType_Check(origin)) {
                //             mro = origin->tp_mro;
                //             mro.Incref();
                //         } else {
                //             PyErr_BadInternalCall();
                //             return NULL;
                //         }
                //     } else {
                //         return NULL;
                //     }
                // } else if (PyType_Check(type)) {
                //     mro = reinterpret_cast<PyTypeObject*>(type)->tp_mro;
                //     mro.Incref();
                // } else {
                //     PyErr_BadArgument();
                //     return NULL;
                // }

                assert(mro.IsValid());
                assert(PyTuple_CheckExact(mro));

                PyPtr<> resolved = PyTuple_New(PyTuple_GET_SIZE(mro.As<PyTupleObject>()));
                if (resolved.IsValid()
                    && ResolveMro(type, mro, resolved, vars)
                    && FillMroHoles(mro, resolved)) {
                    return resolved.Steal();
                } else {
                    return NULL;
                }
            }

            inline PyObject* TypeHints(PyObject* type) {
                PyPtr<> vars = PyDict_New();
                if (vars.IsValid()) {
                    return TypeHints(type, vars);
                } else {
                    return NULL;
                }
            }

            // returns (original_class, attributes, init)
            PyObject* TypeHints(PyObject* type, PyObject* vars) {
                PyPtr<PyTupleObject> mro = ResolveMro(type, vars);

                if (mro.IsValid()) {
                    PyPtr<> attrs;
                    PyPtr<> init;

                    PyObject* mroEntry;
                    PyPtr<> annots(NULL);
                    Py_ssize_t l = PyTuple_GET_SIZE(mro) - 1;
                    for (Py_ssize_t i = 0; i < l; ++i) {
                        mroEntry = PyTuple_GET_ITEM(mro, i);
                        assert(mroEntry != NULL);

                        PyObject* currentType = PyTuple_GET_ITEM(mroEntry, 0);
                        PyObject* currentVars = PyTuple_GET_ITEM(mroEntry, 2);
                        PyPtr<> clsDict = PyObject_GetAttr(currentType, __dict__);
                        if (!clsDict) {
                            PyErr_Clear();
                            continue;
                        }

                        annots = PyObject_GetItem(clsDict, __annotations__);
                        if (annots.IsValid()) {
                            if (attrs.IsNone()) {
                                attrs = PyDict_New();
                                if (!attrs) {
                                    return NULL;
                                }
                            }
                            if (!ResolveAnnots(currentType, annots, currentVars, attrs)) {
                                return NULL;
                            }
                        } else {
                            PyErr_Clear();
                        }

                        if (init.IsNone()) {
                            PyPtr<> initFn = PyObject_GetItem(clsDict, __init__);

                            if (initFn.IsValid()) {
                                init = CallableHints(initFn, currentType, currentVars, true);
                                if (init.IsNull()) {
                                    if (PyErr_Occurred()) {
                                        return NULL;
                                    } else {
                                        init = Py_None;
                                    }
                                }
                            } else {
                                PyErr_Clear();
                            }
                        }
                    }

                    PyObject* res = PyTuple_New(3);
                    if (res != NULL) {
                        PyObject* oc = PyTuple_GET_ITEM(mro, 0);
                        assert(oc != NULL);
                        oc = PyTuple_GET_ITEM(oc, 0);
                        Py_INCREF(oc);
                        PyTuple_SET_ITEM(res, 0, oc);
                        PyTuple_SET_ITEM(res, 1, attrs.Steal());
                        PyTuple_SET_ITEM(res, 2, init.Steal());
                        return res;
                    }
                }
                return NULL;
            }

            inline PyObject* CallableHints(PyObject* callable) {
                PyPtr<> vars = PyDict_New();
                if (vars.IsValid()) {
                    return CallableHints(callable, NULL, vars);
                } else {
                    return NULL;
                }
            }

            inline PyObject* CallableHints(PyObject* callable, PyObject* type) {
                PyPtr<> vars = ResolveTypeVars(type);
                if (vars.IsValid()) {
                    return CallableHints(callable, type, vars);
                } else {
                    return NULL;
                }
            }

            /**
             * result = (
             *      positional arguments = (
             *          arg1 = (name, type[, defaultValue]),
             *          argN = (name, type[, defaultValue])
             *      ),
             *      keyword only arguments = (
             *          arg1 = (name, type[, defaultValue]),
             *          argN = (name, type[, defaultValue])
             *      )
             * )
            */
            inline PyObject* CallableHints(PyObject* callable, PyObject* type, PyObject* vars) {
                return CallableHints(callable, type, vars, false);
            }

            inline PyObject* CallableHints(PyObject* callable, PyObject* type, PyObject* vars, bool optional) {
                PyFunctionObject* func;
                PyObject* bound = type;

                if (CallableInfo(callable, optional, func, bound)) {
                    if (!type && bound) {
                        PyPtr<> oclass = PyObject_GetAttr(bound, __orig_class__);
                        if (oclass) {
                            PyPtr<> nvars = ResolveTypeVars(oclass, vars);
                            if (nvars) {
                                return reinterpret_cast<PyObject*>(ResolveArguments(func, 1, oclass, nvars));
                            } else {
                                return NULL;
                            }
                        } else {
                            PyErr_Clear();
                        }
                    }

                    return reinterpret_cast<PyObject*>(ResolveArguments(func, bound == NULL ? 0 : 1, type, vars));
                } else {
                    return NULL;
                }
            }

            /**
             * returns tuple (
             *      genericClass,
             *      (<ForwardDecl 1>, <ForwardDecl 2>, <ForwardDecl N>)
             * )
             */
            PyObject* UnpackForwardDecl(PyObject* o) {
                assert(o != NULL);
                assert(IsForwardDecl(o));

                ForwardDecl* fwd = (ForwardDecl*)o;
                if (fwd->IsGeneric()) {
                    PyObject* decl = fwd->decl;
                    PyPtr<PyTupleObject> args = PyObject_GetAttr(decl, __args__);
                    if (args) {
                        PyPtr<> origin = PyObject_GetAttr(decl, __origin__);
                        if (origin) {
                            Py_ssize_t l = PyTuple_GET_SIZE(args);
                            PyPtr<PyTupleObject> result = PyTuple_New(l);

                            for (Py_ssize_t i = 0; i < l; ++i) {
                                PyObject* arg = PyTuple_GET_ITEM(args, i);
                                if (IsForwardDecl(arg)) {
                                    Py_INCREF(arg);
                                    PyTuple_SET_ITEM(result, i, arg);
                                } else {
                                    PyObject* newFwd = NewForwardDecl(arg, arg);
                                    if (newFwd != NULL) {
                                        PyTuple_SET_ITEM(result, i, newFwd);
                                    } else {
                                        return NULL;
                                    }
                                }
                            }

                            return PyTuple_Pack(2, origin.AsObject(), result.AsObject());
                        } else {
                            return NULL;
                        }
                    } else {
                        return NULL;
                    }
                } else {
                    PyErr_Format(PyExc_ValueError, "The given 'ForwardDecl' instance is not generic: %R", o);
                    return NULL;
                }
            }

        private:
            PyObject* NewForwardDecl(PyObject* expr, PyObject* decl) {
                assert((ForwardDeclType->ht_type.tp_flags & Py_TPFLAGS_READY) == Py_TPFLAGS_READY);

                PyTypeObject* fwdType = (PyTypeObject*) ForwardDeclType;
                ForwardDecl* self = (ForwardDecl*) fwdType->tp_alloc(fwdType, fwdType->tp_basicsize);
                if (self != NULL) {
                    self->expr = expr;
                    self->decl = decl;
                    self->__args__ = __args__;
                    self->copy_with = copy_with;
                    Py_INCREF(expr);
                    Py_INCREF(decl);
                    Py_INCREF(__args__);
                    Py_INCREF(copy_with);
                }
                return (PyObject*) self;
            }

            PyObject* NewForwardDecl(PyCodeObject* code, PyObject* expr, PyDictObject* globals, PyDictObject* locals) {
                PyPtr<PyTupleObject> result = PyTuple_New(3);
                if (result.IsValid()) {
                    Py_INCREF(code);
                    Py_INCREF(globals);
                    Py_INCREF(locals);

                    PyTuple_SET_ITEM(result, 0, (PyObject*)code);
                    PyTuple_SET_ITEM(result, 1, (PyObject*)globals);
                    PyTuple_SET_ITEM(result, 2, (PyObject*)locals);

                    return NewForwardDecl(expr, result);
                } else {
                    return NULL;
                }
            }

            PyObject* NewForwardDecl(PyObject* ref, PyObject* type, PyObject* locals) {
                assert(IsForwardRef(ref));

                PyPtr<PyCodeObject> code = (PyCodeObject*)PyObject_GetAttr(ref, __forward_code__);
                if (!code) {
                    return NULL;
                }

                PyPtr<> expr = PyObject_GetAttr(ref, __forward_arg__);
                if (!expr) {
                    return NULL;
                }

                PyPtr<> moduleName = PyObject_GetAttr(type, __module__);
                if (!moduleName) {
                    return NULL;
                }

                PyPtr<> module = PyImport_Import(moduleName);
                if (!module) {
                    return NULL;
                }

                PyObject* moduleDict = PyModule_GetDict(module); // borrowed
                assert(moduleDict != NULL);

                return NewForwardDecl((PyCodeObject*)code, expr, (PyDictObject*)moduleDict, (PyDictObject*)locals);
            }

            PyObject* NewForwardDecl(const char* ref, PyObject* expr, PyDictObject* globals, PyDictObject* locals) {
                PyPtr<PyCodeObject> code = (PyCodeObject*)Py_CompileString(ref, "<string>", Py_eval_input);
                if (code.IsValid()) {
                    return NewForwardDecl((PyCodeObject*)code, expr, globals, locals);
                } else {
                    return NULL;
                }
            }

            PyObject* NewForwardDecl(PyUnicodeObject* str, PyObject* type, PyObject* locals) {
                PyPtr<PyUnicodeObject> moduleName = PyObject_GetAttr(type, __module__);
                if (moduleName.IsValid()) {
                    return NewForwardDecl(str, (PyUnicodeObject*)moduleName, locals);
                } else {
                    return NULL;
                }
            }

            PyObject* NewForwardDecl(PyUnicodeObject* str, PyUnicodeObject* moduleName, PyObject* locals) {
                PyPtr<> module = PyImport_Import((PyObject*)moduleName);
                if (module.IsValid()) {
                    PyDictObject* moduleDict = (PyDictObject*)PyModule_GetDict(module);
                    if (moduleDict != NULL) {
                        return NewForwardDecl(str, (PyDictObject*)moduleDict, (PyDictObject*)locals);
                    } else {
                        return NULL;
                    }
                } else {
                    return NULL;
                }
            }

            PyObject* NewForwardDecl(PyUnicodeObject* str, PyDictObject* globals, PyDictObject* locals) {
                PyPtr<PyBytesObject> ascii = PyUnicode_AsASCIIString((PyObject*)str);
                if (ascii.IsValid()) {
                    return NewForwardDecl(PyBytes_AS_STRING(ascii), (PyObject*)str, globals, locals);
                }
                return NULL;
            }

            /**
             * mro_entry = (original_type, generic_type)
             */
            bool ResolveMro(PyObject* type, PyObject *mro, PyObject *resolved, PyObject* vars) {
                PyPtr<PyTupleObject> bases;
                PyPtr<> origin = PyObject_GetAttr(type, __origin__);

                if (!origin) {
                    PyErr_Clear();
                    origin = type;
                    origin.Incref();
                }
                bases = PyObject_GetAttr(origin, __orig_bases__);

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
                return true;
            }

            void ReplaceMro(PyObject *mro, PyObject *resolved, PyObject* orig, PyObject* entry) {
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
                if (locals.IsValid()) {
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
                } else {
                    return NULL;
                }
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
                                PyPtr<> forward = NewForwardDecl((PyUnicodeObject*)value, type, locals);
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
                PyPtr<> res = _SubstType(value, type, vars, locals, &hasFwd);
                if (res.IsValid() && hasFwd && !IsForwardDecl(res)) {
                    return NewForwardDecl(res, res);
                } else {
                    return res.Steal();
                }
            }

            PyObject* _SubstType(PyObject* value, PyObject* type, PyObject* vars, PyObject* locals, bool *hasFwd) {
                if (IsTypeVar(value)) {
                    PyObject* exists = PyDict_GetItem(vars, value);
                    if (exists != NULL && exists != value) {
                        return _SubstType(exists, type, vars, locals, hasFwd);
                    }
                } else if (IsForwardRef(value)) {
                    PyObject* exists = PyDict_GetItem(vars, value);
                    if (exists == NULL) {
                        *hasFwd = true;
                        return NewForwardDecl(value, type, locals);
                    }
                } else if (IsGenericType(value)) {
                    PyPtr<PyTupleObject> args = PyObject_GetAttr(value, __args__);
                    if (args.IsValid()) {
                        Py_ssize_t l = PyTuple_GET_SIZE(args);
                        if (l > 0) {
                            PyPtr<PyTupleObject> newArgs = PyTuple_New(l);
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

                                PyPtr<> copy = PyObject_GetAttr(value, copy_with);
                                if (copy) {
                                    PyPtr<> copyArgs = PyTuple_Pack(1, newArgs.AsObject());
                                    if (copyArgs) {
                                        return PyObject_CallObject(copy, copyArgs);
                                    } else {
                                        return NULL;
                                    }
                                } else {
                                    return NULL;
                                }
                            } else {
                                return NULL;
                            }
                        }
                    }
                    return NULL;
                }

                if (!*hasFwd && IsForwardDecl(value)) {
                    *hasFwd = true;
                }

                Py_INCREF(value);
                return value;
            }

            bool CallableInfo(PyObject* callable, bool optional, PyFunctionObject*& func, PyObject*& bound) {
                if (PyFunction_Check(callable)) {
                    func = (PyFunctionObject*) callable;
					return true;
                } else if (PyMethod_Check(callable)) {
                    func = (PyFunctionObject*) PyMethod_GET_FUNCTION(callable);
					assert(PyFunction_Check(func));
					bound = PyMethod_GET_SELF(callable);
                    return true;
                } else if (PyObject_IsInstance(callable, MethodWrapperType)) {
                    if (!optional) {
                        PyErr_Format(PyExc_TypeError, "Cannot get type hints from built / c-extension method: %R", callable);
                    }
                    return false;
                } else {
                    PyTypeObject* ct = reinterpret_cast<PyTypeObject*>(Py_TYPE(callable));
                    PyObject* mro = ct->tp_mro;
                    assert(mro != NULL);
                    assert(PyTuple_CheckExact(mro));

                    PyObject* mroEntry;
                    PyObject* clsDict;
                    PyPtr<> callFn(NULL);
                    Py_ssize_t l = PyTuple_GET_SIZE(mro) - 1; // skip object from mro
                    for (Py_ssize_t i = 0; i < l; ++i) {
                        mroEntry = PyTuple_GET_ITEM(mro, i);
                        clsDict = PyObject_GetAttr(mroEntry, __dict__);
                        if (clsDict == NULL) {
                            PyErr_Clear();
                            continue;
                        }

                        callFn = PyObject_GetItem(clsDict, __call__);
                        Py_DECREF(clsDict);

                        if (callFn) {
                            bound = callable;
                            return CallableInfo(callFn, optional, func, bound);
                        }
                    }
                }

                PyErr_Format(PyExc_TypeError, "Got unsupported callable: %R", callable);
                return false;
            }

            PyTupleObject* ResolveArguments(PyFunctionObject* func, int offset, PyObject* type, PyObject* vars) {
                PyCodeObject* code = (PyCodeObject*) PyFunction_GET_CODE(func);
                assert(code != NULL && PyCode_Check(code));

                PyObject* globals = PyFunction_GET_GLOBALS(func);
                assert(globals != NULL && PyDict_CheckExact(globals));

                PyPtr<> locals(VarsToLocals(vars));
                if (locals.IsNull()) {
                    return NULL;
                }

                PyPtr<PyTupleObject> result(PyTuple_New(2));
                if (result.IsNull()) {
                    return NULL;
                }

                PyObject* annots = PyFunction_GET_ANNOTATIONS(func);
                if (annots != NULL) {
                    assert(PyDict_CheckExact(annots));
                }

                PyPtr<PyTupleObject> args;
                PyPtr<PyTupleObject> keywords;
                PyObject* defaults;
                PyObject* argName;
                PyObject* argType;
                PyObject* argDef;
                int argcount = code->co_argcount - offset;
                if (argcount > 0) {
                    assert(PyTuple_CheckExact(code->co_varnames));
                    assert(PyTuple_GET_SIZE(code->co_varnames) >= argcount);

                    args = PyTuple_New(argcount);
                    if (args.IsNull()) {
                        return NULL;
                    }

                    defaults = PyFunction_GET_DEFAULTS(func);
                    if (defaults == NULL) {
                        for (int i=offset ; i<code->co_argcount ; ++i) {
                            argName = PyTuple_GET_ITEM(code->co_varnames, i);
                            argType = ResolveArgumentType(annots, argName, type, vars, globals, locals);
                            if (argType == NULL) {
                                return NULL;
                            }

                            PyObject* entry = PyTuple_Pack(2, argName, argType);
                            Py_DECREF(argType);

                            if (entry == NULL) {
                                return NULL;
                            }
                            PyTuple_SET_ITEM(args, i - offset, entry);
                        }
                    } else {
                        assert(PyTuple_CheckExact(defaults));

                        Py_ssize_t defcount = PyTuple_GET_SIZE(defaults);
                        Py_ssize_t defcounter = 0;
                        for (int i=offset ; i<code->co_argcount ; ++i) {
                            argName = PyTuple_GET_ITEM(code->co_varnames, i);
                            argDef = (code->co_argcount - defcount <= i
                                ? PyTuple_GET_ITEM(defaults, defcounter++)
                                : NULL);
                            argType = ResolveArgumentType(annots, argName, type, vars, globals, locals);

                            if (argType == NULL) {
                                return NULL;
                            }

                            PyObject* entry = argDef == NULL
                                ? PyTuple_Pack(2, argName, argType)
                                : PyTuple_Pack(3, argName, argType, argDef);
                            Py_DECREF(argType);

                            if (entry == NULL) {
                                return NULL;
                            }
                            PyTuple_SET_ITEM(args, i - offset, entry);
                        }
                    }
                }

                if (code->co_kwonlyargcount) {
                    assert(PyTuple_GET_SIZE(code->co_varnames) >= code->co_kwonlyargcount + code->co_argcount);

                    keywords = PyTuple_New(code->co_kwonlyargcount);
                    if (keywords.IsNull()) {
                        return NULL;
                    }

                    defaults = PyFunction_GET_KW_DEFAULTS(func);
                    if (defaults == NULL) {
                        for (int i=code->co_argcount ; i<code->co_kwonlyargcount + code->co_argcount ; i++) {
                            argName = PyTuple_GET_ITEM(code->co_varnames, i);
                            argType = ResolveArgumentType(annots, argName, type, vars, globals, locals);
                            if (argType == NULL) {
                                return NULL;
                            }

                            PyObject* entry = PyTuple_Pack(2, argName, argType);
                            Py_DECREF(argType);

                            if (entry == NULL) {
                                return NULL;
                            }
                            PyTuple_SET_ITEM(keywords, i - code->co_argcount, entry);
                        }
                    } else {
                        assert(PyDict_CheckExact(defaults));

                        for (int i=code->co_argcount ; i<code->co_kwonlyargcount + code->co_argcount ; i++) {
                            argName = PyTuple_GET_ITEM(code->co_varnames, i);
                            argDef = PyDict_GetItem(defaults, argName);
                            argType = ResolveArgumentType(annots, argName, type, vars, globals, locals);
                            if (argType == NULL) {
                                return NULL;
                            }

                            PyObject* entry = argDef == NULL
                                ? PyTuple_Pack(2, argName, argType)
                                : PyTuple_Pack(3, argName, argType, argDef);
                            Py_DECREF(argType);

                            if (entry == NULL) {
                                return NULL;
                            }
                            PyTuple_SET_ITEM(keywords, i - code->co_argcount, entry);
                        }
                    }
                }

                PyTuple_SET_ITEM(result, 0, reinterpret_cast<PyObject*>(args.Steal()));
                PyTuple_SET_ITEM(result, 1, reinterpret_cast<PyObject*>(keywords.Steal()));
                return result.Steal();
            }

            inline PyObject* ResolveArgumentType(PyObject* annots, PyObject* name, PyObject* type, PyObject* vars, PyObject* globals, PyObject* locals) {
                if (annots != NULL) {
                    PyObject* value = PyDict_GetItem(annots, name);
                    if (value != NULL) {
                        if (PyUnicode_Check(value)) {
                            return NewForwardDecl((PyUnicodeObject*)value, (PyDictObject*)globals, (PyDictObject*)locals);
                        } else {
                            return SubstType(value, type, vars, locals);
                        }
                    } else {
                        Py_RETURN_NONE;
                    }
                } else {
                    Py_RETURN_NONE;
                }
            }


            PyObject* _typing;
            PyObject* Generic;
            PyObject* GenericAlias;
            PyObject* ForwardRef;
            PyObject* TypeVar;
            PyObject* MethodWrapperType;
            PyHeapTypeObject* ForwardDeclType;

            PyObject* __origin__;
            PyObject* __args__;
            PyObject* __parameters__;
            PyObject* __module__;
            PyObject* __forward_code__;
            PyObject* __forward_arg__;
            PyObject* __orig_bases__;
            PyObject* __orig_class__;
            PyObject* __name__;
            PyObject* __annotations__;
            PyObject* __dict__;
            PyObject* __init__;
            PyObject* __call__;
            PyObject* copy_with;
    };


    // const PyTypeObject ForwardDecl::Type = {
    //     PyVarObject_HEAD_INIT(NULL, 0)
    //     /* tp_name */ 			"yapic.core.ForwardDecl",
    //     /* tp_basicsize */ 		sizeof(ForwardDecl),
    //     /* tp_itemsize */ 		0,
    //     /* tp_dealloc */ 		(destructor) ForwardDecl::__dealloc__,
    //     /* tp_print */ 			0,
    //     /* tp_getattr */ 		0,
    //     /* tp_setattr */ 		0,
    //     /* tp_as_async */ 		0,
    //     /* tp_repr */ 			(reprfunc) ForwardDecl::__repr__,
    //     /* tp_as_number */ 		0,
    //     /* tp_as_sequence */ 	0,
    //     /* tp_as_mapping */ 	0,
    //     /* tp_hash  */ 			0,
    //     /* tp_call */ 			(ternaryfunc) ForwardDecl::__call__,
    //     /* tp_str */ 			0,
    //     /* tp_getattro */ 		0,
    //     /* tp_setattro */ 		0,
    //     /* tp_as_buffer */ 		0,
    //     /* tp_flags */ 			0,
    //     /* tp_doc */ 			0,
    //     /* tp_traverse */ 		0,
    //     /* tp_clear */ 			0,
    //     /* tp_richcompare */ 	0,
    //     /* tp_weaklistoffset */ 0,
    //     /* tp_iter */ 			0,
    //     /* tp_iternext */ 		0,
    //     /* tp_methods */ 		0,
    //     /* tp_members */ 		0,
    //     /* tp_getset */ 		0,
    //     /* tp_base */ 			0,
    //     /* tp_dict */ 			0,
    //     /* tp_descr_get */ 		0,
    //     /* tp_descr_set */ 		0,
    //     /* tp_dictoffset */ 	0,
    //     /* tp_init */ 			0,
    //     /* tp_alloc */ 			0,
    //     /* tp_new */ 			PyType_GenericNew
    // };


}; /* end namespace Yapic */


#endif /* D57C95BE_1134_1461_11A3_990B2EDF5491 */
