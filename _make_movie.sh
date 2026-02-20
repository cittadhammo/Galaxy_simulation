#!/bin/bash
set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
SRC_DIR="$1"
STAMP="$2"
FPS="${3:-24}"
TEMP_DIR="$SRC_DIR/.movie_temp"
ABS_SRC="$SCRIPT_DIR/$SRC_DIR"
ABS_TEMP="$SCRIPT_DIR/$TEMP_DIR"

mkdir -p "$TEMP_DIR"

for f in $(ls "$SRC_DIR"/snapshot_step_*.png | sort -V); do
    step=$(basename "$f" | sed 's/snapshot_step_//;s/.png//')
    padded=$(printf "%07d" "$step")
    cp "$f" "$TEMP_DIR/snapshot_step_${padded}.png"
done

if [ "$STAMP" = "1" ]; then
    TOTAL=$(ls "$TEMP_DIR"/snapshot_step_*.png | wc -l)
    COUNT=0
    for f in "$TEMP_DIR"/snapshot_step_*.png; do
        COUNT=$((COUNT + 1))
        step=$(basename "$f" | sed 's/snapshot_step_//;s/.png//')
        printf "\rAdding stamps: %d/%d" "$COUNT" "$TOTAL"
        magick "$f" -pointsize 48 -fill white -gravity northwest -annotate +50+50 "Step $step" "$f"
    done
    echo ""
fi

cd "$TEMP_DIR"
ffmpeg -y -framerate "$FPS" -pattern_type glob -i "snapshot_step_*.png" -c:v libx264 -pix_fmt yuv420p "$ABS_SRC/movie.mp4"
rm -rf "$TEMP_DIR"
echo "Video saved to $ABS_SRC/movie.mp4"
