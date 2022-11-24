#include "utils/args.h"
#include "uber-shader.h"
#include "pixel-shader.h"

int main(int argc, char** argv)
{
    MY_TRY;
    utils::Args args(argc, argv);
    args.positional("manifest", utils::value<std::string>(), "manifest name to render", 1);
    args.parse();

    auto raw = utils::readFile((CURR_RESDIR / "manifests.json").wstring());
    auto manifests = nlohmann::json::parse(raw.value());
    if (args.has("manifest")) {
        auto m = args.get<std::string>("manifest");
        int idx = -1;
        for (int i = 0; i < manifests.size(); ++i) {
            if (manifests[i]["name"] == m) {
                idx = i;
                break;
            }
        }
        if (idx < 0) {
            std::cerr << FSTR("unknow manifest: {}", m) << std::endl;
            return 1;
        }
        auto& j = manifests[idx];
        std::unique_ptr<BaseShader> shader = nullptr;
        if (j["type"] == "uber") {
            shader = std::make_unique<UberShader>();
        } else if (j["type"] == "pixel") {
            shader = std::make_unique<PixelShader>();
        }
        shader->render(j);
    } else {
        std::cerr << "[manifests]" << std::endl;
        for (auto& item: manifests) {
            std::cerr << " - " << item["name"].get<std::string>() << std::endl;
        }
        return 0;
    }
    MY_CATCH;
}
