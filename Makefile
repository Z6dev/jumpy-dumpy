# directories
HOMEDIR = /home/codespace
RL_DIR = $(HOMEDIR)/raylib/src

# flags

RL_FLAGS = $(RL_DIR)/libraylib.web.a -I$(RL_DIR) -sUSE_GLFW=3

SRCS = src/*.c src/systems/*.c src/game/*.c

EM_FLAGS = -std=gnu99 \
	-I. \
	--shell-file $(RL_DIR)/shell.html \
	--embed-file assets \
	-sENVIRONMENT=web -sWASM=1 -Os -sSINGLE_FILE

# commands

.PHONY: build

build:
	emcc $(SRCS) -o build/index.html $(RL_FLAGS) $(EM_FLAGS)
	emrun --no-browser --port 8080 build/index.html

clean:
	rm -rf build/*