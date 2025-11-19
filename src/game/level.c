#include "raylib.h"
#include "src/game/platformer.h"

#include <math.h>
#include "lib/stb_perlin.h"
#include <stdlib.h>
#include <time.h>

//-----------------------------------------------------------------------------------------

Tile* GetTileAt(Game* game, int x, int y) {
	if (x < 0 || x >= game->width || y < 0 || y >= game->height) {
		static Tile noneTile = {0}; // sentinel returned for out-of-bounds
		noneTile.id = TILE_ID_NONE;
		return &noneTile;
	}

	return &game->tilemap[y * game->width + x];
}

// returns which direction to auto tile,
// 0, 1, 2,
// 3, 4, 5,
// 6, 7, 8,
int GetTileDir(Game* game, int x, int y) {
	bool up = GetTileAt(game, x, y - 1)->id != TILE_ID_NONE;
	bool down = GetTileAt(game, x, y + 1)->id != TILE_ID_NONE;
	bool left = GetTileAt(game, x - 1, y)->id != TILE_ID_NONE;
	bool right = GetTileAt(game, x + 1, y)->id != TILE_ID_NONE;

	if (!up && !left) {
		return 0; // top-left
	}
	if (!up && !right) {
		return 2; // top-right
	}
	if (!down && !left) {
		return 6; // bottom-left
	}
	if (!down && !right) {
		return 8; // bottom-right
	}

	if (!up) {
		return 1; // top edge
	}
	if (!down) {
		return 7; // bottom edge
	}
	if (!left) {
		return 3; // left edge
	}
	if (!right) {
		return 5; // right edge
	}

	return 4; // IT'S IN THE MIDDLE!
}

//-----------------------------------------------------------------------------------------
// Draw Functions
//-----------------------------------------------------------------------------------------

void DrawGameTilemap(Game* game) {
	// Convert screen corners to world coordinates (accounts for camera.offset and camera.zoom)
	Vector2 worldTopLeft = GetScreenToWorld2D((Vector2){0.0f, 0.0f}, game->camera);
	Vector2 worldBottomRight = GetScreenToWorld2D((Vector2){(float)GetScreenWidth(), (float)GetScreenHeight()}, game->camera);

	// Compute tile index range, add 1 tile padding to handle partial tiles at edges
	int startX = (int)floorf(worldTopLeft.x / TILESIZE) - 1;
	int endX = (int)floorf(worldBottomRight.x / TILESIZE) + 1;
	int startY = (int)floorf(worldTopLeft.y / TILESIZE) - 1;
	int endY = (int)floorf(worldBottomRight.y / TILESIZE) + 1;

	// Clamp to map bounds
	if (startX < 0) {
		startX = 0;
	}
	if (startY < 0) {
		startY = 0;
	}
	if (endX >= game->width) {
		endX = game->width;
	}
	if (endY >= game->height) {
		endY = game->height;
	}

	for (int y = startY; y < endY; y++) {
		for (int x = startX; x < endX; x++) {
			Tile tile = *GetTileAt(game, x, y);

			if (tile.id == TILE_ID_NONE) {
				continue;
			}

			int tileVariant = 3 * (tile.id - 1);
			int tileDir = GetTileDir(game, x, y);
			Rectangle tileSrcRec = {
				(tileVariant + (tileDir % 3)) * TILESIZE,
				(int)(tileDir / 3) * TILESIZE,
				TILESIZE,
				TILESIZE,
			};

			DrawTextureRec(txTiles, tileSrcRec, (Vector2){x * TILESIZE, y * TILESIZE}, WHITE);
		}
	}
}

void NewLevel(Game* game) {
	// clear objects
	for (int i = 0; i < game->objectCount; i++) {
		game->objects[i] = (Object){0};
	}

	// procedural terrain parameters
	const float freq = 0.06f;		 // Perlin frequency (controls horizontal stretch)
	const float amp = 8.0f;			 // Perlin amplitude (vertical variation)
	const int baseline_offset = 12;	 // how many tiles from bottom is baseline ground
	const float coin_chance = 0.05f; // chance to spawn a coin block at a column
	const float hole_chance = 0.05f; // chance to start a short hole
	const int max_hole_len = 4;		 // max consecutive hole columns

	// seed RNG for variability
	srand((unsigned)time(NULL));
	float seed_z = (float)(rand() % 1000) / 1000.0f;

	int holeRun = 0;

	// generate column-by-column
	for (int x = 0; x < game->width; x++) {
		// compute a smooth surface using Perlin noise
		float n = stb_perlin_noise3(x * freq, 0.0f, seed_z, 0, 0, 0); // [-1..1]
		float baseline = (float)game->height - baseline_offset;
		int surfaceY = (int)roundf(baseline + n * amp);

		// clamp surface
		if (surfaceY < 1) {
			surfaceY = 1;
		}
		if (surfaceY > game->height - 1) {
			surfaceY = game->height - 1;
		}

		// maybe start a hole
		if (holeRun == 0 && ((float)rand() / (float)RAND_MAX) < hole_chance) {
			holeRun = 1 + (rand() % max_hole_len);
		}
		if (holeRun > 0) {
			// make this column a hole by pushing surface down off-map
			surfaceY = game->height; // no ground this column
			holeRun--;
		}

		// fill tiles for this column
		for (int y = 0; y < game->height; y++) {
			Tile* tile = GetTileAt(game, x, y);
			// below surface = ground
			bool placeable = y >= surfaceY && y < game->height;

			if (placeable) {
				tile->id = TILE_ID_GROUND;
			} else {
				tile->id = TILE_ID_NONE;
			}

			if (placeable && (x == 3 || x == game->width - 3)) {
				tile->id = TILE_ID_GROUND;
			}
		}

		// Occasionally place a coin block in the air a few tiles above the surface
		if (surfaceY > 3 && ((float)rand() / (float)RAND_MAX) < coin_chance) {
			int blockY = surfaceY - 4 - (rand() % 2); // 3-4 tiles above surface
			if (blockY >= 0 && blockY < game->height) {
				Tile* blockTile = GetTileAt(game, x, blockY);
				if (blockTile->id == TILE_ID_NONE) {
					blockTile->id = TILE_ID_BLOCK;
				}
			}
		}
	}

	// place a door object near the far right, on the surface there
	int doorX = game->width - 3;
	if (doorX < 0) {
		doorX = 0;
	}

	// determine the door's surface Y (search upward for first non-none ground tile)
	int doorSurface = game->height - 1;
	for (int y = 0; y < game->height; y++) {
		if (GetTileAt(game, doorX, y)->id == TILE_ID_GROUND) {
			doorSurface = y - 2;
			break;
		}
	}

	// find a free object slot and spawn the door
	AddGameObject(game, (Object){.id = OBJECT_ID_DOOR, .x = doorX * TILESIZE, .y = doorSurface * TILESIZE, .w = 16, .h = 32.});

	// Reset player
	game->level++;
	ResetPlayer(game->player, game->tilemap, (Vector2){game->width, game->height});
}