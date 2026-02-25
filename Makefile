.PHONY: help physic sim snapshot view movie

HEADLESS := 1

help:
	@echo "Usage: make [target] [VAR=value]"
	@echo ""
	@echo "Targets:"
	@echo "  physic          - Run physics with worker (requires DIR)"
	@echo "  sim             - Run interactive galaxy simulation from simulation.cfg"
	@echo "  snapshot        - Generate snapshots from existing checkpoint files (requires DIR and OUT)"
	@echo "  view            - View a simulation state file interactively (requires IN)"
	@echo "  movie           - Create video from snapshots (requires DIR)"
	@echo ""
	@echo "Variables:"
	@echo "  DIR          - Run name (outputs/DIR/)"
	@echo "  IN           - Input state file (.bin) to view (required for view)"
	@echo "  OUT          - Snapshot subfolder name (outputs/DIR/OUT/)"
	@echo "  HEADLESS     - Use xvfb for headless rendering (default: 1)"
	@echo "  DELAY        - Delay between renders in seconds"
	@echo "  STEPS        - Number of steps (default: 2000)"
	@echo "  STATE_INTERVAL - State save interval (default: 40)"
	@echo "  SNAPSHOTS    - Enable snapshots during run (0 or 1, default: 1)"
	@echo "  STAMP       - Add step number overlay on video frames (0 or 1, default: 0)"
	@echo "  FPS         - Video framerate (default: 24)"
	@echo "  DEVICE      - OpenCL device: cpu or gpu (default: auto)"
	@echo "  CAMERA      - Camera view: top, isometric or front (default: top)"
	@echo "  BLOOM_RED   - Red bloom intensity (e.g., 1.0, 2.5)"
	@echo "  BLOOM_BLUE  - Blue bloom intensity (e.g., 1.0, 2.5)"
	@echo "  ANGLE       - Camera elevation angle in degrees (0-360)"
	@echo "  ROT         - Rotation increment per checkpoint (for rotating video)"
	@echo ""
	@echo "Examples:"
	@echo "  make sim                                 # run interactive galaxy sim"
	@echo "  make physic DIR=my_experiment"
	@echo "  make physic DIR=run1 STEPS=5000"
	@echo "  make physic DIR=run1 DEVICE=gpu"
	@echo "  make snapshot DIR=run1 OUT=run1/snap1 CAMERA=isometric BLOOM_RED=2.5"
	@echo "  make snapshot DIR=run1 OUT=run1/snap2 CAMERA=top SINGLE=1"
	@echo "  make snapshot DIR=run1 OUT=run1/rotated ROT=5    # 5 degrees per checkpoint"
	@echo "  make snapshot DIR=run1 OUT=run1/rotated ANGLE=45"
	@echo "  make movie DIR=outputs/run1/snapshots"
	@echo "  make movie DIR=outputs/run1/snapshots STAMP=1"
	@echo "  make view IN=outputs/run1/states/physics_state_step_200.bin"
	@echo "  make movie DIR=A/snapshots              # create video from snapshots"
	@echo "  make movie DIR=A/snapshots STAMP=1     # add step numbers to video"
	@echo "  make movie DIR=A/snapshots FPS=12      # 12 fps (default: 24)"

physic:
	mkdir -p $(DIR)/states
	cp simulation.cfg $(DIR)/simulation.cfg
	$(if $(filter-out 0,$(or $(SNAPSHOTS),1)),\
		bash run_physics_with_worker.sh \
			--worker-camera top \
			--worker-headless \
			--worker-snapshot-dir $(DIR)/snapshots \
			--worker-snapshot-width 3840 \
			--worker-snapshot-height 2160 \
			-- \
			--state-out $(DIR)/states/physics_state.bin \
			--state-interval $(or $(STATE_INTERVAL),3) \
			--config $(DIR)/simulation.cfg \
			--progress-interval $(or $(PROGRESS_INTERVAL),1) \
			--steps $(or $(STEPS),2000) \
			$(if $(DEVICE),--device $(DEVICE)),\
		bash run_physics.sh \
			--state-out $(DIR)/states/physics_state.bin \
			--state-interval $(or $(STATE_INTERVAL),20) \
			--config $(DIR)/simulation.cfg \
			--progress-interval $(or $(PROGRESS_INTERVAL),1) \
			--steps $(or $(STEPS),2000) \
			$(if $(DEVICE),--device $(DEVICE))\
	)

sim:
	bash run_sim.sh

snapshot:
	bash snapshot_from_folder.sh \
		--state-glob "$(DIR)/states/physics_state_step_*.bin" \
		--snapshot-dir "$(DIR)/$(OUT)/" \
		--camera $(or $(CAMERA),top) \
		--config $(DIR)/simulation.cfg \
		--snapshot-width $(or $(WIDTH),3840) \
		--snapshot-height $(or $(HEIGHT),2160) \
		--single \
		$(if $(filter-out 0,$(HEADLESS)),--headless) \
		$(if $(DELAY),--delay $(DELAY)) \
		$(if $(BLOOM_RED),--bloom-red $(BLOOM_RED)) \
		$(if $(BLOOM_BLUE),--bloom-blue $(BLOOM_BLUE)) \
		$(if $(ANGLE),--camera-angle $(ANGLE)) \
		$(if $(ROT),--rotation $(ROT))

view:
	bash run_sim.sh --state-in $(IN)

movie:
	@if ! command -v ffmpeg >/dev/null 2>&1; then echo "Error: ffmpeg not installed"; exit 1; fi && \
	if ! command -v magick >/dev/null 2>&1; then echo "Error: ImageMagick not installed"; exit 1; fi && \
	bash _make_movie.sh "$(DIR)" "$(or $(STAMP),0)" "$(or $(FPS),24)"
