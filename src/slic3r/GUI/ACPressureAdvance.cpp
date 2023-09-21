#include "ACPressureAdvance.hpp"
#include "GUI_App.hpp"
#include "OptionsGroup.hpp"
#include "libslic3r/AppConfig.hpp"
#include "GUI.hpp"
#include "MsgDialog.hpp"
#include <wx/intl.h>

#include "ACDefines.h"

namespace Slic3r {

namespace GUI {

ACPressureAdvanceDialog::ACPressureAdvanceDialog(wxWindow *parent)
    : DPIDialog(parent, wxID_ANY, _L("PressureAdvance"), wxDefaultPosition, wxDefaultSize, wxNO_BORDER, "ACPressureAdvanceDialog")
{
    create();
}
std::vector<wxString> ACPressureAdvanceDialog::getDefaultParEvent() 
{ 
    std::vector<wxString> m_parList = {"0", "0.1", "0.002"};

    return m_parList;
}

void ACPressureAdvanceDialog::create()
{
    AddWindowDrakEdg(this);
    this->SetBackgroundColour(AC_COLOR_WHITE);

    const int &em = em_unit();
    wxSize     _size(42 * em, 26 * em);
    SetSize(_size);

    m_pageSizer = new wxBoxSizer(wxVERTICAL);
    m_page      = new ACStaticBox(this);
    m_page->SetBackgroundColour(AC_COLOR_WHITE);
    m_page->SetCornerRadius(14);
    
    wxBoxSizer *first_sizer = new wxBoxSizer(wxHORIZONTAL);
    wxBoxSizer *second_sizer = new wxBoxSizer(wxHORIZONTAL);
    wxBoxSizer *third_sizer = new wxBoxSizer(wxHORIZONTAL);

    first_label = new ACButton(m_page, _L("Start PA"), "", "", "", wxNO_BORDER);
    setStyleButton(first_label);
    second_label = new ACButton(m_page, _L("End PA"), "", "", "", wxNO_BORDER);
    setStyleButton(second_label);
    third_label = new ACButton(m_page, _L("PA Step"), "", "", "", wxNO_BORDER);
    setStyleButton(third_label);
    std::vector<wxString> m_parList = getDefaultParEvent();

    m_startPA = new wxTextCtrl(m_page,wxID_ANY, m_parList[0],wxDefaultPosition, wxSize(22 * em, 2.8 * em), wxTE_PROCESS_ENTER,wxTextValidator(wxFILTER_NUMERIC));
    m_endPA   = new wxTextCtrl(m_page, wxID_ANY,m_parList[1], wxDefaultPosition, wxSize(22 * em, 2.8 * em), wxTE_PROCESS_ENTER,wxTextValidator(wxFILTER_NUMERIC));
    m_stepPA  = new wxTextCtrl(m_page, wxID_ANY,m_parList[2], wxDefaultPosition, wxSize(22 * em, 2.8 * em), wxTE_PROCESS_ENTER,wxTextValidator(wxFILTER_NUMERIC));
    m_startPA->SetForegroundColour(AC_COLOR_BLACK);
    m_endPA->SetForegroundColour(AC_COLOR_BLACK);
    m_stepPA->SetForegroundColour(AC_COLOR_BLACK);

    m_okBtn = new ACButton(m_page, _L("OK"), "", "", "", wxNO_BORDER);
    m_okBtn->SetPaddingSize(wxSize(40, 6));
    m_okBtn->SetButtonType(ACButton::AC_BUTTON_LV0);

    first_sizer->Add(first_label, 0, wxLEFT,20);
    first_sizer->Add(0, 0, 1, wxEXPAND, 0);
    first_sizer->Add(m_startPA, 0, wxALIGN_RIGHT | wxRIGHT, 20);


    second_sizer->Add(second_label, 0, wxLEFT, 20);
    second_sizer->Add(0, 0, 1, wxEXPAND, 0);
    second_sizer->Add(m_endPA, 0, wxALIGN_RIGHT | wxRIGHT, 20);


    third_sizer->Add(third_label, 0, wxLEFT, 20);
    third_sizer->Add(0, 0, 1, wxEXPAND, 0);
    third_sizer->Add(m_stepPA, 0, wxALIGN_RIGHT | wxRIGHT, 20);

    m_pageSizer->Add(first_sizer,0, wxEXPAND|wxTOP, 20);
    m_pageSizer->Add(second_sizer, 0, wxEXPAND | wxTOP,20);
    m_pageSizer->Add(third_sizer, 0, wxEXPAND | wxTOP,20);
    m_pageSizer->Add(0, 0, 1, wxEXPAND, 0);
    m_pageSizer->Add(m_okBtn, 0, wxBOTTOM |wxALIGN_RIGHT | wxRIGHT, 20);

    ACDialogTopbar *topbar = new ACDialogTopbar(this, _L("Pressure Advance Calibration"), 42, 4.2 * em);
    m_page->SetSizer(m_pageSizer);
    
    m_mainSizer = new wxBoxSizer(wxVERTICAL);
    m_mainSizer->Add(topbar, 0, wxEXPAND | wxTOP | wxLEFT | wxRIGHT, 1);
    m_mainSizer->Add(m_page, 1, wxEXPAND | wxLEFT | wxRIGHT | wxBOTTOM, 1);
    SetSizer(m_mainSizer);
    int screenwidth  = wxSystemSettings::GetMetric(wxSYS_SCREEN_X, NULL);
    int screenheight = wxSystemSettings::GetMetric(wxSYS_SCREEN_Y, NULL);
    SetPosition(wxPoint((screenwidth - _size.x) / 2, (screenheight - _size.y) / 2));
    Layout();
    Refresh();
    m_okBtn->Bind(wxEVT_BUTTON, &ACPressureAdvanceDialog::ConfirmEvent,this);
}
static int getDecimalPartNum(std::string infoStr)
{
    size_t dotPos = infoStr.find('.');
    if (dotPos != std::string::npos) {
        std::string decimalPart = infoStr.substr(dotPos + 1);
        return decimalPart.size();
    }
    return 0;
}

static bool checkContent(wxString info) {
    wxString characters = "0123456789.";
    for (size_t i = 0; i < info.length(); ++i) {
        if (characters.Find(info[i]) == wxString::npos) {
            return false;
        }
    }
    return true;
}


void ACPressureAdvanceDialog::ShowInfoDialog(PAInfoDialogType type) {
    wxString contenStr;
    if (type == ConstraintError) {
        contenStr = _L("Please input valid values:\nStart PA >=0\nEnd PA >= Start PA + PA step\nPA step >= 0.001\n");
    } else {
        contenStr = _L("Please enter numbers and numbers of decimal separator type");
    }
    WarningDialog(this, contenStr).ShowModal();
}

static wxString strChange(wxString infoStr) {
    std::string str = infoStr.ToStdString();

    while (!str.empty() && str.back() == '0' && str.find('.') != std::string::npos) {
        str.pop_back();
    }

    if (!str.empty() && str.back() == '.') {
        str.pop_back();
    }
    return wxString(str);

}
void ACPressureAdvanceDialog::ConfirmEvent(wxCommandEvent &event)
{ 
    wxString m_startString = m_startPA->GetValue();
    if (!checkContent(m_startString)) {
        ShowInfoDialog(NotNumber);
        return;
    }
    wxString m_endString = m_endPA->GetValue();
    if (!checkContent(m_endString)) {
        ShowInfoDialog(NotNumber);
        return;
    }
    wxString m_stepString = m_stepPA->GetValue();
    if (!checkContent(m_stepString)) {
        ShowInfoDialog(NotNumber);
        return;
    }

    if (m_startString.length() <= 0 || m_endString.length() <= 0 || m_stepString.length() <=0) {
        ShowInfoDialog(ConstraintError);
        return;
    }

    m_start_value = std::stof(m_startString.ToStdString());
    m_end_value   = std::stof(m_endString.ToStdString());
    m_step_value  = std::stof(m_stepString.ToStdString());
    if (m_start_value < 0 || m_end_value < m_start_value + m_step_value || m_step_value < 0.001) {
        ShowInfoDialog(ConstraintError);
        return;
    }

    this->EndModal(wxID_OK);
}

float ACPressureAdvanceDialog::get_start_value() 
{
    return m_start_value;
}

float ACPressureAdvanceDialog::get_end_value  () 
{
    return m_end_value;
;
}

float ACPressureAdvanceDialog::get_step_value () 
{
    return m_step_value;
}


void ACPressureAdvanceDialog::Rescale()
{
    m_okBtn->Rescale();
    first_label->Rescale();
    second_label->Rescale();
    third_label->Rescale();
}


void ACPressureAdvanceDialog::OnTextCtrlKillFocusEvent(wxFocusEvent &event)
{
    wxKeyEvent enterKey(wxEVT_CHAR);
    enterKey.m_keyCode = WXK_RETURN;
    auto *eventObject = dynamic_cast<wxTextCtrl *> (event.GetEventObject());
    if (eventObject) {
        enterKey.SetEventObject(eventObject);
        eventObject->GetEventHandler()->ProcessEvent(enterKey);
    }
    event.Skip();
}

void ACPressureAdvanceDialog::setStyleButton(ACButton *bt) {
    
    bt->SetCanFocus(false);
    bt->SetEnable(false);
    bt->SetButtonType(ACButton::AC_BUTTON_LABEL);
    bt->SetPaddingSize(wxSize(0, 4));
}

void ACPressureAdvanceDialog::msw_rescale()
{
    const int &em = em_unit();
    wxSize _size(42 * em, 26 * em);
    this->SetMinSize(_size);

    Fit();

    this->Layout();

    Refresh();
}
void ACPressureAdvanceDialog::show()
{
    
    /*if (markDialog == nullptr)
        setDialogObj(setMarkWindow(this->GetParent(), this));*/
    this->Show();
}

} // GUI
} // Slic3r


