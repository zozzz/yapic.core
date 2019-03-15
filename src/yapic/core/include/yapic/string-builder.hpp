#ifndef E5B23D08_9133_C8FD_140D_4CE32366FBF9
#define E5B23D08_9133_C8FD_140D_4CE32366FBF9

#include <stdbool.h>
#include <Python.h>

// TODO: Maxchar type = Py_UCS4

/*
usage:

AsciiBuilder<256> builder;
builder.AppendAscii('H');
builder.AppendFast(3456); // some unicode char
builder.AppendSafe(...); // same as above, but checking memory and reallocationg, if required
builder.EnsureSize(2345); // allocating extra memory if required

AsciiBuilder<256>::Data* cursor = builder.EnsureSize(2345);
AsciiBuilder<256>::Trait::AppendAscii(&cursor, 'H');

*/


namespace Yapic {

template<typename _DT, int InitialSize>
class _Memory {
public:
	using DT = _DT;

	inline _Memory(): cursor(_initial), _heap(NULL), _end(_initial + InitialSize) {
	}

	inline ~_Memory() {
		if (_heap != NULL) {
			PyMem_Free(_heap);
		}
	}

	inline bool EnsureSize(size_t required) {
		size_t free = FreeSize();
		if (free < required) {
			#ifdef NDEBUG
				free = Size();
				do {
					free <<= 1;
				} while (free < required);
			#else
				free = Size() + (required - FreeSize());
			#endif
			return Resize(free);
		} else {
			return true;
		}
	}

	inline void Reset() { cursor = const_cast<DT*>(Begin()); };
	inline size_t Size() const { return _end - const_cast<DT*>(Begin()); };
	inline size_t UsedSize() const { return cursor - const_cast<DT*>(Begin()); };
	inline size_t FreeSize() const { return _end - cursor; };
	inline const DT* Begin() const { return (_heap ? _heap : _initial); };
	inline const DT* End() const { return _end; };
	inline PyObject* Object() const { return NULL; }

	bool Resize(size_t newSize) {
		size_t pos = UsedSize();
		if (_heap == NULL) {
			_heap = (DT*) PyMem_Malloc(newSize * sizeof(DT));
			if (_heap == NULL) {
				PyErr_NoMemory();
				return false;
			}
			memmove(_heap, _initial, pos * sizeof(DT));
		} else {
			_heap = (DT*) PyMem_Realloc(_heap, newSize * sizeof(DT));
			if (_heap == NULL) {
				PyErr_NoMemory();
				return false;
			}
		}
		_end = _heap + newSize;
		cursor = _heap + pos;
		return true;
	}

	DT* cursor;
private:
	DT* _heap;
	DT* _end;
	DT _initial[InitialSize];
};


class _BytesMemory {
public:
	using DT = char;

	inline _BytesMemory(): cursor(NULL), _bytes(NULL), _end(NULL) {}

	inline ~_BytesMemory() {
		Py_XDECREF(_bytes);
	}

	inline bool EnsureSize(Py_ssize_t required) {
		if (_bytes == NULL) {
			return Resize(required);
		} else {
			Py_ssize_t free = FreeSize();
			if (free < required) {
				#ifdef NDEBUG
					free = Size();
					do {
						free <<= 1;
					} while (free < required);
				#else
					free = Size() + (required - FreeSize());
				#endif
				return Resize(free);
			} else {
				return true;
			}
		}
	}

	inline void Reset() {
		Py_CLEAR(_bytes);
		cursor = _end = NULL;
	};
	inline Py_ssize_t Size() const { return _end - const_cast<DT*>(Begin()); };
	inline Py_ssize_t UsedSize() const { return cursor - const_cast<DT*>(Begin()); };
	inline Py_ssize_t FreeSize() const { return _end - cursor; };
	inline const DT* Begin() const { return _bytes ? reinterpret_cast<DT*>(((PyBytesObject*) _bytes)->ob_sval) : NULL; };
	inline const DT* End() const { return _end; };
	inline PyObject* Object() const { return _bytes; }

