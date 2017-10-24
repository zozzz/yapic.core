#ifndef YB9FEBC2_F133_C8FE_1481_9EAC0401592D
#define YB9FEBC2_F133_C8FE_1481_9EAC0401592D


#define Yapic_Method(__fn, __flags, __doc) \
	{#__fn, (PyCFunction) &__fn, __flags, PyDoc_STR(__doc)},

#define Yapic_METHODS_BEGIN \
	static inline const PyMethodDef* __methods__() { \
		static const PyMethodDef methods[] = {

#define Yapic_METHODS_END \
			{NULL, NULL, 0, NULL} \
		}; \
		return methods; \
	}


// #define Yapic_String(__name, __value) \
// 	static constexpr const char* __name = __value;

#endif /* YB9FEBC2_F133_C8FE_1481_9EAC0401592D */
