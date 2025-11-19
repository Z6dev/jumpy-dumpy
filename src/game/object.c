#include "raylib.h"
#include "src/game/platformer.h"

void AddGameObject(Game* game, Object object) {
	if (game->objectCount >= game->objectLimit) {
		return;
	}

	game->objects[game->objectCount++] = object;
}

Object* GetObjectAt(Game* game, Rectangle hitbox) {
	static Object emptyObj = (Object){0};

	for (int i = 0; i < game->objectCount; i++) {
		Object* obj = &game->objects[i];
		if (obj->id == OBJECT_ID_NONE) {
			continue;
		}

		Rectangle rec = {obj->x, obj->y, obj->w, obj->h};

		if (CheckCollisionRecs(rec, hitbox)) {
			return obj;
		}
	}

	return &emptyObj;
}

void DrawGameObjects(Game* game) {
	for (int i = 0; i < game->objectCount; i++) {
		Object object = game->objects[i];

		if (object.id == OBJECT_ID_NONE) {
			continue;
		}

		Rectangle src = {(object.id - 1) * 16, 0, object.w, object.h};
		Vector2 pos = {object.x, object.y};
		DrawTextureRec(txObjects, src, pos, WHITE);
	}
}