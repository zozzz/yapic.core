
#include "../src/module.hpp"

namespace Example {
	using Yapic::ModuleConst;

	class Module : public Yapic::Module<Module> {
		public:
			static constexpr const char* __name__ = "example";

			ModuleConst<Module> valami;
			ModuleConst<Module> Exc_Almafa;
			ModuleConst<Module> Exc_Almafa2;

			static bool __init__(PyObject* module, Module* state) {
				state->valami = "valami";
				state->Exc_Almafa.SetException("Almafa");
				state->Exc_Almafa2.SetException("Almafa2", state->Exc_Almafa);
			}
	};
}

