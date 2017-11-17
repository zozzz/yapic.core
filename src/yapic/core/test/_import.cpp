#include <yapic/module.hpp>


class ImportModule : public Yapic::Module<ImportModule> {
public:
	static constexpr const char* __name__ = "yapic.core.test._import";

	Yapic::ModuleVar<ImportModule> str_join;
	Yapic::ModuleRef<ImportModule> path;

	static inline int __init__(PyObject* module, Self* state) {
		state->str_join = "join";
		state->path.Import("os", "path");
		return 0;
	}

	static inline PyObject* path_join(PyObject* module, PyObject* args) {
		Self* state = Self::State(module);
		PyObject* join = PyObject_GetAttr(state->path, state->str_join);
		if (join == NULL) {
			return NULL;
		}
		PyObject* res = PyObject_Call(join, args, NULL);
		Py_DECREF(join);
		return res;
	}

	Yapic_METHODS_BEGIN
		Yapic_Method(path_join, METH_VARARGS, "")
	Yapic_METHODS_END
};

PyMODINIT_FUNC PyInit__import(void) {
	return ImportModule::Create();
}