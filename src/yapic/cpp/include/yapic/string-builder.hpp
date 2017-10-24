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

	inline _Memory(): _heap(NULL), _cursor(_initial), _end(_initial + InitialSize) {
	}

	inline ~_Memory() {
		if (_heap != NULL) {
			PyMem_Free(_heap);
		}
	}

	inline DT* EnsureSize(size_t required) {
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
			return _cursor;
		}
	}

	// inline PyObject* ToPython() {
	// 	return Factory::Create(_heap ? _heap : _initial, _cursor);
	// }

	inline void Reset() { _cursor = const_cast<DT*>(Begin()); };
	inline size_t Size() const { return _end - const_cast<DT*>(Begin()); };
	inline size_t UsedSize() const { return _cursor - const_cast<DT*>(Begin()); };
	inline size_t FreeSize() const { return _end - _cursor; };
	inline const DT* Begin() const { return (_heap ? _heap : _initial); };

	DT* Resize(size_t newSize) {
		size_t pos = UsedSize();

		if (_heap == NULL) {
			_heap = (DT*) PyMem_Malloc(newSize * sizeof(DT));
			if (_heap == NULL) {
				PyErr_NoMemory();
				return NULL;
			}
			memmove(_heap, _initial, pos * sizeof(DT));
		} else {
			_heap = (DT*) PyMem_Realloc(_heap, newSize * sizeof(DT));
			if (_heap == NULL) {
				PyErr_NoMemory();
				return NULL;
			}
		}
		_end = _heap + newSize;
		return _cursor = _heap + pos;
	}

	DT* _heap;
	DT* _cursor;
	DT* _end;
	DT _initial[InitialSize];
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


struct _StringTraits {
	template<typename Storage, typename Input>
	static inline void AppendAscii(Storage*& into, Input ch) {
		assert(ch <= 127);
		*(into++) = ch;
	}

	template<typename Storage, typename Input>
	static inline void AppendChar(Storage*& into, Input ch) {
		assert(sizeof(Storage) >= sizeof(Input));
		*(into++) = ch;
	}

	template<typename Storage, typename Mc>
	static inline bool AppendString(Storage*& into, PyObject* obj, Mc& maxchar) {
		int kind = PyUnicode_KIND(obj);
		if (kind > sizeof(Storage)) {
			PxObject* bytes = PyUnicode_AsUTF8String(obj);
			if (bytes == NULL) {
				return NULL;
			}
			bool res = AppendBytes(into, bytes, maxchar);
			Py_DECREF(bytes);
			return res;
		} else {
			maxchar |= PyUnicode_MAX_CHAR_VALUE(obj);
			size_t size = PyUnicode_GET_LENGTH(obj);
			switch (kind) {
				case PyUnicode_1BYTE_KIND:
					CopyStrBytes(into, PyUnicode_1BYTE_DATA(data), size);
				break;
				case PyUnicode_2BYTE_KIND:
					CopyStrBytes(into, PyUnicode_21BYTE_DATA(data), size);
				break;
				case PyUnicode_4BYTE_KIND:
					CopyStrBytes(into, PyUnicode_4BYTE_DATA(data), size);
				break;
				default
					assert(0);
					return false;
				break;
			}
			into += size;
		}
	}

	template<typename Storage, typename Mc>
	static inline bool AppendBytes(Storage*& into, PyObject* obj, Mc& maxchar) {
		// TODO: ez itt nagyon trükkös lesz lehet, mivel ki tudja milyen kódolású
	}

	template<typename Mc, typename Input>
	static inline void UpdateMaxchar(Mc& maxchar, Input ch) {
		maxchar |= ch;
	}

	template<typename Storage, typename Input>
	static inline void Copy(Storage* out, Input* input, size_t length) {
		CopyStrBytes(out, input, length);
	}

	template<typename Storage, typename Input>
	static inline size_t CharSize(const Storage& into, Input ch) {
		assert(sizeof(Storage) >= sizeof(Input));
		return 1;
	}

	template<typename Mc>
	static inline PyObject* NewFromSize(size_t size, Mc maxchar) {
		return PyUnicode_New(size, maxchar);
	}

	template<typename Storage, typename Mc>
	static inline PyObject* NewFromSizeAndData(const Storage* data, size_t length, Mc maxchar) {
		PyObject* str = PyUnicode_New(length, maxchar);
		if (str == NULL) {
			return NULL;
		}
		switch (PyUnicode_KIND(str)) {
			case PyUnicode_1BYTE_KIND:
				Copy(PyUnicode_1BYTE_DATA(str), data, length);
			break;

			case PyUnicode_2BYTE_KIND:
				Copy(PyUnicode_2BYTE_DATA(str), data, length);
			break;

			case PyUnicode_4BYTE_KIND:
				Copy(PyUnicode_4BYTE_DATA(str), data, length);
			break;
			default:
				assert(0);
			break;
		}
		return str;
	}

