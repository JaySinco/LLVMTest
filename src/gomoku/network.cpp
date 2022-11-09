#include "network.h"
#include <iomanip>
#include <filesystem>

void SampleData::flipVerticing()
{
    for (int row = 0; row < kBoardMaxRow; ++row) {
        for (int col = 0; col < kBoardMaxCol / 2; ++col) {
            int a = row * kBoardMaxCol + col;
            int b = row * kBoardMaxCol + kBoardMaxCol - col - 1;
            std::iter_swap(data + a, data + b);
            std::iter_swap(data + kBoardSize + a, data + kBoardSize + b);
            if (kInputFeatureNum > 2) {
                std::iter_swap(data + 2 * kBoardSize + a, data + 2 * kBoardSize + b);
            }
            std::iter_swap(p_label + a, p_label + b);
        }
    }
}

void SampleData::transpose()
{
    for (int row = 0; row < kBoardMaxRow; ++row) {
        for (int col = row + 1; col < kBoardMaxCol; ++col) {
            int a = row * kBoardMaxCol + col;
            int b = col * kBoardMaxCol + row;
            std::iter_swap(data + a, data + b);
            std::iter_swap(data + kBoardSize + a, data + kBoardSize + b);
            if (kInputFeatureNum > 2) {
                std::iter_swap(data + 2 * kBoardSize + a, data + 2 * kBoardSize + b);
            }
            std::iter_swap(p_label + a, p_label + b);
        }
    }
}

std::ostream& operator<<(std::ostream& out, SampleData const& sample)
{
    Move last(kNoMoveYet);
    float first = -1.0f;
    for (int row = 0; row < kBoardMaxRow; ++row) {
        for (int col = 0; col < kBoardMaxCol; ++col) {
            if (sample.data[row * kBoardMaxCol + col] > 0) {
                out << Color::kBlack;
            } else if (sample.data[kBoardSize + row * kBoardMaxCol + col] > 0) {
                out << Color::kWhite;
            } else {
                out << Color::kEmpty;
            }
            if (kInputFeatureNum > 2) {
                if (sample.data[2 * kBoardSize + row * kBoardMaxCol + col] > 0) {
                    assert(last.z() == kNoMoveYet);
                    last = Move(row, col);
                }
            }
            if (kInputFeatureNum > 3) {
                if (first < 0) {
                    first = sample.data[3 * kBoardSize + row * kBoardMaxCol + col];
                } else {
                    assert(first == sample.data[3 * kBoardSize + row * kBoardMaxCol + col]);
                }
            }
        }
        out << "｜";
        for (int col = 0; col < kBoardMaxCol; ++col) {
            out << " " << std::setw(5) << std::fixed << std::setprecision(1)
                << sample.p_label[row * kBoardMaxCol + col] * 100 << "%,";
        }
        out << std::endl;
    }
    out << "↑value=" << sample.v_label[0];
    if (kInputFeatureNum > 2) {
        out << ", last_move=";
        if (last.z() == kNoMoveYet) {
            out << "None";
        } else {
            out << last;
        }
    }
    if (kInputFeatureNum > 3) {
        out << ", fist_hand=" << first;
    }
    out << std::endl;
    return out;
}

std::ostream& operator<<(std::ostream& out, MiniBatch const& batch)
{
    for (int i = 0; i < kBatchSize; ++i) {
        SampleData item;
        std::copy(batch.data + i * kInputFeatureNum * kBoardSize,
                  batch.data + (i + 1) * kInputFeatureNum * kBoardSize, item.data);
        std::copy(batch.p_label + i * kBoardSize, batch.p_label + (i + 1) * kBoardSize,
                  item.p_label);
        std::copy(batch.v_label + i, batch.v_label + (i + 1), item.v_label);
        out << item << std::endl;
    }
    return out;
}

void DataSet::pushWithTransform(SampleData* data)
{
    for (int i = 0; i < 4; ++i) {
        data->transpose();
        pushBack(data);
        data->flipVerticing();
        pushBack(data);
    }
}

void DataSet::makeMiniBatch(MiniBatch* batch) const
{
    assert(index_ > kBatchSize);
    std::uniform_int_distribution<int> uniform(0, size() - 1);
    for (int i = 0; i < kBatchSize; i++) {
        int c = uniform(g_random_engine);
        SampleData* r = buf_ + c;
        std::copy(std::begin(r->data), std::end(r->data),
                  batch->data + kInputFeatureNum * kBoardSize * i);
        std::copy(std::begin(r->p_label), std::end(r->p_label), batch->p_label + kBoardSize * i);
        std::copy(std::begin(r->v_label), std::end(r->v_label), batch->v_label + i);
    }
}

