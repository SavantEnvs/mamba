// repodata.json / package record JSON -> specs::RepoData / specs::PackageInfo (from_json).
#include <cstdint>
#include <nlohmann/json.hpp>
#include "mamba/specs/package_info.hpp"
#include "mamba/specs/repo_data.hpp"

extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
    auto j = nlohmann::json::parse(data, data + size, nullptr, false);
    if (j.is_discarded())
    {
        return -1;
    }
    try
    {
        auto rd = j.get<mamba::specs::RepoData>();
        nlohmann::json out = rd;
        (void) out.dump();
    }
    catch (const std::exception&)
    {
    }
    try
    {
        auto pkg = j.get<mamba::specs::PackageInfo>();
        (void) pkg.str();
        (void) pkg.json_signable();
    }
    catch (const std::exception&)
    {
    }
    return 0;
}
