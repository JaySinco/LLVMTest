#include "prec.h"
#include "../utils.h"
#include <fstream>
#include <random>
#include <opencv2/highgui.hpp>
#include <filesystem>

class FashionMnistDataset: public torch::data::Dataset<FashionMnistDataset>
{
public:
    FashionMnistDataset(const std::wstring &dataRoot, bool train = true)
    {
        std::wstring imagesPath =
            dataRoot + (train ? L"train-images-idx3-ubyte" : L"t10k-images-idx3-ubyte");
        std::wstring labelsPath =
            dataRoot + (train ? L"train-labels-idx1-ubyte" : L"t10k-labels-idx1-ubyte");

        this->readImages(imagesPath);
        this->readLabels(labelsPath);
    }

    ExampleType get(size_t index) override { return {this->data[index], this->target[index]}; }

    c10::optional<size_t> size() const override { return this->data.size(0); }

    void show_rand(int count)
    {
        static std::random_device rd;
        static std::mt19937 e2(rd());
        static std::uniform_int_distribution<int> dist(0, this->data.size(0) - 1);
        std::vector<size_t> indexList;
        for (int i = 0; i < count; ++i) {
            indexList.push_back(dist(e2));
        }
        this->show(indexList);
    }

    void show(const std::vector<size_t> &indexList)
    {
        const int winsize = 250;
        for (int i = 0; i < indexList.size(); ++i) {
            size_t index = indexList[i];
            auto sample = this->data[index][0];
            sample = sample.to(torch::kCPU);
            cv::Mat image(cv::Size(sample.size(0), sample.size(1)), CV_8UC1, sample.data_ptr());
            int label = this->target[index].item<int>();
            std::string winname =
                "{}#{}"_format(FashionMnistDataset::labelDescMap.at(label), index);
            cv::namedWindow(winname, cv::WINDOW_NORMAL);
            cv::resizeWindow(winname, winsize, winsize);
            cv::moveWindow(winname, winsize * i, 0);
            cv::imshow(winname, image);
        }
        cv::waitKey(0);
        cv::destroyAllWindows();
    }

    static const std::map<unsigned, std::string> labelDescMap;

private:
    static int32_t reverseInt(int32_t i)
    {
        unsigned char c1, c2, c3, c4;
        c1 = i & 255;
        c2 = (i >> 8) & 255;
        c3 = (i >> 16) & 255;
        c4 = (i >> 24) & 255;
        return ((int32_t)c1 << 24) + ((int32_t)c2 << 16) + ((int32_t)c3 << 8) + c4;
    }

    void readImages(const std::wstring &path)
    {
        std::ifstream file;
        file.open(path, std::ios::in | std::ios::binary);
        int32_t header[4] = {0};
        for (int i = 0; i < 4; ++i) {
            file.read((char *)&header[i], sizeof(int32_t));
            header[i] = reverseInt(header[i]);
        }
        int32_t images = header[1];
        int32_t rows = header[2];
        int32_t cols = header[3];
        std::cout << "Read {}x{}x{} images from {}\n"_format(images, rows, cols, utils::ws2s(path));
        size_t numel = images * rows * cols;
        auto buf = new unsigned char[numel]{0};
        for (int32_t i = 0; i < images; ++i) {
            file.read((char *)&buf[i * rows * cols], sizeof(unsigned char) * rows * cols);
        }
        this->data = torch::from_blob(
            buf, {images, 1, rows, cols}, [](void *buf) { delete[](unsigned char *) buf; },
            torch::kUInt8);
    }

    void readLabels(const std::wstring &path)
    {
        std::ifstream file;
        file.open(path, std::ios::in | std::ios::binary);
        int32_t header[2] = {0};
        for (int i = 0; i < 2; ++i) {
            file.read((char *)&header[i], sizeof(int32_t));
            header[i] = reverseInt(header[i]);
        }
        int32_t items = header[1];
        std::cout << "Read {} labels from {}\n"_format(items, utils::ws2s(path));
        auto buf = new unsigned char[items]{0};
        for (int32_t i = 0; i < items; ++i) {
            file.read((char *)&buf[i], sizeof(unsigned char));
        }
        torch::Tensor labels = torch::from_blob(
            buf, {items}, [](void *buf) { delete[](unsigned char *) buf; }, torch::kUInt8);
        this->target = labels.to(torch::kLong);
    }

    torch::Tensor data;
    torch::Tensor target;
};

