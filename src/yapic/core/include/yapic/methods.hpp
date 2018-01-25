#ifndef A6C67645_3133_EC87_F988_69F9A065C3AB
#define A6C67645_3133_EC87_F988_69F9A065C3AB

#include <Python.h>

// XXX: decltype helyett esetleg typeof kell GCC-nél
// TODO: kicsit típus biztosabbá tenni a függvény lekérdezéseket
#define Yapic_MethodChecker(__name, __type) \
	template<typename T> \
	struct Has ## __name { \
		typedef char yes[1]; \
		typedef char no[2]; \
		template <typename C> static yes& Test(decltype(&C::__name)); \
		template <typename C> static  no& Test(...); \
		static bool constexpr Value = sizeof(Test<T>(0)) == sizeof(yes); \
	}; \
	template<class T> \
	static inline auto Get ## __name (decltype(&T::__name)*) -> __type { return reinterpret_cast<__type>(&T::__name); } \
	template<class T> \
	static inline auto Get ## __name (...) -> __type { return NULL; } \
	template<class T> \
	static inline auto Call ## __name (decltype(&T::__name)*) -> auto { return T::__name(); } \
	template<class T> \
	static inline auto Call ## __name (...) -> auto { return NULL; }

#define Yapic_HasMethod(__cls, __method) \
	(Yapic::_methods::Has ## __method <__cls> :: Value)

#define Yapic_GetMethod(__cls, __method, __default) \
	(Yapic_HasMethod(__cls, __method) ? Yapic::_methods::Get ## __method <__cls> (NULL) : __default)

#define Yapic_CallMethod(__cls, __method, __default) \
	(Yapic_HasMethod(__cls, __method) ? Yapic::_methods::Call ## __method <__cls> (NULL) : __default)


namespace Yapic { namespace _methods {
	// internal methods
	typedef Py_ssize_t (*flagsfunc)();
	Yapic_MethodChecker(Flags, flagsfunc);
	typedef PyMemberDef* (*membersfunc)();
	Yapic_MethodChecker(Members, membersfunc);
	typedef PyMethodDef* (*methodsfunc)();
	Yapic_MethodChecker(Methods, methodsfunc);
	typedef PyGetSetDef* (*propertiesfunc)();
	Yapic_MethodChecker(Properties, propertiesfunc);


	// basic
	Yapic_MethodChecker(__dealloc__, destructor);
	Yapic_MethodChecker(__getattr__, getattrfunc);
	Yapic_MethodChecker(__setattr__, setattrfunc);
	Yapic_MethodChecker(__repr__, reprfunc);
	Yapic_MethodChecker(__hash__, hashfunc);
	Yapic_MethodChecker(__call__, ternaryfunc);
	Yapic_MethodChecker(__str__, reprfunc);
	Yapic_MethodChecker(__getattro__, getattrofunc);
	Yapic_MethodChecker(__setattro__, setattrofunc);
	Yapic_MethodChecker(__traverse__, traverseproc);
	Yapic_MethodChecker(__clear__, inquiry);
	Yapic_MethodChecker(__cmp__, richcmpfunc);
	Yapic_MethodChecker(__iter__, getiterfunc);
	Yapic_MethodChecker(__next__, iternextfunc);
	Yapic_MethodChecker(__get__, descrgetfunc);
	Yapic_MethodChecker(__set__, descrsetfunc);
	Yapic_MethodChecker(__init__, initproc);
	Yapic_MethodChecker(__alloc__, allocfunc);
	Yapic_MethodChecker(__new__, newfunc);
	Yapic_MethodChecker(__free__, freefunc);

	// async protocol
	// number protocol
	Yapic_MethodChecker(__add__, binaryfunc);
	Yapic_MethodChecker(__sub__, binaryfunc);
	Yapic_MethodChecker(__mul__, binaryfunc);
	Yapic_MethodChecker(__mod__, binaryfunc);
	Yapic_MethodChecker(__divmod__, binaryfunc);
	Yapic_MethodChecker(__pow__, ternaryfunc);
	Yapic_MethodChecker(__neg__, unaryfunc);
	Yapic_MethodChecker(__pos__, unaryfunc);
	Yapic_MethodChecker(__abs__, unaryfunc);
	Yapic_MethodChecker(__bool__, inquiry);
	Yapic_MethodChecker(__invert__, unaryfunc);
	Yapic_MethodChecker(__lshift__, binaryfunc);
	Yapic_MethodChecker(__rshift__, binaryfunc);
	Yapic_MethodChecker(__and__, binaryfunc);
	Yapic_MethodChecker(__xor__, binaryfunc);
	Yapic_MethodChecker(__or__, binaryfunc);
	Yapic_MethodChecker(__int__, unaryfunc);
	Yapic_MethodChecker(__float__, unaryfunc);
	Yapic_MethodChecker(__iadd__, binaryfunc);
	Yapic_MethodChecker(__isub__, binaryfunc);
	Yapic_MethodChecker(__imul__, binaryfunc);
	Yapic_MethodChecker(__imod__, binaryfunc);
	Yapic_MethodChecker(__ipow__, ternaryfunc);
	Yapic_MethodChecker(__ilshift__, binaryfunc);
	Yapic_MethodChecker(__irshift__, binaryfunc);
	Yapic_MethodChecker(__iand__, binaryfunc);
	Yapic_MethodChecker(__ixor__, binaryfunc);
	Yapic_MethodChecker(__ior__, binaryfunc);
	Yapic_MethodChecker(__floordiv__, binaryfunc);
	Yapic_MethodChecker(__truediv__, binaryfunc);
	Yapic_MethodChecker(__ifloordiv__, binaryfunc);
	Yapic_MethodChecker(__itruediv__, binaryfunc);
	Yapic_MethodChecker(__index__, unaryfunc);
	Yapic_MethodChecker(__matmul__, binaryfunc);
	Yapic_MethodChecker(__imatmul__, binaryfunc);
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
	Yapic_MethodChecker(__len__, lenfunc);
	Yapic_MethodChecker(__getitem__, binaryfunc);
	Yapic_MethodChecker(__setitem__, objobjargproc);
	#define Yapic_TypeHasMappingMethods(__cls) (\
		Yapic_HasMethod(__cls, __len__) || \
		Yapic_HasMethod(__cls, __getitem__) || \
		Yapic_HasMethod(__cls, __setitem__))

	Yapic_MethodChecker(__concat__, binaryfunc);
	Yapic_MethodChecker(__repeat__, ssizeargfunc);
	Yapic_MethodChecker(__sq_getitem__, ssizeargfunc);
	Yapic_MethodChecker(__sq_setitem__, ssizeobjargproc);
	Yapic_MethodChecker(__contains__, objobjproc);
	Yapic_MethodChecker(__inplace_concat__, binaryfunc);
	Yapic_MethodChecker(__inplace_repeat__, ssizeargfunc);
	#define Yapic_TypeHasSequenceMethods(__cls) (\
		Yapic_HasMethod(__cls, __concat__) || \
		Yapic_HasMethod(__cls, __repeat__) || \
		Yapic_HasMethod(__cls, __sq_getitem__) || \
		Yapic_HasMethod(__cls, __sq_setitem__) || \
		Yapic_HasMethod(__cls, __contains__) || \
		Yapic_HasMethod(__cls, __inplace_concat__) || \
		Yapic_HasMethod(__cls, __inplace_repeat__))


} /* end namespace methods */
} /* end namespace Yapic */

#endif /* A6C67645_3133_EC87_F988_69F9A065C3AB */
