#ifndef D57C95BE_1134_1461_11A3_990B2EDF5491
#define D57C95BE_1134_1461_11A3_990B2EDF5491

#include <Python.h>


namespace Yapic {

    class Typing {
        public:
            Typing() {
                memset(this, 0, sizeof(this));
            }

            ~Typing() {
                Py_CLEAR(_typing);
                Py_CLEAR(Generic);
                Py_CLEAR(GenericAlias);
            }

            bool Init(PyObject* typingModule) {
                _typing = typingModule;
                Py_INCREF(typingModule);

                return true;
            }

            inline bool IsGeneric(PyObject* object) {
                assert(object != NULL);
                assert(Generic != NULL);
                return false;
            }

            inline bool IsGenericAlias(PyObject* object) {
                assert(object != NULL);
                assert(GenericAlias != NULL);
                return false;
            }

        private:
            PyObject* _typing;
            PyObject* Generic;
            PyObject* GenericAlias;
    };

}; /* end namespace Yapic */


#endif /* D57C95BE_1134_1461_11A3_990B2EDF5491 */
