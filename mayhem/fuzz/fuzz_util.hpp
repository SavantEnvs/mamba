// Shared helpers for the mamba fuzz harnesses (mayhem integration layer).
#pragma once
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <string>
#include <sstream>
#include <unistd.h>
#include "mamba/core/logging.hpp"

namespace fuzz_util
{
    // A private scratch directory under $TMPDIR (fallback /tmp), created once per process.
    inline const std::string& scratch_dir()
    {
        static std::string dir = []
        {
            const char* t = std::getenv("TMPDIR");
            std::string tmpl = std::string((t && *t) ? t : "/tmp") + "/mambafuzz-XXXXXX";
            if (mkdtemp(tmpl.data()) == nullptr)
            {
                std::abort();
            }
            return tmpl;
        }();
        return dir;
    }

    // Overwrite <scratch>/<rel> with the input bytes; return the absolute path.
    inline std::string write_file(const std::string& rel, const uint8_t* data, size_t size)
    {
        std::string path = scratch_dir() + "/" + rel;
        FILE* f = std::fopen(path.c_str(), "wb");
        if (f == nullptr)
        {
            std::abort();
        }
        if (size > 0)
        {
            std::fwrite(data, 1, size, f);
        }
        std::fclose(f);
        return path;
    }

    // With no Console instance, libmamba queues every log record in a process-global buffer that
    // is never drained, so memory grows with each input. Drain it after every input.
    inline void drain_logs()
    {
        std::ostringstream sink;
        mamba::logging::MessageLogger::print_buffer(sink);
    }
}
