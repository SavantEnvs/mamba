// MatchSpec::parse: the package spec strings from the CLI, env files, history and repodata.
#include <cstdint>
#include <string_view>
#include "mamba/specs/match_spec.hpp"

extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
    std::string_view in(reinterpret_cast<const char*>(data), size);
    try
    {
        auto ms = mamba::specs::MatchSpec::parse(in);
        if (ms.has_value())
        {
            (void) ms->to_string();
            (void) ms->conda_build_form();
            (void) ms->to_named_spec();
            (void) ms->is_simple();
        }
        auto mu = mamba::specs::MatchSpec::parse_url(in);
        if (mu.has_value())
        {
            (void) mu->to_string();
        }
    }
    catch (const std::exception&)
    {
    }
    return 0;
}
