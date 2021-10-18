#include "../utils.h"
#include "codegen.h"
#include <boost/algorithm/string/trim.hpp>

int main(int argc, char **argv)
{
    llvm::InitializeNativeTarget();
    llvm::InitializeNativeTargetAsmPrinter();
    llvm::InitializeNativeTargetAsmParser();

    auto codegen = std::make_unique<CodeGen>();
    auto input = utils::readFile(utils::getResAbs(L"calc/input.txt"));
    if (!codegen->eval(input.second)) return -1;
    auto handwriting = utils::readFile(utils::getResAbs(L"calc/handwriting.ll"));
    if (!codegen->evalModule(handwriting.second)) return -1;

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