	bool Resize(Py_ssize_t newSize) {
		Py_ssize_t pos = 0;
		if (_bytes == NULL) {
			_bytes = PyBytes_FromStringAndSize(NULL, newSize);
			if (_bytes == NULL) {
				return false;
			}
		} else {
			pos = UsedSize();
			if (_PyBytes_Resize(&_bytes, newSize) != 0) {
				return false;
			}
		}
		cursor = ((PyBytesObject*) _bytes)->ob_sval + pos;
		_end = ((PyBytesObject*) _bytes)->ob_sval + newSize;
		return true;
	}

	DT* cursor;
private:
	PyObject* _bytes;
	DT* _end;
};


#define __bytelength(T, l) \
	(sizeof(T) == 1 ? l : sizeof(T) == 2 ? l << 1 : l << 2)

template<typename I, typename O, typename S>
static inline void CopyStrBytes(O* dest, const I* input, S length) {
	assert(dest != NULL);
	assert(input != NULL);
	assert(sizeof(I) == 1 || sizeof(I) == 2 || sizeof(I) == 4);

	if (sizeof(I) == sizeof(O)) {
		memcpy(dest, input, __bytelength(I, length));
	} else {
		while (length-- > 0) {
			dest[length] = input[length];
		}
	}
}

#undef __bytelength

template<bool _isucs>
struct _StringTraits {
	static constexpr const int MaxEncodedCharSize = 1;
	static constexpr const bool IsString = 1;
	static constexpr const bool IsUnicode = _isucs;

	template<typename Memory, typename Input>
	static inline void AppendAscii(Memory& mem, Input ch) {
		assert(ch <= 127);
		*(mem.cursor++) = ch;
	}

	template<typename Memory, typename Input>
	static inline void AppendChar(Memory& mem, Input ch) {
		assert(sizeof(Memory::DT) >= sizeof(Input));
		*(mem.cursor++) = ch;
	}

	template<typename Memory, typename Mc>
	static inline bool AppendString(Memory& mem, PyObject* obj, Mc& maxchar) {
		unsigned int kind = (unsigned int)PyUnicode_KIND(obj);
		maxchar |= PyUnicode_MAX_CHAR_VALUE(obj);
		if (sizeof(typename Memory::DT) == 1 && kind > sizeof(typename Memory::DT)) {
			PyErr_SetString(PyExc_UnicodeError, "The given string must be ascii encoded.");
			return false;
		} else {
			size_t size = PyUnicode_GET_LENGTH(obj);
			switch (kind) {
				case PyUnicode_1BYTE_KIND:
					CopyStrBytes(mem.cursor, PyUnicode_1BYTE_DATA(obj), size);
				break;
				case PyUnicode_2BYTE_KIND:
					CopyStrBytes(mem.cursor, PyUnicode_2BYTE_DATA(obj), size);
				break;
				case PyUnicode_4BYTE_KIND:
					CopyStrBytes(mem.cursor, PyUnicode_4BYTE_DATA(obj), size);
				break;
				default:
					assert(0);
					return false;
				break;
			}
			mem.cursor += size;
		}
		return true;
	}

	template<typename Memory, typename Mc>
	static inline bool AppendBytes(Memory& mem, PyObject* obj, Mc& maxchar) {
		// TODO: ez itt nagyon trükkös lesz lehet, mivel ki tudja milyen kódolású
		return false;
	}

	template<typename Mc, typename Input>
	static inline void UpdateMaxchar(Mc& maxchar, Input ch) {
		maxchar |= ch;
	}

	template<typename Mc>
	static inline PyObject* NewFromSize(Py_ssize_t size, Mc maxchar) {
		return PyUnicode_New(size, maxchar);
	}

