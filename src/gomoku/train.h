#pragma once
#include "network.h"

constexpr float C_PUCT = 1.0;
constexpr int EPOCH_PER_GAME = 1;
constexpr int EXPLORE_STEP = 20;
constexpr int TEST_PURE_ITERMAX = 1000;
constexpr int TRAIN_DEEP_ITERMAX = 400;
constexpr int MINUTE_PER_LOG = 3;
constexpr int MINUTE_PER_SAVE = 30;
constexpr int MINUTE_PER_BENCHMARK = 15;
constexpr bool DEBUG_TRAIN_DATA = false;

int selfplay(std::shared_ptr<FIRNet> net, DataSet& dataset, int itermax);
void train(std::shared_ptr<FIRNet> net);
