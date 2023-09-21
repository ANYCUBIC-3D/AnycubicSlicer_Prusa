#pragma once

#include "ACStaticBox.hpp"
#include "ACGauge.hpp"
#include <wx/statline.h>
// using namespace Slic3r::GUI;


class ACButton;
class ACGauge;
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
	void OnCloud(wxCommandEvent& event);
    void OnOpenConfigDialog(wxCommandEvent &event);
    void OnPressureAdvanceDialog(wxCommandEvent &event);

    void Rescale();
    void OnMouseLeftDown(wxMouseEvent &event);
    void OnMouseLeftUp(wxMouseEvent &event);
    void OnMouseMotion(wxMouseEvent &event);

    ACGauge *GetGaueObj() { return m_gauge; }

private:
    wxFrame *m_frame;

    ACButton *m_btImport;
    ACButton *m_btSave;
    ACButton *m_undo_item;
    ACButton *m_redo_item;
	ACButton* m_cloud;
    ACButton *m_config_item;
    ACButton *m_paTest_item;
    ACGauge * m_gauge;
	wxStaticLine* m_line;
    wxPoint   m_delta;
    int m_toolbar_h;
};
