
#include <yapic/module.hpp>
#include <yapic/string-builder.hpp>

class Module;

using ModuleVar = Yapic::ModuleVar<Module>;
using ModuleExc = Yapic::ModuleExc<Module>;
using AsciiBuilder = Yapic::AsciiBuilder<1>;
using UnicodeBuilder = Yapic::UnicodeBuilder<1>;
using Utf8BytesBuilder = Yapic::Utf8BytesBuilder<1>;

class Module : public Yapic::Module<Module> {
public:
	static constexpr const char* __name__ = "yapic.cpp.test._string_builder";

	static PyObject* ascii_builder(PyObject* module, PyObject* str) {
		if (!str || !PyUnicode_CheckExact(str)) {
			PyErr_BadInternalCall();
			return NULL;
		}

		AsciiBuilder builder;
		size_t strSize = PyUnicode_GET_LENGTH(str);
		if (!builder.Memory().EnsureSize(strSize + 6)) {
			return NULL;
		}

		builder.AppendAscii('@');
		builder.AppendAscii('Y');
		builder.AppendAscii('@');
		if (!builder.AppendString(str)) {
			return NULL;
		}
		if (!builder.AppendString("@X@")) {
			return NULL;
		}
		return builder.ToPython();
	}

	static PyObject* unicode_builder(PyObject* module, PyObject* str) {
		if (!str || !PyUnicode_CheckExact(str)) {
			PyErr_BadInternalCall();
			return NULL;
		}

		UnicodeBuilder builder;
		size_t strSize = PyUnicode_GET_LENGTH(str);
		if (!builder.Memory().EnsureSize(strSize + 6)) {
			return NULL;
		}

		builder.AppendAscii('@');
		builder.AppendAscii('Y');
		builder.AppendAscii('@');
		if (!builder.AppendString(str)) {
			return NULL;
		}
		if (!builder.AppendString("@X@")) {
			return NULL;
		}
		return builder.ToPython();
	}

	static PyObject* bytes_builder(PyObject* module, PyObject* str) {
		printf("bytes_builder\n");
		if (!str || (!PyUnicode_CheckExact(str) && !PyBytes_CheckExact(str))) {
			PyErr_BadInternalCall();
			return NULL;
		}

		Utf8BytesBuilder builder;
		if (!builder.Memory().EnsureSize(6)) {
			return NULL;
		}

		printf("SIZE OK\n");

		builder.AppendAscii('@');
		builder.AppendAscii('Y');
		builder.AppendAscii('@');

		printf("ASCII OK\n");

		if (PyUnicode_CheckExact(str)) {
			if (!builder.AppendString(str)) {
				printf("STRING OK\n");
				return NULL;
			}
		} else if (PyBytes_CheckExact(str)) {
			if (!builder.AppendBytes(str)) {
				printf("BYTES OK\n");
				return NULL;
			}
		}

		if (!builder.AppendString("@X@")) {
			return NULL;
		}
		return builder.ToPython();
	}

	Yapic_METHODS_BEGIN
		Yapic_Method(ascii_builder, METH_O, NULL)
		Yapic_Method(unicode_builder, METH_O, NULL)
		Yapic_Method(bytes_builder, METH_O, NULL)
	Yapic_METHODS_END
};


PyMODINIT_FUNC PyInit__string_builder(void) {
	return Module::Create();
}