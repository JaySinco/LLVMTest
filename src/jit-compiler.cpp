#include "jit-compiler.h"
#include "utils.h"
#include <iostream>

JITCompiler::JITCompiler()
    : resolver(llvm::orc::createLegacyLookupResolver(
          es, [this](llvm::StringRef name) { return this->findMangledSymbol(std::string(name)); },
          [](llvm::Error err) {
              std::cerr << "failed to lookup symbol: " << llvm::toString(std::move(err))
                        << std::endl;
          })),
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

std::optional<llvm::orc::VModuleKey> JITCompiler::addModule(std::unique_ptr<llvm::Module> module_)
{
    auto key = this->es.allocateVModule();
    if (auto err = cmpl.addModule(key, std::move(module_))) {
        std::cerr << "failed to add module: " << llvm::toString(std::move(err)) << std::endl;
        return {};
    }
    this->mkeys.push_back(key);
    return key;
}

bool JITCompiler::removeModule(llvm::orc::VModuleKey key)
{
    auto it = llvm::find(this->mkeys, key);
    if (it == this->mkeys.end()) {
        return false;
    }
    this->mkeys.erase(it);
    if (auto err = cmpl.removeModule(key)) {
        std::cerr << "failed to remove module: " << llvm::toString(std::move(err)) << std::endl;
        return false;
    }
    return true;
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
