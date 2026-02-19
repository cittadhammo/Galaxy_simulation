#!/usr/bin/env bash
set -euo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

WORKER_CAMERA="top"
WORKER_SNAPSHOT_DIR=""
WORKER_POLL_INTERVAL=2
WORKER_HEADLESS=0
PHYSICS_ARGS=()

usage() {
  cat <<'EOF'
Usage:
  bash run_physics_with_worker.sh [worker options] -- [physics options]
  bash run_physics_with_worker.sh [physics options]

Examples:
  bash run_physics_with_worker.sh -- --steps 5000 --state-out outputs/physics_state.bin --state-interval 500
  bash run_physics_with_worker.sh --worker-camera isometric --worker-headless -- --steps 5000 --state-out outputs/physics_state.bin --state-interval 500

Worker options:
  --worker-camera <top|isometric>   Snapshot camera view (default: top)
  --worker-snapshot-dir <dir>       Snapshot output dir (default: derived from --state-out)
  --worker-poll-interval <seconds>  Worker polling interval (default: 2)
  --worker-headless                 Use xvfb-run for renderer snapshots
  --help                            Show help

Physics options:
  Passed through to run_physics.sh. Must include:
  - --state-out <path>
  - --state-interval <N> where N > 0
EOF
}

parse_physics_metadata() {
  local -n args_ref=$1
  STATE_OUT=""
  STATE_INTERVAL=0
  CONFIG_PATH=""

  local i=0
  while [[ $i -lt ${#args_ref[@]} ]]; do
    local arg="${args_ref[$i]}"
    if [[ "$arg" == "--state-out" && $((i + 1)) -lt ${#args_ref[@]} ]]; then
      STATE_OUT="${args_ref[$((i + 1))]}"
      i=$((i + 2))
      continue
    fi
    if [[ "$arg" == "--state-interval" && $((i + 1)) -lt ${#args_ref[@]} ]]; then
      STATE_INTERVAL="${args_ref[$((i + 1))]}"
      i=$((i + 2))
      continue
    fi
    if [[ "$arg" == "--config" && $((i + 1)) -lt ${#args_ref[@]} ]]; then
      CONFIG_PATH="${args_ref[$((i + 1))]}"
      i=$((i + 2))
      continue
    fi
    i=$((i + 1))
  done
}

while [[ $# -gt 0 ]]; do
  case "$1" in
    --worker-camera)
      WORKER_CAMERA="${2:-}"
      shift 2
      ;;
    --worker-snapshot-dir)
      WORKER_SNAPSHOT_DIR="${2:-}"
      shift 2
      ;;
    --worker-poll-interval)
      WORKER_POLL_INTERVAL="${2:-}"
      shift 2
      ;;
    --worker-headless)
      WORKER_HEADLESS=1
      shift
      ;;
    --help|-h)
      usage
      exit 0
      ;;
    --)
      shift
      while [[ $# -gt 0 ]]; do
        PHYSICS_ARGS+=("$1")
        shift
      done
      ;;
    *)
      PHYSICS_ARGS+=("$1")
      shift
      ;;
  esac
done

if [[ "${WORKER_CAMERA}" != "top" && "${WORKER_CAMERA}" != "isometric" ]]; then
  echo "Error: --worker-camera must be 'top' or 'isometric'." >&2
  exit 1
fi

parse_physics_metadata PHYSICS_ARGS

if [[ -z "${STATE_OUT}" ]]; then
  echo "Error: physics args must include --state-out <path>." >&2
  exit 1
fi

if ! [[ "${STATE_INTERVAL}" =~ ^[0-9]+$ ]] || [[ "${STATE_INTERVAL}" -le 0 ]]; then
  echo "Error: physics args must include --state-interval <N> with N > 0." >&2
  exit 1
fi

state_dir="$(dirname "${STATE_OUT}")"
state_base="$(basename "${STATE_OUT}")"
state_stem="${state_base%.*}"
if [[ "${state_stem}" == "${state_base}" ]]; then
  state_stem="${state_base}"
fi
STATE_GLOB="${state_dir}/${state_stem}_step_*.bin"

if [[ -z "${WORKER_SNAPSHOT_DIR}" ]]; then
  WORKER_SNAPSHOT_DIR="${state_dir}/checkpoint_snapshots"
fi

WORKER_ARGS=(
  --state-glob "${STATE_GLOB}"
  --snapshot-dir "${WORKER_SNAPSHOT_DIR}"
  --camera "${WORKER_CAMERA}"
  --poll-interval "${WORKER_POLL_INTERVAL}"
)
if [[ "${WORKER_HEADLESS}" -eq 1 ]]; then
  WORKER_ARGS+=(--headless)
fi
if [[ -n "${CONFIG_PATH}" ]]; then
  WORKER_ARGS+=(--config "${CONFIG_PATH}")
fi

echo "Starting checkpoint worker..."
echo "  state glob:    ${STATE_GLOB}"
echo "  snapshot dir:  ${WORKER_SNAPSHOT_DIR}"
echo "  camera:        ${WORKER_CAMERA}"

bash "${ROOT_DIR}/checkpoint_snapshot_worker.sh" "${WORKER_ARGS[@]}" &
WORKER_PID=$!

cleanup() {
  if kill -0 "${WORKER_PID}" >/dev/null 2>&1; then
    kill "${WORKER_PID}" >/dev/null 2>&1 || true
    wait "${WORKER_PID}" 2>/dev/null || true
  fi
}
trap cleanup EXIT INT TERM

echo "Running physics..."
bash "${ROOT_DIR}/run_physics.sh" "${PHYSICS_ARGS[@]}"