	template<typename Storage, typename Mc>
	static inline PyObject* NewFromSizeAndData(const Storage* data, Py_ssize_t length, Mc maxchar) {
		PyObject* str = PyUnicode_New(length, maxchar);
		if (str == NULL) {
			return NULL;
		}
		switch (PyUnicode_KIND(str)) {
			case PyUnicode_1BYTE_KIND:
				CopyStrBytes(PyUnicode_1BYTE_DATA(str), data, length);
			break;

			case PyUnicode_2BYTE_KIND:
				CopyStrBytes(PyUnicode_2BYTE_DATA(str), data, length);
			break;

			case PyUnicode_4BYTE_KIND:
				CopyStrBytes(PyUnicode_4BYTE_DATA(str), data, length);
			break;
			default:
				assert(0);
			break;
		}
		return str;
	}

	template<typename Memory, typename Mc>
	static inline PyObject* ToPython(Memory& mem, Mc maxchar) {
		return NewFromSizeAndData(mem.Begin(), mem.UsedSize(), maxchar);
	}

	template<typename Input>
	static inline Py_ssize_t EncodedCharSize(Input ch) {
		return 1;
	}

	static inline Py_ssize_t RequiredSizeForUnicode(PyObject* obj) {
		assert(obj != NULL);
		return PyUnicode_GET_LENGTH(obj);
	}

	static inline Py_ssize_t RequiredSizeForBytes(PyObject* obj) {
		assert(obj != NULL);
		return PyBytes_GET_SIZE(obj);
	}
};



namespace _Encoding {

	struct _Utf8 {
		static constexpr const int MaxEncodedCharSize = 4;
		static constexpr const bool IsUnicode = true;

		static constexpr const size_t UPPER_2B_CHAR = 0x7FF;
		static constexpr const size_t UPPER_3B_CHAR = 0xFFFF;
		static constexpr const size_t UPPER_4B_CHAR = 0x1FFFFF;

		#pragma warning( push )
		#pragma warning( disable : 4715)

		template<typename Storage, typename Input>
		static inline int AppendChar(Storage* into, Input ch) {
			if (ch <= 0x80) {
				into[0] = ch;
				return 1;
			} else if (ch < 0x800) {
				into[1] = 0x80 | (ch & 0x3F);
				into[0] = 0xC0 | (ch >> 6);
				return 2;
			} else if (sizeof(Input) >= 2 && ch < 0x10000) {
				into[2] = 0x80 | (ch & 0x3F);
				into[1] = 0x80 | ((ch >> 6) & 0x3F);
				into[0] = 0xE0 | (ch >> 12);
				return 3;
			} else if (sizeof(Input) >= 4) {
				assert(ch <= UPPER_4B_CHAR);
				into[3] = 0x80 | (ch & 0x3F);
				into[2] = 0x80 | ((ch >> 6) & 0x3F);
				into[1] = 0x80 | ((ch >> 12) & 0x3F);
				into[0] = 0xF0 | (ch >> 18);
				return 4;
			}
			assert(0);
		}

		template<typename Storage, typename Input>
		static inline Storage* AppendChar2(Storage* into, Input ch) {
			if (ch <= 0x80) {
				into[0] = ch;
				return into + 1;
			} else if (ch < 0x800) {
				into[1] = 0x80 | (ch & 0x3F);
				into[0] = 0xC0 | (ch >> 6);
				return into + 2;
			} else if (sizeof(Input) >= 2 && ch < 0x10000) {
				into[2] = 0x80 | (ch & 0x3F);
				into[1] = 0x80 | ((ch >> 6) & 0x3F);
				into[0] = 0xE0 | (ch >> 12);
				return into + 3;
			} else if (sizeof(Input) >= 4) {
				assert(ch <= UPPER_4B_CHAR);
				into[3] = 0x80 | (ch & 0x3F);
				into[2] = 0x80 | ((ch >> 6) & 0x3F);
				into[1] = 0x80 | ((ch >> 12) & 0x3F);
				into[0] = 0xF0 | (ch >> 18);
				return into + 4;
			}
			assert(0);
		}


