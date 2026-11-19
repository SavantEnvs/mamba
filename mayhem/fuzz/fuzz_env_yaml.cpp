// environment.yml reader (detail::read_yaml_file): dependencies, sel() selectors, pip sub-lists, channels.
#include <cstdint>
#include "mamba/api/install.hpp"
#include "mamba/core/context.hpp"
#include "mamba/specs/match_spec.hpp"
#include "fuzz_util.hpp"

extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
    // Read-only after construction: read_yaml_file takes it as const.
    static mamba::Context ctx{};
    auto path = fuzz_util::write_file("environment.yml", data, size);
    try
    {
        auto contents = mamba::detail::read_yaml_file(ctx, path, "linux-64", false);
        for (const auto& dep : contents.dependencies)
        {
            (void) mamba::specs::MatchSpec::parse(dep);
        }
    }
    catch (const std::exception&)
    {
    }
    return 0;
}
