#pragma once
#include "prec.h"

class JITCompiler
{
public:
    using ObjLayerT = llvm::orc::LegacyRTDyldObjectLinkingLayer;
    using CompileLayerT = llvm::orc::LegacyIRCompileLayer<ObjLayerT, llvm::orc::SimpleCompiler>;

    JITCompiler();
    llvm::TargetMachine &getTargetMachine();
    llvm::orc::VModuleKey addModule(std::unique_ptr<llvm::Module> module_);
    void removeModule(llvm::orc::VModuleKey key);
    llvm::JITSymbol findSymbol(const std::string &name);

private:
    std::string mangle(const std::string &name);
    llvm::JITSymbol findMangledSymbol(const std::string &name);

    llvm::orc::ExecutionSession es;
    std::shared_ptr<llvm::orc::SymbolResolver> resolver;
    std::unique_ptr<llvm::TargetMachine> tm;
    const llvm::DataLayout dl;
    ObjLayerT objl;
    CompileLayerT cmpl;
    std::vector<llvm::orc::VModuleKey> mkeys;
};
