// solver::libsolv::Database::add_repo_from_repodata_json with mamba's own (simdjson) repodata parser.
#include <cstdint>
#include "mamba/fs/filesystem.hpp"
#include "mamba/solver/libsolv/database.hpp"
#include "mamba/solver/libsolv/parameters.hpp"
#include "mamba/specs/channel.hpp"
#include "mamba/specs/match_spec.hpp"
#include "fuzz_util.hpp"

extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
    namespace libsolv = mamba::solver::libsolv;
    auto path = fuzz_util::write_file("repodata.json", data, size);
    try
    {
        // A fresh database (libsolv pool) per input, so no state survives between inputs.
        libsolv::Database db({}, { libsolv::MatchSpecParser::Mamba });
        auto repo = db.add_repo_from_repodata_json(
            mamba::fs::u8path(path),
            "https://conda.anaconda.org/conda-forge/linux-64",
            "conda-forge",
            libsolv::PipAsPythonDependency::Yes,
            libsolv::PackageTypes::CondaOrElseTarBz2,
            libsolv::RepodataParser::Mamba
        );
        if (repo.has_value())
        {
            (void) db.package_count();
            db.for_each_package_in_repo(*repo, [](const auto& pkg) { (void) pkg.str(); });
        }
    }
    catch (const std::exception&)
    {
    }
    return 0;
}
