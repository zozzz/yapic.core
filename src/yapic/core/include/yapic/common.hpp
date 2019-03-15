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


#define Yapic_Member(__name, __type, __flag, __doc) \
	{#__name, __type, offsetof(Self, __name), __flag, __doc},


#define Yapic_MEMBERS_BEGIN \
	static inline const PyMemberDef* __members__() { \
		static const PyMemberDef members[] = {

#define Yapic_MEMBERS_END \
			{NULL, 0, 0, 0, NULL} \
		}; \
		return members; \
	}


#endif /* YB9FEBC2_F133_C8FE_1481_9EAC0401592D */
