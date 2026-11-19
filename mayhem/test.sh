#!/usr/bin/env bash
#
# mayhem/test.sh — RUN this repo's OWN functional test suite (already built by mayhem/build.sh).
# exit 0 = pass. EDIT per repo. PATCH-grade oracle: after an agent patches the source, the grader
# rebuilds (build.sh) then runs this. DELETE this file if the repo has no meaningful tests.
#
# IMPORTANT:
#  * Must assert BEHAVIOR/OUTPUT, not just exit status. The oracle has to check asserted values /
#    golden-output diffs / known-answer results — so a PATCH that "fixes" a bug by making the program
#    exit(0) (or any no-op) FAILS here. Running inputs and checking only "exit 0 / didn't crash" is
#    NOT a functional test (it's trivially reward-hackable) — use the project's real assertion suite.
#  * Do NOT build here — mayhem/build.sh already compiled the test suite (with the project's normal
#    flags). This script only RUNS the pre-built tests and reports counts. If the test runner is
#    missing, that's a build.sh bug — fail loudly rather than silently rebuilding.
#  * REQUIRED OUTPUT — a CTRF (https://ctrf.io) summary so Mayhem/the PATCH grader reads the counts:
#      - writes a CTRF JSON report to ${CTRF_REPORT:-$SRC/ctrf-report.json}, and
#      - prints a one-line `CTRF {...}` marker to stdout (same JSON, compact).
#    Only `results.summary` (with tests/passed/failed/pending/skipped/other) is required.
#    Use the emit_ctrf helper below; it computes tests = passed+failed+skipped and sets the exit
#    code (0 iff failed==0). Map your framework's output to passed/failed/skipped.
set -uo pipefail
[ -n "${SOURCE_DATE_EPOCH:-}" ] || unset SOURCE_DATE_EPOCH
: "${MAYHEM_JOBS:=$(nproc)}"   # build parallelism; env-overridable, falls back to nproc (use -j"$MAYHEM_JOBS")
cd "$SRC"

# emit_ctrf <tool> <passed> <failed> [skipped] [pending] [other]
# Writes a CTRF report (file + stdout `CTRF {...}` marker) and returns non-zero iff failed>0.
emit_ctrf() {
  local tool="$1" passed="$2" failed="$3" skipped="${4:-0}" pending="${5:-0}" other="${6:-0}"
  local tests=$(( passed + failed + skipped + pending + other ))
  cat > "${CTRF_REPORT:-$SRC/ctrf-report.json}" <<JSON
{
  "results": {
    "tool": { "name": "$tool" },
    "summary": {
      "tests": $tests,
      "passed": $passed,
      "failed": $failed,
      "pending": $pending,
      "skipped": $skipped,
      "other": $other
    }
  }
}
JSON
  printf 'CTRF {"results":{"tool":{"name":"%s"},"summary":{"tests":%d,"passed":%d,"failed":%d,"pending":%d,"skipped":%d,"other":%d}}}\n' \
    "$tool" "$tests" "$passed" "$failed" "$pending" "$skipped" "$other"
  [ "$failed" -eq 0 ]
}

# EDIT: RUN the test runner that mayhem/build.sh produced, then map its output to counts.
#   ctest:        (cd build-tests && ctest) ; parse "<P> tests passed, <F> failed out of <T>"
#   gtest binary: ./build-tests/<prog> ; parse "[==========] N ... ran." / "[  PASSED  ] P" / "[ SKIPPED ] S"
#   make/minunit: ./out/<runner> ; parse its pass/fail/total
# Do NOT compile here — if the runner is absent, fail (build.sh should have produced it).

# The upstream libmamba Catch2 suite (built by build.sh with normal flags), restricted to the tests
# that run offline and without a user environment: the spec parsers (MatchSpec, VersionSpec, URLs,
# channels, PackageInfo, repodata), the util helpers, and the libsolv solver/database tests.
T="$SRC/build-tests/libmamba/tests/test_libmamba"
# (The hidden [.integration] tests need a real conda-forge repodata dump; leave them out. "user_home_dir"
# assumes $HOME equals the passwd home, which does not hold under the grader's build identity.
# "Test create_conda_forge utility" and "Create problem graph" download conda-forge/pytorch repodata; the graded run is offline.)
DEFAULT_TAGS='[mamba::specs]~[.integration],[mamba::util]~[.integration]~"user_home_dir",[mamba::solver]~[.integration]~"Test create_conda_forge utility"~"Create problem graph"'
TAGS="${MAMBA_TEST_TAGS:-$DEFAULT_TAGS}"
if [ ! -x "$T" ]; then
  echo "test.sh: $T missing; mayhem/build.sh must build it" >&2
  emit_ctrf "catch2" 0 1 0
  exit 1
fi

scratch="$(mktemp -d "${TMPDIR:-/tmp}/mamba-test.XXXXXX")"
trap 'rm -rf "$scratch"' EXIT

out="$scratch/out.txt"
(cd "$SRC/build-tests/libmamba/tests" && "$T" "$TAGS") >"$out" 2>&1
rc=$?
tail -n 20 "$out"

passed=0; failed=0; skipped=0
# Catch2 summary forms:
#   All tests passed (N assertions in M test cases)
#   test cases: M | P passed | F failed [| S skipped]
if line=$(grep -E '^All tests passed \(' "$out" | tail -n1) && [ -n "$line" ]; then
  passed=$(sed -E 's/.* in ([0-9]+) test case.*/\1/' <<<"$line")
elif line=$(grep -E '^test cases:' "$out" | tail -n1) && [ -n "$line" ]; then
  passed=$(grep -oE '[0-9]+ passed' <<<"$line" | grep -oE '[0-9]+' || echo 0)
  failed=$(grep -oE '[0-9]+ failed' <<<"$line" | grep -oE '[0-9]+' || echo 0)
  skipped=$(grep -oE '[0-9]+ skipped' <<<"$line" | grep -oE '[0-9]+' || echo 0)
fi
# No summary at all (runner neutered/crashed), or a non-zero exit with nothing failed: count a failure.
if [ "$passed" -eq 0 ] && [ "$failed" -eq 0 ]; then
  echo "test.sh: no Catch2 summary from $T (rc=$rc)" >&2
  failed=1
elif [ "$rc" -ne 0 ] && [ "$failed" -eq 0 ]; then
  failed=1
fi

# A minimal number of test cases must have run: a gutted runner that reports nothing must fail.
if [ "$passed" -lt 50 ]; then
  echo "test.sh: only $passed test cases passed (expected >= 50)" >&2
  failed=$((failed + 1))
fi

emit_ctrf "catch2" "$passed" "$failed" "$skipped"
