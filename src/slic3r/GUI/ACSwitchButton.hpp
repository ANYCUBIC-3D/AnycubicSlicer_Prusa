#ifndef slic3r_GUI_ACSwitchButton_hpp_
#define slic3r_GUI_ACSwitchButton_hpp_

#include <wx/event.h>
#include "ACStaticBox.hpp"

wxDECLARE_EVENT(wxEVT_AC_TOGGLEBUTTON, wxCommandEvent);

class ACButton;
class ACSwitchButton : public wxNavigationEnabled<ACStaticBox>
{
public:
    enum SwitchState {
        SW_A = 0, // comSimple
        SW_B = 2, // comExpert
    };
public:
    ACSwitchButton(wxWindow* parent, wxString textA, wxString textB, long style = 0);

    bool Create(wxWindow* parent, wxString textA, wxString textB, long style = 0);

    void SetSwitchState(SwitchState state, bool sendEvent = false);

    void sys_color_changed();
protected:
    void Rescale();
    void messureSize();
private:
    SwitchState m_switchState = SW_A;

    ACButton *m_buttonA = nullptr;
    ACButton *m_buttonB = nullptr;

    int m_padding = 4;
    int m_spacing = 4;
    wxSize m_btMinSize = wxSize(108, 30);
};



#endif // !slic3r_GUI_ACSwitchButton_hpp_
