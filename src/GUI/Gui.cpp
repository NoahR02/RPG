#include "Gui.h"

#include <imgui/imgui.h>
#include <imgui/imgui_impl_glfw.h>
#include <imgui/imgui_impl_opengl3.h>
#include <stb_image/stb_image.h>
#include <iostream>

Gui::Gui(std::shared_ptr<Window>& window) : window(window) {
  IMGUI_CHECKVERSION();
  ImGui::CreateContext();

  ImGuiIO &io = ImGui::GetIO();
  io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
  io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
  io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;
  io.ConfigFlags |= ImGuiConfigFlags_NoMouseCursorChange;

  ImGui_ImplGlfw_InitForOpenGL(this->window->window, true);
  ImGui_ImplOpenGL3_Init("#version 450");
  ImGui::StyleColorsDark();

  glGenFramebuffers(1, &frameBufferObject);
  glBindFramebuffer(GL_FRAMEBUFFER, frameBufferObject);


  glGenTextures(1, &gameViewportTexture);
  glBindTexture(GL_TEXTURE_2D, gameViewportTexture);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, window->getWidth(), window->getHeight(), 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, gameViewportTexture, 0);
}

void Gui::render(TileMap& tileMap, const Camera& camera) {
  // GUI FRAME INIT
  ImGui_ImplOpenGL3_NewFrame();
  ImGui_ImplGlfw_NewFrame();
  ImGui::NewFrame();

  dockingRender();
  createEditor();

  float mainMenuBarHeight;
  // Create Menu Bar.
  if (ImGui::BeginMainMenuBar()) {
    mainMenuBarHeight = ImGui::GetWindowHeight();
    if (ImGui::BeginMenu("File")) ImGui::EndMenu();
    if (ImGui::BeginMenu("Edit")) ImGui::EndMenu();

    ImGui::EndMainMenuBar();
  }

  // Display stats about application.
  ImGui::Begin("Application Info");
  ImGui::Text("Window Width: %d", window->getWidth());
  ImGui::Text("Window Height: %d", window->getHeight());
  ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
  ImGui::End();



  ImGui::Begin("Assets");

  int tileWidth = 16;
  int tileHeight = 16;

  float columns = tileMap.tileMapTexture->width / tileWidth;
  float rows = tileMap.tileMapTexture->height / tileHeight;

  int itemsPerRowPossible = ImGui::GetColumnWidth() / (32 + ImGui::GetStyle().ItemSpacing.x + ImGui::GetStyle().FramePadding.x);
  int count = 0;

  ImGui::BeginGroup();
  int id = 1;
  for (const auto& [ key, value ] : tileMap.spriteSheet.tileSpriteMap) {

    if(count >= itemsPerRowPossible) {
      ImGui::NewLine();
      count = 0;
    }

    TexturePosition texPos = tileMap.spriteSheet.getTile(key);

    texPos.texturePosY = rows - texPos.texturePosY - 1;

      float x0 = static_cast<float>(texPos.texturePosX) / columns;
      float y0 =  static_cast<float>((static_cast<float>(texPos.texturePosY) + 1.0f)) / rows;

      float x1 =  static_cast<float>((texPos.texturePosX + 1.0f)) / columns;
      float y1 =   static_cast<float>(texPos.texturePosY) / rows;

    ImGui::PushID(id);
    if(ImGui::ImageButton((void *) (intptr_t) tileMap.tileMapTexture->rendererID, ImVec2(32.0f, 32.0f),
                 ImVec2(x0, y0), ImVec2(x1, y1))) {
      selectedTileType = key;

      std::vector<unsigned char> pixels(tileMap.tileWidth * tileMap.tileHeight  * 4);


      TexturePosition tileTexturePosition = tileMap.spriteSheet.getTile(selectedTileType);
      glGetTextureSubImage(
        tileMap.tileMapTexture->rendererID,
        0, // level
        tileMap.tileWidth * tileTexturePosition.texturePosX, // x offset
        tileMap.tileHeight * ((tileMap.tileMapTexture->height / tileMap.tileHeight - 1)-tileTexturePosition.texturePosY),// y offset
        0,
        tileMap.tileWidth,
        tileMap.tileHeight,
        1,
        GL_RGBA,
        GL_UNSIGNED_BYTE,
        pixels.size(),
        pixels.data()
      );

      std::vector<unsigned char> buffer(4*tileWidth);
      for(int i = 0; i < tileHeight/2; ++i) {
        memcpy(buffer.data(), pixels.data() + i * buffer.size(), buffer.size());
        memcpy(pixels.data() + i * buffer.size(), pixels.data() + (tileHeight-1-i) * buffer.size(), buffer.size());
        memcpy(pixels.data() + (tileHeight-1-i) * buffer.size(), buffer.data(), buffer.size());
      }

      GLFWimage image {tileMap.tileWidth, tileMap.tileHeight, pixels.data()};
      GLFWcursor* cursor = glfwCreateCursor(&image, 0, 0);
      glfwSetCursor(window->window, cursor);
    }
    ImGui::PopID();
    ++id;
    ImGui::SameLine();

    ++count;
  }

  ImGui::EndGroup();

  ImGui::End();




  // Display Game Window
  ImGui::Begin("Game Window:");

  if (ImGui::IsMouseClicked(ImGuiMouseButton_Left)) {

    if(ImGui::IsWindowFocused()){

      double xpos, ypos;
      glfwGetCursorPos(window->window, &xpos, &ypos);

      float clickX = xpos - ImGui::GetWindowContentRegionMin().x;
      float clickY = ypos - ImGui::GetWindowContentRegionMin().y - mainMenuBarHeight;

      auto view = camera.view;

      clickX /= camera.zoomLevel;
      clickY /= camera.zoomLevel;

      clickX -= camera.view[3][0];
      clickY -= camera.view[3][1];

      std::printf("Click X: %d \n", static_cast<int>((clickX/16)*16));
      std::printf("Click Y: %d \n", static_cast<int>(clickY/16)*16);
      tileMap.addTile({{static_cast<int>((clickX/16))*16, static_cast<int>((clickY/16))*16}, selectedTileType});

      tileMap.upload();
    }

  }

  ImGui::Image((void*)(intptr_t)gameViewportTexture, ImVec2(window->getWidth(), window->getHeight()), ImVec2(0, 1), ImVec2(1, 0));
  ImGui::End();







  ImGui::Render();
  ImGuiIO &io = ImGui::GetIO();
  if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
    GLFWwindow* backup_current_context = glfwGetCurrentContext();
    ImGui::UpdatePlatformWindows();
    ImGui::RenderPlatformWindowsDefault();
    glfwMakeContextCurrent(backup_current_context);
  }

  // Render all draw calls.
  ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

