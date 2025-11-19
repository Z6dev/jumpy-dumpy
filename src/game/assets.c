#include "raylib.h"
#include "src/game/platformer.h"

Texture txTiles = {0};
Texture txPlayer = {0};
Texture txObjects = {0};

void LoadAssetsGame() {
	txTiles = LoadTexture("assets/tiles.png");
	txPlayer = LoadTexture("assets/nuget.png");
	txObjects = LoadTexture("assets/objects.png");
}

void UnloadAssetsGame() {
	UnloadTexture(txTiles);
	UnloadTexture(txPlayer);
	UnloadTexture(txObjects);
}