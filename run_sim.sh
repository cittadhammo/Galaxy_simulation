#!/usr/bin/env bash
set -euo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

if [[ ! -x "${ROOT_DIR}/build/Galaxy_simulation" ]]; then
  bash "${ROOT_DIR}/unix_run.sh"
fi

exec "${ROOT_DIR}/build/Galaxy_simulation" "$@"