	static inline PyObject* CoerceToOneByte(PyObject* o) {
		if (PyUnicode_Check(o)) {
			return PyUnicode_AsLatin1String(o);
		} else if (PyBytes_CheckExact(o)) {
			Py_INCREF(o);
			return o;
		} else {
			PyErr_BadInternalCall();
			return NULL;
		}
	}

	static inline size_t RequiredSize(size_t length) {
		return length;
	}
};



namespace _Encoding {

	struct _Utf8 {
		static constexpr const size_t UPPER_2B_CHAR = 0x7FF;
		static constexpr const size_t UPPER_3B_CHAR = 0xFFFF;
		static constexpr const size_t UPPER_4B_CHAR = 0x1FFFFF;

		template<typename Storage, typename Input>
		static inline void Append(Storage*& into, Input ch) {
			printf("Utf8::Append ch=%lu, size=%d\n", ch, CharSizeInBytes(ch));
			switch (CharSizeInBytes(ch)) {
				case 1:
					*(into++) = ch;
				break;

				case 2:
					printf("Input = %04x\n", ch);
					into[1] = 0x80 | (ch & 0x3F);
					ch >>= 6;
					into[0] = 0xC0 | (ch & 0x1F);
					printf("1. byte = %02x\n", into[0]);
					printf("2. byte = %02x\n", into[1]);
					into += 2;
				break;

				case 3:
					into[2] = 0x80 | (ch & 0x3F);
					ch >>= 6;
					into[1] = 0x80 | (ch & 0x3F);
					ch >>= 6;
					into[0] = 0xE0 | (ch & 0x0F);
					into += 3;
				break;

				case 4:
					into[3] = 0x80 | (ch & 0x3F);
					ch >>= 6;
					into[2] = 0x80 | (ch & 0x3F);
					ch >>= 6;
					into[1] = 0x80 | (ch & 0x3F);
					ch >>= 6;
					into[0] = 0xF0 | (ch & 0x07);
					into += 4;
				break;
			}
		}

		template<typename Input>
		static inline size_t CharSizeInBytes(Input ch) {
			// printf("CharSizeInBytes %s\n", typeid(Input).name());
			if ( ch <= 127) {
				return 1;
			} else if ( ch <= UPPER_2B_CHAR) {
				return 2;
			} else if ( ch <= UPPER_3B_CHAR) {
				return 3;
			} else {
				assert( ch <= UPPER_4B_CHAR);
				return 4;
			}
		}

		static inline size_t RequiredSize(size_t length) {
			return length * 4;
		}

		static inline unsigned char* UnicodeToBytes(PyObject* obj, Py_ssize_t& size) {
			assert(obj != NULL);
			assert(PyUnicode_Check(obj));
			return PyUnicode_AsUTF8AndSize(obj, size);
		}
	};


	struct _Raw {
		template<typename Storage, typename Input>
		static inline void Append(Storage*& into, Input ch) {
			if (sizeof(Storage) >= sizeof(Input)) {
				*(into++) = ch;
			} else {
				// ha nem stimmel a méret akkor csak byte szinten feldarabolja
				switch (sizeof(Input)) {
					case 1:
						*(into++) = ch;
					break;

					case 2:
						into[1] = ch & 0xFF;
						ch >>= 8;
						into[0] = ch & 0xFF;
						into += 2;
					break;

					case 3:
						into[2] = ch & 0xFF;
						ch >>= 8;
						into[1] = ch & 0xFF;
						ch >>= 8;
						into[0] = ch & 0xFF;
						into += 3;
					break;

					case 4:
						into[3] = ch & 0xFF;
						ch >>= 8;
						into[2] = ch & 0xFF;
						ch >>= 8;
						into[1] = ch & 0xFF;
						ch >>= 8;
						into[0] = ch & 0xFF;
						into += 4;
					break;

					default:
						for (int i=sizeof(Input) - 1 ; i>=0 ; --i) {
							into[i] = ch & 0xFF;
							ch >>= 8;
						}
						into += sizeof(Input);
					break;
				}
			}
		}

		template<typename Input>
		static inline size_t CharSizeInBytes(Input ch) {
			return sizeof(Input) / sizeof(char);
		}

		static inline size_t RequiredSize(size_t length) {
			// TODO: ez így rossz
			return length;
		}

