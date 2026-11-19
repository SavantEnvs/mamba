// PrefixData::create over <prefix>/conda-meta/*.json (installed-package records read from any
// prefix on disk: name/version/build/depends/constrains/channel/url/files...).
#include <cstdint>
#include <sys/stat.h>
#include "mamba/core/channel_context.hpp"
#include "mamba/core/prefix_data.hpp"
#include "mamba/specs/match_spec.hpp"
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
    // The only file in conda-meta/ (rewritten for every input), so each run sees just this record.
    fuzz_util::write_file("prefix/conda-meta/x-1.0-0.json", data, size);
    try
    {
        mamba::specs::ChannelResolveParams params;
        params.platforms = { "linux-64", "noarch" };
        params.channel_alias = mamba::specs::CondaURL::parse("https://conda.anaconda.org").value();
        mamba::ChannelContext cc(std::move(params), {});
        // no_pip: don't spawn the prefix's python to list pip packages.
        auto pd = mamba::PrefixData::create(prefix, cc, /*no_pip=*/true);
        if (pd.has_value())
        {
            for (const auto& [name, rec] : pd.value().records())
            {
                (void) rec.str();
                for (const auto& dep : rec.dependencies)
                {
                    (void) mamba::specs::MatchSpec::parse(dep);
                }
            }
            (void) pd.value().sorted_records();
        }
    }
    catch (const std::exception&)
    {
    }
    fuzz_util::drain_logs();
    return 0;
}
