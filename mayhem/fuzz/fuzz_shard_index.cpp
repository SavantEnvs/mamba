// ShardIndexLoader::parse_shard_index: zstd-compressed msgpack repodata_shards index.
#include <cstdint>
#include "mamba/core/shard_index_loader.hpp"
#include "fuzz_util.hpp"

extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
    auto path = fuzz_util::write_file("repodata_shards.msgpack.zst", data, size);
    try
    {
        auto idx = mamba::ShardIndexLoader::parse_shard_index(path);
        (void) idx.has_value();
    }
    catch (const std::exception&)
    {
    }
    fuzz_util::drain_logs();
    return 0;
}
