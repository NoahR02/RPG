#ifndef RPG_GUI_H
#define RPG_GUI_H

#include <memory>
#include "../Window.h"
#include "../TileMap/TileMap.h"

class Gui {

private:

  bool open = true;

  std::shared_ptr<Window> window;

  unsigned int gameViewportTexture;
  unsigned int frameBufferObject;

  void dockingRender();
  void createEditor();

public:

  TileType selectedTileType = static_cast<TileType>(0);

  Gui(std::shared_ptr<Window>& window);

  void render(TileMap& tileMap, const Camera& camera);
  void captureViewport();

  ~Gui();

};


#endif //RPG_GUI_H
