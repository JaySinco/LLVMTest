#include "codegen.h"
#include "utils.h"
#include "antlr4-runtime.h"
#include "gen-cpp/TLexer.h"
#include "gen-cpp/TParser.h"

#define DELIMITER \
    "-------------------------------------------------------------------------------\n"

CodeGen::CodeGen() { this->initModuleAndPass(); }

void CodeGen::initModuleAndPass()
{
    static int id = 0;
    this->module_ = std::make_unique<llvm::Module>("__module_{}"_format(id++), this->llctx);
    this->module_->setDataLayout(this->jit.getTargetMachine().createDataLayout());
    this->passManager = std::make_unique<llvm::legacy::FunctionPassManager>(module_.get());
    // this->passManager->add(llvm::createInstructionCombiningPass());
    // this->passManager->add(llvm::createReassociatePass());
    // this->passManager->add(llvm::createGVNPass());
    // this->passManager->add(llvm::createCFGSimplificationPass());
    this->passManager->doInitialization();
}

llvm::Value *CodeGen::generate(TParser::LiteralExpressionContext *ctx)
{
    double num = std::stod(ctx->Number()->getText());
    return llvm::ConstantFP::get(this->llctx, llvm::APFloat(num));
}

llvm::Value *CodeGen::generate(TParser::IdExpressionContext *ctx)
{
    std::string id = ctx->Identifier()->getText();
    if (this->namedValues.find(id) == this->namedValues.end()) {
        std::cerr << "unknown variable: " << id << std::endl;
        return nullptr;
    }
    return this->namedValues.at(id);
}

llvm::Value *CodeGen::generate(TParser::ExpressionContext *ctx)
{
    if (auto ptr = dynamic_cast<TParser::CallExpressionContext *>(ctx)) {
        return this->generate(ptr);
    } else if (auto ptr = dynamic_cast<TParser::ParenthesesExpressionContext *>(ctx)) {
        return this->generate(ptr);
    } else if (auto ptr = dynamic_cast<TParser::AdditiveExpressionContext *>(ctx)) {
        return this->generate(ptr);
    } else if (auto ptr = dynamic_cast<TParser::LiteralExpressionContext *>(ctx)) {
        return this->generate(ptr);
    } else if (auto ptr = dynamic_cast<TParser::IdExpressionContext *>(ctx)) {
        return this->generate(ptr);
    } else if (auto ptr = dynamic_cast<TParser::RelationalExpressionContext *>(ctx)) {
        return this->generate(ptr);
    } else if (auto ptr = dynamic_cast<TParser::ConditionalExpressionContext *>(ctx)) {
        return this->generate(ptr);
    } else if (auto ptr = dynamic_cast<TParser::EqualityExpressionContext *>(ctx)) {
        return this->generate(ptr);
    } else if (auto ptr = dynamic_cast<TParser::MultiplicativeExpressionContext *>(ctx)) {
        return this->generate(ptr);
    } else {
        std::cerr << "failed to dynamic cast expression: " << ctx->getText() << std::endl;
        return nullptr;
    }
}

llvm::Value *CodeGen::generate(TParser::RelationalExpressionContext *ctx)
{
    llvm::Value *l = this->generate(ctx->expression(0));
    if (l == nullptr) return nullptr;
    llvm::Value *r = this->generate(ctx->expression(1));
    if (r == nullptr) return nullptr;
    llvm::Value *o = nullptr;
    switch (ctx->op->getType()) {
        case TLexer::Less:
            o = this->builder.CreateFCmpULT(l, r);
            break;
        case TLexer::Greater:
            o = this->builder.CreateFCmpUGT(l, r);
            break;
        case TLexer::LessEqual:
            o = this->builder.CreateFCmpULE(l, r);
            break;
        case TLexer::GreaterEqual:
            o = this->builder.CreateFCmpUGE(l, r);
            break;
        default:
            std::cerr << "unknow op: " << ctx->op->getText() << std::endl;
            return nullptr;
    }
    return this->builder.CreateUIToFP(o, llvm::Type::getDoubleTy(this->llctx), "bool");
}

llvm::Value *CodeGen::generate(TParser::EqualityExpressionContext *ctx)
{
    llvm::Value *l = this->generate(ctx->expression(0));
    if (l == nullptr) return nullptr;
    llvm::Value *r = this->generate(ctx->expression(1));
    if (r == nullptr) return nullptr;
    llvm::Value *o = nullptr;
    switch (ctx->op->getType()) {
        case TLexer::Equal:
            o = this->builder.CreateFCmpUEQ(l, r);
            break;
        case TLexer::NotEqual:
            o = this->builder.CreateFCmpUNE(l, r);
            break;
        default:
            std::cerr << "unknow op: " << ctx->op->getText() << std::endl;
            return nullptr;
    }
    return this->builder.CreateUIToFP(o, llvm::Type::getDoubleTy(this->llctx), "bool");
}

