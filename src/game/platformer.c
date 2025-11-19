#include "raylib.h"
#include "raymath.h"

#include "platformer.h"
#include "src/systems/sprites.h"

//------------------------------------------------------

Game NewGame(int width, int height, int objectLimit) {
	Game game = {0};
	game.level = 0;
	game.theme = THEME_GRASS;
	game.score = 0;

	game.player = NewPlayer((Vector2){0.0f, 0.0f}, (Vector2){15.0f, 31.0f});

	game.width = width;
	game.height = height;
	game.tilemap = MemAlloc(sizeof(Tile) * game.width * game.height);
	for (int i = 0; i < game.width * game.height; i++) {
		game.tilemap[i] = (Tile){0};
	}

	game.objectLimit = objectLimit;
	game.objectCount = 0;
	game.objects = MemAlloc(sizeof(Object) * game.objectLimit);
	for (int i = 0; i < game.objectLimit; i++) {
		game.objects[i] = (Object){0};
	}

	// Initialize global game vars
	game.camera.target = (Vector2){
		game.player->frame.x + game.player->frame.width / 2.0f,
		game.player->frame.y + game.player->frame.height / 2.0f,
	};
	game.camera.offset = (Vector2){GetScreenWidth() / 2.0f, GetScreenHeight() / 2.0f};
	game.camera.zoom = 1.0f;
	game.camera.rotation = 0.0f;

	return game;
}

void DestroyGame(Game* game) {
	game->level = 0;
	game->theme = THEME_GRASS;
	game->score = 0;

	*game->player = (Player){0};
	DestroyAnimation(&game->player->anim);
	DestroyPlayer(&game->player);

	game->camera = (Camera2D){0};

	game->width = 0;
	game->height = 0;
	MemFree(game->tilemap);
	game->tilemap = ((void*)0);

	game->objectLimit = 0;
	MemFree(game->objects);
	game->objects = ((void*)0);
}

//----------------------------------------------------------------------------------------------------------------------

void UpdateDrawGame(Game* game) {
	// Update
	//--------------------------------------------------------
	if (IsKeyPressed(KEY_W)) {
		Object* obj = GetObjectAt(game, game->player->frame);
		if (obj->id == OBJECT_ID_DOOR) {
			NewLevel(game);
		}
	}

	UpdateGamePlayer(game);

	/* Update game->camera */

	Vector2 camTargetPos = (Vector2){
		game->player->frame.x + game->player->frame.width,
		game->player->frame.y,
	};

	game->camera.target = Vector2Lerp(game->camera.target, camTargetPos, 0.15f);

	// Draw
	//--------------------------------------------------------
	BeginDrawing();
	ClearBackground(SKYBLUE);

	BeginMode2D(game->camera);

	// Draw Tiles //
	DrawGameTilemap(game);
	DrawGameObjects(game);

	// Draw Player //
	Vector2 pPos = {game->player->frame.x, game->player->frame.y};
	DrawTextureRec(txPlayer, game->player->anim->rect, pPos, WHITE);
	EndMode2D();

	// Draw GUI not bound to game->camera
	//-----------------------------
	DrawText(TextFormat("Score: %d\nLevel: %d", game->score, game->level), 10, 10, 20, RAYWHITE);
	DrawFPS(GetScreenWidth() - 96, 16);

	EndDrawing();
}