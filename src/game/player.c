#include "raylib.h"
#include "raymath.h"

#include "src/game/platformer.h"
#include "src/systems/sprites.h"

//------------------------------------------------------

Player* NewPlayer(Vector2 startPos, Vector2 size) {
	Player* player = MemAlloc(sizeof(Player));
	player->frame = (Rectangle){
		startPos.x,
		startPos.y,
		size.x,
		size.y,
	};

	player->velocity = (Vector2){0.0f, 0.0f};
	player->anim = NewAnimationFromSheet(txPlayer, 3, 4, 0.1f);
	player->movement = (MovementInfo){3.0f, 1.0f, 0.85f, 6};

	return player;
}

void DestroyPlayer(Player** player) {
	MemFree(*player);
	*player = ((void*)0);
}

void ResetPlayer(Player* player, Tile* tilemap, Vector2 bounds) {
	player->frame.x = 3 * TILESIZE;

	for (int y = 0; y < bounds.y; y++) {
		int idx = y * bounds.x + 3;
		if (tilemap[idx].id == TILE_ID_GROUND) {
			player->frame.y = (y - 3) * TILESIZE;
			player->velocity = (Vector2){0, 0};
			player->isGrounded = true;
		}
	}
}

void PlayerMoveAndCollideX(Player* player, Tile* tilemap, Vector2 bounds) {
	// Move horizontally
	player->frame.x += player->velocity.x;
	player->isMoving = player->velocity.x < -0.3f || player->velocity.x > 0.3f;

	// Compute AABB tile coverage
	int bodyTop = (int)(player->frame.y / TILESIZE);
	int bodyBottom = (int)((player->frame.y + player->frame.height) / TILESIZE);
	int bodyLeft = (int)(player->frame.x / TILESIZE);
	int bodyRight = (int)((player->frame.x + player->frame.width) / TILESIZE);

	// Clamp tile indices to world bounds
	if (bodyTop < 0) {
		bodyTop = 0;
	}
	if (bodyLeft < 0) {
		bodyLeft = 0;
	}
	if (bodyBottom >= (int)bounds.y) {
		bodyBottom = (int)bounds.y - 1;
	}
	if (bodyRight >= (int)bounds.x) {
		bodyRight = (int)bounds.x - 1;
	}

	// Check for horizontal collisions
	for (int y = bodyTop; y <= bodyBottom; y++) {
		for (int x = bodyLeft; x <= bodyRight; x++) {
			int idx = y * (int)bounds.x + x;
			Tile tile = tilemap[idx];

			if (tile.id == TILE_ID_NONE) {
				continue; // Skip empty tiles
			}

			Rectangle tileRec = {
				x * TILESIZE,
				y * TILESIZE,
				TILESIZE,
				TILESIZE,
			};

			if (CheckCollisionRecs(player->frame, tileRec)) {
				// Resolve collision depending on movement direction
				if (player->velocity.x > 0) {
					player->frame.x = tileRec.x - player->frame.width - 0.001f;
				} else if (player->velocity.x < 0) {
					player->frame.x = tileRec.x + tileRec.width + 0.001f;
				}

				// Stop horizontal velocity
				player->velocity.x = 0;
			}
		}
	}
}

