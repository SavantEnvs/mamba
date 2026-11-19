// SubdirMetadata: the repodata cache metadata (header of a cached repodata.json and its .state.json).
#include <cstdint>
#include "mamba/core/subdir_index.hpp"
#include "fuzz_util.hpp"

extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
    auto json = fuzz_util::write_file("cache.json", data, size);
    fuzz_util::write_file("cache.state.json", data, size);
    try
    {
        auto a = mamba::SubdirMetadata::read_from_repodata_json(json);
        if (a.has_value())
        {
            (void) a->url();
            (void) a->etag();
        }
        auto b = mamba::SubdirMetadata::read(json);
        if (b.has_value())
        {
            (void) b->last_modified();
            (void) b->cache_control();
        }
    }
    catch (const std::exception&)
    {
    }
    return 0;
}