const std::map<unsigned, std::string> FashionMnistDataset::labelDescMap = {
    {0, "T-shirt"}, {1, "Trouser"}, {2, "Pullover"}, {3, "Dress"}, {4, "Coat"},
    {5, "Sandal"},  {6, "Shirt"},   {7, "Sneaker"},  {8, "Bag"},   {9, "Ankle boot"},
};

struct Net: torch::nn::Module
{
    Net(): fc1(784, 256), fc2(256, 10)
    {
        register_module("fc1", fc1);
        register_module("fc2", fc2);
    }

    torch::Tensor forward(torch::Tensor x)
    {
        x = x.reshape({-1, 784});
        x = torch::relu(fc1->forward(x));
        x = torch::dropout(x, 0.5, is_training());
        x = fc2->forward(x);
        return torch::log_softmax(x, 1);
    }

    torch::nn::Linear fc1;
    torch::nn::Linear fc2;
};

template <typename DataLoader>
void train(int32_t epoch, Net &model, torch::Device device, DataLoader &data_loader,
           torch::optim::Optimizer &optimizer, size_t dataset_size)
{
    model.train();
    size_t batch_idx = 0;
    for (auto &batch: data_loader) {
        auto data = batch.data.to(device), targets = batch.target.to(device);
        optimizer.zero_grad();
        auto output = model.forward(data);
        auto loss = torch::nll_loss(output, targets);
        AT_ASSERT(!std::isnan(loss.template item<float>()));
        loss.backward();
        optimizer.step();

        if (batch_idx++ % 10 == 0) {
            std::cout << "\rTrain Epoch: {} [{:5}/{:5}] Loss: {:.4f}"_format(
                epoch, batch_idx * batch.data.size(0), dataset_size, loss.template item<float>());
        }
    }
}

template <typename DataLoader>
void test(Net &model, torch::Device device, DataLoader &data_loader, size_t dataset_size)
{
    torch::NoGradGuard no_grad;
    model.eval();
    double test_loss = 0;
    int32_t correct = 0;
    for (const auto &batch: data_loader) {
        auto data = batch.data.to(device), targets = batch.target.to(device);
        auto output = model.forward(data);
        test_loss +=
            torch::nll_loss(output, targets, {}, torch::Reduction::Sum).template item<float>();
        auto pred = output.argmax(1);
        correct += pred.eq(targets).sum().template item<int64_t>();
    }

    test_loss /= dataset_size;
    std::cout << "\nTest set: Average loss: {:.4f} | Accuracy: {:.3f}\n"_format(
        test_loss, static_cast<double>(correct) / dataset_size);
}

void fashion_mnist()
{
    const std::wstring kDataRoot = L"./resources/deep/fashion-mnist/";
    FashionMnistDataset train_mnist(kDataRoot);
    FashionMnistDataset test_mnist(kDataRoot, false);
    train_mnist.show_rand(5);

    torch::DeviceType device_type;
    if (torch::cuda::is_available()) {
        std::cout << "CUDA available! Training on GPU." << std::endl;
        device_type = torch::kCUDA;
    } else {
        std::cout << "Training on CPU." << std::endl;
        device_type = torch::kCPU;
    }
    torch::Device device(device_type);

    auto saved_model_path = utils::ws2s(utils::getExeDir() + L"\\fashion-mnist.model.pt");
    Net model;
    model.to(device);
    if (std::filesystem::exists(saved_model_path)) {
        std::cout << "Read model from file: " << saved_model_path << std::endl;
        torch::serialize::InputArchive inArch;
        inArch.load_from(saved_model_path);
        model.load(inArch);
    }

    auto train_dataset = train_mnist.map(torch::data::transforms::Normalize<>(0, 255))
                             .map(torch::data::transforms::Stack<>());
    const size_t train_dataset_size = train_dataset.size().value();
    auto train_loader = torch::data::make_data_loader<torch::data::samplers::SequentialSampler>(
        std::move(train_dataset), 64);

    auto test_dataset = test_mnist.map(torch::data::transforms::Normalize<>(0, 255))
                            .map(torch::data::transforms::Stack<>());
    const size_t test_dataset_size = test_dataset.size().value();
    auto test_loader = torch::data::make_data_loader(std::move(test_dataset), 1000);

    torch::optim::SGD optimizer(model.parameters(), torch::optim::SGDOptions(0.01).momentum(0.5));

    for (size_t epoch = 1; epoch <= 10; ++epoch) {
        train(epoch, model, device, *train_loader, optimizer, train_dataset_size);
        test(model, device, *test_loader, test_dataset_size);
    }

    std::cout << "Save model to file: " << saved_model_path << std::endl;
    torch::serialize::OutputArchive outArch;
    model.save(outArch);
    outArch.save_to(saved_model_path);
}
