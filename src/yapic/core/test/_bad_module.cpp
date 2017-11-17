#include <yapic/module.hpp>


class BadModule : public Yapic::Module<BadModule> {
public:
	static constexpr const char* __name__ = "yapic.core.test._bad_module";

	Yapic::ModuleVar<BadModule> bad;

	static inline int __init__(PyObject* module, Self* state) {
		state->bad = (PyObject*) NULL;
		return 0;
	}
};

PyMODINIT_FUNC PyInit__bad_module(void) {
	return BadModule::Create();
}