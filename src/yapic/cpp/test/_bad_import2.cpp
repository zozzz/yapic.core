#include <yapic/module.hpp>


class BadImport2 : public Yapic::Module<BadImport2> {
public:
	static constexpr const char* __name__ = "yapic.cpp.test._bad_import2";

	Yapic::ModuleRef<BadImport2> bad;

	static inline int __init__(PyObject* module, Self* state) {
		state->bad.Import("os", "wrong");
		return 0;
	}
};

PyMODINIT_FUNC PyInit__bad_import2(void) {
	return BadImport2::Create();
}