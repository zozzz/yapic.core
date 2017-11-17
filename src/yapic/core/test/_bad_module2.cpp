#include <yapic/module.hpp>


class BadModule2 : public Yapic::Module<BadModule2> {
public:
	static constexpr const char* __name__ = "yapic.core.test._bad_module2";

	Yapic::ModuleVar<BadModule2> bad;
	Yapic::ModuleExc<BadModule2> exc;

	static inline int __init__(PyObject* module, Self* state) {
		state->exc.Define("BadError");
		state->exc.Raise("Somthing is wrong");
		state->bad = (PyObject*) NULL;
		return 0;
	}
};

PyMODINIT_FUNC PyInit__bad_module2(void) {
	return BadModule2::Create();
}