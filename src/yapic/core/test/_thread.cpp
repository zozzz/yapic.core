
#include <yapic/module.hpp>
#include <yapic/thread.hpp>

class ThreadModule;

using ModuleVar = Yapic::ModuleVar<ThreadModule>;
using ModuleExc = Yapic::ModuleExc<ThreadModule>;


class ThreadModule : public Yapic::Module<ThreadModule> {
public:
	static constexpr const char* __name__ = "yapic.core.test._thread";

	ModuleVar thread_safe;
	Yapic::Lock* lock;
	Yapic::RLock* rlock;

	static inline int __init__(PyObject* module, ThreadModule* state) {
		state->thread_safe = PyDict_New();

		state->lock = new Yapic::Lock();
		if (state->lock->IsNull()) {
			return -1;
		}

		state->rlock = new Yapic::RLock();
		if (state->rlock->IsNull()) {
			return -1;
		}
		return 0;
	}

	static inline int __clear__(PyObject* module) {
		Self* state = State(module);
		delete state->lock;
		delete state->rlock;
		return Super::__clear__(module);
	}

	static PyObject* set(PyObject* module, PyObject* args, PyObject* kwargs) {
		static char* kwnames[] = {"key", "value", NULL};
		ThreadModule* state = State(module);
		PyObject* key;
		PyObject* value;
		if (PyArg_ParseTupleAndKeywords(args, kwargs, "OO", kwnames, &key, &value)) {
			Yapic::Lock::Auto lock(state->lock);
			if (PyDict_SetItem(state->thread_safe, key, value) < 0) {
				return NULL;
			}
			Py_RETURN_NONE;
		} else {
			return NULL;
		}
	}

	static PyObject* get(PyObject* module, PyObject* key) {
		ThreadModule* state = State(module);
		Yapic::Lock::Auto lock(state->lock);
		PyObject* value = PyDict_GetItemWithError(state->thread_safe, key);
		if (value != NULL) {
			Py_INCREF(value);
		}
		return value;
	}





	static PyObject* rset_sub(PyObject* module, PyObject* args, PyObject* kwargs) {
		static char* kwnames[] = {"key", "value", NULL};
		ThreadModule* state = State(module);
		PyObject* key;
		PyObject* value;
		if (PyArg_ParseTupleAndKeywords(args, kwargs, "OO", kwnames, &key, &value)) {
			Yapic::RLock::Auto lock(state->rlock);
			if (PyDict_SetItem(state->thread_safe, key, value) < 0) {
				return NULL;
			}
			Py_RETURN_NONE;
		} else {
			return NULL;
		}
	}

	static PyObject* rget_sub(PyObject* module, PyObject* key) {
		ThreadModule* state = State(module);
		Yapic::RLock::Auto lock(state->rlock);
		PyObject* value = PyDict_GetItemWithError(state->thread_safe, key);
		if (value != NULL) {
			Py_INCREF(value);
		}
		return value;
	}


	static PyObject* rset(PyObject* module, PyObject* args, PyObject* kwargs) {
		ThreadModule* state = State(module);
		Yapic::RLock::Auto lock(state->rlock);
		return Self::rset_sub(module, args, kwargs);
	}

	static PyObject* rget(PyObject* module, PyObject* key) {
		ThreadModule* state = State(module);
		Yapic::RLock::Auto lock(state->rlock);
		return Self::rget_sub(module, key);
	}


	Yapic_METHODS_BEGIN
		Yapic_Method(set, METH_VARARGS | METH_KEYWORDS, "")
		Yapic_Method(get, METH_O, "")
		Yapic_Method(rset, METH_VARARGS | METH_KEYWORDS, "")
		Yapic_Method(rget, METH_O, "")
	Yapic_METHODS_END
};


PyMODINIT_FUNC PyInit__thread(void) {
	return ThreadModule::Create();
}
