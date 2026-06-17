#pragma once

#include "../modal.hpp"

struct UI;

struct EditorModal : public UIModal {
  public:
    explicit EditorModal(UI* ui);

    void on_escape() override;
    void on_remove() override;
    void render() override;

  private:
};
