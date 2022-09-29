#include "network.h"
#include <iomanip>
#include <filesystem>

void SampleData::flip_verticing()
{
    for (int row = 0; row < BOARD_MAX_ROW; ++row) {
        for (int col = 0; col < BOARD_MAX_COL / 2; ++col) {
            int a = row * BOARD_MAX_COL + col;
            int b = row * BOARD_MAX_COL + BOARD_MAX_COL - col - 1;
            std::iter_swap(data + a, data + b);
            std::iter_swap(data + BOARD_SIZE + a, data + BOARD_SIZE + b);
            if (INPUT_FEATURE_NUM > 2)
                std::iter_swap(data + 2 * BOARD_SIZE + a, data + 2 * BOARD_SIZE + b);
            std::iter_swap(p_label + a, p_label + b);
        }
    }
}

void SampleData::transpose()
{
    for (int row = 0; row < BOARD_MAX_ROW; ++row) {
        for (int col = row + 1; col < BOARD_MAX_COL; ++col) {
            int a = row * BOARD_MAX_COL + col;
            int b = col * BOARD_MAX_COL + row;
            std::iter_swap(data + a, data + b);
            std::iter_swap(data + BOARD_SIZE + a, data + BOARD_SIZE + b);
            if (INPUT_FEATURE_NUM > 2)
                std::iter_swap(data + 2 * BOARD_SIZE + a, data + 2 * BOARD_SIZE + b);
            std::iter_swap(p_label + a, p_label + b);
        }
    }
}

std::ostream& operator<<(std::ostream& out, const SampleData& sample)
{
    Move last(NO_MOVE_YET);
    float first = -1.0f;
    for (int row = 0; row < BOARD_MAX_ROW; ++row) {
        for (int col = 0; col < BOARD_MAX_COL; ++col) {
            if (sample.data[row * BOARD_MAX_COL + col] > 0)
                out << Color::Black;
            else if (sample.data[BOARD_SIZE + row * BOARD_MAX_COL + col] > 0)
                out << Color::White;
            else
                out << Color::Empty;
            if (INPUT_FEATURE_NUM > 2) {
                if (sample.data[2 * BOARD_SIZE + row * BOARD_MAX_COL + col] > 0) {
                    assert(last.z() == NO_MOVE_YET);
                    last = Move(row, col);
                }
            }
            if (INPUT_FEATURE_NUM > 3) {
                if (first < 0)
                    first = sample.data[3 * BOARD_SIZE + row * BOARD_MAX_COL + col];
                else
                    assert(first == sample.data[3 * BOARD_SIZE + row * BOARD_MAX_COL + col]);
            }
        }
        out << "｜";
        for (int col = 0; col < BOARD_MAX_COL; ++col)
            out << " " << std::setw(5) << std::fixed << std::setprecision(1)
                << sample.p_label[row * BOARD_MAX_COL + col] * 100 << "%,";
        out << std::endl;
    }
    out << "↑value=" << sample.v_label[0];
    if (INPUT_FEATURE_NUM > 2) {
        out << ", last_move=";
        if (last.z() == NO_MOVE_YET)
            out << "None";
        else
            out << last;
    }
    if (INPUT_FEATURE_NUM > 3) out << ", fist_hand=" << first;
    out << std::endl;
    return out;
}

std::ostream& operator<<(std::ostream& out, const MiniBatch& batch)
{
    for (int i = 0; i < BATCH_SIZE; ++i) {
        SampleData item;
        std::copy(batch.data + i * INPUT_FEATURE_NUM * BOARD_SIZE,
                  batch.data + (i + 1) * INPUT_FEATURE_NUM * BOARD_SIZE, item.data);
        std::copy(batch.p_label + i * BOARD_SIZE, batch.p_label + (i + 1) * BOARD_SIZE,
                  item.p_label);
        std::copy(batch.v_label + i, batch.v_label + (i + 1), item.v_label);
        out << item << std::endl;
    }
    return out;
}

void DataSet::push_with_transform(SampleData* data)
{
    for (int i = 0; i < 4; ++i) {
        data->transpose();
        push_back(data);
        data->flip_verticing();
        push_back(data);
    }
}

void DataSet::make_mini_batch(MiniBatch* batch) const
{
    assert(index > BATCH_SIZE);
    std::uniform_int_distribution<int> uniform(0, size() - 1);
    for (int i = 0; i < BATCH_SIZE; i++) {
        int c = uniform(g_random_engine);
        SampleData* r = buf + c;
        std::copy(std::begin(r->data), std::end(r->data),
                  batch->data + INPUT_FEATURE_NUM * BOARD_SIZE * i);
        std::copy(std::begin(r->p_label), std::end(r->p_label), batch->p_label + BOARD_SIZE * i);
        std::copy(std::begin(r->v_label), std::end(r->v_label), batch->v_label + i);
    }
}