int PlayerMoveAndCollideY(Player* player, Tile* tilemap, Vector2 bounds) {
	int result = 0;

	// Move vertically
	player->frame.y += player->velocity.y;
	player->isGrounded = false; // Reset each frame

	// Compute AABB tile coverage
	int bodyTop = (int)(player->frame.y / TILESIZE);
	int bodyBottom = (int)((player->frame.y + player->frame.height) / TILESIZE);
	int bodyLeft = (int)(player->frame.x / TILESIZE);
	int bodyRight = (int)((player->frame.x + player->frame.width) / TILESIZE);

	// Clamp to world bounds
	if (bodyTop < 0) {
		bodyTop = 0;
	}
	if (bodyLeft < 0) {
		bodyLeft = 0;
	}
	if (bodyBottom >= (int)bounds.y) {
		bodyBottom = (int)bounds.y - 1;
	}
	if (bodyRight >= (int)bounds.x) {
		bodyRight = (int)bounds.x - 1;
	}

	// Check for vertical collisions
	for (int y = bodyTop; y <= bodyBottom; y++) {
		for (int x = bodyLeft; x <= bodyRight; x++) {
			int idx = y * (int)bounds.x + x;
			Tile tile = tilemap[idx];

			if (tile.id == TILE_ID_NONE) {
				continue; // Skip empty tiles
			}

			// Tile Rectangle
			Rectangle tileRec = {
				x * TILESIZE,
				y * TILESIZE,
				TILESIZE,
				TILESIZE,
			};

			// Check for AABB overlap
			if (CheckCollisionRecs(player->frame, tileRec)) {
				if (player->velocity.y > 0) {
					// Moving down hit the ground
					player->frame.y = tileRec.y - player->frame.height; // Snap to top of tile
					player->isGrounded = true;
				} else if (player->velocity.y < 0) {
					// Moving up hit ceiling
					player->frame.y = tileRec.y + tileRec.height; // Snap below tile

					// Break If tile above is a block
					if (tilemap[idx].id == TILE_ID_BLOCK) {
						tilemap[idx] = (Tile){0};
						result = 1; // Signify that the player hit a block
					}
				}

				// Stop vertical velocity
				player->velocity.y = 0;
			}
		}
	}

	return result;
}

//-----------------------------------------------------------------------------------------------------------------------------------

void UpdateGamePlayer(Game* game) {
	// Timers for coyote time & jump buffering
	float dt = GetFrameTime();
	const float COYOTE_TIME = 0.12f;	  // seconds player can still jump after leaving ground
	const float JUMP_BUFFER_TIME = 0.12f; // seconds to remember a jump press before landing
	static float coyoteTimer = 0.0f;
	static float jumpBufferTimer = 0.0f;

	/* Update Player Movement */

	if (IsKeyDown(KEY_D)) {
		game->player->velocity.x += game->player->movement.acceleration;
	} else if (IsKeyDown(KEY_A)) {
		game->player->velocity.x -= game->player->movement.acceleration;
	}

	game->player->velocity.x *= game->player->movement.deceleration;
	game->player->velocity.x = Clamp(game->player->velocity.x, -game->player->movement.maxSpeed, game->player->movement.maxSpeed);

	// Record jump presses into the buffer
	if (IsKeyPressed(KEY_SPACE)) {
		jumpBufferTimer = JUMP_BUFFER_TIME;
	}

	// Update coyote timer: reset when grounded, otherwise count down
	if (game->player->isGrounded) {
		coyoteTimer = COYOTE_TIME;
	} else {
		coyoteTimer -= dt;
		if (coyoteTimer < 0.0f) {
			coyoteTimer = 0.0f;
		}
	}

	// Count down jump buffer
	if (jumpBufferTimer > 0.0f) {
		jumpBufferTimer -= dt;
		if (jumpBufferTimer < 0.0f) {
			jumpBufferTimer = 0.0f;
		}
	}

	// Perform jump if buffered and allowed by grounding or coyote time
	if (jumpBufferTimer > 0.0f && (game->player->isGrounded || coyoteTimer > 0.0f)) {
		game->player->velocity.y = -game->player->movement.jumpPower;
		// consume both timers so double-triggering is avoided
		jumpBufferTimer = 0.0f;
		coyoteTimer = 0.0f;
		game->player->isGrounded = false;
	}

	game->player->velocity.y += GRAVITY;
	game->player->velocity.y = Clamp(game->player->velocity.y, -10, 10);

	PlayerMoveAndCollideX(game->player, game->tilemap, (Vector2){game->width, game->height});
	if (PlayerMoveAndCollideY(game->player, game->tilemap, (Vector2){game->width, game->height})) {
		game->score++;
	}

	// check if player fall

	if (game->player->frame.y > game->height * TILESIZE) {
		ResetPlayer(game->player, game->tilemap, (Vector2){game->width, game->height});
	}

	/* Update Player Animation */
	if (game->player->isMoving) {
		SetAnimation(game->player->anim, 1); // walk anim

	} else {
		SetAnimation(game->player->anim, 0); // idle anim
	}

	if (!game->player->isGrounded) {
		SetAnimation(game->player->anim, 2); // jump anim
	}

	if (game->player->velocity.x > 0) {
		game->player->anim->direction = 1;
	} else if (game->player->velocity.x < 0) {
		game->player->anim->direction = 0;
	}

	UpdateAnimation(game->player->anim, 1.0f);
}