
#include <yapic/module.hpp>
#include <yapic/pyptr.hpp>

class PyPtrModule;
using Yapic::PyPtr;

#define PyPtrModule_ASSERT(x) \
    if (!(x)) { \
        PyErr_Format(PyExc_AssertionError, "Assert failed: %s", #x); \
        return NULL; \
    }


class PyPtrModule : public Yapic::Module<PyPtrModule> {
public:
	static constexpr const char* __name__ = "yapic.core.test._pyptr";

	static PyObject* test(PyObject* module) {
		PyPtr<> obj(PyDict_New());
        if (!obj) {
            return NULL;
        }
        PyPtrModule_ASSERT(Py_REFCNT(obj) == 1);

        PyPtr<> ob2 = PyDict_New();
        if (!ob2) {
            return NULL;
        }
        PyPtrModule_ASSERT(Py_REFCNT(ob2) == 1);
        PyObject* stolen = ob2.Steal();
        PyPtrModule_ASSERT(ob2.IsNull());
        PyPtrModule_ASSERT(stolen != NULL);
        PyPtrModule_ASSERT(Py_REFCNT(stolen) == 1);
        Py_DECREF(stolen);

        ob2 = obj;
        PyPtrModule_ASSERT(obj.IsValid());
        PyPtrModule_ASSERT(ob2.IsValid());
        PyPtrModule_ASSERT(Py_REFCNT(obj) == 2);
        PyPtrModule_ASSERT(Py_REFCNT(ob2) == 2);
        stolen = ob2.Steal();
        PyPtrModule_ASSERT(Py_REFCNT(stolen) == 2);
        obj = NULL;
        PyPtrModule_ASSERT(Py_REFCNT(stolen) == 1);


        Py_RETURN_NONE;
	}

	Yapic_METHODS_BEGIN
		Yapic_Method(test, METH_NOARGS, NULL)
	Yapic_METHODS_END
};


PyMODINIT_FUNC PyInit__pyptr(void) {
	return PyPtrModule::Create();
}