		template<typename Memory, typename Input>
		static inline void AppendChar3(Memory& mem, Input ch) {
			auto c = mem.cursor;
			if (ch <= 0x80) {
				c[0] = ch;
				c += 1;
			} else if (ch < 0x800) {
				c[1] = 0x80 | (ch & 0x3F);
				c[0] = 0xC0 | (ch >> 6);
				c += 2;
			} else if (sizeof(Input) >= 2 && ch < 0x10000) {
				c[2] = 0x80 | (ch & 0x3F);
				c[1] = 0x80 | ((ch >> 6) & 0x3F);
				c[0] = 0xE0 | (ch >> 12);
				c += 3;
			} else if (sizeof(Input) >= 4) {
				assert(ch <= UPPER_4B_CHAR);
				c[3] = 0x80 | (ch & 0x3F);
				c[2] = 0x80 | ((ch >> 6) & 0x3F);
				c[1] = 0x80 | ((ch >> 12) & 0x3F);
				c[0] = 0xF0 | (ch >> 18);
				c += 4;
			}
			mem.cursor = c;
		}

		#pragma warning( pop )

		template<typename Storage, typename Input>
		static inline void AppendChar(Storage* into, Input ch, Storage** out) {
			if (ch <= 0x80) {
				*(into++) = ch;
			} else if (ch < 0x800) {
				*(into++) = 0xC0 | (ch >> 6);
				*(into++) = 0x80 | (ch & 0x3F);
			} else if (sizeof(Input) >= 2 && ch < 0x10000) {
				*(into++) = 0xE0 | (ch >> 12);
				*(into++) = 0x80 | ((ch >> 6) & 0x3F);
				*(into++) = 0x80 | (ch & 0x3F);
			} else if (sizeof(Input) >= 4) {
				assert(ch <= UPPER_4B_CHAR);
				*(into++) = 0xF0 | (ch >> 18);
				*(into++) = 0x80 | ((ch >> 12) & 0x3F);
				*(into++) = 0x80 | ((ch >> 6) & 0x3F);
				*(into++) = 0x80 | (ch & 0x3F);
			}
			*out = into;

			// if (ch <= 0x80) {
			// 	into[0] = ch;
			// 	*out = into + 1;
			// } else if (ch < 0x800) {
			// 	into[1] = 0x80 | (ch & 0x3F);
			// 	into[0] = 0xC0 | (ch >> 6);
			// 	*out = into + 2;
			// } else if (sizeof(Input) >= 2 && ch < 0x10000) {
			// 	into[2] = 0x80 | (ch & 0x3F);
			// 	into[1] = 0x80 | ((ch >> 6) & 0x3F);
			// 	into[0] = 0xE0 | (ch >> 12);
			// 	*out = into + 3;
			// } else if (sizeof(Input) >= 4) {
			// 	assert(ch <= UPPER_4B_CHAR);
			// 	into[3] = 0x80 | (ch & 0x3F);
			// 	into[2] = 0x80 | ((ch >> 6) & 0x3F);
			// 	into[1] = 0x80 | ((ch >> 12) & 0x3F);
			// 	into[0] = 0xF0 | (ch >> 18);
			// 	*out = into + 4;
			// }
		}