		static inline unsigned char* UnicodeToBytes(PyObject* obj, Py_ssize_t& size) {
			assert(obj != NULL);
			assert(PyUnicode_Check(obj));
			size = PyUnicode_GET_LENGTH(obj) * PyUnicode_KIND(obj);
			return PyUnicode_1BYTE_DATA(obj);
		}
	};
} // end namespace _Encoding


template<typename Storage, typename Encoding>
struct _ByteTraits {
	template<typename Input>
	static inline void AppendAscii(Storage*& into, Input ch) {
		assert(ch <= 127);
		// printf("AppendAscii %s ch=%lu cursor=%p\n", typeid(Storage).name(), ch, into);
		*(into++) = ch;
	}

	template<typename Input>
	static inline void AppendChar(Storage*& into, Input ch) {
		Encoding::Append(into, ch);
	}

	template<typename Mc>
	static inline bool AppendString(Storage*& into, PyObject* obj, Mc& maxchar) {
		size_t size;
		const unsigned char* data = Encoding::UnicodeToBytes(obj, size);
		if (data == NULL) {
			return false;
		}
		__append(into, data, size);
		return true;
	}

	template<typename Mc>
	static inline bool AppendBytes(Storage*& into, PyObject* obj, Mc& maxchar) {
		assert(obj != NULL);
		assert(PyBytes_CheckExact(obj));
		__append(into, PyBytes_AS_STRING(obj), PyBytes_GET_SIZE(obj));
		return true;
	}

	static inline void __append(Storage*& into, const unsigned char* input, size_t size) {
		CopyStrBytes(into, input, size);
		into += size;
	}

	template<typename Mc, typename Input>
	static inline void UpdateMaxchar(Mc& maxchar, Input ch) {
	}

	template<typename Input>
	static inline void Copy(Storage* out, size_t outSize, Input* input, size_t inputSize) {
		assert(outSize <= inputSize);
		CopyStrBytes(out, input, inputSize);
	}

	template<typename Input>
	static inline size_t CharSize(const Storage& into, Input input) {
		if (sizeof(Input) == 1) {
			return 1;
		} else {
			return Encoding::CharSizeInBytes(input);
		}
	}

	template<typename Mc>
	static inline PyObject* NewFromSize(size_t size, Mc maxchar) {
		// little hacky
		PyObject* bytes = PyMem_Malloc(); // TODO: ...
		return NULL;
	}

	template<typename Mc>
	static inline PyObject* NewFromSizeAndData(const Storage* data, size_t size, Mc maxchar) {
		return PyBytes_FromStringAndSize(data, size);
	}

	static inline PyObject* CoerceToOneByte(PyObject* o) {
		Py_INCREF(o);
		return o;
	}

	static inline size_t RequiredSize(size_t length) {
		return length * 4;
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
		assert(_memory._cursor < _memory._end);
		Trait::AppendAscii(_memory._cursor, ch);
	}

	template<typename Input>
	inline void AppendChar(Input ch) {
		assert(_memory._cursor < _memory._end);
		Trait::UpdateMaxchar(_maxchar, ch);
		Trait::AppendChar(_memory._cursor, ch);
	}

	template<typename Input>
	inline bool AppendSafe(Input ch) {
		if (!_memory.EnsureSize(Trait::CharSize(_memory._cursor, ch))) {
			return false;
		}
		assert(_memory._cursor < _memory._end);
		Trait::UpdateMaxchar(_maxchar, ch);
		Trait::AppendChar(_memory._cursor, ch);
	}

	inline void AppendString(const char* str) {
		assert(str != NULL);
		AppendString(str, strlen(str));
	}

	inline void AppendString(const char* str, size_t size) {
		assert(str != NULL);
		assert(size >= 0);
		CopyStrBytes(_memory._cursor, str, size);
		_memory._cursor += size;
	}

	inline bool AppendStringSafe(const char* str) {
		assert(str != NULL);
		return AppendStringSafe(str, strlen(str));
	}

	inline bool AppendStringSafe(const char* str, size_t size) {
		assert(str != NULL);
		assert(size >= 0);
		if (!_memory.EnsureSize(size)) {
			return false;
		}
		return AppendString(str, size);
	}

	inline bool AppendString(PyObject* obj) {
		assert(obj != NULL);
		assert(PyUnicode_CheckExact(obj));
		return Trait::AppendString(obj);
	}

	inline bool AppendStringSafe(PyObject* obj) {
		assert(obj != NULL);
		assert(PyUnicode_CheckExact(obj));
		if (!_memory.EnsureSize(PyUnicode_GET_LENGTH(obj))) {
			return false;
		}
		return AppendString(obj);
	}

