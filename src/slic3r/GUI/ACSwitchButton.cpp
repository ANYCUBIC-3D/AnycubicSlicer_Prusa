#include "ACSwitchButton.hpp"
#include "ACStateColor.hpp"
#include "ACLabel.hpp"
#include "ACStateHandler.hpp"
#include "wxExtensions.hpp"

#include <wx/dcgraph.h>
#include "ACButton.hpp"

#include "ACDefines.h"

wxDEFINE_EVENT(wxEVT_AC_TOGGLEBUTTON, wxCommandEvent);

ACSwitchButton::ACSwitchButton(wxWindow* parent, wxString textA, wxString textB, long style)
{
    Create(parent, textA, textB, style);
}

bool ACSwitchButton::Create(wxWindow* parent, wxString textA, wxString textB, long style)
{
    ACStaticBox::Create(parent, wxID_ANY, wxDefaultPosition, wxDefaultSize, style);

    ACStateColor bgColor;
    bgColor.append(AC_COLOR_PANEL_BG, ACStateColor::Normal);
    SetBackgroundColor(bgColor);
    SetCornerRadius(12);
    // 
    m_buttonA = new ACButton(this, textA, "", "", "", wxBORDER_NONE);
    m_buttonB = new ACButton(this, textB, "", "", "", wxBORDER_NONE);

    m_buttonA->SetButtonType(ACButton::AC_BUTTON_LV0);
    m_buttonB->SetButtonType(ACButton::AC_BUTTON_LV0_N);
    m_buttonA->SetPaddingSize(wxSize(12,6));
    m_buttonB->SetPaddingSize(wxSize(12,6));
    m_buttonA->Bind(wxEVT_BUTTON, [this](wxCommandEvent& event) {
        SetSwitchState(SW_A, true);
    });

    m_buttonB->Bind(wxEVT_BUTTON, [this](wxCommandEvent& event) {
        SetSwitchState(SW_B, true);
    });

    wxBoxSizer* mainSizer = new wxBoxSizer(wxHORIZONTAL);
    mainSizer->Add(m_buttonA, 0, wxLEFT|wxTOP|wxBOTTOM, m_padding);
    mainSizer->AddSpacer(m_spacing);
    mainSizer->Add(m_buttonB, 0, wxTOP|wxBOTTOM|wxRIGHT, m_padding);

    SetSizerAndFit(mainSizer);

    messureSize();

    return true;
}


void ACSwitchButton::SetSwitchState(SwitchState state, bool sendEvent)
{
    if (m_switchState == state)
        return;

    m_switchState = state;

    m_buttonA->SetButtonType(m_switchState == SW_A ? ACButton::AC_BUTTON_LV0 : ACButton::AC_BUTTON_LV0_N);
    m_buttonB->SetButtonType(m_switchState == SW_A ? ACButton::AC_BUTTON_LV0_N : ACButton::AC_BUTTON_LV0);

    if (sendEvent) {
        // wxEVT_TOGGLEBUTTON wxCommandEvent
        wxCommandEvent event(wxEVT_AC_TOGGLEBUTTON, GetId());
        event.SetEventObject(this);
        event.SetInt(m_switchState);
        GetEventHandler()->ProcessEvent(event);
    }
}

void ACSwitchButton::sys_color_changed()
{
    // AC : TODO
}


void ACSwitchButton::Rescale()
{
    messureSize();
}



void ACSwitchButton::messureSize()
{
    wxSize oldSize = GetSize();
    wxSize btAMinSize = m_buttonA == nullptr ? wxSize(0, 0) : m_buttonA->GetMinSize();
    wxSize btBMinSize = m_buttonB == nullptr ? wxSize(0, 0) : m_buttonB->GetMinSize();
    wxSize minSize(btAMinSize.x + btBMinSize.x + m_padding*2 + m_spacing, std::max(btAMinSize.y, btBMinSize.y) + m_padding*2);

    minSize.x = std::max(m_btMinSize.x, minSize.x);
    minSize.y = std::max(m_btMinSize.y, minSize.y);

    wxSize curSize = GetSize();
    SetMinSize(minSize);

    if (curSize.x < minSize.x || curSize.y < minSize.y)
        SetSize(std::max(curSize.x, minSize.x), std::max(curSize.y, minSize.y));
}


