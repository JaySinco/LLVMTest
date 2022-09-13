#include "mcts.h"
#include "train.h"
#include <iostream>

const char* usage =
    "Usage: gomoku <command>\n\n"
    "Commands:\n"
    "   train      Train model from scatch or parameter file\n"
    "   play       Play with trained model\n"
    "   benchmark  Benchmark between two mcts deep players\n\n";

const char* train_usage =
    "Usage: gomoku train <net>\n\n"
    "Options:\n"
    "   <net>      verno of network(must >= 0), which is the suffix of parameter file basename\n"
    "              if equal to zero, train from scratch; otherwise continue to train model from "
    "last check-point\n\n";

const char* play_usage =
    "Usage: gomoku play <color> <net> [itermax]\n\n"
    "Options:\n"
    "   <color>    '0' if human take first hand, '1' otherwise\n"
    "              specially '-1' means let computer selfplay\n"
    "   <net>      verno of network(must > 0), which is the suffix of parameter file basename\n"
    "   [itermax]  itermax for mcts deep player\n"
    "              if not given, default from global configure\n\n";

const char* benchmark_usage =
    "Usage: gomoku benchmark <net1> <net2> [itermax]\n\n"
    "Options:\n"
    "   <net1>     verno of network to compare(must > 0), which is the suffix of parameter file "
    "basename\n"
    "   <net2>     see above\n"
    "   [itermax]  itermax for mcts deep player\n"
    "              if not given, default from global configure\n\n";

std::random_device g_random_device;
std::mt19937 g_random_engine(g_random_device());

int main(int argc, char* argv[])
{
    if (argc > 1 && strcmp(argv[1], "train") == 0) {
        if (argc == 3) {
            long long verno = std::atoi(argv[2]);
            std::shared_ptr<FIRNet> net = std::make_shared<FIRNet>(verno);
            train(net);
            return 0;
        }
        std::cout << train_usage;
        return -1;
    }
    if (argc > 1 && strcmp(argv[1], "play") == 0) {
        if (argc == 4 || argc == 5) {
            int itermax = TRAIN_DEEP_ITERMAX;
            if (argc == 5) itermax = std::atoi(argv[4]);
            std::cout << "mcts_itermax=" << itermax << std::endl;
            long long verno = std::atoi(argv[3]);
            auto net = std::make_shared<FIRNet>(verno);
            auto p1 = MCTSDeepPlayer(net, itermax, C_PUCT);
            if (strcmp(argv[2], "0") == 0) {
                auto p0 = HumanPlayer("human");
                play(p0, p1, false);
            } else if (strcmp(argv[2], "1") == 0) {
                auto p0 = HumanPlayer("human");
                play(p1, p0, false);
            } else if (strcmp(argv[2], "-1") == 0) {
                auto p0 = MCTSDeepPlayer(net, itermax, C_PUCT);
                play(p0, p1, false);
            }
            return 0;
        }
        std::cout << play_usage;
        return -1;
    }
    if (argc > 1 && strcmp(argv[1], "benchmark") == 0) {
        if (argc == 4 || argc == 5) {
            int itermax = TRAIN_DEEP_ITERMAX;
            if (argc == 5) itermax = std::atoi(argv[4]);
            std::cout << "mcts_itermax=" << itermax << std::endl;
            long long verno1 = std::atoi(argv[2]);
            auto net1 = std::make_shared<FIRNet>(verno1);
            long long verno2 = std::atoi(argv[3]);
            auto net2 = std::make_shared<FIRNet>(verno2);
            auto p1 = MCTSDeepPlayer(net1, itermax, C_PUCT);
            auto p2 = MCTSDeepPlayer(net2, itermax, C_PUCT);
            benchmark(p1, p2, 10, false);
            return 0;
        }
        std::cout << benchmark_usage;
        return -1;
    }
    std::cout << usage;
    return -1;
}