#ifndef M113C3DD_7133_EC86_1330_6B1A54E57AEE
#define M113C3DD_7133_EC86_1330_6B1A54E57AEE


// usage: YAPIC_MODULE_INIT("package", "module", Module)
#define YAPIC_MODULE_INIT(__package, __name, __module) \
	PyMODINIT_FUNC PyInit_ ## __name(void) { \
		return __module.Init(__package, __name); \
	}


namespace Yapic {

/**
 * usage:
 *
 * class ModuleImpl {
 * public:
 *     ModuleImpl(PyModuleDef* def, PyObject* module) { ... }
 * };
 *
 * extern const Yapic::Module<ModuleImpl> Module(
 * "Docstring..."
 * );
 */
template<typename Impl>
class Module {
public:
	Module(): Module(NULL) { }
	Module(const char* doc): def{PyModuleDef_HEAD_INIT} {
		this->def.m_doc = doc;
		this->def.m_methods = Yapic_CallOptionalMethod(Impl, Methods);
		// this->def.m_size = sizeof(Impl);
	}

	inline int Init(const char* package, const char* name) {
		char* qname;
		int nl = strlen(name);
		int pl = 0;
		if (package && (pl = strlen(package)) > 0) {
			qname = PyMem_Malloc((pl + nl + 2) * sizeof(char));
			if (qname == NULL) { PyErr_NoMemory(); return -1; }
			memcpy(qname, package, pl * sizeof(char));
			qname[pl] = '.';
			memcpy(qname + pl + 1, name, nl * sizeof(char));
			qname[pl + nl + 1] = 0;
		} else {
			qname = PyMem_Malloc((nl + 1) + sizeof(char));
			if (qname == NULL) { PyErr_NoMemory(); return -1; }
			memcpy(qname, name, nl * sizeof(char));
		}
		this->def.m_name = qname;

		this->module = PyModule_Create(const_cast<PyModuleDef*>(this->def));
		if (this->module == NULL) {
			return -1;
		}

		try {
			this->state = new Impl(&this->def, this->module);
		} catch (...) {
			Py_CLEAR(this->module);
			return -1;
		}
		return 0;
	}

	inline const PyModuleDef* Definition() const { return &this->def; }
	inline Impl* State() const { return &this->state; }

protected:
	PyModuleDef def;
	Impl* state;
	PyObject* module;
};


} /* end namespace Yapic */

#endif /* M113C3DD_7133_EC86_1330_6B1A54E57AEE */
