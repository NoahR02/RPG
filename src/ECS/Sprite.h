#ifndef RPG_SPRITE_H
#define RPG_SPRITE_H


#include "../TextureRectangle.h"
#include "../Color.h"

struct Sprite {

  float x;
  float y;

  float width;
  float height;

  float rotation = 0.0f;

  TextureRectangle texturePosition;
  Color color;

  void construct(int x, int y, int width, int height, float rotation, const TextureRectangle& texturePosition, const Color& color) {
    this->x = x;
    this->y = y;
    this->width = width;
    this->height = height;
    this->rotation = rotation;
    this->texturePosition = texturePosition;
    this->color = color;
  }

};

#endif //RPG_SPRITE_H
