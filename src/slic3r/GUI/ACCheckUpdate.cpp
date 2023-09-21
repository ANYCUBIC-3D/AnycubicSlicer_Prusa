//#include "ACCheckUpdate.hpp"
//
//#include <wx/intl.h>
//
//
//#include "ACDialogTopbar.hpp"
//
//
//namespace Slic3r {
//
//namespace GUI {
//
//
//void CheckVersionDialog::SetParInfoToWindow() {
//    auto     accheckUpdate   = dynamic_cast<ACCheckUpdate *>(m_info_parent);
//    wxString softwareVersion = from_u8((boost::format(_u8L("AnycubicSlicer_%s")) % accheckUpdate->GetNowVersionString()).str());
//
//    m_version->SetLabel(softwareVersion);
//    m_tipInfo->SetLabel("");
//    if (accheckUpdate->GetCheckEnableState()) {
//        m_checkAutomatically->SetChecked(true);
//    }
//}
//
//void CheckVersionDialog::Build()
//{
//    m_buildIndex = true;
//    this->SetBackgroundColour(AC_COLOR_WHITE);
//    const int &em = em_unit();
//    wxSize     _size(76 * em, 32 * em);
//    SetSize(_size);
//
//    m_pageSizer = new wxBoxSizer(wxVERTICAL);
//    m_page = new ACStaticBox(this);
//
//
//    m_version = new ACButton(m_page, "", "", "", "", wxNO_BORDER);
//    m_version->SetButtonType(ACButton::AC_BUTTON_LABEL_2);
//
//    m_checkAutomatically = new ACButton(m_page, _L("Automatically"), "", "", "", wxNO_BORDER);
//    m_checkAutomatically->SetCheckStyle(ACButton::CHECKSTYLE_ON_BOX);
//    m_checkAutomatically->SetButtonType(ACButton::AC_BUTTON_LV3);
//    
//
//    checkForUpdates = new ACButton(m_page, _L("Check for Updates"), "", "", "", wxNO_BORDER);
//    checkForUpdates->SetButtonType(ACButton::AC_BUTTON_LV0);
//    checkForUpdates->SetSize(wxSize(10 * em, 4 * em));
//
//    m_tipInfo = new ACButton(m_page, "", "", "", "", wxNO_BORDER);
//    m_tipInfo->SetButtonType(ACButton::AC_BUTTON_LABEL_2);
//    m_tipInfo->SetTextColor(AC_COLOR_BOARD_RED);
//
//    m_pageSizer->Add(m_version, 0, wxALIGN_CENTER | wxTOP, 12);
//    m_pageSizer->Add(m_checkAutomatically, 0, wxALIGN_CENTER | wxTOP, 12);
//    m_pageSizer->Add(checkForUpdates, 0, wxALIGN_CENTER | wxTOP, 12);
//    m_pageSizer->Add(m_tipInfo, 0, wxALIGN_CENTER | wxTOP, 12);
//
//    ACDialogTopbar* topbar = new ACDialogTopbar(this, "", 76);
//
//    
//    m_page->SetBackgroundColour(AC_COLOR_WHITE);
//    m_page->SetCornerRadius(14);
//    m_page->SetSizer(m_pageSizer);
//
//    m_mainSizer = new wxBoxSizer(wxVERTICAL);
//    m_mainSizer->Add(topbar, 0, wxEXPAND);
//    m_mainSizer->Add(m_page, 1, wxEXPAND | wxALL, 14);
//
//    SetSizer(m_mainSizer);
//
//    Layout();
//    Refresh();
//    m_checkAutomatically->Bind(wxEVT_BUTTON, &CheckVersionDialog::SetAutomaticallyEvent, this);
//    checkForUpdates->Bind(wxEVT_BUTTON, &CheckVersionDialog::StartCheckForUpdatesEvent, this);
//
//    int screenwidth  = wxSystemSettings::GetMetric(wxSYS_SCREEN_X, NULL);
//    int screenheight = wxSystemSettings::GetMetric(wxSYS_SCREEN_Y, NULL);
//    SetPosition(wxPoint((screenwidth - 76 * em_unit()) / 2, (screenheight - 32 * em_unit()) / 2));
//}
//
//void CheckVersionDialog::SetAutomaticallyEvent(wxCommandEvent &event)
//{
//    dynamic_cast<ACCheckUpdate *>(m_info_parent)->SetCheckEnableState(!m_checkAutomatically->GetChecked());
//}
//void CheckVersionDialog::StartCheckForUpdatesEvent(wxCommandEvent &event) {
//
//    dynamic_cast<ACCheckUpdate *>(m_info_parent)->StartCheckForUpdates();
//}
//
//void CheckVersionDialog::msw_rescale()
//{
//    const int &em = em_unit();
//    wxSize     _size(76 * em, 32 * em);
//
//    this->SetMinSize(_size);
//
//    Fit();
//
//    this->Layout();
//
//    Refresh();
//}
//
//
//void CheckUpdateInfoDialog::SetParInfoToWindow()
//{
//    Freeze();
//    auto     accheckUpdate      = dynamic_cast<ACCheckUpdate *>(m_info_parent);
//
//    wxString nowSoftwareVersion = from_u8((boost::format(_u8L("Current version: %s")) % accheckUpdate->GetNowVersionString()).str());
//    m_nowVersion->SetLabel(nowSoftwareVersion);
//
//    wxString onLineSoftwareVersion = from_u8((boost::format(_u8L("AnycubicSlicer_%s")) % accheckUpdate->GetOnlineVersionString()).str());
//    m_onLineVersion->SetLabel(onLineSoftwareVersion);
//
//    wxString onLineSoftwareVersionUpdate = from_u8((boost::format(_u8L("V%s update")) % accheckUpdate->GetOnlineVersionString()).str());
//    m_onlineVersionUpdate->SetLabel(onLineSoftwareVersionUpdate);
//
//    wxString onLineSoftwareVesionTime = accheckUpdate->GetOnlineVesionTime();
//    m_onlineVersionTimes->SetLabel(onLineSoftwareVesionTime);
//    const int &em = em_unit();
//    /*improveContentSizer->Clear();
//    for (wxString improveInfo : accheckUpdate->GetImproveInfoEN()) {
//        wxString _improveInfo = from_u8((boost::format(_u8L("- %s")) % improveInfo).str());
//        ACButton *_objBtn = new ACButton(m_page, _improveInfo, "", "", "", wxNO_BORDER);
//        _objBtn->SetButtonType(ACButton::AC_BUTTON_LABEL_2);
//        improveContentSizer->Add(_objBtn, 0, wxALIGN_CENTER_VERTICAL | wxLEFT, 20);
//    }*/
//    wxString improveStr = "";
//    std::vector<wxString> info;
//    auto language = accheckUpdate->GetAppConfig()->get("translation_language");
//    if (language.empty() || language == "en") {
//        info = accheckUpdate->GetImproveAndBeginInfoList_EN();
//    }else {
//        info = accheckUpdate->GetImproveAndBeginInfoList_ZH();
//    }
//
//    for (wxString improveInfo : info) {
//        improveStr += improveInfo + "\n";
//    }
//    improveText->SetValue(improveStr);
//    //bugFixesContentSizer->Clear();
//    /*for (wxString improveInfo : accheckUpdate->GetBugFixesInfoEN()) {
//        wxString _improveInfo = from_u8((boost::format(_u8L("- %s")) % improveInfo).str());
//
//        ACButton *_objBtn = new ACButton(m_page, _improveInfo, "", "", "", wxNO_BORDER);
//        _objBtn->SetButtonType(ACButton::AC_BUTTON_LABEL_2);
//        bugFixesContentSizer->Add(_objBtn, 0, wxALIGN_CENTER_VERTICAL | wxLEFT, 20);
//    }*/
//
//    /*wxString fixesStr = "";
//    for (wxString fixesInfo : accheckUpdate->GetBugFixesInfoEN()) {
//        fixesStr += fixesInfo + "\n";
//    }
//    bugFixesText->SetValue(fixesStr);*/
//    msw_rescale();
//
//}
//void CheckUpdateInfoDialog::Build()
//{
//    m_buildIndex = true;
//    this->SetBackgroundColour(AC_COLOR_WHITE);
//    const int &em = em_unit();
//    wxSize     _size(76 * em, 80 * em);
//    SetSize(_size);
//
//    m_pageSizer = new wxBoxSizer(wxVERTICAL);
//    m_page      = new ACStaticBox(this);
//    m_page->SetBorderWidth(1);
//    m_page->SetBorderColor(AC_COLOR_FONT_GRAY);
//
//    wxBoxSizer *upSizer = new wxBoxSizer(wxHORIZONTAL);
//    wxBoxSizer *upConterSizer = new wxBoxSizer(wxVERTICAL);
//    wxBoxSizer *downSizer = new wxBoxSizer(wxHORIZONTAL);
//
//    m_logButton = new ACButton(this, "", "logo_32px.png", "logo_32px.png", "logo_32px.png", wxNO_BORDER);
//
//    m_nowVersion = new ACButton(this, "", "", "", "", wxNO_BORDER);
//    m_nowVersion->SetButtonType(ACButton::AC_BUTTON_LABEL_2);
//
//    m_onLineVersion = new ACButton(this, "", "", "", "", wxNO_BORDER);
//    m_onLineVersion->SetButtonType(ACButton::AC_BUTTON_LABEL_2);
//
//    wxString btIconNameCloseNor   = "software_close-nor-gray";
//    wxString btIconNameCloseHover = "icon-close_28-click";
//    m_close_button  = new ACButton(this, "", btIconNameCloseNor, btIconNameCloseHover, btIconNameCloseHover, wxBORDER_NONE,wxSize(4 * em, 4 * em));
//    m_close_button->SetPaddingSize(wxSize((0), (0)));
//    m_close_button->SetButtonType(ACButton::AC_BUTTON_ICON);
//
//    upConterSizer->Add(0, 0, 1, wxEXPAND, 0);
//    upConterSizer->Add(m_nowVersion, 0, wxRIGHT, 10);
//    upConterSizer->Add(m_onLineVersion, 0, wxRIGHT, 10);
//
//    upSizer->Add(m_logButton, 0, wxEXPAND);
//    upSizer->Add(upConterSizer, 1, wxEXPAND);
//    upSizer->Add(m_close_button, 0, wxEXPAND);
//
//
//    laterBtn = new ACButton(this, _L("Later"), "", "", "", wxNO_BORDER);
//    laterBtn->SetButtonType(ACButton::AC_BUTTON_LV1);
//
//    updateNowBtn = new ACButton(this, _L("Update Now"), "", "", "", wxNO_BORDER);
//    updateNowBtn->SetButtonType(ACButton::AC_BUTTON_LV0);
//
//    downSizer->Add(0, 0, 1, wxEXPAND, 0);
//    downSizer->Add(laterBtn, 0, wxRIGHT|wxBOTTOM, 10);
//    downSizer->Add(updateNowBtn, 0, wxRIGHT | wxBOTTOM, 10);
//
//
//    m_onlineVersionUpdate = new ACButton(m_page, "", "", "", "", wxNO_BORDER);
//    m_onlineVersionUpdate->SetButtonType(ACButton::AC_BUTTON_LABEL_2);;
//
//    
//    m_onlineVersionTimes = new ACButton(m_page, "", "", "", "", wxNO_BORDER);
//    m_onlineVersionTimes->SetButtonType(ACButton::AC_BUTTON_LABEL_2);
//
//    /*m_improveTitle = new ACButton(m_page, _L("Improve:"), "", "", "", wxNO_BORDER);
//    m_improveTitle->SetButtonType(ACButton::AC_BUTTON_LABEL_2);*/
//
//    improveText = new wxTextCtrl(m_page, wxID_ANY, wxEmptyString, wxDefaultPosition, wxSize(68 * em, 40 * em),
//                                 wxTE_AUTO_URL | wxNO_BORDER | wxTE_MULTILINE | wxTE_READONLY);
//
//    //improveContentSizer = new wxBoxSizer(wxVERTICAL);
//
//    /*m_bugFixesTitle = new ACButton(m_page, _L("Bug Fixes:"), "", "", "", wxNO_BORDER);
//    m_bugFixesTitle->SetButtonType(ACButton::AC_BUTTON_LABEL_2);*/
//
//    //bugFixesContentSizer = new wxBoxSizer(wxVERTICAL);
//    /*bugFixesText = new wxTextCtrl(m_page, wxID_ANY, wxEmptyString, wxDefaultPosition, wxSize(68 * em, 20 * em),
//                                  wxTE_AUTO_URL | wxNO_BORDER | wxTE_MULTILINE | wxTE_READONLY);*/
//
//    m_pageSizer->Add(m_onlineVersionUpdate, 0, wxALIGN_CENTER_VERTICAL | wxLEFT |wxTOP, 10);
//    m_pageSizer->Add(m_onlineVersionTimes, 0, wxALIGN_CENTER_VERTICAL | wxLEFT  , 20);
//    //m_pageSizer->Add(m_improveTitle, 0, wxALIGN_CENTER_VERTICAL | wxLEFT, 10);
//    m_pageSizer->Add(improveText, 0, wxALIGN_CENTER_VERTICAL | wxLEFT, 40);
//    //m_pageSizer->Add(m_bugFixesTitle, 0, wxALIGN_CENTER_VERTICAL | wxLEFT, 10);
//    //m_pageSizer->Add(bugFixesText, 0, wxALIGN_CENTER_VERTICAL | wxLEFT , 40);
//    //m_pageSizer->Add(bugFixesContentSizer, 0, wxALIGN_CENTER_VERTICAL | wxLEFT | wxTOP, 10);
//
//    ACDialogTopbar *topbar = new ACDialogTopbar(this, "", 76);
//
//    m_page->SetBackgroundColour(AC_COLOR_WHITE);
//    m_page->SetCornerRadius(14);
//    m_page->SetSizer(m_pageSizer);
//
//    m_mainSizer = new wxBoxSizer(wxVERTICAL);
//    m_mainSizer->Add(topbar, 0, wxEXPAND);
//    m_mainSizer->Add(upSizer, 0, wxEXPAND);
//    m_mainSizer->Add(m_page, 1, wxEXPAND | wxALL, 14);
//    m_mainSizer->Add(downSizer, 0, wxEXPAND);
//
//    SetSizer(m_mainSizer);
//
//    Layout();
//    Refresh();
//    m_close_button->Bind(wxEVT_BUTTON, &CheckUpdateInfoDialog::LaterEvent, this);
//    laterBtn->Bind(wxEVT_BUTTON, &CheckUpdateInfoDialog::LaterEvent, this);
//    updateNowBtn->Bind(wxEVT_BUTTON, &CheckUpdateInfoDialog::UpdateNowEvent, this);
//
//    int screenwidth  = wxSystemSettings::GetMetric(wxSYS_SCREEN_X, NULL);
//    int screenheight = wxSystemSettings::GetMetric(wxSYS_SCREEN_Y, NULL);
//    SetPosition(wxPoint((screenwidth - 76 * em_unit()) / 2, (screenheight - 80 * em_unit()) / 2));
//}
//
//void CheckUpdateInfoDialog::LaterEvent(wxCommandEvent &event) { this->Close(); }
//void CheckUpdateInfoDialog::UpdateNowEvent(wxCommandEvent &event)
//{
//    std::string version = dynamic_cast<ACCheckUpdate *>(m_info_parent)->GetOnlineVersion().to_string();
//    wxGetApp().sendDownLoadCanceEvent(version);
//}
//
//void CheckUpdateInfoDialog::msw_rescale()
//{
//    const int &em = em_unit();
//    wxSize     _size(76 * em, 80 * em);
//
//    this->SetMinSize(_size);
//
//    Fit();
//
//    this->Layout();
//
//    Refresh();
//    Thaw();
//}
//
//
//void CheckUpdateProgressDialog::SetParInfoToWindow() { dynamic_cast<ACCheckUpdate *>(m_info_parent)->SetDownloadProgressPercentage(0.0f);}
//
//void CheckUpdateProgressDialog::Build()
//{
//    m_buildIndex = true;
//    this->SetBackgroundColour(AC_COLOR_WHITE);
//    const int &em = em_unit();
//    wxSize     _size(76 * em, 80 * em);
//    SetSize(_size);
//
//    m_pageSizer = new wxBoxSizer(wxVERTICAL);
//    m_page      = new ACStaticBox(this);
//
//    m_logButton = new ACButton(m_page, _L("Installing Updates for AnycubicSlicer"), "logo_32px.png", "logo_32px.png", "logo_32px.png",
//                               wxNO_BORDER, wxSize(3 * em, 3 * em));
//    m_logButton->SetPaddingSize(wxSize(10, 6));
//    m_logButton->SetSpacing(8);
//    m_logButton->SetButtonType(ACButton::AC_BUTTON_LABEL_2);
//    m_logButton->SetCanFocus(false);
//
//
//    int      TOPBAR_ICON_SIZE      = int(2.6 * em);
//    wxString iconName_Iconize      = "software_minimization-nor";
//    wxString iconName_IconizeHover = "software_minimization-hover";
//
//    wxString m_iconName_Maximize = "software_maximization-nor";
//    wxString m_iconName_MaximizeHover = "software_maximization-hover";
//    wxString m_iconName_Window         = "software_window-nor";
//    wxString m_iconName_WindowHover    = "software_window-hover";
//
//    wxString iconName_Close      = "software_close-nor";
//    wxString iconName_CloseHover = "software_close-hover";
//
//    m_btIconize  = new ACButton(m_page, "", iconName_Iconize, iconName_IconizeHover, iconName_Iconize, wxNO_BORDER,
//                               wxSize(TOPBAR_ICON_SIZE, TOPBAR_ICON_SIZE));
//    m_btMaximize = new ACButton(m_page, "", m_iconName_Window, m_iconName_WindowHover, m_iconName_WindowHover, wxNO_BORDER,
//                                wxSize(TOPBAR_ICON_SIZE, TOPBAR_ICON_SIZE));
//    m_btClose    = new ACButton(m_page, "", iconName_Close, iconName_CloseHover, iconName_Close, wxNO_BORDER,
//                             wxSize(TOPBAR_ICON_SIZE, TOPBAR_ICON_SIZE));
//
//    m_btIconize->SetButtonType(ACButton::AC_BUTTON_ICON);
//    m_btMaximize->SetButtonType(ACButton::AC_BUTTON_ICON);
//    m_btClose->SetButtonType(ACButton::AC_BUTTON_ICON);
//    m_btIconize->SetPaddingSize(wxSize(0, 0));
//    m_btMaximize->SetPaddingSize(wxSize(0, 0));
//    m_btMaximize->SetEnable(false);
//    m_btClose->SetPaddingSize(wxSize(0, 0));
//
//    wxBoxSizer *hSizer_up = new wxBoxSizer(wxHORIZONTAL);
//
//    hSizer_up->Add(m_btIconize, wxALIGN_CENTER_VERTICAL);
//    hSizer_up->AddSpacer(10);
//    hSizer_up->Add(m_btMaximize, wxALIGN_CENTER_VERTICAL);
//    hSizer_up->AddSpacer(10);
//    hSizer_up->Add(m_btClose, wxALIGN_CENTER_VERTICAL);
//
//
//    wxBoxSizer *hSizer_right = new wxBoxSizer(wxHORIZONTAL);
//    hSizer_right->Add(m_logButton,0, wxALIGN_CENTER_VERTICAL);
//    hSizer_right->Add(0, 0, 1, wxEXPAND, 0);
//    hSizer_right->Add(hSizer_up, 0, wxALIGN_CENTER_VERTICAL | wxALIGN_RIGHT | wxRIGHT, 10);
//
//
//
//    m_InstallingUpdates = new ACButton(m_page, _L("InstallingUpdates"), "", "", "", wxNO_BORDER);
//    m_InstallingUpdates->SetButtonType(ACButton::AC_BUTTON_LABEL_2);
//
//
//    m_downLoadInfoLabel = new ACButton(m_page, "", "", "", "", wxNO_BORDER);
//    m_downLoadInfoLabel->SetButtonType(ACButton::AC_BUTTON_LABEL_2);
//
//    m_downLoadgauge = new wxGauge(m_page, wxID_ANY, 100, wxDefaultPosition, wxSize(65 * em, 2 * em), wxGA_HORIZONTAL);
//
//
//    m_cancel_btn = new ACButton(m_page, _L("Cancel"), "", "", "", wxNO_BORDER);
//    m_cancel_btn->SetButtonType(ACButton::AC_BUTTON_LV1);
//
//   
//
//    m_pageSizer->Add(hSizer_right, 0, wxALIGN_CENTER_VERTICAL | wxLEFT | wxTOP, 10);
//    m_pageSizer->Add(m_InstallingUpdates, 0, wxALIGN_CENTER_VERTICAL | wxLEFT | wxTOP, 10);
//    m_pageSizer->Add(m_downLoadInfoLabel, 0, wxALIGN_CENTER_VERTICAL | wxLEFT | wxTOP, 10);
//    m_pageSizer->Add(m_downLoadgauge, 0, wxALIGN_CENTER_VERTICAL | wxLEFT | wxTOP, 10);
//    m_pageSizer->Add(0, 0, 1, wxEXPAND, 0);
//    m_pageSizer->Add(m_cancel_btn, 0, wxALIGN_CENTER_VERTICAL | wxALIGN_RIGHT | wxRIGHT | wxBOTTOM, 10);
//
//    ACDialogTopbar *topbar = new ACDialogTopbar(this, "", 76);
//
//    m_page->SetBackgroundColour(AC_COLOR_WHITE);
//    m_page->SetCornerRadius(14);
//    m_page->SetSizer(m_pageSizer);
//
//    m_mainSizer = new wxBoxSizer(wxVERTICAL);
//    m_mainSizer->Add(topbar, 0, wxEXPAND);
//    m_mainSizer->Add(m_page, 1, wxEXPAND | wxALL, 14);
//
//    SetSizer(m_mainSizer);
//
//    Layout();
//    Refresh();
//
//    m_cancel_btn->Bind(wxEVT_BUTTON, &CheckUpdateProgressDialog::CancelEvent, this);
//    m_btClose->Bind(wxEVT_BUTTON, &CheckUpdateProgressDialog::CancelEvent, this);
//    m_btIconize->Bind(wxEVT_BUTTON, &CheckUpdateProgressDialog::OnIconSizeEvent, this);
//    int screenwidth  = wxSystemSettings::GetMetric(wxSYS_SCREEN_X, NULL);
//    int screenheight = wxSystemSettings::GetMetric(wxSYS_SCREEN_Y, NULL);
//    SetPosition(wxPoint((screenwidth - 76 * em_unit()) / 2, (screenheight - 80 * em_unit()) / 2));
//}
//
//void CheckUpdateProgressDialog::OnIconSizeEvent(wxCommandEvent &event) { this->Iconize(); }
//
//void CheckUpdateProgressDialog::CancelEvent(wxCommandEvent &event) { wxGetApp().sendDownLoadFinishEvent(); }
//
//void CheckUpdateProgressDialog::msw_rescale()
//{
//    const int &em = em_unit();
//    wxSize      _size(76 * em, 80 * em);
//
//    this->SetMinSize(_size);
//
//    Fit();
//
//    this->Layout();
//
//    Refresh();
//}
//
//
//void CheckUpdateProgressFinishDialog::SetParInfoToWindow() {
//
//}
//void CheckUpdateProgressFinishDialog::Build()
//{
//    m_buildIndex = true;
//    this->SetBackgroundColour(AC_COLOR_WHITE);
//    const int &em = em_unit();
//    wxSize     _size(70 * em, 25 * em);
//    SetSize(_size);
//
//    m_pageSizer = new wxBoxSizer(wxVERTICAL);
//    m_page      = new ACStaticBox(this);
//
//    m_logButton = new ACButton(m_page, "", "logo_32px.png", "logo_32px.png", "logo_32px.png",wxNO_BORDER, wxSize(6 * em, 6 * em));
//    m_pageSizer_center = new wxBoxSizer(wxHORIZONTAL);
//    m_pageSizer_center_right = new wxBoxSizer(wxVERTICAL);
//
//    m_prepareInfo = new ACButton(m_page, _L("Prepare to install the update"), "", "", "", wxNO_BORDER);
//    m_prepareInfo->SetButtonType(ACButton::AC_BUTTON_LABEL_2);
//    m_prepareInfo->SetPaddingSize(wxSize(0, 0));
//
//    m_prepareInfo_content = new ACButton(m_page, _L("Update content is download. Do you want to install it now?"), "checkbox-on-nor",
//                                         "checkbox-on-nor", "checkbox-on-nor", wxNO_BORDER, wxSize(1 * em, 1 * em));
//    m_prepareInfo_content->SetButtonType(ACButton::AC_BUTTON_LABEL_2);
//    m_prepareInfo_content->SetPaddingSize(wxSize(0,0));
//
//    m_pageSizer_center_right->Add(m_prepareInfo, 0, wxLEFT | wxTOP, 10);
//    m_pageSizer_center_right->Add(m_prepareInfo_content, 0,  wxLEFT , 10);
//
//    m_pageSizer_center->Add(m_logButton, 0, wxALIGN_CENTER_VERTICAL | wxLEFT | wxTOP, 10);
//    m_pageSizer_center->Add(m_pageSizer_center_right, 0, wxALIGN_CENTER_VERTICAL | wxLEFT | wxTOP, 10);
//
//    m_cancel_btn = new ACButton(m_page, _L("Cancel"), "", "", "", wxNO_BORDER);
//    m_cancel_btn->SetButtonType(ACButton::AC_BUTTON_LV1);
//
//    m_installNow_btn = new ACButton(m_page, _L("Install Now"), "", "", "", wxNO_BORDER);
//    m_installNow_btn->SetButtonType(ACButton::AC_BUTTON_LV0);
//
//    m_pageSizer_bottom = new wxBoxSizer(wxHORIZONTAL);
//    m_pageSizer_bottom->Add(0, 0, 1, wxEXPAND, 0);
//    m_pageSizer_bottom->Add(m_cancel_btn, 0, wxALIGN_CENTER_VERTICAL | wxRIGHT , 10);
//    m_pageSizer_bottom->Add(m_installNow_btn, 0, wxALIGN_CENTER_VERTICAL | wxRIGHT, 10);
//
//    m_pageSizer->Add(m_pageSizer_center, 0, wxEXPAND);
//    m_pageSizer->Add(0, 0, 1, wxEXPAND, 0);
//    m_pageSizer->Add(m_pageSizer_bottom, 0, wxEXPAND);
//    
//    ACDialogTopbar *topbar = new ACDialogTopbar(this, _L("Software Update"), 20);
//
//    m_page->SetBackgroundColour(AC_COLOR_WHITE);
//    m_page->SetCornerRadius(14);
//    m_page->SetSizer(m_pageSizer);
//
//    m_mainSizer = new wxBoxSizer(wxVERTICAL);
//    m_mainSizer->Add(topbar, 0, wxEXPAND);
//    m_mainSizer->Add(m_page, 1, wxEXPAND | wxALL, 14);
//
//    SetSizer(m_mainSizer);
//
//    Layout();
//    Refresh();
//
//    m_cancel_btn->Bind(wxEVT_BUTTON, &CheckUpdateProgressFinishDialog::CancelEvent, this);
//    m_installNow_btn->Bind(wxEVT_BUTTON, &CheckUpdateProgressFinishDialog::InstallNowEvent, this);
//
//    int screenwidth  = wxSystemSettings::GetMetric(wxSYS_SCREEN_X, NULL);
//    int screenheight = wxSystemSettings::GetMetric(wxSYS_SCREEN_Y, NULL);
//    SetPosition(wxPoint((screenwidth - 76 * em_unit()) / 2, (screenheight - 80 * em_unit()) / 2));
//}
//
//void CheckUpdateProgressFinishDialog::CancelEvent(wxCommandEvent &event) { this->Close(); }
//void CheckUpdateProgressFinishDialog::InstallNowEvent(wxCommandEvent &event) { wxGetApp().sendUpdateNowEvent(); }
//
//void CheckUpdateProgressFinishDialog::msw_rescale()
//{
//    const int &em = em_unit();
//    wxSize     _size(70 * em, 25 * em);
//
//    this->SetMinSize(_size);
//
//    Fit();
//
//    this->Layout();
//
//    Refresh();
//}
//
//ACCheckUpdate::ACCheckUpdate(wxWindow *parent)
//{
//    m_parent     = parent;
//    m_app_config = get_app_config();
//
//    GetNowSoftwareVersionInfo();
//
//
//    CreateCheckVersionDialog();
//
//    CreateCheckUpdateInfoDiaglog();
//
//    CreateCheckUpdateProgressDialog();
//
//    CreateCheckUpdateProgressFinishDialog();
//
//}
//
//void ACCheckUpdate::CreateCheckVersionDialog()
//{ 
//    m_checkVersionDialog = new CheckVersionDialog(m_parent);
//    m_checkVersionDialog->SetInParent(this);
//    m_checkVersionDialog->Build();
//}
//
//void ACCheckUpdate::CreateCheckUpdateInfoDiaglog()
//{ 
//    m_checkUpdateInfoDialog = new CheckUpdateInfoDialog(m_parent);
//    m_checkUpdateInfoDialog->SetInParent(this);
//    m_checkUpdateInfoDialog->Build();
//
//}
//
//void ACCheckUpdate::CreateCheckUpdateProgressDialog()
//{ 
//
//    m_checkUpdateProgressDialog = new CheckUpdateProgressDialog(m_parent); 
//    m_checkUpdateProgressDialog->SetInParent(this);
//    m_checkUpdateProgressDialog->Build();
//}
//
//void ACCheckUpdate::CreateCheckUpdateProgressFinishDialog()
//{
//    m_checkUpdateProgressFinishDialog = new CheckUpdateProgressFinishDialog(m_parent);
//    m_checkUpdateProgressFinishDialog->SetInParent(this);
//    m_checkUpdateProgressFinishDialog->Build();
//}
//
//
//bool ACCheckUpdate::StartCheckForUpdates()
//{ 
//    wxGetApp().app_version_check_public(true);
//    return true;
//}
//
//void ACCheckUpdate::GetNowSoftwareVersionInfo()
//{
//    if (m_app_config->orig_version().valid()) {
//        m_now_version = m_app_config->orig_version();
//    }
//}
//
//bool ACCheckUpdate::GetCheckEnableState()
//{
//
//    if (m_app_config->get("notify_release") != "none") {
//
//        return true;
//    }
//    return false; 
//}
//void ACCheckUpdate::SetIniFileDownInfo(wxString info)
//{
//    if (m_checkVersionDialog->IsShown()) {
//        m_checkVersionDialog->GetTipBtn()->SetLabel(info);
//    }
//}
//
//void ACCheckUpdate::SetDownloadProgressPercentage(float percentage) {
//
//    int      softSize     = GetOnlineSoftwareSize();
//    wxString _nowSumSize  = wxString::Format("%.2f MB of ", percentage > 0 ? percentage * softSize / 1024 / 1024 : 0.0f);
//    wxString _fileSumSize = wxString::Format("%.2f MB,", softSize * 1.0f / 1024 / 1024);
//    wxString _gap = wxString::Format("%d", percentage > 0 ? int(percentage * 100) : 0);
//
//    wxString downLoadInfo = from_u8((boost::format(_u8L("Downloading(%s complete)")) % (_nowSumSize + _fileSumSize + _gap + "%")).str());
//    m_checkUpdateProgressDialog->GetdownLoadInfoLabel()->SetLabel(downLoadInfo);
//
//    m_checkUpdateProgressDialog->GetdownLoadgauge()->SetValue(int(percentage * 100));
//}
//
//static bool isContainString(const wxString &str, const wxString &target) { return str.Contains(target); }
//
//static std::vector<std::string> GetListInfoSelectInfo(std::vector<std::string> &info, wxString start_str, wxString end_str)
//{
//    std::vector<std::string> new_info;
//    auto start_it = std::find(info.begin(), info.end(), start_str);
//    auto end_it   = std::find(info.begin(), info.end(), end_str);
//
//    if (start_it != info.end() && end_it != info.end())
//    {
//        ++start_it; 
//        while (start_it != end_it)
//        {
//            std::cout << *start_it << std::endl;
//            new_info.push_back(*start_it);
//            ++start_it;
//        }
//    }
//    return new_info;
//}
//
//static bool isContainBetween(const wxString &str, wxString target1, const wxString target2)
//{
//    int index1 = str.Find(target1);
//    int index2 = str.Find(target2);
//    if ((index1 != wxNOT_FOUND || index2 != wxNOT_FOUND)) {
//        return index1 > 0 ? index1:index2;
//    }
//    return false;
//}
//
//static std::vector<wxString> findLinesContainBetween(std::vector<std::string> &lines, const wxString target1, const wxString target2)
//{
//    bool                  index = false;
//    std::vector<wxString> result;
//    std::vector<wxString> new_result;
//    for (const std::string &line : lines) {
//        int resultIndex = isContainBetween(line, target1, target2);
//        if (resultIndex > 0 || index) {
//            if (resultIndex > 0)
//                index = !index;
//            wxString wxstr(line.c_str(), wxConvUTF8);
//            result.push_back(wxstr);
//        }
//    }
//    if (result.size() > 0) {
//        for (int i = 1; i <= result.size() - 2; ++i) {
//            new_result.push_back(result[i]);
//        }
//    }
//    return new_result;
//}
//
//static std::string getSubstringAfterChar(std::string &str, char target,int _index = 0)
//{
//    size_t index = str.find(target);
//    if (index != std::string::npos) {
//        return str.substr(index + 1 + _index);
//    }
//    return "";
//}
//bool ACCheckUpdate::SetIniFileInfo(std::vector<std::string> &info, bool showDialog)
//{ 
//#ifdef WIN32
//    std::vector<std::string> software = GetListInfoSelectInfo(info, "[WIN64]", "[OSX]");
//#else
//    std::vector<std::string> software = GetListInfoSelectInfo(info, "[OSX]", "[INFO]");
//#endif
//    if (software.size() == 4) {
//        SetOnlineVersion(getSubstringAfterChar(software[0], '='));
//        if (GetNowVersion() < GetOnlineVersion()) {
//            SetOnlineSoftwarePath(getSubstringAfterChar(software[1], '=', 1));
//            SetOnlineSoftwareSize(getSubstringAfterChar(software[3], '='));
//            SetnlineVesionTime(getSubstringAfterChar(software[2], '='));
//            SetImproveAndBeginInfoList_EN(findLinesContainBetween(info, "begin_en", "end_en"));
//            SetImproveAndBeginInfoList_ZH(findLinesContainBetween(info, "begin_zh", "end_zh"));
//            if (showDialog || (m_app_config->get("notify_release") != "none" && !showDialog))
//                ShowCheckUpdateInfoDialog();
//            return true;
//        }
//    }
//    SetIniFileDownInfo(_L("AnycubicSlicer is up to date."));
//    return false;
//}
//
//
//void ACCheckUpdate::SetCheckEnableState(bool index) { 
//
//    m_app_config->set("notify_release", index ? "all" : "none"); 
//}
//
//
//void ACCheckUpdate::CloseAllDialog()
//{
//    if (m_checkVersionDialog->IsShown())
//        m_checkVersionDialog->Close();
//    if (m_checkUpdateInfoDialog->IsShown())
//        m_checkUpdateInfoDialog->Close();
//    if (m_checkUpdateProgressDialog->IsShown())
//        m_checkUpdateProgressDialog->Close();
//    if (m_checkUpdateProgressFinishDialog->IsShown())
//        m_checkUpdateProgressFinishDialog->Close();
//}
//
//void ACCheckUpdate::ShowCheckVersionDialog()
//{ 
//    CloseAllDialog();
//    if (!m_checkVersionDialog->IsShown()) {
//        m_checkVersionDialog->SetParInfoToWindow();
//        m_checkVersionDialog->Raise();
//        m_checkVersionDialog->Show();
//    }
//}
//void ACCheckUpdate::ShowCheckUpdateInfoDialog()
//{
//    CloseAllDialog();
//    if (!m_checkUpdateInfoDialog->IsShown()) {
//        m_checkUpdateInfoDialog->SetParInfoToWindow();
//        m_checkUpdateInfoDialog->Raise();
//        m_checkUpdateInfoDialog->Show();
//    }
//}
//void ACCheckUpdate::ShowCheckUpdateProgressDialog()
//{
//    CloseAllDialog();
//    if (!m_checkUpdateProgressDialog->IsShown()) {
//        m_checkUpdateProgressDialog->SetParInfoToWindow();
//        m_checkUpdateProgressDialog->Raise();
//        m_checkUpdateProgressDialog->Show();
//    }
//}
//void ACCheckUpdate::ShowCheckUpdateProgressFinishDialog()
//{
//    CloseAllDialog();
//    if (!m_checkUpdateProgressFinishDialog->IsShown()) {
//        m_checkUpdateProgressFinishDialog->SetParInfoToWindow();
//        m_checkUpdateProgressFinishDialog->Raise();
//        m_checkUpdateProgressFinishDialog->Show();
//    }
//}
//
//} // GUI
//} // Slic3r
//
//
