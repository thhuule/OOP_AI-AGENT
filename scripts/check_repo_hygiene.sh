#!/usr/bin/env bash
set -euo pipefail

if [[ "${1:-}" != "--package" ]]; then
  echo "Usage: ./scripts/check_repo_hygiene.sh --package" >&2
  exit 2
fi

repo_root="$(git rev-parse --show-toplevel)"
cd "$repo_root"

failed=0
fail() {
  echo "FAIL: $1" >&2
  failed=1
}

for path in config.json memory.db notes.txt result.txt capital.txt calc.txt data.txt output.txt report.txt; do
  if git ls-files --error-unmatch -- "$path" >/dev/null 2>&1; then
    fail "generated or secret file is tracked: $path"
  fi
  if [[ -e "$path" ]]; then
    fail "local file must not be included in the package: $path"
  fi
done

for path in build .idea; do
  if [[ -e "$path" ]]; then
    fail "local directory must not be included in the package: $path"
  fi
done

unexpected_results="$(git ls-files benchmark/results | grep -Ev '^benchmark/results/latest/' || true)"
if [[ -n "$unexpected_results" ]]; then
  fail "benchmark results outside benchmark/results/latest/ are tracked"
  echo "$unexpected_results" >&2
fi

if [[ "$failed" -ne 0 ]]; then
  exit 1
fi

echo "PASS: repository is clean for packaging"
