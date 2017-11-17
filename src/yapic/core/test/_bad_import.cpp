#include <yapic/module.hpp>


class BadImport : public Yapic::Module<BadImport> {
public:
	static constexpr const char* __name__ = "yapic.core.test._bad_import";

	Yapic::ModuleRef<BadImport> bad;

	static inline int __init__(PyObject* module, Self* state) {
		state->bad.Import("something.wrong");
		return 0;
	}
};

PyMODINIT_FUNC PyInit__bad_import(void) {
	return BadImport::Create();
}