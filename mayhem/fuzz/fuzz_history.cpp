// History over <prefix>/conda-meta/history (regex-parsed revision log + MatchSpecs of user requests).
#include <cstdint>
#include <sys/stat.h>
#include "mamba/core/channel_context.hpp"
#include "mamba/core/history.hpp"
#include "fuzz_util.hpp"

extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
    static const std::string prefix = []
    {
        std::string p = fuzz_util::scratch_dir() + "/prefix";
        mkdir(p.c_str(), 0700);
        mkdir((p + "/conda-meta").c_str(), 0700);
        return p;
    }();
    fuzz_util::write_file("prefix/conda-meta/history", data, size);
    try
    {
        mamba::specs::ChannelResolveParams params;
        params.platforms = { "linux-64", "noarch" };
        params.channel_alias = mamba::specs::CondaURL::parse("https://conda.anaconda.org").value();
        mamba::ChannelContext cc(std::move(params), {});
        mamba::History h(prefix, cc);
        (void) h.get_user_requests();
        (void) h.get_requested_specs_map();
    }
    catch (const std::exception&)
    {
    }
    return 0;
}