		template<typename Memory>
		static inline void AppendString(Memory& mem, PyObject* obj) {
			switch (PyUnicode_KIND(obj)) {
				case PyUnicode_1BYTE_KIND:
					if (PyUnicode_IS_ASCII(obj)) {
						CopyStrBytes(mem.cursor, PyUnicode_1BYTE_DATA(obj), PyUnicode_GET_LENGTH(obj));
						mem.cursor += PyUnicode_GET_LENGTH(obj);
					} else {
						__AppendString(mem, PyUnicode_1BYTE_DATA(obj), PyUnicode_GET_LENGTH(obj));
					}
				break;

				case PyUnicode_2BYTE_KIND:
					__AppendString(mem, PyUnicode_2BYTE_DATA(obj), PyUnicode_GET_LENGTH(obj));
				break;

				case PyUnicode_4BYTE_KIND:
					__AppendString(mem, PyUnicode_4BYTE_DATA(obj), PyUnicode_GET_LENGTH(obj));
				break;
			}
		}

		// template<typename Memory, typename Input>
		// static inline void __AppendString(Memory& mem, Input* data, Py_ssize_t size) {
		// 	Input* end = data + size;
		// 	Memory::DT* into = mem.cursor;
		// 	while (data < end) {
		// 		into += AppendChar(into, *(data++));
		// 	}
		// 	mem.cursor = into;
		// }

		template<typename Memory, typename Input>
		static inline void __AppendString(Memory& mem, Input* data, Py_ssize_t size) {
			Input* end = data + size;
			while (data < end) {
				AppendChar3(mem, *(data++));
			}
		}

		template<typename Input>
		static inline Py_ssize_t EncodedCharSize(Input ch) {
			if (ch <= (Input)127) {
				return 1;
			} else if (ch <= (Input)UPPER_2B_CHAR) {
				return 2;
			} else if (ch <= (Input)UPPER_3B_CHAR) {
				return 3;
			} else {
				assert(ch <= (Input)UPPER_4B_CHAR);
				return 4;
			}
		}

		static inline Py_ssize_t RequiredSizeForUnicode(PyObject* obj) {
			assert(obj != NULL);
			return PyUnicode_GET_LENGTH(obj) * (
				PyUnicode_KIND(obj) == PyUnicode_1BYTE_KIND
				? 1
				: PyUnicode_KIND(obj) == PyUnicode_2BYTE_KIND
					? 3
					: 4
			);
		}

		static inline Py_ssize_t RequiredSizeForBytes(PyObject* obj) {
			assert(obj != NULL);
			return PyBytes_GET_SIZE(obj);
		}
	};


	struct _Raw {
		static constexpr const int MaxEncodedCharSize = 1;
		static constexpr const bool IsUnicode = false;

		template<typename Storage, typename Input>
		static inline int AppendChar(Storage* into, Input ch) {
			if (sizeof(Storage) >= sizeof(Input)) {
				into[0] = ch;
				return 1;
			} else {
				// ha nem stimmel a méret akkor csak byte szinten feldarabolja
				// TODO: még kitaláni, hogy jó ötlet-e
				assert(0);
				// switch (sizeof(Input)) {
				// 	case 1:
				// 		*(into++) = ch;
				// 	break;

				// 	case 2:
				// 		into[1] = ch & 0xFF;
				// 		ch >>= 8;
				// 		into[0] = ch & 0xFF;
				// 		into += 2;
				// 	break;

				// 	case 3:
				// 		into[2] = ch & 0xFF;
				// 		ch >>= 8;
				// 		into[1] = ch & 0xFF;
				// 		ch >>= 8;
				// 		into[0] = ch & 0xFF;
				// 		into += 3;
				// 	break;

				// 	case 4:
				// 		into[3] = ch & 0xFF;
				// 		ch >>= 8;
				// 		into[2] = ch & 0xFF;
				// 		ch >>= 8;
				// 		into[1] = ch & 0xFF;
				// 		ch >>= 8;
				// 		into[0] = ch & 0xFF;
				// 		into += 4;
				// 	break;

				// 	default:
				// 		for (int i=sizeof(Input) - 1 ; i>=0 ; --i) {
				// 			into[i] = ch & 0xFF;
				// 			ch >>= 8;
				// 		}
				// 		into += sizeof(Input);
				// 	break;
				// }
			}
		}

