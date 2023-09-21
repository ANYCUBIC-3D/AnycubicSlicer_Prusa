#include "ACCloudAddMachine.hpp"

#include "ACCloudMachine.hpp"

#include <wx/sizer.h>
#include <wx/panel.h>
#include <wx/font.h>
#include <wx/dcgraph.h>
#include <string>
#include <stdio.h>
#include "../wxExtensions.hpp"
#include "../MsgDialog.hpp"
#include "../MainFrame.hpp"

namespace Slic3r {

namespace GUI {

ACCloudAddMachine::ACCloudAddMachine(wxWindow *parent)
    : DPIDialog(parent, wxID_ANY, "", wxDefaultPosition, wxDefaultSize, wxNO_BORDER)
    , m_parent(parent)
{ 
	Init(); 
}

ACCloudAddMachine::~ACCloudAddMachine() {

}


wxSize ACCloudAddMachine::GetWindowSize()
{
    int winSize_W = 80 * em_unit();
    int winSize_H = 52 * em_unit();
    return wxSize(winSize_W, winSize_H);
}


void ACCloudAddMachine::SetButtonStyle(ACButton *btn)
{
    btn->SetCornerRadius(6);
    btn->SetPaddingSize(wxSize(0, 0));
    btn->SetSpacing(0);
    btn->SetTextColor(AC_COLOR_MAIN_BLUE);

}

void ACCloudAddMachine::SetButtonStyle_Label(ACButton *btn)
{
    btn->SetCanFocus(false);
    btn->SetEnable(false);
    btn->SetPaddingSize(wxSize(0, 0));
    btn->SetButtonType(ACButton::AC_BUTTON_LABEL);
}

wxPanel *ACCloudAddMachine::CreateInputEvent() {
    wxPanel *infoPanel = new wxPanel(this);

    wxBoxSizer *centerSizer = new wxBoxSizer(wxHORIZONTAL);
    std::vector<ACTextInput *> inputList;
    for (int i = 0; i < 19; i++) {
        if (i == 4 || i == 9 || i == 14) {
            wxBoxSizer *lineSizer = new wxBoxSizer(wxVERTICAL);
            wxPanel *m_line = new wxPanel(infoPanel, wxID_ANY, wxDefaultPosition, wxSize(16, 1), wxTAB_TRAVERSAL);
            m_line->SetBackgroundColour(wxColour(219, 219, 219));
            lineSizer->Add(0, 0, 1, wxEXPAND);
            lineSizer->Add(m_line, 0,wxEXPAND);
            lineSizer->Add(0, 0, 1, wxEXPAND);
            centerSizer->Add(lineSizer, 0, wxEXPAND | wxLEFT, 8);
        } else {
            ACTextInput *inputInfo = new ACTextInput(infoPanel,"","", "",wxSize(), wxDefaultPosition, wxSize(32, 48),wxTE_CENTER);
            inputInfo->GetTextCtrl()->SetFont(ACLabel::Head_18);
            inputInfo->GetTextCtrl()->SetSize(wxSize(30, 30));
            inputInfo->GetTextCtrl()->SetForegroundColour(AC_COLOR_MAIN_BLUE);
            inputInfo->SetBackgroundColor(AC_COLOR_WHITEBLUE);
            inputInfo->SetBackgroundColor2(AC_COLOR_WHITEBLUE);
            inputInfo->GetTextCtrl()->SetBackgroundColour(AC_COLOR_WHITEBLUE);
            centerSizer->Add(inputInfo, 0, wxLEFT, 8);
            inputList.push_back(inputInfo);
        }
    }

    for (size_t i = 0; i < inputList.size(); ++i) {
        inputList[i]->GetTextCtrl()->Bind(wxEVT_CHAR, [i, inputList, infoPanel, this](wxKeyEvent &event) {
            int    keyCode = event.GetKeyCode();
            if ((keyCode >= '0' && keyCode <= '9') || (keyCode >= 'A' && keyCode <= 'Z') || (keyCode >= 'a' && keyCode <= 'z') ||
                keyCode == WXK_BACK) {
                size_t nextIndex;
                if (event.GetKeyCode() == WXK_BACK) {
                    if (inputList[i]->GetLabel().length() != 0) {
                        nextIndex = i;
                    } else {
                        nextIndex = (i - 1) % inputList.size();
                    }
                    inputList[nextIndex]->SetLabel("");
                    bool isAdd = true;
                    wxString sumStr;
                    for (ACTextInput *input : inputList) {
                        wxString label = input->GetLabel();
                        if (label == "")
                            isAdd = false;
                        sumStr += label;
                    }
                    if (isAdd) {
                        if (m_showErrInfo->IsShown()) {
                            m_showErrInfo->Hide();
                        }
                        if (!m_buttonAdd->IsEnabled()) {
                            m_buttonAdd->Enable(true);
                            m_CNInfo = sumStr;
                        }
                    } else {
                        if (!m_showErrInfo->IsShown()) {
                            m_showErrInfo->Show();
                        }
                        if (m_buttonAdd->IsEnabled())
                            m_buttonAdd->Enable(false);
                    }
                    if (i == 0)
                        return;

                } else {
                    inputList[i]->SetLabel(std::toupper(static_cast<wxChar>(event.GetUnicodeKey()), std::locale()));
                    bool isAdd = true;
                    wxString sumStr;
                    for (ACTextInput *input : inputList) {
                        wxString label = input->GetLabel();
                        if (label == "")
                            isAdd = false;
                        sumStr += label;
                    }
                    if (isAdd) {
                        if (m_showErrInfo->IsShown()) {
                            m_showErrInfo->Hide();
                        }
                        if (!m_buttonAdd->IsEnabled()) {
                            m_buttonAdd->Enable(true);
                            m_CNInfo = sumStr;
                        }
                    } else {
                        if (!m_showErrInfo->IsShown()) {
                            m_showErrInfo->Show();
                        }
                        if (m_buttonAdd->IsEnabled())
                            m_buttonAdd->Enable(false);
                    }
                    if (i == inputList.size() - 1) {
                        infoPanel->SetFocus();
                        return;
                    }
                    nextIndex = (i + 1) % inputList.size();
                }
                inputList[nextIndex]->SetFocus();
                inputList[nextIndex]->SetEditable(true);
            }
        });
    }
    inputList[0]->SetFocus();
    inputList[0]->SetEditable(true);

    infoPanel->SetSizer(centerSizer);
    return infoPanel;
}

void ACCloudAddMachine::Init()
{
    AddWindowDrakEdg(this);
    
    this->SetBackgroundColour(AC_COLOR_WHITE);
    m_up_sizer  = new wxBoxSizer(wxHORIZONTAL);
    wxBoxSizer* up_sizer_left = new wxBoxSizer(wxHORIZONTAL);
    wxBoxSizer *up_sizer_right = new wxBoxSizer(wxVERTICAL);
    m_up_sizer               = new wxBoxSizer(wxHORIZONTAL);
    m_down_sizer             = new wxBoxSizer(wxHORIZONTAL);
    m_center_sizer           = new wxBoxSizer(wxHORIZONTAL); 
    m_pageSizer              = new wxBoxSizer(wxVERTICAL);
    m_mainSizer              = new wxBoxSizer(wxVERTICAL);

    wxString img_name = "icon-addCloudPrinter";

    ACButton *showImg = new ACButton(this, "", img_name, img_name, img_name, wxNO_BORDER, wxSize(240, 180));
    showImg->SetPaddingSize(wxSize(0, 0));
    showImg->SetButtonType(ACButton::AC_BUTTON_ICON);
    up_sizer_left->Add(showImg, 0, wxEXPAND);

    ACButton* right_1 = new ACButton(this, _L("1. Enter the system menu on your printer"), "", "", "");
    SetButtonStyle_Label(right_1);
    ACButton *right_2 = new ACButton(this, _L("2. Click the service button"), "", "", "");
    SetButtonStyle_Label(right_2);
    ACButton *right_3 = new ACButton(this, _L("3. Enter the 16 digit Device CN number you see on the printer"), "", "", "");
    SetButtonStyle_Label(right_3);
    up_sizer_right->Add(right_1, 0, wxTOP,46);
    up_sizer_right->Add(right_2, 0, wxTOP,24);
    up_sizer_right->Add(right_3, 0, wxTOP,24);

    m_inputPanel = CreateInputEvent();
    m_inputPanel->SetBackgroundColour(AC_COLOR_WHITE);
    m_center_sizer->Add(m_inputPanel, 0, wxEXPAND|wxLEFT,38);

    m_showErrInfo = new ACButton(this, _L("Device CN can not be empty!"), "", "", "");
    SetButtonStyle_Label(m_showErrInfo);
    m_showErrInfo->SetTextColor(AC_COLOR_BOARD_RED);

    m_buttonCancel = new ACButton(this, _L("Cancel"), "", "", "");
    SetButtonStyle(m_buttonCancel);
    m_buttonCancel->SetButtonType(ACButton::AC_BUTTON_TYPE::AC_BUTTON_LV2);
    m_buttonCancel->SetMinSize(wxSize(96, 30));

    m_buttonAdd = new ACButton(this, _L("Add"), "", "", "");
    SetButtonStyle(m_buttonAdd);
    m_buttonAdd->SetButtonType(ACButton::AC_BUTTON_TYPE::AC_BUTTON_LV0);
    m_buttonAdd->SetMinSize(wxSize(96, 30));
    m_buttonAdd->Enable(false);

    m_down_sizer->Add(m_showErrInfo, 0, wxLEFT, 20);
    m_down_sizer->Add(0, 0, 1, wxEXPAND, 5);
    m_down_sizer->Add(m_buttonCancel, 0, wxLEFT, 20);
    m_down_sizer->Add(m_buttonAdd, 0, wxLEFT, 20);


    m_up_sizer->Add(up_sizer_left, 0, wxLEFT, 42);
    m_up_sizer->Add(up_sizer_right, 0, wxLEFT, 10);

    m_pageSizer->Add(m_up_sizer, 0, wxEXPAND|wxTOP,20);
    ACButton* enter_label = new ACButton(this, _L("Enter Device CN"), "", "", "");
    SetButtonStyle_Label(enter_label);
    m_pageSizer->Add(enter_label, 0, wxLEFT, 47);
    m_pageSizer->Add(m_center_sizer, 0, wxEXPAND | wxTOP, 14);
    m_pageSizer->Add(0, 0, 1, wxEXPAND, 5);
    m_pageSizer->Add(m_down_sizer, 0, wxEXPAND | wxALL, 20);


    

    ACDialogTopbar *topbar = new ACDialogTopbar(this, "", 46);
    topbar->SetTitle(_L("Add Cloud Printer"));
    m_mainSizer->Add(topbar, 0, wxEXPAND|wxLEFT|wxRIGHT|wxTOP,1);
    m_mainSizer->Add(m_pageSizer, 1, wxEXPAND | wxLEFT | wxRIGHT | wxBOTTOM,1);

    SetSize(GetWindowSize());
    SetSizer(m_mainSizer);
    Layout();
    Refresh();
    int screenwidth  = wxSystemSettings::GetMetric(wxSYS_SCREEN_X, NULL);
    int screenheight = wxSystemSettings::GetMetric(wxSYS_SCREEN_Y, NULL);
    SetPosition(wxPoint((screenwidth - GetWindowSize().x) / 2, (screenheight - GetWindowSize().y) / 2));
    m_buttonCancel->Bind(wxEVT_BUTTON, &ACCloudAddMachine::OnButtonCancelEvent, this, m_buttonCancel->GetId());
    m_buttonAdd->Bind(wxEVT_BUTTON, &ACCloudAddMachine::OnButtonAddEvent, this, m_buttonAdd->GetId());
}


void ACCloudAddMachine::OnButtonCancelEvent(wxCommandEvent &event) {
    this->EndModal(wxID_NO);
}
void ACCloudAddMachine::OnButtonAddEvent(wxCommandEvent &event)
{
    this->EndModal(wxID_YES);
}

void ACCloudAddMachine::msw_rescale()
{
    wxSize     _size = GetWindowSize();
    this->SetMinSize(_size);

    Fit();

    this->Layout();

    Refresh();
}


    } // GUI
} // Slic3r
