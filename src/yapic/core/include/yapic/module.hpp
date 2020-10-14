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
	inline void Free() { Py_CLEAR(ref); }
	inline PyObject* Ref() const { return ref; }

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
		assert(this->ref == NULL);
		if (val == NULL) {
			if (!PyErr_Occurred()) {
				PyErr_BadInternalCall();
			}
			throw _ModuleConst<Module>::Error;
		} else {
			this->ref = val;
			this->_register();
		}
		return *this;
	}

	inline PyObject* Value() const { return this->ref; }

	inline ModuleVar<Module>& Export(const char* name) {
		assert(this->ref != NULL);
		if (this->ref != NULL) {
			// PyModule_AddObject steals a reference to value
			if (PyModule_AddObject(Module::Instance(), name, (PyObject*) this->ref) == 0) {
				Py_INCREF(this->ref);
			} else {
				throw _ModuleConst<Module>::Error;
			}
		}
		return *this;
	}
};


template<typename Module>
class ModuleExc : public _ModuleConst<Module> {
public:
	inline void Define(const char* name, PyObject* base, PyObject* dict) {
		std::string excName(Module::__name__);
		excName += '.';
		excName += name;
		this->ref = PyErr_NewException(excName.c_str(), base, dict);
		if (this->ref == NULL) {
			throw _ModuleConst<Module>::Error;
		}

		this->_register();

		// PyModule_AddObject steals a reference to value
		if (PyModule_AddObject(Module::Instance(), name, (PyObject*) this->ref) == 0) {
			Py_INCREF(this->ref);
		} else {
			throw _ModuleConst<Module>::Error;
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
		PyErr_FormatV(this->ref, message, vargs);
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
		assert(this->ref == NULL);
		this->ref = PyImport_ImportModule(moduleName);
		if (this->ref == NULL) {
			throw _ModuleConst<Module>::Error;
		}
	}

	// Import("module", "name") == from module import name
	//		name == ref
	// Import("module.name", "o1", "o2") = from module import o1, o2
	//		o1 == ref.o1
	//		02 == ref.o2
	template<typename... T>
	inline void Import(const char* moduleName, T... objNames) {
		assert(this->ref == NULL);

		bool res = true;
		const int size = sizeof...(objNames);

		PyObject* fromList = PyList_New(size + 1);
		if (fromList == NULL) {
			throw _ModuleConst<Module>::Error;
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

		this->ref = PyImport_ImportModuleEx(moduleName, NULL, NULL, fromList);
		if (this->ref == NULL) {
			res = false;
			goto end;
		} else {
			if (size == 1) {
				PyObject *obj = PyObject_GetAttr(this->ref, PyList_GET_ITEM(fromList, 0));
				if (obj == NULL) {
					res = false;
					goto end;
				}
				Py_SETREF(this->ref, obj);
			}
			this->_register();
		}

	end:
		Py_XDECREF(fromList);
		if (!res) {
			throw _ModuleConst<Module>::Error;
		}
	}

	inline bool CheckExact(const PyTypeObject* type) {
		assert(type != NULL);
		return type == reinterpret_cast<const PyTypeObject*>(this->ref);
	}

	inline bool CheckExact(const PyObject* obj) {
		assert(obj != NULL);
		return CheckExact(reinterpret_cast<const PyTypeObject*>(Py_TYPE(obj)));
	}

	inline bool Check(const PyObject* obj) {
		assert(obj != NULL);
		return PyObject_TypeCheck(const_cast<PyObject*>(obj), reinterpret_cast<PyTypeObject*>(this->ref));
	}

	inline bool IsSubclass(const PyTypeObject* type) {
		assert(type != NULL);
		return PyObject_IsSubclass(const_cast<PyObject*>(reinterpret_cast<const PyObject*>(type)), this->ref);
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
			return Self::_State();
		}

		static inline Self*& _State() {
			static Self* state = NULL;
			return state;
		}

		static inline PyObject* Instance() {
			return _Instance();
		}

		static inline PyObject*& _Instance() {
			static PyObject* instance = NULL;
			return instance;
		}

		static inline Self* State(PyObject* module) {
			Self* state = (Self*) PyModule_GetState(module);
			assert(state != NULL);
			return state;
		}

#if PY_VERSION_HEX >= 0x03090000
		static inline PyObject* Create() {
			// this assert is happen when, using same names on derived module class name
			assert(Self::Definition()->m_base.m_index == 0);
			return PyModuleDef_Init(const_cast<PyModuleDef*>(Self::Definition()));
		}
#else
		static inline PyObject* Create() {
			// this assert is happen when, using same names on derived module class name
			assert(Self::Definition()->m_base.m_index == 0);

			PyObject* module = PyModule_Create(const_cast<PyModuleDef*>(Self::Definition()));
			if (module == NULL) {
				return NULL;
			}
			PyState_AddModule(module, const_cast<PyModuleDef*>(Self::Definition()));

			if (Self::__exec__(module) == 0) {
				return module;
			} else {
				Py_DECREF(module);
				return NULL;
			}
		}
#endif

		static const PyModuleDef* Definition() {
			static const PyModuleDef def = {
				PyModuleDef_HEAD_INIT,
				Self::__name__,
				Self::__doc__,
				sizeof(Self),
				const_cast<PyMethodDef*>(Self::__methods__()),
#if PY_VERSION_HEX >= 0x03090000
					const_cast<PyModuleDef_Slot*>(Self::__slots__()),
#else
					0,
#endif
				Yapic_GetTypeMethod(Self, __traverse__),
				Self::__clear__,
				Yapic_GetTypeMethod(Self, __free__)
			};
			return &def;
		}

		static inline const PyMethodDef* __methods__() {
			return NULL;
		}

#if PY_VERSION_HEX >= 0x03090000
		static inline const PyModuleDef_Slot* __slots__() {
			static const PyModuleDef_Slot slots[] = {
				{Py_mod_exec, (void*) Self::__exec__},
				{0, NULL}
			};
			return slots;
		}
#endif

		static inline int __init__(PyObject* module, Self* state) {
			return 0;
		}

		static inline int __exec__(PyObject* module) {
			assert(Self::Instance() == NULL);
			PyObject*& instance = Self::_Instance();
			instance = module;

			assert(Self::State() == NULL);
			Self*& state = Self::_State();
			state = State(module);

			Self::State()->__vars = new VarsList();
			try {
				return Self::__init__(module, Self::State());
			} catch (...) {
				return -1;
			}
		}

		static inline int __traverse__(PyObject* module, visitproc visit, void *arg) {
			Self* state = State(module);
			for (auto c : *state->__vars) {
				Py_VISIT(c->Ref());
			}
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
