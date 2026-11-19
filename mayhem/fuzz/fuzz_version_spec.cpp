// Version::parse + VersionSpec::parse (boolean version expressions). Input: "<version>\n<spec>".
#include <cstdint>
#include <string_view>
#include "mamba/specs/version.hpp"
#include "mamba/specs/version_spec.hpp"

extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
    std::string_view in(reinterpret_cast<const char*>(data), size);
    auto nl = in.find('\n');
    std::string_view a = in.substr(0, nl);
    std::string_view b = (nl == std::string_view::npos) ? in : in.substr(nl + 1);
    try
    {
        auto ver = mamba::specs::Version::parse(a);
        if (ver.has_value())
        {
            (void) ver->to_string();
        }
        auto spec = mamba::specs::VersionSpec::parse(b);
        if (spec.has_value())
        {
            (void) spec->to_string();
            (void) spec->to_string_conda_build();
            (void) spec->expression_size();
            if (ver.has_value())
            {
                (void) spec->contains(*ver);
            }
        }
    }
    catch (const std::exception&)
    {
    }
    return 0;
}
