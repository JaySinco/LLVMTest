#include "utils.h"
#include "codegen.h"
#include <boost/algorithm/string/trim.hpp>

int main(int argc, char **argv)
{
    FLAGS_logtostderr = 1;
    FLAGS_minloglevel = 0;
    gflags::ParseCommandLineFlags(&argc, &argv, true);
    google::InitGoogleLogging(argv[0]);

    llvm::InitializeNativeTarget();
    llvm::InitializeNativeTargetAsmPrinter();
    llvm::InitializeNativeTargetAsmParser();

    auto codegen = std::make_unique<CodeGen>();
    if (!codegen->eval(utils::readFile(L"sample/input.txt").second)) return -1;
    if (!codegen->evalModule(utils::readFile(L"sample/handwriting.ll").second)) return -1;
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
