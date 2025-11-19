#ifndef PLATFORMER_H
#define PLATFORMER_H

#include "raylib.h"
#include "src/systems/sprites.h"

#define TILESIZE 16
#define GRAVITY 0.3f

extern Texture txTiles;
extern Texture txPlayer;
extern Texture txObjects;

void LoadAssetsGame();
void UnloadAssetsGame();

//------------------------------------------------------

typedef enum TileId {
	TILE_ID_NONE,
	TILE_ID_GROUND,
	TILE_ID_BLOCK,
} TileId;

typedef enum ObjectId {
	OBJECT_ID_NONE,
	OBJECT_ID_DOOR,
} ObjectId;

/*----------------------------*/

typedef struct Tile {
	int id;
} Tile;

typedef struct Object {
	int id;
	int x, y, w, h;
} Object;

//--------------------------------------------------------

typedef struct MovementInfo {
	float maxSpeed;
	float acceleration;
	float deceleration;
	float jumpPower;
} MovementInfo;

typedef struct Player {
	Rectangle frame;
	Vector2 velocity;
	MovementInfo movement;
	Animation* anim;
	bool isGrounded;
	bool isMoving;
} Player;

Player* NewPlayer(Vector2 startPos, Vector2 size);
void DestroyPlayer(Player** player);
void ResetPlayer(Player* player, Tile* tilemap, Vector2 bounds);
void PlayerMoveAndCollideX(Player* player, Tile* tilemap, Vector2 bounds);
int PlayerMoveAndCollideY(Player* player, Tile* tilemap, Vector2 bounds); // returns 1 if player hits a block


//--------------------------------------------------------
typedef enum Theme {
	THEME_GRASS,
	THEME_SNOW,
} Theme;

typedef struct Game {
	Theme theme;
	unsigned short score;
	unsigned short level;

	Camera2D camera;
	Player* player;

	int width, height;
	Tile* tilemap;

	int objectLimit;
	int objectCount;
	Object* objects;
} Game;

Game NewGame(int width, int height, int objectLimit);
void DestroyGame(Game* game);
void UpdateDrawGame(Game* game);
void NewLevel(Game* game);

void UpdateGamePlayer(Game* game);

void AddGameObject(Game* game, Object object);
Object* GetObjectAt(Game* game, Rectangle hitbox);

Tile* GetTileAt(Game* game, int x, int y);
int GetTileDir(Game* game, int x, int y);
void DrawGameTilemap(Game* game);
void DrawGameObjects(Game* game);

#endif // PLATFORMER_H