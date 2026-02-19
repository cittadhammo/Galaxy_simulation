#!/usr/bin/env bash
set -euo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
STATE_GLOB="${ROOT_DIR}/outputs/physics_state_step_*.bin"
SNAPSHOT_DIR="${ROOT_DIR}/outputs/checkpoint_snapshots"
CAMERA_VIEW="top"
POLL_INTERVAL=2
HEADLESS=0
BASE_CONFIG=""

usage() {
  cat <<'EOF'
Usage:
  bash checkpoint_snapshot_worker.sh [options]

Options:
  --state-glob <glob>            Glob for checkpoint state files.
                                 Default: outputs/physics_state_step_*.bin
  --snapshot-dir <dir>           Output directory for PNG snapshots.
                                 Default: outputs/checkpoint_snapshots
  --camera <top|isometric>       Camera view for snapshot rendering (default: top)
  --config <path>                Base simulation config used for rendering.
                                 The worker keeps all values and overrides camera_view.
  --poll-interval <seconds>      Poll delay between scans (default: 2)
  --headless                     Run renderer via xvfb-run
  --help                         Show this help

Behavior:
  - Watches for new checkpoint bin files and renders one PNG per file.
  - Output name format: snapshot_step_<N>.png
  - Existing PNG files are skipped.
EOF
}

while [[ $# -gt 0 ]]; do
  case "$1" in
    --state-glob)
      STATE_GLOB="${2:-}"
      shift 2
      ;;
    --snapshot-dir)
      SNAPSHOT_DIR="${2:-}"
      shift 2
      ;;
    --camera)
      CAMERA_VIEW="${2:-}"
      shift 2
      ;;
    --config)
      BASE_CONFIG="${2:-}"
      shift 2
      ;;
    --poll-interval)
      POLL_INTERVAL="${2:-}"
      shift 2
      ;;
    --headless)
      HEADLESS=1
      shift
      ;;
    --help|-h)
      usage
      exit 0
      ;;
    *)
      echo "Unknown option: $1" >&2
      usage
      exit 1
      ;;
  esac
done

if [[ "${CAMERA_VIEW}" != "top" && "${CAMERA_VIEW}" != "isometric" ]]; then
  echo "Error: --camera must be 'top' or 'isometric'." >&2
  exit 1
fi

mkdir -p "${SNAPSHOT_DIR}"
CAMERA_CFG="${SNAPSHOT_DIR}/.worker_camera.cfg"
if [[ -n "${BASE_CONFIG}" && ! -f "${BASE_CONFIG}" ]]; then
  echo "Error: --config file not found: ${BASE_CONFIG}" >&2
  exit 1
fi

if [[ -n "${BASE_CONFIG}" ]]; then
  awk -v cam="${CAMERA_VIEW}" '
    {
      line = $0
      sub(/#.*/, "", line)
      n = split(line, tokens, /[[:space:]]+/)
      prefix = ""
      out = ""
      has_cfg = 0
      has_camera = 0
      for (i = 1; i <= n; ++i)
      {
        tok = tokens[i]
        if (tok == "")
          continue
        if (tok == "SIMCFG" || tok == "BATCHCFG")
        {
          if (prefix == "")
            prefix = tok
          continue
        }
        eq = index(tok, "=")
        if (eq <= 1 || eq >= length(tok))
          continue
        key = substr(tok, 1, eq - 1)
        if (key == "camera_view")
        {
          tok = "camera_view=" cam
          has_camera = 1
        }
        if (out != "")
          out = out " "
        out = out tok
        has_cfg = 1
      }
      if (has_cfg)
      {
        if (!has_camera)
          out = out " camera_view=" cam
        if (prefix == "")
          prefix = "SIMCFG"
        print prefix " " out
        exit
      }
    }
  ' "${BASE_CONFIG}" > "${CAMERA_CFG}"
fi

if [[ ! -s "${CAMERA_CFG}" ]]; then
  echo "SIMCFG camera_view=${CAMERA_VIEW}" > "${CAMERA_CFG}"
fi

if [[ ! -x "${ROOT_DIR}/build/Galaxy_simulation" ]]; then
  echo "Renderer executable not found, building first..."
  bash "${ROOT_DIR}/unix_run.sh"
fi

if [[ "${HEADLESS}" -eq 1 ]] && ! command -v xvfb-run >/dev/null 2>&1; then
  echo "Error: --headless requested but xvfb-run is not available." >&2
  exit 1
fi

echo "Watching checkpoints: ${STATE_GLOB}"
echo "Writing snapshots:   ${SNAPSHOT_DIR}"
echo "Camera view:         ${CAMERA_VIEW}"
if [[ -n "${BASE_CONFIG}" ]]; then
  echo "Base config:         ${BASE_CONFIG}"
fi

render_one() {
  local state_file="$1"
  local step="$2"
  local final_png="${SNAPSHOT_DIR}/snapshot_step_${step}.png"
  local temp_dir="${SNAPSHOT_DIR}/.tmp_render_${step}"

  if [[ -f "${final_png}" ]]; then
    return 0
  fi

  rm -rf "${temp_dir}"
  mkdir -p "${temp_dir}"

  if [[ "${HEADLESS}" -eq 1 ]]; then
    xvfb-run -a -s "-screen 0 1920x1080x24" \
      "${ROOT_DIR}/build/Galaxy_simulation" \
      --state-in "${state_file}" \
      --batch-steps 1 \
      --snapshots 1 \
      --output-dir "${temp_dir}" \
      --config "${CAMERA_CFG}"
  else
    "${ROOT_DIR}/build/Galaxy_simulation" \
      --state-in "${state_file}" \
      --batch-steps 1 \
      --snapshots 1 \
      --output-dir "${temp_dir}" \
      --config "${CAMERA_CFG}"
  fi

  if [[ ! -f "${temp_dir}/snapshot_0.png" ]]; then
    echo "Failed: ${state_file} (snapshot_0.png missing)" >&2
    rm -rf "${temp_dir}"
    return 1
  fi

  mv "${temp_dir}/snapshot_0.png" "${final_png}"
  rm -rf "${temp_dir}"
  echo "Rendered step ${step}: ${final_png}"
}

while true; do
  mapfile -t files < <(compgen -G "${STATE_GLOB}" | sort -V || true)
  for state_file in "${files[@]}"; do
    base="$(basename "${state_file}")"
    if [[ "${base}" =~ _step_([0-9]+)\.bin$ ]]; then
      step="${BASH_REMATCH[1]}"
      render_one "${state_file}" "${step}" || true
    fi
  done
  sleep "${POLL_INTERVAL}"
done
