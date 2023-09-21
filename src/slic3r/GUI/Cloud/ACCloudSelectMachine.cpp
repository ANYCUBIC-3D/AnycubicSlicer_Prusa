#include "ACCloudSelectMachine.hpp"

#include "ACCloudMachine.hpp"
#include "ACCloudManger.hpp"
#include <wx/sizer.h>
#include <wx/panel.h>
#include <wx/font.h>
#include <wx/dcgraph.h>
#include <string>
#include <stdio.h>
#include "../wxExtensions.hpp"
#include "../MsgDialog.hpp"
#include "../MainFrame.hpp"
#include <wx/richtooltip.h>

namespace Slic3r {

namespace GUI {

ACCloudSelectMachine::ACCloudSelectMachine(wxWindow *parent)
    : DPIDialog(parent, wxID_ANY, "", wxDefaultPosition, wxDefaultSize, wxNO_BORDER)
    , m_parent(parent)
{ 
	Init(); 
}

ACCloudSelectMachine::~ACCloudSelectMachine() {
    if (m_timer != nullptr) {
        delete m_timer;
    }
}

wxPanel *ACCloudSelectMachine::CreateShowPanel(std::vector<PrinterData>& printerInfo)
{

    std::vector<PrinterData> a_printerInfo;
    std::vector<PrinterData> un_printerInfo;

    for (PrinterData data : printerInfo) {
        if (data.status == 1) {
            a_printerInfo.push_back(data);
        } else {
            un_printerInfo.push_back(data);
        }
    }

    wxPanel *infoPanel = new wxPanel(this);
    infoPanel->SetBackgroundColour(AC_COLOR_WHITE);
    m_down_sizer = new wxBoxSizer(wxHORIZONTAL);
    wxBoxSizer *panelPageSizer = new wxBoxSizer(wxVERTICAL);
    unvaliable_sizer   = new wxBoxSizer(wxHORIZONTAL);


    m_Avprinter = new ACPrinterContainer(infoPanel, a_printerInfo,false);
    m_Unvprinter = new ACPrinterContainer(infoPanel, un_printerInfo,false);


    wxString m_iconName_undo_nor = "icon-alert-circle";
    wxString m_iconName_undo_hov = "icon-alert-circle";
    wxString m_iconName_undo_dis = "icon-alert-circle";

    m_UnailablePrinter = new ACButton(infoPanel, _L("Unvailable Printer (2)"), "", "", "");
    SetButtonStyle_Label(m_UnailablePrinter);

    m_UnailablePrinter_btn        = new ACButton(infoPanel, "", m_iconName_undo_nor, m_iconName_undo_hov, m_iconName_undo_dis, wxNO_BORDER, wxSize(24, 24));
    m_UnailablePrinter_btn->SetPaddingSize(wxSize(0, 0));
    m_UnailablePrinter_btn->SetButtonType(ACButton::AC_BUTTON_ICON);
    wxString tipInfo = _L("Printers that meet one of the following\nconditions are unavailable:\n * Printer status if offline\n * Printer status is busy\n * Printer type is not filament 3D printer\n");
    /*wxRichToolTip wxTip(_L("Printers that meet one of the following conditions are unavailable\n"), _L(" * Printer status if offline\n * Printer status is busy\n * Printer type is not filament 3D printer\n"));
    wxTip.SetIcon(wxICON_INFORMATION);
    wxTip.ShowFor(m_UnailablePrinter_btn);*/
    
    m_UnailablePrinter_btn->SetToolTip(tipInfo);

    unvaliable_sizer->Add(m_UnailablePrinter,0,wxLEFT,20);
    unvaliable_sizer->Add(m_UnailablePrinter_btn,0,wxLEFT,5);
    wxPanel *mLine = new wxPanel(infoPanel);
    mLine->SetBackgroundColour(AC_COLOR_BD_BLACK);
    mLine->SetSize(GetWindowSize().x, 1);

    wxBoxSizer *showNoPrinterInfoSizer = new wxBoxSizer(wxHORIZONTAL);

    if (a_printerInfo.size() > 0) {
        panelPageSizer->Add(m_Avprinter , 1, wxEXPAND | wxALL, 20);
    } else {
        ACButton *noAvaliablePrinterInfo = new ACButton(infoPanel, _L("No available cloud printers found in your account."), "", "", "");
        SetButtonStyle_Label(noAvaliablePrinterInfo);
        showNoPrinterInfoSizer->Add(noAvaliablePrinterInfo, 0, wxBOTTOM, 50);
        panelPageSizer->Add(showNoPrinterInfoSizer, 0, wxLEFT|wxTOP, 20);
    }
    panelPageSizer->Add(unvaliable_sizer, 0, wxEXPAND);
    panelPageSizer->Add(mLine, 0, wxEXPAND | wxALL, 20);
    panelPageSizer->Add(m_Unvprinter, 1, wxEXPAND | wxALL, 20);

    panelPageSizer->Add(m_down_sizer, 0, wxEXPAND|wxTOP,40);


    m_showTipInfoBtn   = new ACButton(infoPanel, _L(""), "", "", "",0,wxSize(16,16));
    m_showTipInfoBtn->SetFont(ACLabel::Body_13);

    m_buttonCancel     = new ACButton(infoPanel, _L("Cancel"), "", "", "");
    m_buttonStartPrint = new ACButton(infoPanel, _L("Start Printing"), "", "", "");
    SetButtonStyle(m_buttonCancel);
    m_buttonCancel->SetMinSize(wxSize(113, 33));
    m_buttonCancel->SetButtonType(ACButton::AC_BUTTON_TYPE::AC_BUTTON_LV2);
    SetButtonStyle(m_buttonStartPrint);
    m_buttonStartPrint->SetButtonType(ACButton::AC_BUTTON_TYPE::AC_BUTTON_LV0);
    m_buttonStartPrint->SetMinSize(wxSize(150, 33));

    m_down_sizer->Add(0, 0, 1, wxEXPAND, 5);
    m_down_sizer->Add(m_showTipInfoBtn, 0, wxALL, 20);
    m_down_sizer->Add(0, 0, 1, wxEXPAND, 5);
    m_down_sizer->Add(m_buttonCancel, 0, wxTOP | wxBOTTOM, 20);
    m_down_sizer->Add(m_buttonStartPrint, 0, wxALL, 20);

    m_buttonCancel->Bind(wxEVT_BUTTON, &ACCloudSelectMachine::OnButtonCancelEvent, this, m_buttonCancel->GetId());
    m_buttonStartPrint->Bind(wxEVT_BUTTON, &ACCloudSelectMachine::OnButtonStartPrintEvent, this, m_buttonStartPrint->GetId());
    infoPanel->SetSizer(panelPageSizer);
    SetAvailablePrinterNum(a_printerInfo.size());
    SetUnailablePrinterNum(un_printerInfo.size());
    return infoPanel;
}

void ACCloudSelectMachine::SetTipBtnStyle(bool isSucce, bool isAddObj,int opStyle)
{
    //opStyle : 0. refresh  1.rename 2.delete
    if (!m_showTipInfoBtn->IsShown()) {
        m_showTipInfoBtn->Show();
    }
    wxString labelInfo;
    wxSize   btnSize;
    wxColour textColor;
    wxColour bgColor;
    wxString icoName;
    if (!isAddObj) {
        if (isSucce) {
            textColor = wxColour(114, 194, 64);
            bgColor   = wxColour(228, 245, 217);
            icoName   = "icon-successfully-nor";
            if (opStyle == 0) {
                labelInfo = _L("Refresh successful");
                btnSize   = wxSize(193, 49);
            } else if (opStyle == 1) {
                labelInfo = _L("Printer rename successful");
                btnSize   = wxSize(237, 49);
            } else if (opStyle == 2) {
                labelInfo = _L("Printer Has Been Deleted");
                btnSize   = wxSize(232, 49);
            }
        } else {
            icoName   = "icon-fail-nor";
            textColor = wxColour(236, 91, 86);
            bgColor   = wxColour(255, 246, 245);
            if (opStyle == 0) {
                labelInfo = _L("Refresh failed. Please check your network and try again");
                btnSize   = wxSize(420, 49);
            } else if (opStyle == 1) {
                labelInfo = _L("Rename failed. Please check your network and try again");
                btnSize   = wxSize(425, 49);
            } else if (opStyle == 2) {
                labelInfo = _L("Failed To Delete Printer. Please Check Your Network And Try Again");
                btnSize   = wxSize(491, 49);
            }
        }
    }else {
        if (isSucce) {
            icoName   = "icon-successfully-nor";
            labelInfo = _L("Printer Added Successfully");
            btnSize   = wxSize(247, 49);
            textColor = wxColour(114, 194, 64);
            bgColor   = wxColour(228, 245, 217);
        } else {
            icoName   = "icon-fail-nor";
            labelInfo = _L("Printer Added Fail");
            btnSize   = wxSize(247, 49);
            textColor = wxColour(236, 91, 86);
            bgColor   = wxColour(255, 246, 245);
        }
    }
    m_showTipInfoBtn->SetBackgroundColor(bgColor);
    m_showTipInfoBtn->SetTextColor(textColor);
    m_showTipInfoBtn->SetBorderColor(textColor);
    m_showTipInfoBtn->SetMinSize(btnSize);
    m_showTipInfoBtn->SetLabel(labelInfo);
    m_showTipInfoBtn->SetIcon(icoName);
    m_showTipInfoBtn->SetSpacing(4);

    if (isAddObj) {
        center_label_Sizer->Layout();
    } else {
        m_down_sizer->Layout();
    }
    
}

wxPanel *ACCloudSelectMachine::CreateEmptyPanel() 
{

    wxPanel *emptyPanel = new wxPanel(this);
    emptyPanel->SetBackgroundColour(AC_COLOR_WHITE);
    wxBoxSizer *panelPageSizer = new wxBoxSizer(wxVERTICAL);
    wxBoxSizer *leftSizer = new wxBoxSizer(wxHORIZONTAL);
    wxBoxSizer *centerSizer    = new wxBoxSizer(wxVERTICAL);

    center_label_Sizer = new wxBoxSizer(wxVERTICAL);
    wxBoxSizer *center_btn_Sizer = new wxBoxSizer(wxVERTICAL);

    ACButton *m_empty_buttonAddNew = CreateAddPrinterEvent(emptyPanel);
    m_empty_buttonAddNew->SetButtonType(ACButton::AC_BUTTON_TYPE::AC_BUTTON_LV0);
    m_empty_buttonAddNew->SetMinSize(wxSize(159, 33));

    center_btn_Sizer->Add(m_empty_buttonAddNew, 0, wxLEFT, 25);

    wxString  m_iconName_noprinter    = "icon-noprinter";

    ACButton *empty_printer_img = new ACButton(emptyPanel, "", m_iconName_noprinter, m_iconName_noprinter, m_iconName_noprinter,wxNO_BORDER, wxSize(450, 212));
    empty_printer_img->SetPaddingSize(wxSize(0, 0));
    empty_printer_img->SetButtonType(ACButton::AC_BUTTON_ICON);

    

    ACButton *empty_info = new ACButton(emptyPanel, _L("No Cloud Printer Found In Your Account"), "", "", "");
    SetButtonStyle_Label(empty_info);
    


    m_showTipInfoBtn = new ACButton(emptyPanel, _L("22222222"), "", "", "",0,wxSize(16,16));
    m_showTipInfoBtn->SetFont(ACLabel::Body_13);


    center_label_Sizer->Add(empty_info, 0, wxTOP, 5);
    center_label_Sizer->Add(center_btn_Sizer, 0, wxALL, 20);
    center_label_Sizer->Add(m_showTipInfoBtn, 0, wxTOP, 5);
    

    centerSizer->Add(empty_printer_img, 0, wxLEFT, 344);
    centerSizer->Add(center_label_Sizer, 0, wxLEFT, 435);
    


    panelPageSizer->Add(centerSizer, 1, wxTOP, 124);

    emptyPanel->SetSizer(panelPageSizer);

    m_isAddPanel = true;

    return emptyPanel;
}

wxSize ACCloudSelectMachine::GetWindowSize() {
    int winSize_W = 118 * em_unit();
    int winSize_H = 68 * em_unit();
    return wxSize(winSize_W, winSize_H);
}

std::vector<PrinterData> ACCloudSelectMachine::GetCloudInfo(bool index)
{
    return Cloud->GetPrinterInfo();
    std::vector<PrinterData> m_info;
    if (index)
        return m_info;
    PrinterData              data;
    data.fileName = std::string("4MAXPRO20_thumbnail");
    data.name     = "My Printer 1 123456789 123456789123456789123";
    data.status   = 0;
    data.type     = "Kobra 2 neo";
    data.id       = 0;
    m_info.push_back(data);

    data.fileName = std::string("Kobra2 Pro_thumbnail");
    data.name     = "My Printer 2 2341afwetg";
    data.status   = 1;
    data.type     = "Kobra 2 pro";
    data.id       = 1;
    m_info.push_back(data);

    data.fileName = std::string("Kobra2_thumbnail");
    data.name     = "My Printer 3";
    data.status   = 2;
    data.type     = "Kobra 2";
    data.id       = 2;
    m_info.push_back(data);

    data.fileName = std::string("I3MEGAS_thumbnail");
    data.name     = "My Printer 4";
    data.status   = 0;
    data.type     = "Kobra 2 neo";
    data.id       = 3;
    m_info.push_back(data);

    data.fileName = std::string("MEGA0_thumbnail");
    data.name     = "My Printer 5";
    data.status   = 1;
    data.type     = "Kobra 2 pro";
    data.id       = 4;
    m_info.push_back(data);

    data.fileName = std::string("Kobra2_thumbnail");
    data.name     = "My Printer 6";
    data.status   = 2;
    data.type     = "Kobra 2";
    data.id       = 5;
    m_info.push_back(data);

    data.fileName = std::string("Kobra2 Neo_thumbnail");
    data.name     = "My Printer 7";
    data.status   = 0;
    data.type     = "Kobra 2 neo";
    data.id       = 6;
    m_info.push_back(data);

    data.fileName = std::string("Kobra2 Pro_thumbnail");
    data.name     = "My Printer 8";
    data.status   = 1;
    data.type     = "Kobra 2 pro";
    data.id       = 7;
    m_info.push_back(data);

    data.fileName = std::string("Kobra2_thumbnail");
    data.name     = "My Printer 9";
    data.status   = 2;
    data.type     = "Kobra 2";
    data.id       = 8;
    m_info.push_back(data);

    data.fileName = std::string("Kobra2_thumbnail");
    data.name     = "My Printer 10";
    data.status   = 2;
    data.type     = "Kobra 2";
    data.id       = 9;
    m_info.push_back(data);

    data.fileName = std::string("4MAXPRO20_thumbnail");
    data.name     = "My Printer 11 123456789 123456789123456789123";
    data.status   = 0;
    data.type     = "Kobra 2 neo";
    data.id       = 10;
    m_info.push_back(data);

    data.fileName = std::string("Kobra2 Pro_thumbnail");
    data.name     = "My Printer 12 2341afwetg";
    data.status   = 1;
    data.type     = "Kobra 2 pro";
    data.id       = 11;
    m_info.push_back(data);

    data.fileName = std::string("Kobra2_thumbnail");
    data.name     = "My Printer 13";
    data.status   = 2;
    data.type     = "Kobra 2";
    data.id       = 12;
    m_info.push_back(data);

    data.fileName = std::string("I3MEGAS_thumbnail");
    data.name     = "My Printer 14";
    data.status   = 0;
    data.type     = "Kobra 2 neo";
    data.id       = 13;
    m_info.push_back(data);

    data.fileName = std::string("MEGA0_thumbnail");
    data.name     = "My Printer 15";
    data.status   = 1;
    data.type     = "Kobra 2 pro";
    data.id       = 14;
    m_info.push_back(data);

    data.fileName = std::string("Kobra2_thumbnail");
    data.name     = "My Printer 16";
    data.status   = 2;
    data.type     = "Kobra 2";
    data.id       = 15;
    m_info.push_back(data);

    data.fileName = std::string("Kobra2 Neo_thumbnail");
    data.name     = "My Printer 17";
    data.status   = 0;
    data.type     = "Kobra 2 neo";
    data.id       = 16;
    m_info.push_back(data);

    data.fileName = std::string("Kobra2 Pro_thumbnail");
    data.name     = "My Printer 18";
    data.status   = 1;
    data.type     = "Kobra 2 pro";
    data.id       = 17;
    m_info.push_back(data);

    data.fileName = std::string("Kobra2_thumbnail");
    data.name     = "My Printer 19";
    data.status   = 2;
    data.type     = "Kobra 2";
    data.id       = 18;
    m_info.push_back(data);

    data.fileName = std::string("Kobra2_thumbnail");
    data.name     = "My Printer 20";
    data.status   = 2;
    data.type     = "Kobra 2";
    data.id       = 19;
    m_info.push_back(data);

    data.fileName = std::string("4MAXPRO20_thumbnail");
    data.name     = "My Printer 21 123456789 123456789123456789123";
    data.status   = 0;
    data.type     = "Kobra 2 neo";
    data.id       = 20;
    m_info.push_back(data);

    data.fileName = std::string("Kobra2 Pro_thumbnail");
    data.name     = "My Printer 22 2341afwetg";
    data.status   = 1;
    data.type     = "Kobra 2 pro";
    data.id       = 21;
    m_info.push_back(data);

    data.fileName = std::string("Kobra2_thumbnail");
    data.name     = "My Printer 23";
    data.status   = 2;
    data.type     = "Kobra 2";
    data.id       = 22;
    m_info.push_back(data);

    data.fileName = std::string("I3MEGAS_thumbnail");
    data.name     = "My Printer 24";
    data.status   = 0;
    data.type     = "Kobra 2 neo";
    data.id       = 23;
    m_info.push_back(data);

    data.fileName = std::string("MEGA0_thumbnail");
    data.name     = "My Printer 25";
    data.status   = 1;
    data.type     = "Kobra 2 pro";
    data.id       = 24;
    m_info.push_back(data);

    data.fileName = std::string("Kobra2_thumbnail");
    data.name     = "My Printer 26";
    data.status   = 2;
    data.type     = "Kobra 2";
    data.id       = 25;
    m_info.push_back(data);

    data.fileName = std::string("Kobra2 Neo_thumbnail");
    data.name     = "My Printer 27";
    data.status   = 0;
    data.type     = "Kobra 2 neo";
    data.id       = 26;
    m_info.push_back(data);

    data.fileName = std::string("Kobra2 Pro_thumbnail");
    data.name     = "My Printer 28";
    data.status   = 1;
    data.type     = "Kobra 2 pro";
    data.id       = 27;
    m_info.push_back(data);

    data.fileName = std::string("Kobra2_thumbnail");
    data.name     = "My Printer 29";
    data.status   = 2;
    data.id       = 28;
    data.type     = "Kobra 2";
    m_info.push_back(data);

    data.fileName = std::string("Kobra2_thumbnail");
    data.name     = "My Printer 30";
    data.status   = 2;
    data.type     = "Kobra 2";
    data.id       = 29;
    m_info.push_back(data);
    return m_info;
}

void ACCloudSelectMachine::SetButtonStyle(ACButton* btn) 
{
    btn->SetCornerRadius(6);
    btn->SetPaddingSize(wxSize(0, 0));
    btn->SetSpacing(0);
    btn->SetTextColor(AC_COLOR_MAIN_BLUE);

}

void ACCloudSelectMachine::SetButtonStyle_Label(ACButton *btn)
{
    btn->SetCanFocus(false);
    btn->SetEnable(false);
    btn->SetPaddingSize(wxSize(0, 0));
    btn->SetButtonType(ACButton::AC_BUTTON_LABEL);
}


ACButton* ACCloudSelectMachine::CreateAddPrinterEvent(wxWindow* win) {
    ACButton * btn = new ACButton(win, _L("+ Add Printer"), "", "", "");
    SetButtonStyle(btn);
    btn->SetButtonType(ACButton::AC_BUTTON_TYPE::AC_BUTTON_LV2);
    btn->SetMinSize(wxSize(108, 30));

    btn->Bind(wxEVT_BUTTON, &ACCloudSelectMachine::OnButtonAddPrinterEvent, this, btn->GetId());
    return btn;
}

void ACCloudSelectMachine::OnButtonAddPrinterEvent(wxCommandEvent &event) 
{

}

void ACCloudSelectMachine::Init()
{
    AddWindowDrakEdg(this);
    m_PrinterInfo = GetCloudInfo();
    if (m_PrinterInfo.size() == 0) {
        Cloud->PrinterList();
    }
    m_Avprinter  = nullptr;
    m_Unvprinter = nullptr;

    m_up_sizer         = new wxBoxSizer(wxHORIZONTAL);
    m_AvailablePrinter = new ACButton(this, _L("Available Printer (0)"), "", "", "");
    SetButtonStyle_Label(m_AvailablePrinter);

    if (m_PrinterInfo.size() > 0) {
        m_show_panel = CreateShowPanel(m_PrinterInfo);
    } else {
        m_show_panel = CreateEmptyPanel();
    }

    this->SetBackgroundColour(AC_COLOR_WHITE);
    m_show_panel->SetBackgroundColour(AC_COLOR_WHITE);

    wxBoxSizer *btn_up_sizer = new wxBoxSizer(wxHORIZONTAL);

    m_pageSizer = new wxBoxSizer(wxVERTICAL);
    // m_page      = new ACStaticBox(this);

    m_buttonRefresh = new ACButton(this, _L("Refresh"), "", "", "");
    m_buttonAddNew  = CreateAddPrinterEvent(this);
    SetButtonStyle(m_buttonRefresh);
    m_buttonRefresh->SetButtonType(ACButton::AC_BUTTON_TYPE::AC_BUTTON_LV2);
    m_buttonRefresh->SetMinSize(wxSize(96, 30));

    btn_up_sizer->Add(m_buttonRefresh, 0, wxRIGHT, 20);
    btn_up_sizer->Add(m_buttonAddNew, 0, wxBOTTOM, 10);

    m_up_sizer->Add(m_AvailablePrinter, 0, wxLEFT, 20);
    m_up_sizer->Add(0, 0, 1, wxEXPAND, 5);
    m_up_sizer->Add(btn_up_sizer, 0, wxRIGHT, 20);

    wxPanel *mLine = new wxPanel(this);
    mLine->SetBackgroundColour(AC_COLOR_BD_BLACK);
    mLine->SetSize(GetWindowSize().x, 1);

    m_pageSizer->Add(m_up_sizer, 0, wxEXPAND | wxTOP, 40);
    m_pageSizer->Add(mLine, 0, wxEXPAND | wxBOTTOM | wxRIGHT | wxLEFT, 20);

    m_pageSizer->Add(m_show_panel, 1, wxEXPAND);

    m_mainSizer = new wxBoxSizer(wxVERTICAL);

    ACDialogTopbar *topbar = new ACDialogTopbar(this, "", 46);
    topbar->SetTitle(_L("Select Your Printer"));
    m_mainSizer->Add(topbar, 0, wxEXPAND | wxLEFT | wxRIGHT | wxTOP, 1);
    m_mainSizer->Add(m_pageSizer, 1, wxEXPAND | wxLEFT | wxRIGHT | wxBOTTOM, 1);

    SetSize(GetWindowSize());
    SetSizer(m_mainSizer);
    Layout();
    Refresh();
    int screenwidth  = wxSystemSettings::GetMetric(wxSYS_SCREEN_X, NULL);
    int screenheight = wxSystemSettings::GetMetric(wxSYS_SCREEN_Y, NULL);
    SetPosition(wxPoint((screenwidth - GetWindowSize().x) / 2, (screenheight - GetWindowSize().y) / 2));

    Bind(EVT_ACLOUD_PRINTER_CHECK_CLICK, &ACCloudSelectMachine::CheckClickPrinterEvent, this);
    m_buttonRefresh->Bind(wxEVT_BUTTON, &ACCloudSelectMachine::OnButtonRefreshEvent, this, m_buttonRefresh->GetId());
    Bind(EVT_ACCLOUD_PRINTER_LIST, &ACCloudSelectMachine::GetUpdatePrinterListEvent, this);
    Bind(EVT_OPERAT_PRINTER_RENAME, &ACCloudSelectMachine::GetOperatPrinterEvent, this);

}
void ACCloudSelectMachine::OnButtonRefreshEvent(wxCommandEvent &event) 
{ 
    m_opModel = 0;
    Cloud->PrinterList();
    m_buttonRefresh->Enable(false);
}

void ACCloudSelectMachine::OnTimer(wxTimerEvent& event) {
    m_timeCount++;
    if (m_timeCount > 5 && m_timer) {
        m_timeCount = 0;
        m_showTipInfoBtn->Hide();
        if (m_isAddPanel) {
            center_label_Sizer->Layout();
        } else {
            m_down_sizer->Layout();
        }
        m_timer->Stop();
    }
}

void ACCloudSelectMachine::GetUpdatePrinterListEvent(wxCommandEvent &event) 
{ 
    int result = event.GetInt();
    if (m_timer == nullptr) {
        m_timer = new wxTimer(this, wxID_ANY);
        this->Bind(wxEVT_TIMER, &ACCloudSelectMachine::OnTimer, this, m_timer->GetId());
    }
    bool isResult;
    //OK
    if (result == 100) {
        RefreshContentEvent(true);
        isResult = true;
    } else {
        isResult = false;
    }
    if (m_opModel == 0) {
        m_isOpResult = isResult;
        if (!m_buttonRefresh->IsEnabled())
            m_buttonRefresh->Enable(true);
        SetTipBtnStyle(m_isOpResult, m_isAddPanel, m_opModel);
        if (m_timer->IsRunning())
            m_timer->Stop();
        m_timer->Start(1000);
    }
}

void ACCloudSelectMachine::GetOperatPrinterEvent(wxCommandEvent& event) 
{
    if (m_timer == nullptr) {
        m_timer = new wxTimer(this, wxID_ANY);
        this->Bind(wxEVT_TIMER, &ACCloudSelectMachine::OnTimer, this, m_timer->GetId());
    }
    int result = event.GetInt();
    if (result == 0) {
        m_isOpResult = false;
    } else if (result == 1) {
        m_isOpResult = true;
    }
    wxString infoStr = event.GetString();
    if (infoStr == "Rename") {
        m_opModel = 1;
    } else if (infoStr == "Del") {
        m_opModel = 2;
    }
    SetTipBtnStyle(m_isOpResult, m_isAddPanel, m_opModel);
    if (m_timer->IsRunning())
        m_timer->Stop();

    m_timer->Start(1000);

}


void ACCloudSelectMachine::OnButtonCancelEvent(wxCommandEvent &event) 
{
    this->EndModal(wxID_NO);
}
void ACCloudSelectMachine::OnButtonStartPrintEvent(wxCommandEvent &event) 
{
    if (m_SelectPrinter.fileName.length() == 0) {
        WarningDialog dialog(wxGetApp().mainframe, wxString::Format(_L("empty")),_L("Warning"), wxYES | wxNO);
        dialog.Bind(wxEVT_CLOSE_WINDOW, [&dialog](wxCloseEvent &event) { dialog.EndModal(wxID_NO); });
        if (dialog.ShowModal() == wxID_NO) {
            
            return;
        }
    }

    this->EndModal(wxID_YES);
}

void ACCloudSelectMachine::CheckClickPrinterEvent(wxCommandEvent &evt) 
{

    int id = evt.GetInt();
    if (m_Avprinter != nullptr) {
        for (ACPrinterMeta *info : m_Avprinter->GetACPrinterMetaObj()) {
            if (info->data.id_printer != id && info->isChecked) {
                info->SetChecked(false);
                info->SetButtonState(info->buttonState);
            }
        }
    }
    if (m_Unvprinter != nullptr) {
        for (ACPrinterMeta *info : m_Unvprinter->GetACPrinterMetaObj()) {
            if (info->data.id_printer != id && info->isChecked) {
                info->SetChecked(false);
                info->SetButtonState(info->buttonState);
            }
        }
    }
    for (PrinterData info : m_PrinterInfo) {
        if (info.id_printer == id) {
            if (info.status != 1) {
                ErrorDialog msg(nullptr, _L("is offline"), false);
                msg.ShowModal();
                if (m_Unvprinter != nullptr) {
                    for (ACPrinterMeta *unInfo : m_Unvprinter->GetACPrinterMetaObj()) {
                        if (unInfo->data.id_printer == id) {
                            unInfo->SetChecked(false);
                            unInfo->SetButtonState(unInfo->buttonState);
                            unInfo->isMousePressed = false;
                            break;
                        }
                    }
                }
                break;
            }
            SetPrinterDataEvent(info);
            break;
        }
    }
    RefreshContentEvent();
}

void ACCloudSelectMachine::RefreshContentEvent(bool all) 
{
    if (all) {
        m_SelectPrinter.Clear();
        m_PrinterInfo = GetCloudInfo(false);
        m_pageSizer->Detach(m_show_panel);
        delete m_show_panel;
        if (m_PrinterInfo.size() > 0) {
            m_show_panel = CreateShowPanel(m_PrinterInfo);
        } else {
            m_show_panel = CreateEmptyPanel();
        }
        m_pageSizer->Add(m_show_panel, 1, wxEXPAND);
        m_pageSizer->Layout();
    }
    
    Layout();
}


void ACCloudSelectMachine::SetAvailablePrinterNum(int num) 
{
    wxString info = wxString::Format(_L("Available Printer (%d)"), num);
    m_AvailablePrinter->SetLabel(info);
    m_up_sizer->Layout();

}
void ACCloudSelectMachine::SetUnailablePrinterNum(int num) 
{
    wxString info = wxString::Format(_L("Unvailable Printer (%d)"), num);
    m_UnailablePrinter->SetLabel(info);
    unvaliable_sizer->Layout();
}

void ACCloudSelectMachine::msw_rescale()
{
    wxSize     _size = GetWindowSize();
    this->SetMinSize(_size);

    Fit();

    this->Layout();

    Refresh();
}


    } // GUI
} // Slic3r
