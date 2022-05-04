#include "../utils.h"
#include <glog/logging.h>
#include <pybind11/embed.h>
#include <pybind11/stl.h>
#include <pybind11/numpy.h>

namespace py = pybind11;
using namespace py::literals;

int main(int argc, char** argv)
{
    FLAGS_logtostderr = 1;
    FLAGS_minloglevel = 0;
    gflags::ParseCommandLineFlags(&argc, &argv, true);
    google::InitGoogleLogging(argv[0]);

    TRY_;
    py::scoped_interpreter guard;
    PySys_SetPath(L"" DEPS_PYTHON_SYS_PATH);
    py::module sys = py::module::import("sys");
    py::print(sys.attr("path"));
    py::module_ mpl = py::module_::import("matplotlib");
    mpl.attr("use")("TkAgg");
    py::module_ plt = py::module_::import("matplotlib.pyplot");
    std::vector<float> buf = {8, 11, 1, 5, 6, 7};
    auto capsule = py::capsule(buf.data(), [](void* v) {});
    py::array_t<float> arr({3, 2}, {sizeof(float) * 2, sizeof(float)}, buf.data(), capsule);
    py::print(arr);
    plt.attr("subplot")(1, 2, 1);
    plt.attr("imshow")(arr);
    plt.attr("subplot")(1, 2, 2);
    plt.attr("plot")(std::vector<int>{1, 2, 3}, std::vector<int>{4, 2, 3}, "b");
    plt.attr("show")("block"_a = true);
    CATCH_;
}