std::ostream& operator<<(std::ostream& out, const DataSet& set)
{
    for (int i = 0; i < set.size(); ++i) out << set.get(i) << std::endl;
    return out;
}

FIRNetModule::FIRNetModule()
    : conv1(torch::nn::Conv2dOptions(INPUT_FEATURE_NUM, 32, 3).padding(1)),
      conv2(torch::nn::Conv2dOptions(32, 64, 3).padding(1)),
      conv3(torch::nn::Conv2dOptions(64, 128, 3).padding(1)),
      act_conv1(128, INPUT_FEATURE_NUM, 1),
      act_fc1(INPUT_FEATURE_NUM * BOARD_MAX_ROW * BOARD_MAX_COL, BOARD_MAX_ROW * BOARD_MAX_COL),
      val_conv1(128, 2, 1),
      val_fc1(2 * BOARD_MAX_ROW * BOARD_MAX_COL, 64),
      val_fc2(64, 1)
{
    register_module("conv1", conv1);
    register_module("conv2", conv2);
    register_module("conv3", conv3);
    register_module("act_conv1", act_conv1);
    register_module("act_fc1", act_fc1);
    register_module("val_conv1", val_conv1);
    register_module("val_fc1", val_fc1);
    register_module("val_fc2", val_fc2);
}

FIRNet::FIRNet(long long verno): update_cnt(verno), optimizer(module_.parameters(), WEIGHT_DECAY)
{
    if (update_cnt > 0) {
        load_param();
    }
    this->set_lr(calc_init_lr());
}

float FIRNet::calc_init_lr()
{
    float multiplier;
    if (update_cnt < LR_DROP_STEP1)
        multiplier = 1.0f;
    else if (update_cnt >= LR_DROP_STEP1 && update_cnt < LR_DROP_STEP2)
        multiplier = 1e-1;
    else if (update_cnt >= LR_DROP_STEP2 && update_cnt < LR_DROP_STEP3)
        multiplier = 1e-2;
    else
        multiplier = 1e-3;
    float lr = INIT_LEARNING_RATE * multiplier;
    spdlog::info("init learning_rate={}", lr);
    return lr;
}

void FIRNet::adjust_lr()
{
    float multiplier = 1.0f;
    switch (update_cnt) {
        case LR_DROP_STEP1:
            multiplier = 1e-1;
            break;
        case LR_DROP_STEP2:
            multiplier = 1e-2;
            break;
        case LR_DROP_STEP3:
            multiplier = 1e-3;
            break;
    }
    if (multiplier < 1.0f) {
        float lr = INIT_LEARNING_RATE * multiplier;
        this->set_lr(lr);
        spdlog::info("adjusted learning_rate={}", lr);
    }
}

void FIRNet::set_lr(float lr)
{
    for (auto& group: optimizer.param_groups()) {
        if (group.has_options()) {
            auto& options = static_cast<torch::optim::AdamOptions&>(group.options());
            options.lr(lr);
        }
    }
}

FIRNet::~FIRNet() {}

std::string FIRNet::make_param_file_name()
{
    std::ostringstream filename;
    filename << "FIR-" << BOARD_MAX_ROW << "x" << BOARD_MAX_COL << "@" << update_cnt << ".pt";
    return utils::ws2s(utils::getExeDir()) + "\\" + filename.str();
}

void FIRNet::load_param()
{
    auto file_name = make_param_file_name();
    spdlog::info("loading parameters from {}", file_name);
    if (!std::filesystem::exists(file_name)) {
        throw std::runtime_error(fmt::format("file not exist: {}", file_name));
    }
    torch::serialize::InputArchive input_archive;
    input_archive.load_from(file_name);
    module_.load(input_archive);
}

void FIRNet::save_param()
{
    auto file_name = make_param_file_name();
    spdlog::info("saving parameters into {}", file_name);
    torch::serialize::OutputArchive output_archive;
    module_.save(output_archive);
    output_archive.save_to(file_name);
}

