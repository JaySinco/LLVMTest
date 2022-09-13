#include <iomanip>
#include "network.h"

// #define MX_TRY try {
// #define MX_CATCH                                    \
//     }                                               \
//     catch (dmlc::Error & err)                       \
//     {                                               \
//         std::cout << MXGetLastError() << std::endl; \
//         std::exit(-1);                              \
//     }

// using namespace mxnet::cpp;

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
        int c = uniform(global_random_engine);
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

// Symbol dense_layer(const std::string& name, Symbol data, int num_hidden,
//                    const std::string& act_type)
// {
//     Symbol w(name + "_w"), b(name + "_b");
//     Symbol out = FullyConnected("fc_" + name, data, w, b, num_hidden);
//     if (act_type != "None") out = Activation("act_" + name, out, act_type);
//     return out;
// }

// Symbol convolution_layer(const std::string& name, Symbol data, int num_filter, Shape kernel,
//                          Shape stride, Shape pad, bool use_act, bool use_bn = USE_BATCH_NORM)
// {
//     Symbol conv_w(name + "_w");
//     Symbol conv_b(name + "_b");
//     Symbol out = Convolution("conv_" + name, data, conv_w, conv_b, kernel, num_filter,
//     stride,
//                              Shape(1, 1), pad);
//     if (use_bn) {
//         Symbol gamma(name + "_bn_gamma");
//         Symbol beta(name + "_bn_beta");
//         Symbol mmean(name + "_bn_mmean");
//         Symbol mvar(name + "_bn_mvar");
//         out = BatchNorm("bn_" + name, out, gamma, beta, mmean, mvar);
//     }
//     if (use_act) out = Activation("relu_" + name, out, "relu");
//     return out;
// }

// Symbol residual_layer(const std::string& name, Symbol data, int num_filter)
// {
//     Symbol conv1 = convolution_layer(name + "_conv1", data, num_filter, Shape(3, 3), Shape(1,
//     1),
//                                      Shape(1, 1), true);
//     Symbol conv2 = convolution_layer(name + "_conv2", conv1, num_filter, Shape(3, 3),
//     Shape(1, 1),
//                                      Shape(1, 1), false);
//     return Activation("relu_" + name, data + conv2, "relu");
// }

// Symbol residual_block(const std::string& name, Symbol data, int num_block, int num_filter)
// {
//     Symbol out = data;
//     for (int i = 0; i < num_block; ++i)
//         out = residual_layer(name + "_block" + std::to_string(i + 1), out, num_filter);
//     return out;
// }

// Symbol middle_layer(Symbol data)
// {
//     Symbol middle_conv = convolution_layer("middle_conv", data, NET_NUM_FILTER, Shape(3, 3),
//                                            Shape(1, 1), Shape(1, 1), true);
//     Symbol middle_residual =
//         residual_block("middle_res", middle_conv, NET_NUM_RESIDUAL_BLOCK, NET_NUM_FILTER);
//     return middle_residual;
// }

// std::pair<Symbol, Symbol> plc_layer(Symbol data, Symbol label)
// {
//     Symbol plc_conv =
//         convolution_layer("plc_conv", data, 2, Shape(1, 1), Shape(1, 1), Shape(0, 0), true);
//     Symbol plc_logist_out = dense_layer("plc_logist_out", plc_conv, BOARD_SIZE, "None");
//     Symbol plc_out = softmax("plc_out", plc_logist_out);
//     Symbol plc_m_loss = -1 * elemwise_mul(label, log_softmax(plc_logist_out));
//     Symbol plc_loss = MakeLoss(mean(sum(plc_m_loss, dmlc::optional<Shape>(Shape(1)))));
//     return std::make_pair(plc_out, plc_loss);
// }

// std::pair<Symbol, Symbol> val_layer(Symbol data, Symbol label)
// {
//     Symbol val_conv =
//         convolution_layer("val_conv", data, 1, Shape(1, 1), Shape(1, 1), Shape(0, 0), true);
//     Symbol val_dense = dense_layer("val_dense", val_conv, NET_NUM_FILTER, "relu");
//     Symbol val_out = dense_layer("val_logist_out", val_dense, 1, "tanh");
//     Symbol val_loss = MakeLoss(mean(square(elemwise_sub(val_out, label))));
//     return std::make_pair(val_out, val_loss);
// }

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

    // MX_TRY
    // build_graph();

    // bind_train();
    // if (update_cnt == 0) {
    //     loss.InferArgsMap(ctx, &args_map, args_map);
    //     auxs_map = loss_train->aux_dict();
    //     init_param();
    // }
    // bind_predict();
    // optimizer = OptimizerRegistry::Find("sgd");
    // optimizer->SetParam("momentum", 0.9)
    //     ->SetParam("clip_gradient", 10)
    //     ->SetParam("lr", calc_init_lr())
    //     ->SetParam("wd", WEIGHT_DECAY);
    // MX_CATCH
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

