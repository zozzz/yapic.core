#ifndef BF21EC34_7133_EC86_106C_D646EB842063
#define BF21EC34_7133_EC86_106C_D646EB842063

#ifdef __GNUG__
#		include <cxxabi.h>
#		include <execinfo.h>
#endif

#include "./methods.hpp"


#define Yapic_CallOptionalMethod(__cls, __method) \
	(Call ## __method <__cls> (NULL))

namespace Yapic {

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
#else
#	ifdef __GNUG__

		template<typename T>
		void ClassBaseName<T>::Determine() {
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

#	endif
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


	/**
	 * Usage:
	 *
	 * class Shape: public Yapic::Object {
	 * public:
	 *     static const Yapic::Type<Shape> Type("This is shape!!!");
	 * };
	 *
	 *
	 * class Circle: public Shape {
	 * public:
	 *     static const Yapic::Type<Circle> Type(Shape::Type, "This is circle!!!");
	 * };
	 */
	template<typename Impl, typename Base, typename Allocator=TypeAllocator<Impl>>
	class Type: public Allocator {
	public:
		using Allocator::Alloc;
		using Allocator::Dealloc;
		using Self = Type<Impl, Base, Allocator>;

		Type(const char* doc = NULL) {
			// PyVarObject_HEAD_INIT(NULL, 0)

			this->def.tp_name = this->Name();
			this->def.tp_basicsize = sizeof(Impl);
			this->def.tp_itemsize = 0;
			this->def.tp_dealloc = Yapic_GetMethod(Impl, __dealloc__, NULL);
			// this->def.tp_name = this->Name();
			// this->def.tp_name = this->Name();
			// this->def.tp_name = this->Name();
			// this->def.tp_name = this->Name();
			// this->def.tp_name = this->Name();
			// this->def.tp_name = this->Name();
			// this->def.tp_name = this->Name();
			// this->def.tp_name = this->Name();
			// this->def.tp_name = this->Name();
			// this->def.tp_name = this->Name();
			// this->def.tp_name = this->Name();
			// this->def.tp_name = this->Name();
			// this->def.tp_name = this->Name();
			// this->def.tp_name = this->Name();
			// this->def.tp_name = this->Name();
			// this->def = {
			// 	PyVarObject_HEAD_INIT(NULL, 0)
			// 	/* tp_name */ 			this->Name(),
			// 	/* tp_basicsize */ 		sizeof(Impl),
			// 	/* tp_itemsize */ 		0,
			// 	/* tp_dealloc */ 		(destructor) Yapic_GetMethod(Impl, __dealloc__, NULL),
			// 	/* tp_print */ 			NULL,
			// 	/* tp_getattr */ 		Yapic_GetMethod(Impl, __getattr__, NULL),
			// 	/* tp_setattr */ 		Yapic_GetMethod(Impl, __setattr__, NULL),
			// 	/* tp_as_async */ 		NULL,
			// 	/* tp_repr */ 			Yapic_GetMethod(Impl, __repr__, NULL),
			// 	/* tp_as_number */ 		Yapic_TypeHasNumberMethods(Impl) ? Self::NumberMethods() : NULL,
			// 	/* tp_as_sequence */ 	Yapic_TypeHasSequenceMethods(Impl) ? Self::SequenceMethods() : NULL,
			// 	/* tp_as_mapping */ 	Yapic_TypeHasMappingMethods(Impl) ? Self::MappingMethods() : NULL,
			// 	/* tp_hash  */ 			Yapic_GetMethod(Impl, __hash__, NULL),
			// 	/* tp_call */ 			Yapic_GetMethod(Impl, __call__, NULL),
			// 	/* tp_str */ 			Yapic_GetMethod(Impl, __str__, NULL),
			// 	/* tp_getattro */ 		Yapic_GetMethod(Impl, __getattro__, NULL),
			// 	/* tp_setattro */ 		Yapic_GetMethod(Impl, __setattro__, NULL),
			// 	/* tp_as_buffer */ 		NULL,
			// 	/* tp_flags */ 			Yapic_CallMethod(Impl, Flags, Py_TPFLAGS_DEFAULT),
			// 	/* tp_doc */ 			doc,
			// 	/* tp_traverse */ 		Yapic_GetMethod(Impl, __traverse__, NULL),
			// 	/* tp_clear */ 			Yapic_GetMethod(Impl, __clear__, NULL),
			// 	/* tp_richcompare */ 	Yapic_GetMethod(Impl, __cmp__, NULL),
			// 	/* tp_weaklistoffset */ NULL,
			// 	/* tp_iter */ 			Yapic_GetMethod(Impl, __iter__, NULL),
			// 	/* tp_iternext */ 		Yapic_GetMethod(Impl, __next__, NULL),
			// 	/* tp_methods */ 		Yapic_CallMethod(Impl, Methods, NULL),
			// 	/* tp_members */ 		Yapic_CallMethod(Impl, Members, NULL),
			// 	/* tp_getset */ 		Yapic_CallMethod(Impl, Properties, NULL),
			// 	/* tp_base */ 			NULL,
			// 	/* tp_dict */ 			NULL,
			// 	/* tp_descr_get */ 		Yapic_GetMethod(Impl, __get__, NULL),
			// 	/* tp_descr_set */ 		Yapic_GetMethod(Impl, __set__, NULL),
			// 	/* tp_dictoffset */ 	NULL,
			// 	/* tp_init */ 			Yapic_GetMethod(Impl, __init__, NULL),
			// 	/* tp_alloc */ 			Yapic_GetMethod(Impl, __alloc__, NULL),
			// 	/* tp_new */ 			Yapic_GetMethod(Impl, __new__, PyType_GenericNew)
			// };
		}

		// ~Type() {

		// }

		inline Impl* Alloc() { return this->Alloc(this->Definition()); }
		inline const PyTypeObject* Definition() { return &this->def; }
		inline operator PyTypeObject*() const { return &this->def; }
		inline const char* Name() const { return this->name.Value(); }

	protected:
		PyTypeObject def;
		ClassBaseName<Impl> name;

		static inline PyMappingMethods* MappingMethods() {
			static PyMappingMethods methods = {
				Yapic_GetMethod(Impl, __len__, NULL),
				Yapic_GetMethod(Impl, __getitem__, NULL),
				Yapic_GetMethod(Impl, __setitem__, NULL)
			};
			return &methods;
		}

		static inline PySequenceMethods* SequenceMethods() {
			static PySequenceMethods methods = {
				Yapic_GetMethod(Impl, __len__, NULL),
				Yapic_GetMethod(Impl, __concat__, NULL),
				Yapic_GetMethod(Impl, __repeat__, NULL),
				Yapic_GetMethod(Impl, __sq_getitem__, NULL),
				NULL, // TODO: was_sq_slice ???
				Yapic_GetMethod(Impl, __sq_setitem__, NULL),
				NULL, // TODO: was_sq_ass_slice ???
				Yapic_GetMethod(Impl, __contains__, NULL),
				Yapic_GetMethod(Impl, __inplace_concat__, NULL),
				Yapic_GetMethod(Impl, __inplace_repeat__, NULL)
			};
			return &methods;
		}

		static inline PyNumberMethods* NumberMethods() {
			static PyNumberMethods methods = {
				Yapic_GetMethod(Impl, __add__, NULL),
				Yapic_GetMethod(Impl, __sub__, NULL),
				Yapic_GetMethod(Impl, __mul__, NULL),
				Yapic_GetMethod(Impl, __mod__, NULL),
				Yapic_GetMethod(Impl, __divmod__, NULL),
				Yapic_GetMethod(Impl, __pow__, NULL),
				Yapic_GetMethod(Impl, __neg__, NULL),
				Yapic_GetMethod(Impl, __pos__, NULL),
				Yapic_GetMethod(Impl, __abs__, NULL),
				Yapic_GetMethod(Impl, __bool__, NULL),
				Yapic_GetMethod(Impl, __invert__, NULL),
				Yapic_GetMethod(Impl, __lshift__, NULL),
				Yapic_GetMethod(Impl, __rshift__, NULL),
				Yapic_GetMethod(Impl, __and__, NULL),
				Yapic_GetMethod(Impl, __xor__, NULL),
				Yapic_GetMethod(Impl, __or__, NULL),
				Yapic_GetMethod(Impl, __int__, NULL),
				NULL, // TODO: maybe reverese operation?
				Yapic_GetMethod(Impl, __float__, NULL),
				Yapic_GetMethod(Impl, __iadd__, NULL),
				Yapic_GetMethod(Impl, __isub__, NULL),
				Yapic_GetMethod(Impl, __imul__, NULL),
				Yapic_GetMethod(Impl, __imod__, NULL),
				Yapic_GetMethod(Impl, __ipow__, NULL),
				Yapic_GetMethod(Impl, __ilshift__, NULL),
				Yapic_GetMethod(Impl, __irshift__, NULL),
				Yapic_GetMethod(Impl, __iand__, NULL),
				Yapic_GetMethod(Impl, __ixor__, NULL),
				Yapic_GetMethod(Impl, __ior__, NULL),
				Yapic_GetMethod(Impl, __floordiv__, NULL),
				Yapic_GetMethod(Impl, __truediv__, NULL),
				Yapic_GetMethod(Impl, __ifloordiv__, NULL),
				Yapic_GetMethod(Impl, __itruediv__, NULL),
				Yapic_GetMethod(Impl, __index__, NULL),
				Yapic_GetMethod(Impl, __matmul__, NULL),
				Yapic_GetMethod(Impl, __imatmul__, NULL)
			};
			return &methods;
		}
	};


	// template<typename >
	class _BuiltinType {
	public:
		inline const PyTypeObject* Definition() {

		}
	};


	class BuiltinObject {
	public:
		using YapicType = _BuiltinType;

	};

	#define YapicType_HEAD(...) \
		public: \
			using YapicType = Yapic::Type<__VA_ARGS__>; \
			static const YapicType Type;

	#define YapicType_DECL(__cls, __doc) \
		const __cls::YapicType __cls::Type(__doc);


	// template<class Self, class Super=BuiltinObject>
	class Object: public PyObject {
	public:

		// using YapicType = Type<Self, Super>;
		// static const YapicType Type;

		// static void __dealloc__(PyObject* self) {
		// 	Self::Type.Dealloc(self);
		// }
	};


} /* end namespace Yapic */

#endif /* BF21EC34_7133_EC86_106C_D646EB842063 */