std::ostream& operator<<(std::ostream& out, DataSet const& set)
{
    for (int i = 0; i < set.size(); ++i) {
        out << set.get(i) << std::endl;
    }
    return out;
}

FIRNetModule::FIRNetModule()
    : conv1_(torch::nn::Conv2dOptions(kInputFeatureNum, 32, 3).padding(1)),
      conv2_(torch::nn::Conv2dOptions(32, 64, 3).padding(1)),
      conv3_(torch::nn::Conv2dOptions(64, 128, 3).padding(1)),
      act_conv1_(128, kInputFeatureNum, 1),
      act_fc1_(kInputFeatureNum * kBoardMaxRow * kBoardMaxCol, kBoardMaxRow * kBoardMaxCol),
      val_conv1_(128, 2, 1),
      val_fc1_(2 * kBoardMaxRow * kBoardMaxCol, 64),
      val_fc2_(64, 1)
{
    register_module("conv1", conv1_);
    register_module("conv2", conv2_);
    register_module("conv3", conv3_);
    register_module("act_conv1", act_conv1_);
    register_module("act_fc1", act_fc1_);
    register_module("val_conv1", val_conv1_);
    register_module("val_fc1", val_fc1_);
    register_module("val_fc2", val_fc2_);
}

FIRNet::FIRNet(int64_t verno): update_cnt_(verno), optimizer_(module_.parameters(), kWeightDecay)
{
    if (update_cnt_ > 0) {
        loadParam();
    }
    this->setLR(calcInitLR());
}

float FIRNet::calcInitLR() const
{
    float multiplier;
    if (update_cnt_ < kDropStepLR1) {
        multiplier = 1.0f;
    } else if (update_cnt_ >= kDropStepLR1 && update_cnt_ < kDropStepLR2) {
        multiplier = 1e-1;
    } else if (update_cnt_ >= kDropStepLR2 && update_cnt_ < kDropStepLR3) {
        multiplier = 1e-2;
    } else {
        multiplier = 1e-3;
    }
    float lr = kInitLearningRate * multiplier;
    spdlog::info("init learning_rate={}", lr);
    return lr;
}

void FIRNet::adjustLR()
{
    float multiplier = 1.0f;
    switch (update_cnt_) {
        case kDropStepLR1:
            multiplier = 1e-1;
            break;
        case kDropStepLR2:
            multiplier = 1e-2;
            break;
        case kDropStepLR3:
            multiplier = 1e-3;
            break;
    }
    if (multiplier < 1.0f) {
        float lr = kInitLearningRate * multiplier;
        this->setLR(lr);
        spdlog::info("adjusted learning_rate={}", lr);
    }
}

void FIRNet::setLR(float lr)
{
    for (auto& group: optimizer_.param_groups()) {
        if (group.has_options()) {
            auto& options = static_cast<torch::optim::AdamOptions&>(group.options());
            options.lr(lr);
        }
    }
}

FIRNet::~FIRNet() = default;

std::string FIRNet::makeParamFileName() const
{
    std::ostringstream filename;
    filename << "FIR-" << kBoardMaxRow << "x" << kBoardMaxCol << "@" << update_cnt_ << ".pt";
    return utils::ws2s(utils::getExeDir()) + "\\" + filename.str();
}

void FIRNet::loadParam()
{
    auto file_name = makeParamFileName();
    spdlog::info("loading parameters from {}", file_name);
    if (!std::filesystem::exists(file_name)) {
        throw std::runtime_error(fmt::format("file not exist: {}", file_name));
    }
    torch::serialize::InputArchive input_archive;
    input_archive.load_from(file_name);
    module_.load(input_archive);
}

void FIRNet::saveParam()
{
    auto file_name = makeParamFileName();
    spdlog::info("saving parameters into {}", file_name);
    torch::serialize::OutputArchive output_archive;
    module_.save(output_archive);
    output_archive.save_to(file_name);
}