llvm::Value *CodeGen::generate(TParser::MultiplicativeExpressionContext *ctx)
{
    llvm::Value *l = this->generate(ctx->expression(0));
    if (l == nullptr) return nullptr;
    llvm::Value *r = this->generate(ctx->expression(1));
    if (r == nullptr) return nullptr;
    switch (ctx->op->getType()) {
        case TLexer::Star:
            return this->builder.CreateFMul(l, r);
        case TLexer::Div:
            return this->builder.CreateFDiv(l, r);
        default:
            std::cerr << "unknow op: " << ctx->op->getText() << std::endl;
            return nullptr;
    }
}

llvm::Value *CodeGen::generate(TParser::AdditiveExpressionContext *ctx)
{
    llvm::Value *l = this->generate(ctx->expression(0));
    if (l == nullptr) return nullptr;
    llvm::Value *r = this->generate(ctx->expression(1));
    if (r == nullptr) return nullptr;
    switch (ctx->op->getType()) {
        case TLexer::Plus:
            return this->builder.CreateFAdd(l, r);
        case TLexer::Minus:
            return this->builder.CreateFSub(l, r);
        default:
            std::cerr << "unknow op: " << ctx->op->getText() << std::endl;
            return nullptr;
    }
}

llvm::Value *CodeGen::generate(TParser::ParenthesesExpressionContext *ctx)
{
    return this->generate(ctx->expression());
}

llvm::Value *CodeGen::generate(TParser::ConditionalExpressionContext *ctx) { return nullptr; }

llvm::Function *CodeGen::getFunction(const std::string &name)
{
    if (auto *func = this->module_->getFunction(name)) {
        return func;
    }
    if (this->signatures.find(name) == this->signatures.end()) {
        std::cerr << "unknow function signature: " << name << std::endl;
        return nullptr;
    }
    auto &args = this->signatures.at(name);
    std::vector<llvm::Type *> paramType(args.size(), llvm::Type::getDoubleTy(this->llctx));
    llvm::FunctionType *funcType =
        llvm::FunctionType::get(llvm::Type::getDoubleTy(this->llctx), paramType, false);
    llvm::Function *func = llvm::Function::Create(funcType, llvm::Function::ExternalLinkage, name,
                                                  this->module_.get());
    for (int i = 0; i < args.size(); ++i) {
        func->getArg(i)->setName(args.at(i));
    }
    return func;
}

llvm::Value *CodeGen::generate(TParser::CallExpressionContext *ctx)
{
    std::string name = ctx->Identifier()->getText();
    if (this->signatures.find(name) == this->signatures.end()) {
        std::cerr << "unknown function: " << name << std::endl;
        return nullptr;
    }
    auto &args = this->signatures[name];
    std::vector<TParser::ExpressionContext *> exprList;
    if (ctx->expressionList() != nullptr) {
        exprList = ctx->expressionList()->expression();
    }
    if (args.size() != exprList.size()) {
        std::cerr << "{} expected {} args, got {}"_format(name, args.size(), exprList.size())
                  << std::endl;
        return nullptr;
    }
    std::vector<llvm::Value *> argv;
    for (auto expr: exprList) {
        llvm::Value *v = this->generate(expr);
        if (v == nullptr) {
            return nullptr;
        }
        argv.push_back(v);
    }
    llvm::Function *func = this->getFunction(name);
    if (func == nullptr) {
        return nullptr;
    }
    return this->builder.CreateCall(func, argv);
}

void CodeGen::printModule()
{
    std::string str;
    llvm::raw_string_ostream ss(str);
    this->module_->print(ss, nullptr);
    std::cout << str << DELIMITER;
}

bool CodeGen::writeFunctionBody(llvm::Function *func, TParser::ExpressionContext *expr)
{
    llvm::BasicBlock *block = llvm::BasicBlock::Create(this->llctx, "entry", func);
    this->builder.SetInsertPoint(block);
    this->namedValues.clear();
    for (auto &arg: func->args()) {
        this->namedValues[arg.getName().str()] = &arg;
    }
    std::string funcName = func->getName().str();
    llvm::Value *retval = this->generate(expr);
    if (retval == nullptr) {
        std::cerr << "failed to generate function {}, erase it"_format(funcName) << std::endl;
        func->eraseFromParent();
        this->signatures.erase(funcName);
        return false;
    }
    this->builder.CreateRet(retval);
    std::string errmsg;
    llvm::raw_string_ostream ss(errmsg);
    if (llvm::verifyFunction(*func, &ss)) {
        this->printModule();
        std::cerr << "failed to verify function " << func->getName().str()
                  << ", erase it: " << errmsg << std::endl;
        func->eraseFromParent();
        this->signatures.erase(funcName);
        return false;
    }
    this->passManager->run(*func);
    this->printModule();
    return true;
}