		template<typename Input>
		static inline Py_ssize_t EncodedCharSize(Input ch) {
			return sizeof(Input) / sizeof(char);
		}

		static inline Py_ssize_t RequiredSizeForUnicode(PyObject* obj) {
			assert(obj != NULL);
			return PyUnicode_GET_LENGTH(obj) * PyUnicode_KIND(obj);
		}

		static inline Py_ssize_t RequiredSizeForBytes(PyObject* obj) {
			assert(obj != NULL);
			return PyBytes_GET_SIZE(obj);
		}
	};
} // end namespace _Encoding


template<typename Encoding>
struct _ByteTraits {
	static constexpr const int MaxEncodedCharSize = 1;
	static constexpr const bool IsString = false;
	static constexpr const bool IsUnicode = Encoding::IsUnicode;

	template<typename Memory, typename Input>
	static inline void AppendAscii(Memory& mem, Input ch) {
		assert(ch <= 127);
		// printf("AppendAscii %s ch=%lu cursor=%p\n", typeid(Storage).name(), ch, into);
		*(mem.cursor++) = ch;
	}

	template<typename Memory, typename Input>
	static inline void AppendChar(Memory& mem, Input ch) {
		// Encoding::AppendChar3(mem, ch);
		mem.cursor = Encoding::AppendChar2(mem.cursor, ch);
	}

	template<typename Memory, typename Mc>
	static inline bool AppendString(Memory& mem, PyObject* obj, Mc& maxchar) {
		assert(obj != NULL);
		assert(PyUnicode_CheckExact(obj));
		Encoding::AppendString(mem, obj);
		return true;
	}

	template<typename Memory, typename Mc>
	static inline bool AppendBytes(Memory& mem, PyObject* obj, Mc& maxchar) {
		assert(obj != NULL);
		assert(PyBytes_CheckExact(obj));
		CopyStrBytes(mem.cursor, PyBytes_AS_STRING(obj), PyBytes_GET_SIZE(obj));
		mem.cursor += PyBytes_GET_SIZE(obj);
		return true;
	}

	template<typename Mc, typename Input>
	static inline void UpdateMaxchar(Mc& maxchar, Input ch) {
	}

	// template<typename Mc>
	// static inline PyObject* NewFromSize(size_t size, Mc maxchar) {
	// 	// little hacky
	// 	PyObject* bytes = PyMem_Malloc(); // TODO: ...
	// 	return NULL;
	// }

	// template<typename Mc>
	// static inline PyObject* NewFromSizeAndData(const Storage* data, size_t size, Mc maxchar) {
	// 	return PyBytes_FromStringAndSize(data, size);
	// }

	template<typename Input>
	static inline Py_ssize_t EncodedCharSize(Input ch) {
		return Encoding::EncodedCharSize(ch);
	}

	static inline Py_ssize_t RequiredSizeForUnicode(PyObject* obj) {
		return Encoding::RequiredSizeForUnicode(obj);
	}

	static inline Py_ssize_t RequiredSizeForBytes(PyObject* obj) {
		return Encoding::RequiredSizeForBytes(obj);
	}

	template<typename Memory, typename Mc>
	static inline PyObject* ToPython(Memory& mem, Mc maxchar) {
		if (mem.Resize(mem.UsedSize())) {
			PyObject* res = mem.Object();
			Py_INCREF(res);
			return res;
		} else {
			return NULL;
		}
	}
};


template<typename _Trait, typename _Mem>
class _StringBuilder {
public:
	using Trait = _Trait;
	using Storage = typename _Mem::DT;

	inline _StringBuilder(): _maxchar(127) {}

	template<typename Input>
	inline void AppendAscii(Input ch) {
		assert(_memory.cursor < _memory.End());
		Trait::AppendAscii(_memory, ch);
	}

	template<typename Input>
	inline void AppendChar(Input ch) {
		assert(_memory.cursor < _memory.End());
		Trait::UpdateMaxchar(_maxchar, ch);
		Trait::AppendChar(_memory, ch);
	}

