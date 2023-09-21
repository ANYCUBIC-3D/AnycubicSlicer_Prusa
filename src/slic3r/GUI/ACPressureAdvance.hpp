#ifndef slic3r_ACPressureAdvance_hpp_
#define slic3r_ACPressureAdvance_hpp_

#include "GUI.hpp"
#include "GUI_Utils.hpp"
#include "wxExtensions.hpp"
#include "ACTextInput.hpp"
#include "ACButton.hpp"

#include <wx/dialog.h>
#include <wx/timer.h>
#include <vector>
#include <map>
#include <wx/textctrl.h>
#include <wx/regex.h>
#include <wx/wx.h>

namespace Slic3r {

namespace GUI {

class ACPressureAdvanceDialog : public DPIDialog
{
public:
enum PAInfoDialogType {
    ConstraintError,
    NotNumber,
};
public:
    explicit ACPressureAdvanceDialog(wxWindow *parent);
    ~ACPressureAdvanceDialog() = default;
    void show();
    /*void setDialogObj(wxDialog *indexObj) { markDialog = indexObj; }*/
    void setStyleButton(ACButton *bt);
    std::vector<wxString> getDefaultParEvent();
    void               OnTextCtrlKillFocusEvent(wxFocusEvent& event);
    void               Rescale();
    void                  ConfirmEvent(wxCommandEvent &event);
    void                  ShowInfoDialog(PAInfoDialogType type);

    float get_start_value();
    float get_end_value  ();
    float get_step_value ();

private:
    void create();
    void msw_rescale();
    void on_dpi_changed(const wxRect &suggested_rect) override { msw_rescale(); }
    /*wxDialog *   markDialog  = nullptr;*/
    wxBoxSizer *m_mainSizer = nullptr;
    wxBoxSizer * m_pageSizer = nullptr;
    ACStaticBox *m_page      = nullptr;
    wxTextCtrl *              m_startPA;
    wxTextCtrl *               m_endPA;
    wxTextCtrl *               m_stepPA;
    ACButton *                 m_okBtn;
    ACButton *                 first_label;
    ACButton *                 second_label;
    ACButton *                 third_label;
    bool                       isShortPriter        = false;

    float m_start_value = 0;
    float m_end_value   = 0.1f;
    float m_step_value  = 0.002f;

    std::map<std::string, int> defaultPrinterParMap = {
        {"Mega S",1},
        {"Mega Pro", 1},
        {"Mega x", 1},
        {"Vyper", 1}, 
        {"Kobra", 0}, 
        {"Kobra Go", 1}, 
        {"Kobra Neo", 0}, 
        {"Kobra Plus", 1}, 
        {"Kobra Max", 1}, 
        {"Kobra 2", 0}, 
        {"Kobra 2 Pro", 0}, 
        {"Kobra 2 PLus", 0}, 
        {"Kobra 2 Max", 0}
    };
};

} // GUI
} // Slic3r

#endif /* slic3r_ACPreferences_hpp_ */
