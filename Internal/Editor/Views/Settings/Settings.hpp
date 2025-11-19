//
// Created by wpsimon09 on 26/02/25.
//

#ifndef SETTINGS_HPP
#define SETTINGS_HPP
#include "Editor/Views/UserInterface/IUserInterfaceElement.hpp"

class Client;

namespace VEditor {
class Editor;

class Settings : public IUserInterfaceElement
{
  public:
    Settings(Client& client, Editor* editor);

    void Render() override;
    void Resize(int newWidth, int newHeight) override;
    void Update() override;

    void Open();

  private:
    void RenderEngineSettings();
    void RenderApplicationSettings();
    void RenderEditorSettings();
    void RenderRenderingSettings();

    Client& m_client;
    Editor* m_editor;

    std::vector<const char*> m_toneMappingCurves = {"Aces aproximated", "Uchimura", "Aces full"};
    std::vector<const char*> m_cameraOption      = {"Pinhole", "Thin lens"}
};

}  // namespace VEditor

#endif  //SETTINGS_HPP
