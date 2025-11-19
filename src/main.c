#include "raylib.h"
#include "src/game/platformer.h"

#ifdef __EMSCRIPTEN__
#include "emscripten/emscripten.h"
#endif

Game game = {0};

void RunStepFrame() {
	UpdateDrawGame(&game);
}

int main(void) {
	InitWindow(640, 360, "Jumpy Dumpy");

	LoadAssetsGame();

	game = NewGame(80, 40, 16);
	NewLevel(&game);

#ifdef __EMSCRIPTEN__
	emscripten_set_main_loop(RunStepFrame, 60, 1);
#else
	SetTargetFPS(60);
	while (!WindowShouldClose()) {
		UpdateDrawGame(&game);
	}
#endif

	UnloadAssetsGame();
	DestroyGame(&game);
	CloseWindow();
}