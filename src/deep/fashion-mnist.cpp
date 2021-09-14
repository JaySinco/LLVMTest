#include "prec.h"
#include "../utils.h"
#include <fstream>
#include <random>
#include <opencv2/highgui.hpp>

class FashionMnistDataset: public torch::data::Dataset<FashionMnistDataset>
{
public:
    FashionMnistDataset(const std::wstring &dataRoot, bool testMode = false)
    {
        std::wstring imagesPath =
            dataRoot + (testMode ? L"t10k-images-idx3-ubyte" : L"train-images-idx3-ubyte");
        std::wstring labelsPath =
            dataRoot + (testMode ? L"t10k-labels-idx1-ubyte" : L"train-labels-idx1-ubyte");

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
            auto sample = this->data[index];
            sample = sample.to(torch::kCPU);
            cv::Mat image(cv::Size(sample.size(0), sample.size(1)), CV_8UC1, sample.data_ptr());
            int label = this->target[index][0].item<int>();
            std::string winname =
                "{}#{}"_format(FashionMnistDataset::labelDescMap.at(label), index);
            cv::namedWindow(winname, cv::WINDOW_NORMAL);
            cv::resizeWindow(winname, winsize, winsize);
            cv::moveWindow(winname, winsize * i, 0);
            cv::imshow(winname, image);
        }
        cv::waitKey(0);
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
        std::cout << "reading {}x{}x{} images from {}\n"_format(images, rows, cols,
                                                                utils::ws2s(path));
        size_t numel = images * rows * cols;
        auto buf = new unsigned char[numel]{0};
        for (int32_t i = 0; i < images; ++i) {
            file.read((char *)&buf[i * rows * cols], sizeof(unsigned char) * rows * cols);
        }
        this->data = torch::from_blob(
            buf, {images, rows, cols, 1}, [](void *buf) { delete[](unsigned char *) buf; },
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
        std::cout << "reading {} labels from {}\n"_format(items, utils::ws2s(path));
        auto buf = new unsigned char[items]{0};
        for (int32_t i = 0; i < items; ++i) {
            file.read((char *)&buf[i], sizeof(unsigned char));
        }
        this->target = torch::from_blob(
            buf, {items, 1}, [](void *buf) { delete[](unsigned char *) buf; }, torch::kUInt8);
    }

    torch::Tensor data;
    torch::Tensor target;
};

const std::map<unsigned, std::string> FashionMnistDataset::labelDescMap = {
    {0, "T-shirt"}, {1, "Trouser"}, {2, "Pullover"}, {3, "Dress"}, {4, "Coat"},
    {5, "Sandal"},  {6, "Shirt"},   {7, "Sneaker"},  {8, "Bag"},   {9, "Ankle boot"},
};

void fashion_mnist()
{
    const std::wstring kDataRoot = L"./resources/deep/fashion-mnist/";
    FashionMnistDataset trainMinst = FashionMnistDataset(kDataRoot);
    FashionMnistDataset testMinst = FashionMnistDataset(kDataRoot, true);
    auto trainDataset = trainMinst.map(torch::data::transforms::Stack<>());
    auto testDataset = testMinst.map(torch::data::transforms::Stack<>());
    trainMinst.show_rand(5);
}
