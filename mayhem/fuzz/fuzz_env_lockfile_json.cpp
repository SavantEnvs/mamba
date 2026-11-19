// read_environment_lockfile, mambajs JSON lockfile format.
#include <cstdint>
#include "mamba/core/env_lockfile.hpp"
#include "fuzz_util.hpp"

extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
    auto path = fuzz_util::write_file("env-lock.json", data, size);
    try
    {
        auto lf = mamba::read_environment_lockfile(path, mamba::EnvLockfileFormat::mambajs_json);
        if (lf.has_value())
        {
            (void) lf->get_packages_for({ "main", "linux-64", "conda" });
            (void) lf->get_packages_for({});
        }
    }
    catch (const std::exception&)
    {
    }
    return 0;
}
