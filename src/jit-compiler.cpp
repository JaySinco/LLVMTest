#include "jit-compiler.h"
#include "utils.h"

JITCompiler::JITCompiler()
    : resolver(llvm::orc::createLegacyLookupResolver(
          es, [this](llvm::StringRef name) { return this->findMangledSymbol(std::string(name)); },
          [](llvm::Error err) { llvm::cantFail(std::move(err), "failed to lookup"); })),
      tm(llvm::EngineBuilder{}.selectTarget()),
      dl(tm->createDataLayout()),
      objl(
          llvm::AcknowledgeORCv1Deprecation, es,
          [this](llvm::orc::VModuleKey) {
              return ObjLayerT::Resources{std::make_shared<llvm::SectionMemoryManager>(), resolver};
          }),
      cmpl(llvm::AcknowledgeORCv1Deprecation, objl, llvm::orc::SimpleCompiler(*tm))
{
    llvm::sys::DynamicLibrary::LoadLibraryPermanently(nullptr);
}

llvm::TargetMachine &JITCompiler::getTargetMachine() { return *tm; }

llvm::orc::VModuleKey JITCompiler::addModule(std::unique_ptr<llvm::Module> module_)
{
    auto key = this->es.allocateVModule();
    llvm::cantFail(cmpl.addModule(key, std::move(module_)));
    this->mkeys.push_back(key);
    return key;
}

void JITCompiler::removeModule(llvm::orc::VModuleKey key)
{
    this->mkeys.erase(llvm::find(this->mkeys, key));
    llvm::cantFail(cmpl.removeModule(key));
}

llvm::JITSymbol JITCompiler::findSymbol(const std::string &name)
{
    return this->findMangledSymbol(this->mangle(name));
}

std::string JITCompiler::mangle(const std::string &name)
{
    std::string mangledName;
    {
        llvm::raw_string_ostream mangledNameStream(mangledName);
        llvm::Mangler::getNameWithPrefix(mangledNameStream, name, this->dl);
    }
    return mangledName;
}

llvm::JITSymbol JITCompiler::findMangledSymbol(const std::string &name)
{
    const bool exportedSymbolsOnly = false;
    for (auto key: llvm::make_range(this->mkeys.rbegin(), this->mkeys.rend())) {
        if (auto sym = this->cmpl.findSymbolIn(key, name, exportedSymbolsOnly)) {
            return sym;
        }
    }
    if (auto symAddr = llvm::RTDyldMemoryManager::getSymbolAddressInProcess(name)) {
        return llvm::JITSymbol(symAddr, llvm::JITSymbolFlags::Exported);
    }
    if (name.length() > 2 && name[0] == '_') {
        if (auto symAddr = llvm::RTDyldMemoryManager::getSymbolAddressInProcess(name.substr(1))) {
            return llvm::JITSymbol(symAddr, llvm::JITSymbolFlags::Exported);
        }
    }
    return nullptr;
}