void mapping_data(int id, float data[INPUT_FEATURE_NUM * BOARD_SIZE])
{
    int n = 0;
    while (true) {
        if (n == id) break;
        // transpose
        for (int row = 0; row < BOARD_MAX_ROW; ++row) {
            for (int col = row + 1; col < BOARD_MAX_COL; ++col) {
                int a = row * BOARD_MAX_COL + col;
                int b = col * BOARD_MAX_COL + row;
                std::iter_swap(data + a, data + b);
                std::iter_swap(data + BOARD_SIZE + a, data + BOARD_SIZE + b);
                if (INPUT_FEATURE_NUM > 2)
                    std::iter_swap(data + 2 * BOARD_SIZE + a, data + 2 * BOARD_SIZE + b);
            }
        }
        ++n;
        if (n == id) break;
        // flip_verticing
        for (int row = 0; row < BOARD_MAX_ROW; ++row) {
            for (int col = 0; col < BOARD_MAX_COL / 2; ++col) {
                int a = row * BOARD_MAX_COL + col;
                int b = row * BOARD_MAX_COL + BOARD_MAX_COL - col - 1;
                std::iter_swap(data + a, data + b);
                std::iter_swap(data + BOARD_SIZE + a, data + BOARD_SIZE + b);
                if (INPUT_FEATURE_NUM > 2)
                    std::iter_swap(data + 2 * BOARD_SIZE + a, data + 2 * BOARD_SIZE + b);
            }
        }
        ++n;
    }
}

Move mapping_move(int id, Move mv)
{
    int n = 0, r, c;
    while (true) {
        if (n == id) break;
        // transpose
        r = mv.c(), c = mv.r();
        mv = Move(r, c);
        ++n;
        if (n == id) break;
        // flip_verticing
        r = mv.r(), c = BOARD_MAX_COL - mv.c() - 1;
        mv = Move(r, c);
        ++n;
    }
    return mv;
}

std::pair<torch::Tensor, torch::Tensor> FIRNetModule::forward(torch::Tensor input)
{
    // common layers
    auto x = torch::relu(conv1->forward(input));
    x = torch::relu(conv2->forward(x));
    x = torch::relu(conv3->forward(x));
    // action policy layers
    auto x_act = torch::relu(act_conv1(x));
    x_act = x_act.view({-1, INPUT_FEATURE_NUM * BOARD_MAX_ROW * BOARD_MAX_COL});
    x_act = torch::softmax(act_fc1(x_act), 1);
    // state value layers
    auto x_val = torch::relu(val_conv1->forward(x));
    x_val = x_val.view({-1, 2 * BOARD_MAX_ROW * BOARD_MAX_COL});
    x_val = torch::relu(val_fc1(x_val));
    x_val = torch::tanh(val_fc2(x_val));
    return {x_act, x_val};
}

void FIRNet::evalState(const State& state, float value[1],
                       std::vector<std::pair<Move, float>>& net_move_priors)
{
    torch::NoGradGuard no_grad;
    module_.eval();
    float buf[INPUT_FEATURE_NUM * BOARD_SIZE] = {0.0f};
    state.fill_feature_array(buf);
    std::uniform_int_distribution<int> uniform(0, 7);
    int transform_id = uniform(g_random_engine);
    mapping_data(transform_id, buf);
    auto data = torch::from_blob(
        buf, {1, INPUT_FEATURE_NUM, BOARD_MAX_ROW, BOARD_MAX_COL}, [](void* buf) {},
        torch::kFloat32);
    auto&& [x_act, x_val] = module_.forward(data);
    float priors_sum = 0.0f;
    for (const auto mv: state.get_options()) {
        Move mapped = mapping_move(transform_id, mv);
        float prior = x_act[0][mapped.z()].item<float>();
        net_move_priors.push_back(std::make_pair(mv, prior));
        priors_sum += prior;
    }
    if (priors_sum < 1e-8) {
        spdlog::info("wield policy prob, lr might be too large: sum={}, available_move_n={}",
                     priors_sum, net_move_priors.size());
        for (auto& item: net_move_priors) item.second = 1.0f / float(net_move_priors.size());
    } else {
        for (auto& item: net_move_priors) item.second /= priors_sum;
    }
    value[0] = x_val[0][0].item<float>();
}

float FIRNet::train_step(MiniBatch* batch)
{
    module_.train();
    auto data = torch::from_blob(
        batch->data, {BATCH_SIZE, INPUT_FEATURE_NUM, BOARD_MAX_ROW, BOARD_MAX_COL},
        [](void* buf) {}, torch::kFloat32);
    auto plc_label = torch::from_blob(
        batch->p_label, {BATCH_SIZE, BOARD_SIZE}, [](void* buf) {}, torch::kFloat32);
    auto val_label = torch::from_blob(
        batch->p_label, {BATCH_SIZE, 1}, [](void* buf) {}, torch::kFloat32);
    auto&& [x_act, x_val] = module_.forward(data);
    auto value_loss = torch::mse_loss(x_val, val_label);
    auto policy_loss = -torch::mean(torch::sum(plc_label * torch::log(x_act), 1));
    auto loss = value_loss + policy_loss;
    optimizer.zero_grad();
    loss.backward();
    optimizer.step();
    adjust_lr();
    ++update_cnt;
    return loss.item<float>();
}