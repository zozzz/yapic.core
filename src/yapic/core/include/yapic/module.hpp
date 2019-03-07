#ifndef Y9E57ECE_9133_C8FE_1260_AD9C7DDC5FCB
#define Y9E57ECE_9133_C8FE_1260_AD9C7DDC5FCB

#include <string>
#include <vector>
#include <iterator>
#include <cstdarg>

#include <Python.h>
#include "./type.hpp"
#include "./common.hpp"


namespace Yapic {


template<typename Module>
class _ModuleConst {
public:
	static const int Error = 42;

	// XXX: constructor not needed, because module state allocator 0-ing every byte
	inline void Free() {
		if (ref != NULL) {
			Py_CLEAR(ref);
		}
	}

	inline operator PyObject* () { assert(ref != NULL); return ref; }
	inline bool Eq(PyObject* other) const { return ref == other; }
protected:
	PyObject* ref;
	inline void _register() {
		Module::State()->__vars->push_back(this);
	}
};


template<typename Module>
class ModuleVar : public _ModuleConst<Module> {
public:
	inline ModuleVar<Module>& operator=(const char* val) { return Value(val); }
	inline ModuleVar<Module>& operator=(Py_ssize_t val) { return Value(val); }
	inline ModuleVar<Module>& operator=(PyObject* obj) { return Value(obj); }
	inline ModuleVar<Module>& Value(const char* val) { return Value(PyUnicode_InternFromString(val)); }
	inline ModuleVar<Module>& Value(Py_ssize_t val) { return Value(PyLong_FromSsize_t(val)); }

	inline ModuleVar<Module>& Value(PyObject* val) {
		assert(ref == NULL);
		if (val == NULL) {
			if (!PyErr_Occurred()) {
				PyErr_BadInternalCall();
			}
			throw _ModuleConst::Error;
		} else {
			ref = val;
			_register();
		}
		return *this;
	}

	inline PyObject* Value() const { return ref; }

	inline ModuleVar<Module>& Export(const char* name) {
		assert(ref != NULL);
		if (ref != NULL) {
			Py_INCREF(ref);
			if (PyModule_AddObject(Module::Instance(), name, (PyObject*) ref) == -1) {
				Py_DECREF(ref);
				throw _ModuleConst::Error;
			}
		}
	}
};


template<typename Module>
class ModuleExc : public _ModuleConst<Module> {
public:
	inline void Define(const char* name, PyObject* base, PyObject* dict) {
		std::string excName(Module::__name__);
		excName += '.';
		excName += name;
		ref = PyErr_NewException(excName.c_str(), base, dict);
		if (ref == NULL) {
			throw _ModuleConst::Error;
		}
		_register();
		// Py_INCREF(ref);
		if (PyModule_AddObject(Module::Instance(), name, (PyObject*) ref) == -1) {
			// Py_DECREF(ref);
			throw _ModuleConst::Error;
		}
	}

	inline void Define(const char* name, PyObject* base) {
		Define(name, base, NULL);
	}

	inline void Define(const char* name) {
		Define(name, NULL, NULL);
	}

	// TODO: call PyErr_SetString if vargs is not present (in compile time)
	inline PyObject* Raise(const char* message, ...) {
		va_list vargs;
		va_start(vargs, message);
		PyErr_FormatV(ref, message, vargs);
		va_end(vargs);
		return NULL;
	}

	// inline PyObject* Raise(const char* message) {
	// 	PyErr_SetString(ref, message);
	// 	return NULL;
	// }
};

template<typename Module>
class ModuleRef : public _ModuleConst<Module> {
public:
	// Import("module.name") == from module.name import *
	//		module.name == ref // only import __all__ if specified
	inline void Import(const char* moduleName) {
		assert(ref == NULL);
		ref = PyImport_ImportModule(moduleName);
		if (ref == NULL) {
			throw _ModuleConst::Error;
		}
	}

