// UnresolvedChannel::parse + Channel::resolve: channel names/paths/URLs from the CLI and env files.
#include <cstdint>
#include <string_view>
#include "mamba/specs/channel.hpp"
#include "mamba/specs/conda_url.hpp"
#include "mamba/specs/unresolved_channel.hpp"

extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
    using namespace mamba::specs;
    std::string_view in(reinterpret_cast<const char*>(data), size);
    try
    {
        auto uc = UnresolvedChannel::parse(in);
        if (!uc.has_value())
        {
            return 0;
        }
        (void) uc->str();
        ChannelResolveParams params;
        params.platforms = { "linux-64", "noarch" };
        params.channel_alias = CondaURL::parse("https://conda.anaconda.org").value();
        params.home_dir = "/home/user";
        params.current_working_dir = "/home/user/work";
        auto chans = Channel::resolve(std::move(*uc), params);
        if (chans.has_value())
        {
            for (const auto& c : *chans)
            {
                (void) c.display_name();
                (void) c.platform_urls();
                (void) c.id();
            }
        }
    }
    catch (const std::exception&)
    {
    }
    return 0;
}
