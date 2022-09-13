#pragma once
#include "prec.h"
#include "game.h"

constexpr int BATCH_SIZE = 512;
constexpr int BUFFER_SIZE = 10000;
constexpr float INIT_LEARNING_RATE = 2e-3;
constexpr float WEIGHT_DECAY = 1e-4;
constexpr int LR_DROP_STEP1 = 2000;
constexpr int LR_DROP_STEP2 = 8000;
constexpr int LR_DROP_STEP3 = 10000;

struct SampleData
{
    float data[INPUT_FEATURE_NUM * BOARD_SIZE] = {0.0f};
    float p_label[BOARD_SIZE] = {0.0f};
    float v_label[1] = {0.0f};

    void flip_verticing();
    void transpose();
};
std::ostream& operator<<(std::ostream& out, const SampleData& sample);

struct MiniBatch
{
    float data[BATCH_SIZE * INPUT_FEATURE_NUM * BOARD_SIZE] = {0.0f};
    float p_label[BATCH_SIZE * BOARD_SIZE] = {0.0f};
    float v_label[BATCH_SIZE * 1] = {0.0f};
};
std::ostream& operator<<(std::ostream& out, const MiniBatch& batch);

class DataSet
{
private:
    long long index;
    SampleData* buf;

public:
    DataSet(): index(0) { buf = new SampleData[BUFFER_SIZE]; }
    ~DataSet() { delete[] buf; }
    int size() const { return (index > BUFFER_SIZE) ? BUFFER_SIZE : index; }
    long long total() const { return index; }
    void push_back(const SampleData* data)
    {
        buf[index % BUFFER_SIZE] = *data;
        ++index;
    }
    void push_with_transform(SampleData* data);
    const SampleData& get(int i) const
    {
        assert(i < size());
        return buf[i];
    }
    void make_mini_batch(MiniBatch* batch) const;
};
std::ostream& operator<<(std::ostream& out, const DataSet& set);

class FIRNetModule: public torch::nn::Module
{
public:
    FIRNetModule();
    std::pair<torch::Tensor, torch::Tensor> forward(torch::Tensor x);

private:
    torch::nn::Conv2d conv1;
    torch::nn::Conv2d conv2;
    torch::nn::Conv2d conv3;
    torch::nn::Conv2d act_conv1;
    torch::nn::Linear act_fc1;
    torch::nn::Conv2d val_conv1;
    torch::nn::Linear val_fc1;
    torch::nn::Linear val_fc2;
};

class FIRNet
{
    long long update_cnt;
    FIRNetModule module_;
    torch::optim::Adam optimizer;

public:
    FIRNet(long long verno);
    ~FIRNet();
    long long verno() { return update_cnt; }
    void save_param();
    void load_param();
    void set_lr(float lr);
    float calc_init_lr();
    void adjust_lr();
    std::string make_param_file_name();
    float train_step(MiniBatch* batch);
    void evalState(const State& state, float value[1],
                   std::vector<std::pair<Move, float>>& move_priors);
};
