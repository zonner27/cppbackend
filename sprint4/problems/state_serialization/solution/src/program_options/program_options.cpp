#include "program_options.h"

std::optional<Args> ParseCommandLine(int argc, const char * const argv[]) {
    namespace po = boost::program_options;
    namespace fs = boost::filesystem;

    po::options_description desc{"All options"s};

    Args args;
    args.base_path = fs::system_complete(fs::path(argv[0]).parent_path());

    desc.add_options()
            ("help,h", "produce help message")
            ("tick-period,t", po::value(&args.tick_period)->value_name("milliseconds"s), "set tick period")
            ("config-file,c", po::value(&args.config_file)->value_name("file"s), "set config file path")
            ("www-root,w", po::value(&args.www_root)->value_name("dir"s), "set static files root")
            ("randomize-spawn-points", po::bool_switch(&args.randomize_spawn_points), "spawn dogs at random positions")
            ("state-file", po::value(&args.state_file)->value_name("file"s), "set file to save the game state")
            ("save-state-period", po::value(&args.save_state_period)->value_name("milliseconds"s), "sets the period for automatic saving of the server status");

    // variables_map хранит значения опций после разбора
    po::variables_map vm;
    po::store(po::parse_command_line(argc, argv, desc), vm);
    po::notify(vm);

    if (vm.contains("help"s)) {
        std::cout << desc;
        return std::nullopt;
    }

    if (!vm.contains("config-file"s)) {
        throw std::runtime_error("Config files have not been specified"s);
    }

    if (!vm.contains("www-root"s)) {
        throw std::runtime_error("static files root is not specified"s);
    }

    return args;

}