	template<typename Input>
	inline bool AppendCharSafe(Input ch) {
		if (EnsureSize(Trait::EncodedCharSize(ch))) {
			assert(_memory.cursor < _memory.End());
			Trait::UpdateMaxchar(_maxchar, ch);
			Trait::AppendChar(_memory, ch);
			return true;
		} else {
			return false;
		}
	}

	inline bool AppendString(const char* str) {
		assert(str != NULL);
		return AppendString(str, strlen(str));
	}

	inline bool AppendString(const char* str, size_t size) {
		assert(str != NULL);
		assert(size >= 0);
		CopyStrBytes(_memory.cursor, str, size);
		_memory.cursor += size;
		return true;
	}

	inline bool AppendStringSafe(const char* str) {
		assert(str != NULL);
		return AppendStringSafe(str, strlen(str));
	}

	inline bool AppendStringSafe(const char* str, size_t size) {
		assert(str != NULL);
		assert(size >= 0);
		return EnsureSize(size) && AppendString(str, size);
	}

	inline bool AppendString(PyObject* obj) {
		assert(obj != NULL);
		assert(PyUnicode_CheckExact(obj));
		return Trait::AppendString(_memory, obj, _maxchar);
	}

	inline bool AppendStringSafe(PyObject* obj) {
		assert(obj != NULL);
		assert(PyUnicode_CheckExact(obj));
		return EnsureSize(Trait::RequiredSizeForUnicode(obj)) && AppendString(obj);
	}

	inline bool AppendBytes(PyObject* obj) {
		assert(obj != NULL);
		assert(PyBytes_CheckExact(obj));
		return Trait::AppendBytes(_memory, obj, _maxchar);
	}

	inline bool AppendBytesSafe(PyObject* obj) {
		assert(obj != NULL);
		assert(PyBytes_CheckExact(obj));
		return EnsureSize(Trait::RequiredSizeForBytes(obj)) && AppendBytes(obj);
	}

	inline bool EnsureSize(Py_ssize_t size) {
		return _memory.EnsureSize(size);
	}

	inline bool EnsureSize(PyObject* obj) {
		if (PyUnicode_CheckExact(obj)) {
			return EnsureSize(Trait::RequiredSizeForUnicode(obj));
		} else if (PyBytes_CheckExact(obj)) {
			return EnsureSize(Trait::RequiredSizeForBytes(obj));
		}
		PyErr_BadInternalCall();
		return false;
	}

	inline PyObject* ToPython() {
		// printf("ToPython cursor=%p end=%p\n", _memory.cursor, _memory.End());
		assert(_memory.cursor <= _memory.End());
		PyObject* res = Trait::ToPython(_memory, _maxchar);
		_memory.Reset();
		return res;
	}
private:
	_Mem _memory;
	Py_UCS4 _maxchar;
};


// template<typename _Trait, typename _Mem>
// class _ChunkBuilder {
// public:
// 	using Trait = _Trait;
// };





template<typename DT, int size, bool isucs>
using StringBuilder = _StringBuilder<_StringTraits<isucs>, _Memory<DT, size>>;

template<int size>
using AsciiBuilder = StringBuilder<char, size, false>;

template<int size>
using UnicodeBuilder = StringBuilder<Py_UCS4, size, true>;



using Utf8Bytes = _ByteTraits<_Encoding::_Utf8>;
using RawBytes = _ByteTraits<_Encoding::_Raw>;

template<typename _Trait>
using BytesBuilder = _StringBuilder<_Trait, _BytesMemory>;
using RawBytesBuilder = BytesBuilder<RawBytes>;
using Utf8BytesBuilder = BytesBuilder<Utf8Bytes>;


} // end namespace Yapic
#endif /* E5B23D08_9133_C8FD_140D_4CE32366FBF9 */
