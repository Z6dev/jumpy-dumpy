#ifndef SPRITES_H
#define SPRITES_H

#include "raylib.h"

// Animation sprite struct for spritesheet,
// Sprite sheet must have same frame count across multiple anims,
// Each animation is a vertical row,
// Each frame of an animation is a horizontal column,
// e.g:
//	1. | | | | |
//	2. | | | | |
// Use an Enum to denote animation, Im too lazy to implement a binary search...
typedef struct Animation {
	Rectangle rect;
	Vector2 size;

	float timeDelay;
	float timer;

	unsigned char animationCount;
	unsigned char currentAnimation;
	
	unsigned char frameCount;
	unsigned char currentFrame;

	bool direction; // 0 = left, 1 = right
} Animation;

Animation* NewAnimationFromSheet(Texture2D texture, unsigned char animCount, unsigned char frameCount, float timeDelay);
void DestroyAnimation(Animation** animation);
void SetAnimation(Animation* animation, unsigned char animId);
void UpdateAnimation(Animation* animation, float speedMul);

#endif // SPRITES_H