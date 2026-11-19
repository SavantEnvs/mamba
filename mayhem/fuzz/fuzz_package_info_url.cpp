// PackageInfo::from_url: explicit-spec URLs (@EXPLICIT files, CLI), filename -> name/version/build.
#include <cstdint>
#include <string_view>
#include <nlohmann/json.hpp>
#include "mamba/specs/package_info.hpp"

extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
    std::string_view in(reinterpret_cast<const char*>(data), size);
    try
    {
        auto pkg = mamba::specs::PackageInfo::from_url(in);
        if (pkg.has_value())
        {
            (void) pkg->str();
            (void) pkg->json_signable();
            (void) pkg->field("name");
        }
    }
    catch (const std::exception&)
    {
    }
    return 0;
}
