// mamba::extract on a package archive from a channel: .tar.bz2 (libarchive, any filter/format)
// or .conda (zip holding metadata.json + info-*.tar.zst + pkg-*.tar.zst).
// Input byte 0 picks the file suffix extract() dispatches on (even: .tar.bz2, odd: .conda); the
// rest is the archive. The destination is wiped and the cwd restored after every input.
#include <cstdint>
#include <filesystem>
#include <system_error>
#include <unistd.h>
#include "mamba/core/package_handling.hpp"
#include "fuzz_util.hpp"

namespace
{
    // Extraction restores permissions from the archive (ARCHIVE_EXTRACT_PERM), so make every
    // directory writable before removing the tree.
    void wipe(const std::filesystem::path& dir)
    {
        std::error_code ec;
        if (!std::filesystem::exists(dir, ec))
        {
            return;
        }
        for (auto it = std::filesystem::recursive_directory_iterator(
                 dir,
                 std::filesystem::directory_options::skip_permission_denied,
                 ec
             );
             !ec && it != std::filesystem::recursive_directory_iterator();
             it.increment(ec))
        {
            if (it->is_directory(ec) && !it->is_symlink(ec))
            {
                std::filesystem::permissions(it->path(), std::filesystem::perms::owner_all, ec);
            }
        }
        std::filesystem::permissions(dir, std::filesystem::perms::owner_all, ec);
        std::filesystem::remove_all(dir, ec);
        if (std::filesystem::exists(dir, ec))
        {
            std::abort();  // leftover state would leak into the next input
        }
    }
}

extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
    if (size < 1)
    {
        return -1;
    }
    static const std::filesystem::path cwd = std::filesystem::current_path();
    const bool conda = (data[0] & 1) != 0;
    auto file = fuzz_util::write_file(conda ? "pkg.conda" : "pkg.tar.bz2", data + 1, size - 1);
    const std::filesystem::path dest = fuzz_util::scratch_dir() + "/out";
    mamba::ExtractOptions options{};
    options.sparse = false;
    options.subproc_mode = mamba::extract_subproc_mode::mamba_package;
    try
    {
        mamba::extract(file, dest, options);
    }
    catch (const std::exception&)
    {
    }
    std::error_code ec;
    std::filesystem::current_path(cwd, ec);
    wipe(dest);
    fuzz_util::drain_logs();
    return 0;
}
