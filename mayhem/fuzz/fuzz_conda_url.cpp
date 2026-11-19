// CondaURL::parse (util::URL underneath): channel/package URLs with tokens and platforms.
#include <cstdint>
#include <string_view>
#include "mamba/specs/conda_url.hpp"

extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
    std::string_view in(reinterpret_cast<const char*>(data), size);
    try
    {
        auto url = mamba::specs::CondaURL::parse(in);
        if (url.has_value())
        {
            (void) url->str();
            (void) url->pretty_str();
            (void) url->token();
            (void) url->platform();
            (void) url->package();
            (void) url->path_without_token();
            url->clear_token();
            (void) url->str(mamba::specs::CondaURL::Credentials::Show);
        }
    }
    catch (const std::exception&)
    {
    }
    return 0;
}
