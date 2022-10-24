#include "uber-shader.h"
#include "pixel-shader.h"
#include <argparse/argparse.hpp>

int main(int argc, char** argv)
{
    argparse::ArgumentParser prog("shaders");
    prog.add_argument("manifest").default_value("").help("manifest name to render");

    try {
        prog.parse_args(argc, argv);
    } catch (std::exception const& err) {
        spdlog::error("{}\n", err.what());
        std::cerr << prog;
        std::exit(1);
    }

    TRY_;
    auto raw = utils::readFile((__DIRNAME__ / "manifests.json").wstring());
    auto manifests = nlohmann::json::parse(raw.value());
    if (prog.is_used("manifest")) {
        auto m = prog.get<std::string>("manifest");
        if (!manifests.contains(m)) {
            std::cerr << fmt::format("unknow manifest: {}", m) << std::endl;
            return 1;
        }
        auto& j = manifests[m];
        std::unique_ptr<BaseShader> shader = nullptr;
        if (j["type"] == "uber") {
            shader.reset(new UberShader);
        } else if (j["type"] == "pixel") {
            shader.reset(new PixelShader);
        }
        shader->render(j);
    } else {
        std::cerr << "[manifests]" << std::endl;
        for (auto& [key, value]: manifests.items()) {
            std::cerr << " - " << key << std::endl;
        }
        return 0;
    }
    CATCH_;
}
