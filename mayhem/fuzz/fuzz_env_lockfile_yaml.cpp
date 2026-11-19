// read_environment_lockfile, conda YAML lockfile format (conda-lock *-lock.yml).
#include <cstdint>
#include "mamba/core/env_lockfile.hpp"
#include "fuzz_util.hpp"

extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
    auto path = fuzz_util::write_file("env-lock.yml", data, size);
    try
    {
        auto lf = mamba::read_environment_lockfile(path, mamba::EnvLockfileFormat::conda_yaml);
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
