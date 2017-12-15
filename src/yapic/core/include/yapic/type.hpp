#ifndef Y85B3FE5_F133_C8FE_FA27_71A7C3CADE93
#define Y85B3FE5_F133_C8FE_FA27_71A7C3CADE93

#include <type_traits>
#include <typeinfo>
#include <Python.h>

// XXX: decltype helyett esetleg typeof kell GCC-nél
// TODO: kicsit típus biztosabbá tenni a függvény lekérdezéseket
#define Yapic_MethodChecker(__name, __type, __default) \
	template<typename T> \
	struct Has ## __name { \
		typedef char yes[1]; \
		typedef char no[2]; \
		template <typename C> static yes& Test(decltype(&C::__name)); \
		template <typename C> static  no& Test(...); \
		static bool constexpr Value = sizeof(Test<T>(0)) == sizeof(yes); \
	}; \
	template<class T> \
	static inline auto Optional ## __name (decltype(&T::__name)*) -> __type { return reinterpret_cast<__type>(&T::__name); } \
	template<class T> \
	static inline auto Optional ## __name (...) -> __type { return __default; }

#define Yapic_HasMethod(__cls, __method) \
	(Has ## __method <__cls> :: Value)

#define Yapic_GetTypeMethod(__cls, __method) \
	(Optional ## __method <__cls> (NULL))


#define Yapic_PrivateNew \
	static PyObject* __new__(PyTypeObject *type, PyObject *args, PyObject *kwargs) { \
		PyErr_Format(PyExc_NotImplementedError, "Cann not instantiate %s class from python.", type->tp_name); \
		return NULL; \
	}


template<typename T>
class ClassBaseName {
	public:
		ClassBaseName(): _name(NULL) {
			Determine();
			if (_name != NULL) {
				Reduce();
			}
		}
		~ClassBaseName() {
			if (_name != NULL) {
				PyMem_Free(_name);
			}
		}

		inline const char* Value() const { return _name; }

	protected:
		void Determine();

		void Reduce() {
			size_t len = strlen(_name);
			size_t nameLen = 0;

			for (size_t i = len ; i>= 0 ; --i) {
				if (_name[i] == ':' || _name[i] == ' ') {
					--nameLen;
					break;
				} else {
					++nameLen;
				}
			}

			memmove(_name, _name + (len - nameLen), nameLen * sizeof(char));
			_name = (char*) PyMem_Realloc(_name, (nameLen + 1) * sizeof(char));
			_name[nameLen] = '\0';
		}
	private:
		char* _name;
};

#ifdef _MSC_VER
	template<typename T>
	void ClassBaseName<T>::Determine() {
		const char* name = typeid(T).name();
		size_t len = strlen(name);
		_name = (char*) PyMem_Malloc((len + 1) * sizeof(char));
		memcpy(_name, name, len);
		_name[len] = '\0';
	}
#endif

#ifdef __GNUG__
#	include <cxxabi.h>
#	include <execinfo.h>

	template<typename T>
	void ClassBaseName::Determine() {
		int status;
		const char* name = typeid(T).name();
		size_t len = strlen(name);
		_name = (char*) PyMem_Malloc((len + 1) * sizeof(char));
		abi::__cxa_demangle(c, _name, len + 1, &status);
		if (status != 0) {
			PyMem_Free(_name);
			_name = NULL;
		}
	}

#endif






namespace Yapic {

	// basic
	Yapic_MethodChecker(__dealloc__, destructor, NULL);
	Yapic_MethodChecker(__getattr__, getattrfunc, NULL);
	Yapic_MethodChecker(__setattr__, setattrfunc, NULL);
	Yapic_MethodChecker(__repr__, reprfunc, NULL);
	Yapic_MethodChecker(__hash__, hashfunc, NULL);
	Yapic_MethodChecker(__call__, ternaryfunc, NULL);
	Yapic_MethodChecker(__str__, reprfunc, NULL);
	Yapic_MethodChecker(__getattro__, getattrofunc, NULL);
	Yapic_MethodChecker(__setattro__, setattrofunc, NULL);
	Yapic_MethodChecker(__traverse__, traverseproc, NULL);
	Yapic_MethodChecker(__clear__, inquiry, NULL);
	Yapic_MethodChecker(__cmp__, richcmpfunc, NULL);
	Yapic_MethodChecker(__iter__, getiterfunc, NULL);
	Yapic_MethodChecker(__next__, iternextfunc, NULL);
	Yapic_MethodChecker(__get__, descrgetfunc, NULL);
	Yapic_MethodChecker(__set__, descrsetfunc, NULL);
	Yapic_MethodChecker(__init__, initproc, NULL);
	Yapic_MethodChecker(__alloc__, allocfunc, NULL);
	Yapic_MethodChecker(__new__, newfunc, PyType_GenericNew);
	Yapic_MethodChecker(__free__, freefunc, NULL);

	// async protocol
	// number protocol
	Yapic_MethodChecker(__add__, binaryfunc, NULL);
	Yapic_MethodChecker(__sub__, binaryfunc, NULL);
	Yapic_MethodChecker(__mul__, binaryfunc, NULL);
	Yapic_MethodChecker(__mod__, binaryfunc, NULL);
	Yapic_MethodChecker(__divmod__, binaryfunc, NULL);
	Yapic_MethodChecker(__pow__, ternaryfunc, NULL);
	Yapic_MethodChecker(__neg__, unaryfunc, NULL);
	Yapic_MethodChecker(__pos__, unaryfunc, NULL);
	Yapic_MethodChecker(__abs__, unaryfunc, NULL);
	Yapic_MethodChecker(__bool__, inquiry, NULL);
	Yapic_MethodChecker(__invert__, unaryfunc, NULL);
	Yapic_MethodChecker(__lshift__, binaryfunc, NULL);
	Yapic_MethodChecker(__rshift__, binaryfunc, NULL);
	Yapic_MethodChecker(__and__, binaryfunc, NULL);
	Yapic_MethodChecker(__xor__, binaryfunc, NULL);
	Yapic_MethodChecker(__or__, binaryfunc, NULL);
	Yapic_MethodChecker(__int__, unaryfunc, NULL);
	Yapic_MethodChecker(__float__, unaryfunc, NULL);
	Yapic_MethodChecker(__iadd__, binaryfunc, NULL);
	Yapic_MethodChecker(__isub__, binaryfunc, NULL);
	Yapic_MethodChecker(__imul__, binaryfunc, NULL);
	Yapic_MethodChecker(__imod__, binaryfunc, NULL);
	Yapic_MethodChecker(__ipow__, ternaryfunc, NULL);
	Yapic_MethodChecker(__ilshift__, binaryfunc, NULL);
	Yapic_MethodChecker(__irshift__, binaryfunc, NULL);
	Yapic_MethodChecker(__iand__, binaryfunc, NULL);
	Yapic_MethodChecker(__ixor__, binaryfunc, NULL);
	Yapic_MethodChecker(__ior__, binaryfunc, NULL);
	Yapic_MethodChecker(__floordiv__, binaryfunc, NULL);
	Yapic_MethodChecker(__truediv__, binaryfunc, NULL);
	Yapic_MethodChecker(__ifloordiv__, binaryfunc, NULL);
	Yapic_MethodChecker(__itruediv__, binaryfunc, NULL);
	Yapic_MethodChecker(__index__, unaryfunc, NULL);
	Yapic_MethodChecker(__matmul__, binaryfunc, NULL);
	Yapic_MethodChecker(__imatmul__, binaryfunc, NULL);
	#define Yapic_TypeHasNumberMethods(__cls) (\
		Yapic_HasMethod(__cls, __add__) || \
		Yapic_HasMethod(__cls, __sub__) || \
		Yapic_HasMethod(__cls, __mul__) || \
		Yapic_HasMethod(__cls, __mod__) || \
		Yapic_HasMethod(__cls, __divmod__) || \
		Yapic_HasMethod(__cls, __pow__) || \
		Yapic_HasMethod(__cls, __neg__) || \
		Yapic_HasMethod(__cls, __pos__) || \
		Yapic_HasMethod(__cls, __abs__) || \
		Yapic_HasMethod(__cls, __bool__) || \
		Yapic_HasMethod(__cls, __invert__) || \
		Yapic_HasMethod(__cls, __lshift__) || \
		Yapic_HasMethod(__cls, __rshift__) || \
		Yapic_HasMethod(__cls, __and__) || \
		Yapic_HasMethod(__cls, __xor__) || \
		Yapic_HasMethod(__cls, __or__) || \
		Yapic_HasMethod(__cls, __int__) || \
		Yapic_HasMethod(__cls, __float__) || \
		Yapic_HasMethod(__cls, __iadd__) || \
		Yapic_HasMethod(__cls, __isub__) || \
		Yapic_HasMethod(__cls, __imul__) || \
		Yapic_HasMethod(__cls, __imod__) || \
		Yapic_HasMethod(__cls, __ipow__) || \
		Yapic_HasMethod(__cls, __ilshift__) || \
		Yapic_HasMethod(__cls, __irshift__) || \
		Yapic_HasMethod(__cls, __iand__) || \
		Yapic_HasMethod(__cls, __ixor__) || \
		Yapic_HasMethod(__cls, __ior__) || \
		Yapic_HasMethod(__cls, __floordiv__) || \
		Yapic_HasMethod(__cls, __truediv__) || \
		Yapic_HasMethod(__cls, __ifloordiv__) || \
		Yapic_HasMethod(__cls, __itruediv__) || \
		Yapic_HasMethod(__cls, __index__) || \
		Yapic_HasMethod(__cls, __matmul__) || \
		Yapic_HasMethod(__cls, __imatmul__))

	// sequence protocol
	Yapic_MethodChecker(__sq_len__, lenfunc, NULL);
	Yapic_MethodChecker(__sq_concat__, binaryfunc, NULL);
	Yapic_MethodChecker(__sq_repeat__, ssizeargfunc, NULL);
	Yapic_MethodChecker(__sq_getitem__, ssizeargfunc, NULL);
	Yapic_MethodChecker(__sq_setitem__, ssizeobjargproc, NULL);
	Yapic_MethodChecker(__sq_contains__, objobjproc, NULL);
	Yapic_MethodChecker(__sq_inplace_concat__, binaryfunc, NULL);
	Yapic_MethodChecker(__sq_inplace_repeat__, ssizeargfunc, NULL);
	#define Yapic_TypeHasSequenceMethods(__cls) (\
		Yapic_HasMethod(__cls, __sq_len__) || \
		Yapic_HasMethod(__cls, __sq_concat__) || \
		Yapic_HasMethod(__cls, __sq_repeat__) || \
		Yapic_HasMethod(__cls, __sq_getitem__) || \
		Yapic_HasMethod(__cls, __sq_setitem__) || \
		Yapic_HasMethod(__cls, __sq_contains__) || \
		Yapic_HasMethod(__cls, __sq_inplace_concat__) || \
		Yapic_HasMethod(__cls, __sq_inplace_repeat__))

	// mapping protocol
	Yapic_MethodChecker(__mp_len__, lenfunc, NULL);
	Yapic_MethodChecker(__mp_getitem__, binaryfunc, NULL);
	Yapic_MethodChecker(__mp_setitem__, objobjargproc, NULL);
	#define Yapic_TypeHasMappingMethods(__cls) (\
		Yapic_HasMethod(__cls, __mp_len__) || \
		Yapic_HasMethod(__cls, __mp_getitem__) || \
		Yapic_HasMethod(__cls, __mp_setitem__))

	// buffer protocol


	template<typename _self, typename _super>
	class Type: public _super {
		public:
			using Self = _self;
			using Super = _super;

			static inline Self* Alloc() {
				return Alloc(const_cast<PyTypeObject*>(Self::PyType()));
			}

			static inline Self* Alloc(PyTypeObject* type) {
				assert(type != NULL);
				assert((type->tp_flags & Py_TPFLAGS_READY) == Py_TPFLAGS_READY);
				return (Self*) type->tp_alloc(type, sizeof(Self));
			}

			static inline bool Check(void* o) {
				assert(o != NULL);
				return PyObject_TypeCheck(o, const_cast<PyTypeObject*>(Self::PyType()));
			}

			static inline bool CheckExact(void* o) {
				assert(o != NULL);
				return Py_TYPE(o) == Self::PyType();
			}

			static inline bool Register(PyObject* module) {
				return Self::Register(module, Self::Name());
			}

			static inline bool Register(PyObject* module, const char* name) {
				PyTypeObject* type = const_cast<PyTypeObject*>(Self::PyType());
				type->tp_base = const_cast<PyTypeObject*>(Self::_BaseType());
				if (PyType_Ready(type) < 0) {
					return false;
				}
				Py_INCREF(type);
				if (PyModule_AddObject(module, name, (PyObject*) type) < 0) {
					Py_DECREF(type);
					return false;
				}
				return true;
			}

			static inline const char* Name() {
				static const ClassBaseName<Self> name;
				return name.Value();
			}

			static inline void __dealloc__(void* self) {
				Py_TYPE(self)->tp_free((PyObject*) self);
			}

			static inline PyMethodDef* __methods__() {
				return NULL;
			}

			static inline unsigned long Flags() {
				return Py_TPFLAGS_DEFAULT;
			}

			static inline const PyTypeObject* _BaseType() {
				const PyTypeObject* type = Super::PyType();
				assert(type != NULL);
				if (type->tp_flags & Py_TPFLAGS_READY) {
					return type;
				} else {
					return Super::PyType();
				}
			}

			static inline const PyTypeObject* PyType() {
				static const PyTypeObject type = {
					PyVarObject_HEAD_INIT(NULL, 0)
					/* tp_name */ 			Self::Name(),
					/* tp_basicsize */ 		sizeof(Self),
					/* tp_itemsize */ 		static_cast<Py_ssize_t>(0),
					/* tp_dealloc */ 		(destructor) Yapic_GetTypeMethod(Self, __dealloc__),
					/* tp_print */ 			NULL,
					/* tp_getattr */ 		Yapic_GetTypeMethod(Self, __getattr__),
					/* tp_setattr */ 		Yapic_GetTypeMethod(Self, __setattr__),
					/* tp_as_async */ 		NULL,
					/* tp_repr */ 			Yapic_GetTypeMethod(Self, __repr__),
					/* tp_as_number */ 		Yapic_TypeHasNumberMethods(Self) ? Self::_NumberMethods() : NULL,
					/* tp_as_sequence */ 	Yapic_TypeHasSequenceMethods(Self) ? Self::_SequenceMethods() : NULL,
					/* tp_as_mapping */ 	Yapic_TypeHasMappingMethods(Self) ? Self::_MappingMethods() : NULL,
					/* tp_hash  */ 			Yapic_GetTypeMethod(Self, __hash__),
					/* tp_call */ 			Yapic_GetTypeMethod(Self, __call__),
					/* tp_str */ 			Yapic_GetTypeMethod(Self, __str__),
					/* tp_getattro */ 		Yapic_GetTypeMethod(Self, __getattro__),
					/* tp_setattro */ 		Yapic_GetTypeMethod(Self, __setattro__),
					/* tp_as_buffer */ 		NULL,
					/* tp_flags */ 			Self::Flags(),
					/* tp_doc */ 			NULL,
					/* tp_traverse */ 		Yapic_GetTypeMethod(Self, __traverse__),
					/* tp_clear */ 			Yapic_GetTypeMethod(Self, __clear__),
					/* tp_richcompare */ 	Yapic_GetTypeMethod(Self, __cmp__),
					/* tp_weaklistoffset */ NULL,
					/* tp_iter */ 			Yapic_GetTypeMethod(Self, __iter__),
					/* tp_iternext */ 		Yapic_GetTypeMethod(Self, __next__),
					/* tp_methods */ 		const_cast<PyMethodDef*>(Self::__methods__()),
					/* tp_members */ 		NULL,
					/* tp_getset */ 		NULL,
					/* tp_base */ 			NULL,
					/* tp_dict */ 			NULL,
					/* tp_descr_get */ 		Yapic_GetTypeMethod(Self, __get__),
					/* tp_descr_set */ 		Yapic_GetTypeMethod(Self, __set__),
					/* tp_dictoffset */ 	NULL,
					/* tp_init */ 			Yapic_GetTypeMethod(Self, __init__),
					/* tp_alloc */ 			Yapic_GetTypeMethod(Self, __alloc__),
					/* tp_new */ 			Yapic_GetTypeMethod(Self, __new__)
				};
				return &type;
			}
		protected:
			static inline PyMappingMethods* _MappingMethods() {
				static PyMappingMethods methods = {
					Yapic_GetTypeMethod(Self, __mp_len__),
					Yapic_GetTypeMethod(Self, __mp_getitem__),
					Yapic_GetTypeMethod(Self, __mp_setitem__)
				};
				return &methods;
			}

			static inline PySequenceMethods* _SequenceMethods() {
				static PySequenceMethods methods = {
					Yapic_GetTypeMethod(Self, __sq_len__),
					Yapic_GetTypeMethod(Self, __sq_concat__),
					Yapic_GetTypeMethod(Self, __sq_repeat__),
					Yapic_GetTypeMethod(Self, __sq_getitem__),
					NULL, // TODO: was_sq_slice ???
					Yapic_GetTypeMethod(Self, __sq_setitem__),
					NULL, // TODO: was_sq_ass_slice ???
					Yapic_GetTypeMethod(Self, __sq_contains__),
					Yapic_GetTypeMethod(Self, __sq_inplace_concat__),
					Yapic_GetTypeMethod(Self, __sq_inplace_repeat__)
				};
				return &methods;
			}

			static inline PyNumberMethods* _NumberMethods() {
				static PyNumberMethods methods = {
					Yapic_GetTypeMethod(Self, __add__),
					Yapic_GetTypeMethod(Self, __sub__),
					Yapic_GetTypeMethod(Self, __mul__),
					Yapic_GetTypeMethod(Self, __mod__),
					Yapic_GetTypeMethod(Self, __divmod__),
					Yapic_GetTypeMethod(Self, __pow__),
					Yapic_GetTypeMethod(Self, __neg__),
					Yapic_GetTypeMethod(Self, __pos__),
					Yapic_GetTypeMethod(Self, __abs__),
					Yapic_GetTypeMethod(Self, __bool__),
					Yapic_GetTypeMethod(Self, __invert__),
					Yapic_GetTypeMethod(Self, __lshift__),
					Yapic_GetTypeMethod(Self, __rshift__),
					Yapic_GetTypeMethod(Self, __and__),
					Yapic_GetTypeMethod(Self, __xor__),
					Yapic_GetTypeMethod(Self, __or__),
					Yapic_GetTypeMethod(Self, __int__),
					NULL, // TODO: maybe reverese operation?
					Yapic_GetTypeMethod(Self, __float__),
					Yapic_GetTypeMethod(Self, __iadd__),
					Yapic_GetTypeMethod(Self, __isub__),
					Yapic_GetTypeMethod(Self, __imul__),
					Yapic_GetTypeMethod(Self, __imod__),
					Yapic_GetTypeMethod(Self, __ipow__),
					Yapic_GetTypeMethod(Self, __ilshift__),
					Yapic_GetTypeMethod(Self, __irshift__),
					Yapic_GetTypeMethod(Self, __iand__),
					Yapic_GetTypeMethod(Self, __ixor__),
					Yapic_GetTypeMethod(Self, __ior__),
					Yapic_GetTypeMethod(Self, __floordiv__),
					Yapic_GetTypeMethod(Self, __truediv__),
					Yapic_GetTypeMethod(Self, __ifloordiv__),
					Yapic_GetTypeMethod(Self, __itruediv__),
					Yapic_GetTypeMethod(Self, __index__),
					Yapic_GetTypeMethod(Self, __matmul__),
					Yapic_GetTypeMethod(Self, __imatmul__)
				};
				return &methods;
			}
	};

	class AbstractObject {};

	template<typename _traits>
	class BuiltinObject : public Type<BuiltinObject<_traits>, AbstractObject> {
		public:
			using Builtin = BuiltinObject<_traits>;

			static inline bool Register(PyObject* module) {
				if (std::is_same<Self, Builtin>::value) {
					assert(0);
					return false;
				} else {
					return Self::Register(PyObject* module);
				}
			}

			static inline const PyTypeObject* PyType() {
				if (std::is_same<Self, Builtin>::value) {
					return _traits::Type;
				} else {
					return Self::PyType();
				}
			}
		public:
			typename _traits::PyType ob_base;
	};

	// struct ListTrait {
	// 	static constexpr const char* Name = "list";
	// 	// ...
	// };

	// typedef BuiltinObject<ListTrait> List;

	struct ObjectTraits {
		static constexpr const PyTypeObject* Type = &PyBaseObject_Type;
		typedef PyObject PyType;
	};
	typedef BuiltinObject<ObjectTraits> Object;


	struct ListTraits {
		static constexpr const PyTypeObject* Type = &PyList_Type;
		typedef PyListObject PyType;
	};
	typedef BuiltinObject<ListTraits> List;






	//...........................................................//
	////////////////////////// TEST CODE //////////////////////////
	//˙˙˙˙˙˙˙˙˙˙˙˙˙˙˙˙˙˙˙˙˙˙˙˙˙˙˙˙˙˙˙˙˙˙˙˙˙˙˙˙˙˙˙˙˙˙˙˙˙˙˙˙˙˙˙˙˙˙˙//


	class MyType1 : public Type<MyType1, Object> {
		public:
			PyObject* test;

			static inline void New() {
				printf("%s::New\n", Self::Name());
			}
	};

	class MyType2 : public Type<MyType2, MyType1> {
		public:
			PyObject* test2;

			static inline void New() {
				printf("%s::New\n", Self::Name());
				Super::New();
			}
	};

	class MyList : public Type<MyList, List> {
	};

	struct Reference {
		PyObject_HEAD
		PyObject* test;
		PyObject* test2;
	};

} // end namespace Yapic
#endif /* Y85B3FE5_F133_C8FE_FA27_71A7C3CADE93 */