	inline bool AppendBytes(PyObject* obj) {
		assert(obj != NULL);
		assert(PyBytes_CheckExact(obj));
		return Trait::AppendBytes(obj);
	}

	inline bool AppendBytesSafe(PyObject* obj) {
		assert(obj != NULL);
		assert(PyBytes_CheckExact(obj));
		if (!_memory.EnsureSize(PyBytes_GET_SIZE(obj))) {
			return false;
		}
		return AppendBytes(obj);
	}

	// bool AppendString(PyObject* obj) {
	// 	assert(obj != NULL);
	// 	assert(PyUnicode_CheckExact(obj));

	// 	int kind = PyUnicode_KIND(obj);
	// 	if (kind > sizeof(Storage)) {
	// 		if (sizeof(Storage) == 1) {
	// 			PyObject* bytes = Trait::CoerceToOneByte(obj);
	// 			if (bytes == NULL) {
	// 				return false;
	// 			}
	// 			bool res = AppendBytes(bytes);
	// 			Py_DECREF(bytes);
	// 			return res;
	// 		} else {
	// 			assert(0);
	// 		}
	// 	}

	// 	_maxchar |= PyUnicode_MAX_CHAR_VALUE(obj);

	// 	switch (kind) {
	// 		case PyUnicode_1BYTE_KIND:
	// 			return AppendString(PyUnicode_1BYTE_DATA(obj), 0, PyUnicode_GET_LENGTH(obj));
	// 		break;

	// 		case PyUnicode_2BYTE_KIND:
	// 			return AppendString(PyUnicode_2BYTE_DATA(obj), 0, PyUnicode_GET_LENGTH(obj));
	// 		break;

	// 		case PyUnicode_4BYTE_KIND:
	// 			return AppendString(PyUnicode_4BYTE_DATA(obj), 0, PyUnicode_GET_LENGTH(obj));
	// 		break;

	// 		default:
	// 			PyErr_BadInternalCall();
	// 			return false;
	// 		break;
	// 	}
	// }

	// inline bool AppendString(const char* str) {
	// 	assert(str != NULL);
	// 	return AppendString((const unsigned char*) str, 0, strlen(str));
	// }

	// template<typename Input>
	// inline bool AppendString(Input* str, size_t beginOffset, size_t endOffset) {
	// 	assert(sizeof(Input) <= sizeof(Storage));

	// 	size_t size = endOffset - beginOffset;
	// 	if (size == 0) {
	// 		return true;
	// 	}
	// 	Storage* cursor = _memory.EnsureSize(size);
	// 	if (!cursor) {
	// 		return false;
	// 	}
	// 	CopyStrBytes(cursor, str + beginOffset, size);
	// 	_memory._cursor += size;
	// 	// TODO: Trait::Copy ...
	// 	// if (!_memory.EnsureSize(Trait::RequiredSize(size))) {
	// 	// 	return false;
	// 	// }
	// 	// while (size--) {
	// 	// 	Trait::AppendChar(_memory._cursor, *(str++));
	// 	// }
	// 	return true;
	// }

	// inline bool AppendBytes(PyObject* obj) {
	// 	assert(obj != NULL);
	// 	assert(PyBytes_CheckExact(obj));
	// 	return AppendString((unsigned char*) PyBytes_AS_STRING(obj), 0, PyBytes_GET_SIZE(obj));
	// }

	inline _Mem& Memory() { return _memory; };

	inline PyObject* ToPython() {
		assert(_memory._cursor <= _memory._end);
		return Trait::NewFromSizeAndData(_memory.Begin(), _memory.UsedSize(), _maxchar);
	}
private:
	_Mem _memory;
	size_t _maxchar;
};


// template<typename _Trait, typename _Mem>
// class _ChunkBuilder {
// public:
// 	using Trait = _Trait;
// };





template<typename DT, int size>
using StringBuilder = _StringBuilder<_StringTraits, _Memory<DT, size>>;

template<int size>
using AsciiBuilder = StringBuilder<char, size>;

template<int size>
using UnicodeBuilder = StringBuilder<Py_UCS4, size>;



using Utf8Bytes = _ByteTraits<char, _Encoding::_Utf8>;
using RawBytes = _ByteTraits<char, _Encoding::_Raw>;

template<typename _Trait, int size>
using BytesBuilder = _StringBuilder<_Trait, _Memory<char, size>>;

template<int size>
using RawBytesBuilder = BytesBuilder<RawBytes, size>;

template<int size>
using Utf8BytesBuilder = BytesBuilder<Utf8Bytes, size>;


} // end namespace Yapic
#endif /* E5B23D08_9133_C8FD_140D_4CE32366FBF9 */
