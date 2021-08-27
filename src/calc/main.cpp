#include "../utils.h"
#include "codegen.h"
#include <boost/algorithm/string/trim.hpp>

int main(int argc, char **argv)
{
    llvm::InitializeNativeTarget();
    llvm::InitializeNativeTargetAsmPrinter();
    llvm::InitializeNativeTargetAsmParser();

    auto codegen = std::make_unique<CodeGen>();
    if (!codegen->eval(utils::readFile(L"resources/calc/input.txt").second)) return -1;
    if (!codegen->evalModule(utils::readFile(L"resources/calc/handwriting.ll").second)) return -1;
    while (true) {
        std::cout << ">>> ";
        std::string line;
        if (!std::getline(std::cin, line)) {
            break;
        }
        boost::trim_right(line);
        if (line.size() <= 0) {
            continue;
        }
        if (line.back() != ';') {
            line.push_back(';');
        }
        codegen->eval(line);
    }
    return 0;
}
