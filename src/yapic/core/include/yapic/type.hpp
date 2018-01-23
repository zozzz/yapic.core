#ifndef BF21EC34_7133_EC86_106C_D646EB842063
#define BF21EC34_7133_EC86_106C_D646EB842063


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
	static inline auto Optional ## __name (...) -> __type { return __default; } \
	template<class T> \
	static inline auto Call ## __name (decltype(&T::__name)*) -> __type { return T::__name(); } \
	template<class T> \
	static inline auto Call ## __name (...) -> auto { return __default; }


#define Yapic_HasMethod(__cls, __method) \
	(Has ## __method <__cls> :: Value)

#define Yapic_GetMethod(__cls, __method) \
	(Optional ## __method <__cls> (NULL))

#define Yapic_CallOptionalMethod(__cls, __method) \
	(Call ## __method <__cls> (NULL))

namespace Yapic {

	// internal
	typedef Py_ssize_t (*flagsfunc)();
	Yapic_MethodChecker(Flags, flagsfunc, Py_TPFLAGS_DEFAULT);
	typedef PyMethodDef* (*methodsfunc)();
	Yapic_MethodChecker(Methods, methodsfunc, NULL);
	typedef PyMemberDef* (*membersfunc)();
	Yapic_MethodChecker(Members, membersfunc, NULL);

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

	// mapping & sequence protocol
	Yapic_MethodChecker(__len__, lenfunc, NULL);
	Yapic_MethodChecker(__getitem__, binaryfunc, NULL);
	Yapic_MethodChecker(__setitem__, objobjargproc, NULL);
	#define Yapic_TypeHasMappingMethods(__cls) (\
		Yapic_HasMethod(__cls, __len__) || \
		Yapic_HasMethod(__cls, __getitem__) || \
		Yapic_HasMethod(__cls, __setitem__))

	Yapic_MethodChecker(__concat__, binaryfunc, NULL);
	Yapic_MethodChecker(__repeat__, ssizeargfunc, NULL);
	Yapic_MethodChecker(__sq_getitem__, ssizeargfunc, NULL);
	Yapic_MethodChecker(__sq_setitem__, ssizeobjargproc, NULL);
	Yapic_MethodChecker(__contains__, objobjproc, NULL);
	Yapic_MethodChecker(__inplace_concat__, binaryfunc, NULL);
	Yapic_MethodChecker(__inplace_repeat__, ssizeargfunc, NULL);
	#define Yapic_TypeHasSequenceMethods(__cls) (\
		Yapic_HasMethod(__cls, __concat__) || \
		Yapic_HasMethod(__cls, __repeat__) || \
		Yapic_HasMethod(__cls, __sq_getitem__) || \
		Yapic_HasMethod(__cls, __sq_setitem__) || \
		Yapic_HasMethod(__cls, __contains__) || \
		Yapic_HasMethod(__cls, __inplace_concat__) || \
		Yapic_HasMethod(__cls, __inplace_repeat__))


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
		abi::__cxa_demangle(name, _name, &len, &status);
		if (status != 0) {
			PyMem_Free(_name);
			_name = NULL;
		}
	}

#endif



	template<typename T>
	class TypeAllocator {
		public:
			inline T* Alloc(PyTypeObject* type) {
				assert(type != NULL);
				assert((type->tp_flags & Py_TPFLAGS_READY) == Py_TPFLAGS_READY);
				return (T*) type->tp_alloc(type, type->tp_basicsize);
			}

			inline void Dealloc(void* obj) {
				assert(obj != NULL);
				assert(Py_TYPE(obj)->tp_free != NULL);
				Py_TYPE(obj)->tp_free(obj);
			}
	};


	template<typename T, int size>
	class FreeList {
		public:
			using TA = TypeAllocator<T>;

			inline T* Alloc(const PyTypeObject* type) {
				if (this->free_space > 0) {
					T* res = this->instances[--this->free_space];
					PyObject_INIT((PyObject*) res, type);
					return (T*) res;
				}
				return TA::Alloc(type);
			}

			inline void Dealloc(void* obj) {
				if (this->free_space < size) {
					this->instances[this->free_space++] = (T*) obj;
				} else {
					TA::Dealloc(obj);
				}
			}

		protected:
			T instances[size];
			int free_space = 0;
	};


	template<typename Impl, typename Base, typename Allocator=TypeAllocator<Impl>>
	class Type: public Allocator {
	public:
		using Allocator::Alloc;
		using Allocator::Dealloc;

		Type() : Type(NULL) { }

		Type(Base base) : base(base) {
			this->def = {
				PyVarObject_HEAD_INIT(NULL, 0)
				/* tp_name */ 			this->Name(),
				/* tp_basicsize */ 		sizeof(Impl),
				/* tp_itemsize */ 		0,
				/* tp_dealloc */ 		(destructor) Yapic_GetMethod(Impl, __dealloc__),
				/* tp_print */ 			NULL,
				/* tp_getattr */ 		Yapic_GetMethod(Impl, __getattr__),
				/* tp_setattr */ 		Yapic_GetMethod(Impl, __setattr__),
				/* tp_as_async */ 		NULL,
				/* tp_repr */ 			Yapic_GetMethod(Impl, __repr__),
				/* tp_as_number */ 		NULL, // Yapic_TypeHasNumberMethods(Impl) ? Type<Impl>::NumberMethods() : NULL,
				/* tp_as_sequence */ 	NULL, // Yapic_TypeHasSequenceMethods(Impl) ? Type<Impl>::SequenceMethods() : NULL,
				/* tp_as_mapping */ 	NULL, // Yapic_TypeHasMappingMethods(Impl) ? Type<Impl>::MappingMethods() : NULL,
				/* tp_hash  */ 			Yapic_GetMethod(Impl, __hash__),
				/* tp_call */ 			Yapic_GetMethod(Impl, __call__),
				/* tp_str */ 			Yapic_GetMethod(Impl, __str__),
				/* tp_getattro */ 		Yapic_GetMethod(Impl, __getattro__),
				/* tp_setattro */ 		Yapic_GetMethod(Impl, __setattro__),
				/* tp_as_buffer */ 		NULL,
				/* tp_flags */ 			Yapic_CallOptionalMethod(Impl, Flags),
				/* tp_doc */ 			NULL,
				/* tp_traverse */ 		Yapic_GetMethod(Impl, __traverse__),
				/* tp_clear */ 			Yapic_GetMethod(Impl, __clear__),
				/* tp_richcompare */ 	Yapic_GetMethod(Impl, __cmp__),
				/* tp_weaklistoffset */ NULL,
				/* tp_iter */ 			Yapic_GetMethod(Impl, __iter__),
				/* tp_iternext */ 		Yapic_GetMethod(Impl, __next__),
				/* tp_methods */ 		Yapic_CallOptionalMethod(Impl, Methods),
				/* tp_members */ 		Yapic_CallOptionalMethod(Impl, Members),
				/* tp_getset */ 		NULL,
				/* tp_base */ 			NULL,
				/* tp_dict */ 			NULL,
				/* tp_descr_get */ 		Yapic_GetMethod(Impl, __get__),
				/* tp_descr_set */ 		Yapic_GetMethod(Impl, __set__),
				/* tp_dictoffset */ 	NULL,
				/* tp_init */ 			Yapic_GetMethod(Impl, __init__),
				/* tp_alloc */ 			Yapic_GetMethod(Impl, __alloc__),
				/* tp_new */ 			Yapic_GetMethod(Impl, __new__)
			};
		}

		// ~Type() {

		// }

		inline Impl* Alloc() { return this->Alloc(this->Definition()); }
		inline const PyTypeObject* Definition() { return &this->def; }
		inline const char* Name() const { return this->name.Value(); }

	protected:
		PyTypeObject def;
		Base base;
		ClassBaseName<Impl> name;

		static inline PyMappingMethods MappingMethods() {
			return {
				Yapic_GetMethod(Impl, __len__),
				Yapic_GetMethod(Impl, __getitem__),
				Yapic_GetMethod(Impl, __setitem__)
			};
		}

		static inline PySequenceMethods SequenceMethods() {
			return {
				Yapic_GetMethod(Impl, __len__),
				Yapic_GetMethod(Impl, __concat__),
				Yapic_GetMethod(Impl, __repeat__),
				Yapic_GetMethod(Impl, __sq_getitem__),
				NULL, // TODO: was_sq_slice ???
				Yapic_GetMethod(Impl, __sq_setitem__),
				NULL, // TODO: was_sq_ass_slice ???
				Yapic_GetMethod(Impl, __contains__),
				Yapic_GetMethod(Impl, __inplace_concat__),
				Yapic_GetMethod(Impl, __inplace_repeat__)
			};
		}

		static inline PyNumberMethods NumberMethods() {
			return {
				Yapic_GetMethod(Impl, __add__),
				Yapic_GetMethod(Impl, __sub__),
				Yapic_GetMethod(Impl, __mul__),
				Yapic_GetMethod(Impl, __mod__),
				Yapic_GetMethod(Impl, __divmod__),
				Yapic_GetMethod(Impl, __pow__),
				Yapic_GetMethod(Impl, __neg__),
				Yapic_GetMethod(Impl, __pos__),
				Yapic_GetMethod(Impl, __abs__),
				Yapic_GetMethod(Impl, __bool__),
				Yapic_GetMethod(Impl, __invert__),
				Yapic_GetMethod(Impl, __lshift__),
				Yapic_GetMethod(Impl, __rshift__),
				Yapic_GetMethod(Impl, __and__),
				Yapic_GetMethod(Impl, __xor__),
				Yapic_GetMethod(Impl, __or__),
				Yapic_GetMethod(Impl, __int__),
				NULL, // TODO: maybe reverese operation?
				Yapic_GetMethod(Impl, __float__),
				Yapic_GetMethod(Impl, __iadd__),
				Yapic_GetMethod(Impl, __isub__),
				Yapic_GetMethod(Impl, __imul__),
				Yapic_GetMethod(Impl, __imod__),
				Yapic_GetMethod(Impl, __ipow__),
				Yapic_GetMethod(Impl, __ilshift__),
				Yapic_GetMethod(Impl, __irshift__),
				Yapic_GetMethod(Impl, __iand__),
				Yapic_GetMethod(Impl, __ixor__),
				Yapic_GetMethod(Impl, __ior__),
				Yapic_GetMethod(Impl, __floordiv__),
				Yapic_GetMethod(Impl, __truediv__),
				Yapic_GetMethod(Impl, __ifloordiv__),
				Yapic_GetMethod(Impl, __itruediv__),
				Yapic_GetMethod(Impl, __index__),
				Yapic_GetMethod(Impl, __matmul__),
				Yapic_GetMethod(Impl, __imatmul__)
			};
		}
	};



} /* end namespace Yapic */

#endif /* BF21EC34_7133_EC86_106C_D646EB842063 */
