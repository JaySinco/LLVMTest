#include "prec.h"

struct TensorDataset: public torch::data::Dataset<TensorDataset>
{
    TensorDataset(torch::Tensor data, torch::Tensor target)
        : data(std::move(data)), target(std::move(target))
    {
    }

    ExampleType get(size_t index) override { return {data[index], target[index]}; }

    c10::optional<size_t> size() const override { return data.size(0); }

    torch::Tensor data;
    torch::Tensor target;
};

void linear_regression()
{
    std::vector<float> wVec{2.0, -3.4};
    const int nFeature = wVec.size();
    const float b = 4.2;
    const int totalSize = 1000;
    const int batchSize = 10;
    const int nEpoch = 3;

    auto w = torch::from_blob(wVec.data(), {nFeature, 1}, torch::kFloat32);
    auto X = torch::normal(0, 1, {totalSize, nFeature});
    auto y = torch::matmul(X, w) + b;
    y += torch::normal(0, 0.01, y.sizes());

    auto dataset = TensorDataset{X, y}.map(torch::data::transforms::Stack<>());
    auto loader = torch::data::make_data_loader<torch::data::samplers::DistributedRandomSampler>(
        std::move(dataset), batchSize);

    torch::nn::Sequential net(torch::nn::Linear(2, 1));
    auto layer0 = net[0]->as<torch::nn::Linear>();
    layer0->weight.data().normal_(0, 0.01);
    layer0->bias.data().fill_(0);
    auto loss = torch::nn::MSELoss();
    auto optimizer = torch::optim::SGD(net->parameters(), 0.03);

    for (int i = 0; i < nEpoch; ++i) {
        for (auto &batch: *loader) {
            auto l = loss->forward(net->forward(batch.data), batch.target);
            optimizer.zero_grad();
            l.backward();
            optimizer.step();
        }
        std::cout << "\repoch " << i
                  << " , loss=" << loss->forward(net->forward(X), y).item<double>();
    }
    std::cout << std::endl;
    std::cout << "w =" << layer0->weight.data() << std::endl;
    std::cout << "b =" << layer0->bias.data() << std::endl;
}