// void FIRNet::build_graph()
// {
//     auto middle = middle_layer(Symbol::Variable("data"));
//     auto plc_pair = plc_layer(middle, Symbol::Variable("plc_label"));
//     auto val_pair = val_layer(middle, Symbol::Variable("val_label"));
//     plc = plc_pair.first;
//     val = val_pair.first;
//     loss = plc_pair.second + val_pair.second;
//     loss_arg_names = loss.ListArguments();
// }

// void FIRNet::bind_train()
// {
//     args_map["data"] = data_train;
//     args_map["plc_label"] = plc_label;
//     args_map["val_label"] = val_label;
//     loss_train = loss.SimpleBind(ctx, args_map, std::map<std::string, NDArray>(),
//                                  std::map<std::string, OpReqType>(), auxs_map);
// }

// void FIRNet::bind_predict()
// {
//     args_map["data"] = data_predict;
//     plc_predict = plc.SimpleBind(ctx, args_map, std::map<std::string, NDArray>(),
//                                  std::map<std::string, OpReqType>(), auxs_map);
//     val_predict = val.SimpleBind(ctx, args_map, std::map<std::string, NDArray>(),
//                                  std::map<std::string, OpReqType>(), auxs_map);
//     args_map.erase("data");
//     args_map.erase("plc_label");
//     args_map.erase("val_label");
// }

// void FIRNet::init_param()
// {
//     auto xavier_init = Xavier(Xavier::gaussian, Xavier::in, 2.34);
//     for (auto& arg: args_map) {
//         xavier_init(arg.first, &arg.second);
//     }
//     auto mean_init = Constant(0.0f);
//     auto mvar_init = Constant(BN_MVAR_INIT);
//     for (auto& aux: auxs_map) {
//         if (aux.first.find("_bn_mmean") != -1)
//             mean_init(aux.first, &aux.second);
//         else if (aux.first.find("_bn_mvar") != -1)
//             mvar_init(aux.first, &aux.second);
//     }
// }

std::string FIRNet::make_param_file_name()
{
    std::ostringstream filename;
    filename << "FIR-" << BOARD_MAX_COL << "x" << NET_NUM_FILTER << "i" << NET_NUM_RESIDUAL_BLOCK
             << "@" << update_cnt << ".pt";
    return filename.str();
}

void FIRNet::load_param()
{
    auto file_name = make_param_file_name();
    spdlog::info("loading parameters from {}", file_name);
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

// void brief_NDArray(std::ostream& out, const std::string& name, const NDArray& nd)
// {
//     out << std::left << std::setw(40) << name << " (";
//     auto shape = nd.GetShape();
//     for (int i = 0; i < shape.size(); ++i) {
//         out << shape[i];
//         if (i != shape.size() - 1) out << ", ";
//     }
//     out << ") = [";
//     auto data = nd.GetData();
//     auto num = nd.Size() > 3 ? 3 : nd.Size();
//     for (int i = 0; i < num; ++i) {
//         out << data[i];
//         if (i != num - 1) out << ", ";
//     }
//     if (num < nd.Size()) out << "...";
//     out << "]\n";
// }

// void FIRNet::show_param(std::ostream& out)
// {
//     out << "=== trainable parameters ===\n";
//     for (const auto& arg: args_map) brief_NDArray(out, arg.first, arg.second);
//     out << "\n=== auxiliary parameters ===\n";
//     for (const auto& aux: auxs_map) brief_NDArray(out, aux.first, aux.second);
// }

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
    int transform_id = uniform(global_random_engine);
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
        spdlog::info("wield policy probality yield by network: sum={}, available_move_n={}",
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

    // data_train.SyncCopyFromCPU(batch->data, BATCH_SIZE * INPUT_FEATURE_NUM * BOARD_SIZE);
    // plc_label.SyncCopyFromCPU(batch->p_label, BATCH_SIZE * BOARD_SIZE);
    // val_label.SyncCopyFromCPU(batch->v_label, BATCH_SIZE);
    // loss_train->Forward(true);
    // loss_train->Backward();
    // for (int i = 0; i < loss_arg_names.size(); ++i) {
    //     if (loss_arg_names[i] == "data" || loss_arg_names[i] == "plc_label" ||
    //         loss_arg_names[i] == "val_label")
    //         continue;
    //     optimizer->Update(i, loss_train->arg_arrays[i], loss_train->grad_arrays[i]);
    // }
    // ++update_cnt;
    // adjust_lr();
    // NDArray::WaitAll();
    // return loss_train->outputs[0].GetData()[0];
}