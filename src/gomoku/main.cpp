#include "mcts.h"
#include "train.h"
#include <argparse/argparse.hpp>
#include <iostream>

std::random_device g_random_device;
std::mt19937 g_random_engine(g_random_device());

void runCmdTrain(long long verno)
{
    spdlog::info("verno={}", verno);
    auto net = std::make_shared<FIRNet>(verno);
    train(net);
}

void runCmdPlay(int color, long long verno, int itermax)
{
    spdlog::info("color={}, verno={}, itermax={}", color, verno, itermax);
    auto net = std::make_shared<FIRNet>(verno);
    auto p1 = MCTSDeepPlayer(net, itermax, kCpuct);
    if (color == 0) {
        auto p0 = HumanPlayer("human");
        play(p0, p1, false);
    } else if (color == 1) {
        auto p0 = HumanPlayer("human");
        play(p1, p0, false);
    } else if (color == -1) {
        auto p0 = MCTSDeepPlayer(net, itermax, kCpuct);
        play(p0, p1, false);
    }
}

void runCmdBenchmark(long long verno1, long long verno2, int itermax)
{
    spdlog::info("verno1={}, verno2={}, itermax={}", verno1, verno2, itermax);
    auto net1 = std::make_shared<FIRNet>(verno1);
    auto net2 = std::make_shared<FIRNet>(verno2);
    auto p1 = MCTSDeepPlayer(net1, itermax, kCpuct);
    auto p2 = MCTSDeepPlayer(net2, itermax, kCpuct);
    benchmark(p1, p2, 10, false);
}

int main(int argc, char* argv[])
{
    TRY_;
    argparse::ArgumentParser prog("gomoku");

    // gomoku train
    argparse::ArgumentParser train_cmd("train");
    train_cmd.add_description("train model from scatch or checkpoint");
    train_cmd.add_argument("verno")
        .help("verno of checkpoint; 0 to train from scratch")
        .scan<'i', long long>();
    prog.add_subparser(train_cmd);

    // gomoku play
    argparse::ArgumentParser play_cmd("play");
    play_cmd.add_description("play with trained model");
    play_cmd.add_argument("color")
        .help("first hand color; human(0), computer(1), selfplay(-1)")
        .scan<'i', int>();
    play_cmd.add_argument("verno").help("verno of checkpoint").scan<'i', long long>();
    play_cmd.add_argument("itermax")
        .help("itermax for mcts deep player")
        .scan<'i', int>()
        .default_value(kTrainDeepItermax);
    prog.add_subparser(play_cmd);

    // gomoku benchmark
    argparse::ArgumentParser benchmark_cmd("benchmark");
    benchmark_cmd.add_description("benchmark between two mcts deep players");
    benchmark_cmd.add_argument("verno1")
        .help("verno of checkpoint to compare")
        .scan<'i', long long>();
    benchmark_cmd.add_argument("verno2").help("see above").scan<'i', long long>();
    benchmark_cmd.add_argument("itermax")
        .help("itermax for mcts deep player")
        .scan<'i', int>()
        .default_value(kTrainDeepItermax);
    prog.add_subparser(benchmark_cmd);

    // parse args
    try {
        prog.parse_args(argc, argv);
    } catch (std::exception const& err) {
        spdlog::error("{}\n", err.what());
        if (prog.is_subcommand_used("train")) {
            std::cerr << train_cmd;
        } else if (prog.is_subcommand_used("play")) {
            std::cerr << play_cmd;
        } else if (prog.is_subcommand_used("benchmark")) {
            std::cerr << benchmark_cmd;
        } else {
            std::cerr << prog;
        }
        std::exit(1);
    }

    // run cmd
    if (prog.is_subcommand_used("train")) {
        auto verno = train_cmd.get<long long>("verno");
        runCmdTrain(verno);
    } else if (prog.is_subcommand_used("play")) {
        auto color = play_cmd.get<int>("color");
        auto verno = play_cmd.get<long long>("verno");
        auto itermax = play_cmd.get<int>("itermax");
        runCmdPlay(color, verno, itermax);
    } else if (prog.is_subcommand_used("benchmark")) {
        auto verno1 = benchmark_cmd.get<long long>("verno1");
        auto verno2 = benchmark_cmd.get<long long>("verno2");
        auto itermax = benchmark_cmd.get<int>("itermax");
        runCmdBenchmark(verno1, verno2, itermax);
    } else {
        std::cerr << prog;
        std::exit(0);
    }
    CATCH_;
}