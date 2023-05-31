#pragma once

#include "ACStaticBox.hpp"

// using namespace Slic3r::GUI;

class ACButton;
class ACToolBar : public ACStaticBox
{
public:
    ACToolBar(wxFrame *parent);
    ~ACToolBar();

    void Init(wxFrame *parent);

    void UpdateToolbarWidth(int width);

    void OnAddToPlate(wxCommandEvent &event);
    void OnSaveProject(wxCommandEvent &event);
    void OnUndo(wxCommandEvent &event);
    void OnRedo(wxCommandEvent &event);
    void OnOpenConfigDialog(wxCommandEvent &event);

    void Rescale();
private:
    wxFrame *m_frame;

    ACButton *m_btImport;
    ACButton *m_btSave;
    ACButton *m_undo_item;
    ACButton *m_redo_item;
    ACButton *m_config_item;

    int m_toolbar_h;
};
