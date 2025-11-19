#include "raylib.h"
#include "src/systems/sprites.h"

//-------------------------------------------------------------

Animation* NewAnimationFromSheet(Texture2D texture, unsigned char animCount, unsigned char frameCount, float timeDelay) {
	Animation* animation = MemAlloc(sizeof(Animation));
	animation->size = (Vector2){
		(unsigned char)(texture.width / frameCount),
		(unsigned char)(texture.height / animCount),
	};

	animation->animationCount = animCount;
	animation->currentAnimation = 0;
	animation->frameCount = frameCount;
	animation->currentFrame = 0;

	animation->rect = (Rectangle){
		0,
		0,
		animation->size.x,
		animation->size.y,
	};

	animation->timeDelay = timeDelay;
	animation->timer = 0.0f;

	animation->direction = 1;

	return animation;
}

void DestroyAnimation(Animation** animation) {
	MemFree(*animation);
	*animation = ((void*)0);
}

void SetAnimation(Animation* animation, unsigned char animId) {
	animation->currentAnimation = animId;
	animation->rect.x = animation->currentFrame * animation->size.x;
	animation->rect.y = animation->currentAnimation * animation->size.y;
}

void UpdateAnimation(Animation* animation, float speedMul) {
	animation->timer += GetFrameTime() * speedMul;

	if (animation->timer >= animation->timeDelay) {
		animation->currentFrame++;
		animation->timer = 0.0f;

		if (animation->currentFrame >= animation->frameCount) {
			animation->currentFrame = 0;
		}

		animation->rect.x = animation->currentFrame * animation->size.x;
		animation->rect.y = animation->currentAnimation * animation->size.y;
	}

	/* flip sprite */
	if (animation->direction == 0) {
		animation->rect.width = -animation->size.x;
	} else {
		animation->rect.width = animation->size.x;
	}
}