// Start Docking Render
void Gui::dockingRender() {
  static bool opt_fullscreen_persistant = true;
  bool opt_fullscreen = opt_fullscreen_persistant;
  static ImGuiDockNodeFlags dockspace_flags = ImGuiDockNodeFlags_None;

  // Enable possible use of a menu bar.
  ImGuiWindowFlags window_flags = ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking;


  if(opt_fullscreen) {
    ImGuiViewport* viewport = ImGui::GetMainViewport();
    ImGui::SetNextWindowPos(viewport->Pos);
    ImGui::SetNextWindowSize(viewport->Size);
    ImGui::SetNextWindowViewport(viewport->ID);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
    window_flags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
    window_flags |= ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;
  }

  // Renders over draw calls...
  if(dockspace_flags & ImGuiDockNodeFlags_PassthruCentralNode) window_flags |= ImGuiWindowFlags_NoBackground;

  ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));

  ImGui::Begin("###DockSpace", &open, window_flags);

  ImGui::PopStyleVar();
  if(opt_fullscreen) ImGui::PopStyleVar(2);
  ImGuiIO &io = ImGui::GetIO();
  if(io.ConfigFlags & ImGuiConfigFlags_DockingEnable) {
    ImGuiID dockspace_id = ImGui::GetID("MyDockSpace");
    ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), dockspace_flags);
  }

  ImGui::End();
}


// Docking

void Gui::createEditor() {

}

// SHUTDOWN
Gui::~Gui() {
  ImGui_ImplOpenGL3_Shutdown();
  ImGui_ImplGlfw_Shutdown();
  ImGui::DestroyContext();
}

void Gui::captureViewport() {
  glBindFramebuffer(GL_FRAMEBUFFER, frameBufferObject);
  glBindTexture(GL_TEXTURE_2D, gameViewportTexture);
}
