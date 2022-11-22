#include "utils/args.h"
#include "uber-shader.h"
#include "pixel-shader.h"

int main(int argc, char** argv)
{
    MY_TRY;
    namespace po = boost::program_options;
    boost::program_options::options_description opt_args("Optional arguments");
    auto opts = opt_args.add_options();
    opts("manifest", po::value<std::string>(), "manifest name to render");
    opts("help,h", po::bool_switch(), "shows help message and exits");

    po::positional_options_description pos_args;
    pos_args.add("manifest", 1);

    po::variables_map vm;
    po::command_line_parser parser(argc, argv);
    po::store(parser.options(opt_args).positional(pos_args).run(), vm);

    if (vm["help"].as<bool>()) {
        utils::printUsage(argv[0], &opt_args, &pos_args);
        return 1;
    }

    utils::initLogger(argv[0]);
    auto raw = utils::readFile((CURR_RESDIR / "manifests.json").wstring());
    auto manifests = nlohmann::json::parse(raw.value());
    if (vm.count("manifest")) {
        auto m = vm["manifest"].as<std::string>();
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