	// Import("module", "name") == from module import name
	//		name == ref
	// Import("module.name", "o1", "o2") = from module import o1, o2
	//		o1 == ref.o1
	//		02 == ref.o2
	template<typename... T>
	inline void Import(const char* moduleName, T... objNames) {
		assert(ref == NULL);

		bool res = true;
		const int size = sizeof...(objNames);

		PyObject* fromList = PyList_New(size + 1);
		if (fromList == NULL) {
			throw _ModuleConst::Error;
		}

		int i=0;
		for (auto objName : {objNames...}) {
			PyObject* oName = PyUnicode_InternFromString(objName);
			if (oName == NULL) {
				res = false;
				goto end;
			}
			PyList_SET_ITEM(fromList, i, oName);
			++i;
		}

		ref = PyImport_ImportModuleEx(moduleName, NULL, NULL, fromList);
		if (ref == NULL) {
			res = false;
			goto end;
		} else {
			if (size == 1) {
				PyObject *obj = PyObject_GetAttr(ref, PyList_GET_ITEM(fromList, 0));
				if (obj == NULL) {
					res = false;
					goto end;
				}
				Py_SETREF(ref, obj);
			}
			_register();
		}

	end:
		Py_XDECREF(fromList);
		if (!res) {
			throw _ModuleConst::Error;
		}
	}

	inline bool CheckExact(const PyTypeObject* type) {
		assert(type != NULL);
		return type == reinterpret_cast<const PyTypeObject*>(ref);
	}

	inline bool CheckExact(const PyObject* obj) {
		assert(obj != NULL);
		return CheckExact(reinterpret_cast<const PyTypeObject*>(Py_TYPE(obj)));
	}

	inline bool Check(const PyObject* obj) {
		assert(obj != NULL);
		return PyObject_TypeCheck(const_cast<PyObject*>(obj), reinterpret_cast<PyTypeObject*>(ref));
	}

	inline bool IsSubclass(const PyTypeObject* type) {
		assert(type != NULL);
		return PyObject_IsSubclass(const_cast<PyObject*>(reinterpret_cast<const PyObject*>(type)), ref);
	}
};


template<typename _self>
class Module {
	public:
		using Self = _self;
		using Super = Module<_self>;
		using VarsList = std::vector<_ModuleConst<Self>*>;

		static constexpr const char* __doc__ = NULL;
		VarsList* __vars; // state variable

		static inline Self* State() {
			return State(Self::Instance());
		}

		static inline Self* State(PyObject* module) {
			Self* state = (Self*) PyModule_GetState(module);
			assert(state != NULL);
			return state;
		}

		static inline PyObject* Instance() {
			PyObject* module = PyState_FindModule(const_cast<PyModuleDef*>(Self::Definition()));
			assert(module != NULL);
			return module;
		}

		static inline PyObject* Create() {
			PyObject* module = PyModule_Create(const_cast<PyModuleDef*>(Self::Definition()));
			if (module == NULL) {
				return NULL;
			}
			PyState_AddModule(module, const_cast<PyModuleDef*>(Self::Definition()));
			State(module)->__vars = new VarsList();
			try {
				if (Self::__init__(module, State(module)) == -1) {
					Py_DECREF(module);
					return NULL;
				}
			} catch (...) {
				Py_DECREF(module);
				return NULL;
			}
			return module;
		}

		static inline const PyModuleDef* Definition() {
			static const PyModuleDef def = {
				PyModuleDef_HEAD_INIT,
				Self::__name__,
				Self::__doc__,
				sizeof(Self),
				const_cast<PyMethodDef*>(Self::__methods__()),
				0,
				Yapic_GetTypeMethod(Self, __traverse__),
				Self::__clear__,
				Yapic_GetTypeMethod(Self, __free__)
			};
			return &def;
		}

		static inline const PyMethodDef* __methods__() {
			return NULL;
		}

		static inline int __init__(PyObject* module, Self* state) {
			return 0;
		}

		static inline int __clear__(PyObject* module) {
			Self* state = State(module);
			for (auto c : *state->__vars) {
				c->Free();
			}
			state->__vars->clear();
			return 0;
		}
};


} // end namespace Yapic
#endif /* Y9E57ECE_9133_C8FE_1260_AD9C7DDC5FCB */