llvm::Function *CodeGen::generate(TParser::FunctionSignatureContext *ctx)
{
    std::string funcName = ctx->Identifier()->getText();
    if (this->signatures.find(funcName) != this->signatures.end()) {
        std::cerr << "function already exist: " << funcName << std::endl;
        return nullptr;
    }
    std::vector<antlr4::tree::TerminalNode *> idList;
    if (ctx->argumentList() != nullptr) {
        idList = ctx->argumentList()->Identifier();
    }
    auto &args = this->signatures[funcName];
    for (int i = 0; i < idList.size(); ++i) {
        args.push_back(idList.at(i)->getText());
    }
    return this->getFunction(funcName);
}

void CodeGen::generate(TParser::ExternalFunctionContext *ctx)
{
    this->generate(ctx->functionSignature());
    return;
}

void CodeGen::generate(TParser::FunctionDefinitionContext *ctx)
{
    std::string funcName = ctx->functionSignature()->Identifier()->getText();
    llvm::Function *func = this->generate(ctx->functionSignature());
    if (func == nullptr) {
        return;
    }
    if (!this->writeFunctionBody(func, ctx->expression())) {
        return;
    }
    this->jit.addModule(std::move(this->module_));
    this->initModuleAndPass();
    return;
}

void CodeGen::generate(TParser::ExpressionStatementContext *ctx)
{
    std::vector<llvm::Type *> paramType;
    llvm::FunctionType *funcType =
        llvm::FunctionType::get(llvm::Type::getDoubleTy(this->llctx), paramType, false);
    const char *funcName = "__anonymous";
    llvm::Function *func = llvm::Function::Create(funcType, llvm::Function::ExternalLinkage,
                                                  funcName, this->module_.get());

    if (!this->writeFunctionBody(func, ctx->expression())) {
        return;
    }
    auto key = this->jit.addModule(std::move(this->module_));
    this->initModuleAndPass();
    if (!key) {
        return;
    }
    std::shared_ptr<void> moduleGuard(nullptr, [&](void *) { this->jit.removeModule(*key); });
    auto symbol = this->jit.findSymbol(funcName);
    auto addr = symbol.getAddress();
    if (!addr) {
        std::cerr << "failed to get address: " << llvm::toString(addr.takeError()) << std::endl;
        return;
    }
    auto *fp = (double (*)())(intptr_t)*addr;
    if (fp == nullptr) {
        std::cerr << "symbol address is NULL" << std::endl;
        return;
    }
    std::cout << fp() << std::endl;
    return;
}

void CodeGen::generate(TParser::StatementContext *ctx)
{
    if (auto ptr = dynamic_cast<TParser::ExternalFunctionContext *>(ctx)) {
        this->generate(ptr);
    } else if (auto ptr = dynamic_cast<TParser::FunctionDefinitionContext *>(ctx)) {
        this->generate(ptr);
    } else if (auto ptr = dynamic_cast<TParser::ExpressionStatementContext *>(ctx)) {
        this->generate(ptr);
    } else {
        std::cerr << "failed to dynamic cast statement: " << ctx->getText() << std::endl;
    }
}

void CodeGen::generate(TParser::ProgramContext *ctx)
{
    std::vector<TParser::StatementContext *> statList = ctx->statement();
    for (auto &stat: statList) {
        this->generate(stat);
    }
    return;
}

class ErrorListener: public antlr4::BaseErrorListener
{
    virtual void syntaxError(antlr4::Recognizer *recognizer, antlr4::Token *offendingSymbol,
                             size_t line, size_t charPositionInLine, const std::string &msg,
                             std::exception_ptr e) override
    {
        throw std::runtime_error("line {}:{} {}"_format(line, charPositionInLine, msg));
    }
};

void CodeGen::eval(const std::string &code)
{
    try {
        antlr4::ANTLRInputStream ais(code);
        TLexer lexer(&ais);
        lexer.removeErrorListeners();
        ErrorListener lerr;
        lexer.addErrorListener(&lerr);
        antlr4::CommonTokenStream tokens(&lexer);
        TParser parser(&tokens);
        parser.removeErrorListeners();
        parser.addErrorListener(&lerr);
        auto tree = parser.program();
        std::cout << tree->toStringTree(&parser, true) << std::endl << DELIMITER;
        this->generate(tree);
    } catch (std::exception &e) {
        std::cerr << e.what() << std::endl;
    }
}
