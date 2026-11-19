// Configuration rc-file loading (.condarc / .mambarc YAML): Configuration::load() with the input
// as the only rc file, which parses it and computes every configurable from it (channels,
// channel_alias, custom_channels/multichannels, pkgs_dirs, envs_dirs, proxy_servers, ...).
#include <cstdint>
#include <cstdlib>
#include <sys/stat.h>
#include <vector>
#include "mamba/api/configuration.hpp"
#include "mamba/core/context.hpp"
#include "fuzz_util.hpp"

extern "C" int LLVMFuzzerInitialize(int*, char***)
{
    // A root prefix inside the scratch dir, so loading never resolves or creates one in $HOME.
    std::string root = fuzz_util::scratch_dir() + "/root";
    mkdir(root.c_str(), 0700);
    setenv("MAMBA_ROOT_PREFIX", root.c_str(), 1);
    return 0;
}

extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
    auto path = fuzz_util::write_file(".condarc", data, size);
    // A fresh Context + Configuration per input, so no configuration state carries over.
    try
    {
        mamba::Context ctx{};
        mamba::Configuration config{ ctx };
        config.at("rc_files").set_value<std::vector<mamba::fs::u8path>>({ path });
        config.load();
        (void) config.dump();
    }
    catch (...)
    {
    }
    fuzz_util::drain_logs();
    return 0;
}
