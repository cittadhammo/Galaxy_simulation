#!/bin/bash
set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
SRC_DIR="$1"
STAMP="$2"
FPS="${3:-24}"
ABS_SRC="$SCRIPT_DIR/$SRC_DIR"

# Create temp directory
TEMP_DIR="$ABS_SRC/.movie_temp"
mkdir -p "$TEMP_DIR"

# Copy files with sequential numbering
i=1
for f in $(ls "$ABS_SRC"/snapshot_step_*.png | sort -V); do
    padded=$(printf "%04d" "$i")
    cp "$f" "$TEMP_DIR/snapshot_${padded}.png"
    i=$((i+1))
done

# Use ffmpeg with sequential frames
cd "$TEMP_DIR"
ffmpeg -y -framerate "$FPS" -i "snapshot_%04d.png" -c:v libx264 -pix_fmt yuv420p "$ABS_SRC/movie.mp4" 2>&1 | grep -E "frame=|Video saved"

# Cleanup
rm -rf "$TEMP_DIR"
echo "Video saved to $ABS_SRC/movie.mp4"