static void mappingData(int id, float data[kInputFeatureNum * kBoardSize])
{
    int n = 0;
    while (true) {
        if (n == id) {
            break;
        }
        // transpose
        for (int row = 0; row < kBoardMaxRow; ++row) {
            for (int col = row + 1; col < kBoardMaxCol; ++col) {
                int a = row * kBoardMaxCol + col;
                int b = col * kBoardMaxCol + row;
                std::iter_swap(data + a, data + b);
                std::iter_swap(data + kBoardSize + a, data + kBoardSize + b);
                if (kInputFeatureNum > 2) {
                    std::iter_swap(data + 2 * kBoardSize + a, data + 2 * kBoardSize + b);
                }
            }
        }
        ++n;
        if (n == id) {
            break;
        }
        // flip_verticing
        for (int row = 0; row < kBoardMaxRow; ++row) {
            for (int col = 0; col < kBoardMaxCol / 2; ++col) {
                int a = row * kBoardMaxCol + col;
                int b = row * kBoardMaxCol + kBoardMaxCol - col - 1;
                std::iter_swap(data + a, data + b);
                std::iter_swap(data + kBoardSize + a, data + kBoardSize + b);
                if (kInputFeatureNum > 2) {
                    std::iter_swap(data + 2 * kBoardSize + a, data + 2 * kBoardSize + b);
                }
            }
        }
        ++n;
    }
}

static Move mappingMove(int id, Move mv)
{
    int n = 0, r, c;
    while (true) {
        if (n == id) {
            break;
        }
        // transpose
        r = mv.c(), c = mv.r();
        mv = Move(r, c);
        ++n;
        if (n == id) {
            break;
        }
        // flip_verticing
        r = mv.r(), c = kBoardMaxCol - mv.c() - 1;
        mv = Move(r, c);
        ++n;
    }
    return mv;
}

std::pair<torch::Tensor, torch::Tensor> FIRNetModule::forward(torch::Tensor input)
{
    // common layers
    auto x = torch::relu(conv1_->forward(input));
    x = torch::relu(conv2_->forward(x));
    x = torch::relu(conv3_->forward(x));
    // action policy layers
    auto x_act = torch::relu(act_conv1_(x));
    x_act = x_act.view({-1, kInputFeatureNum * kBoardMaxRow * kBoardMaxCol});
    x_act = torch::softmax(act_fc1_(x_act), 1);
    // state value layers
    auto x_val = torch::relu(val_conv1_->forward(x));
    x_val = x_val.view({-1, 2 * kBoardMaxRow * kBoardMaxCol});
    x_val = torch::relu(val_fc1_(x_val));
    x_val = torch::tanh(val_fc2_(x_val));
    return {x_act, x_val};
}

void FIRNet::evalState(State const& state, float value[1],
                       std::vector<std::pair<Move, float>>& net_move_priors)
{
    torch::NoGradGuard no_grad;
    module_.eval();
    float buf[kInputFeatureNum * kBoardSize] = {0.0f};
    state.fillFeatureArray(buf);
    std::uniform_int_distribution<int> uniform(0, 7);
    int transform_id = uniform(g_random_engine);
    mappingData(transform_id, buf);
    auto data = torch::from_blob(
        buf, {1, kInputFeatureNum, kBoardMaxRow, kBoardMaxCol}, [](void* buf) {}, torch::kFloat32);
    auto&& [x_act, x_val] = module_.forward(data);
    float priors_sum = 0.0f;
    for (auto const mv: state.getOptions()) {
        Move mapped = mappingMove(transform_id, mv);
        float prior = x_act[0][mapped.z()].item<float>();
        net_move_priors.emplace_back(mv, prior);
        priors_sum += prior;
    }
    if (priors_sum < 1e-8) {
        spdlog::info("wield policy prob, lr might be too large: sum={}, available_move_n={}",
                     priors_sum, net_move_priors.size());
        for (auto& item: net_move_priors) {
            item.second = 1.0f / static_cast<float>(net_move_priors.size());
        }
    } else {
        for (auto& item: net_move_priors) {
            item.second /= priors_sum;
        }
    }
    value[0] = x_val[0][0].item<float>();
}

float FIRNet::trainStep(MiniBatch* batch)
{
    module_.train();
    auto data = torch::from_blob(
        batch->data, {kBatchSize, kInputFeatureNum, kBoardMaxRow, kBoardMaxCol}, [](void* buf) {},
        torch::kFloat32);
    auto plc_label = torch::from_blob(
        batch->p_label, {kBatchSize, kBoardSize}, [](void* buf) {}, torch::kFloat32);
    auto val_label = torch::from_blob(
        batch->p_label, {kBatchSize, 1}, [](void* buf) {}, torch::kFloat32);
    auto&& [x_act, x_val] = module_.forward(data);
    auto value_loss = torch::mse_loss(x_val, val_label);
    auto policy_loss = -torch::mean(torch::sum(plc_label * torch::log(x_act), 1));
    auto loss = value_loss + policy_loss;
    optimizer_.zero_grad();
    loss.backward();
    optimizer_.step();
    adjustLR();
    ++update_cnt_;
    return loss.item<float>();
}