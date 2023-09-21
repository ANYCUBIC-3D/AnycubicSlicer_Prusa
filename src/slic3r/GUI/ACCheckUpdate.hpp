//#ifndef slic3r_ACCheckUpdate_hpp_
//#define slic3r_ACCheckUpdate_hpp_
//
//#include "GUI.hpp"
//#include "GUI_App.hpp"
//#include "MainFrame.hpp"
//#include "GUI_Utils.hpp"
//#include "wxExtensions.hpp"
//#include "libslic3r/Semver.hpp"
//#include "ACDefines.h"
//#include "libslic3r/AppConfig.hpp"
//#include "I18N.hpp"
//#include <wx/dialog.h>
//#include <wx/timer.h>
//#include <vector>
//#include <map>
//
//
//namespace Slic3r {
//
//namespace GUI {
//
//class ConfigOptionsGroup;
//
//class CheckVersionDialog : public DPIDialog
//{
//public:
//    explicit CheckVersionDialog(wxWindow *parent)
//        : DPIDialog(parent, wxID_ANY, "", wxDefaultPosition, wxDefaultSize, wxNO_BORDER | wxSTAY_ON_TOP, "CheckVersionDialog")
//    {}
//    ~CheckVersionDialog() = default;
//
//    void SetInParent(wxObject *info) { m_info_parent = info; };
//    void Build();
//
//    void SetAutomaticallyEvent(wxCommandEvent &event);
//    void StartCheckForUpdatesEvent(wxCommandEvent &event);
//
//    bool GetBuildIndex() { return m_buildIndex; }
//
//    ACButton *GetTipBtn() { return m_tipInfo; }
//    ACButton *GeCheckForUpdatesBtn() { return checkForUpdates; }
//
//
//    void SetParInfoToWindow();
//
//private:
//    bool         m_buildIndex{false};
//    ACButton *m_version;
//    wxBoxSizer *m_pageSizer;
//    wxBoxSizer * m_mainSizer;
//    ACStaticBox *m_page;
//    ACButton *   m_checkAutomatically;
//    ACButton *   checkForUpdates;
//    ACButton *   m_tipInfo;
//    wxObject *   m_info_parent;
//    void msw_rescale();
//    void on_dpi_changed(const wxRect &suggested_rect) override { msw_rescale(); }
//    
//};
//
//class CheckUpdateInfoDialog : public DPIDialog
//{
//public:
//    explicit CheckUpdateInfoDialog(wxWindow *parent)
//        : DPIDialog(parent, wxID_ANY, "", wxDefaultPosition, wxDefaultSize, wxNO_BORDER | wxSTAY_ON_TOP, "CheckUpdateInfoDialog")
//    {}
//    ~CheckUpdateInfoDialog() = default;
//    void SetInParent(wxObject *info) { m_info_parent = info; };
//    void Build();
//
//    void LaterEvent(wxCommandEvent &event);
//    void UpdateNowEvent(wxCommandEvent &event);
//    bool GetBuildIndex() { return m_buildIndex; }
//
//    void SetParInfoToWindow();
//
//private:
//    bool         m_buildIndex{false};
//    ACButton *   m_logButton;
//    ACButton *   m_nowVersion;
//    ACButton *   m_onLineVersion;
//    ACButton *   m_close_button;
//
//    ACButton *  m_onlineVersionUpdate;
//    ACButton *  m_onlineVersionTimes;
//    ACButton *  m_improveTitle;
//    ACButton *  m_bugFixesTitle;
//
//    wxBoxSizer *improveContentSizer;
//    wxBoxSizer *bugFixesContentSizer;
//    wxTextCtrl *improveText;
//    wxTextCtrl *bugFixesText;
//
//    wxBoxSizer * m_pageSizer;
//    ACStaticBox *m_page;
//    wxBoxSizer * m_mainSizer;
//
//    ACButton *laterBtn;
//    ACButton *updateNowBtn;
//
//    ACButton *   m_tipInfo;
//    wxObject *m_info_parent;
//    void msw_rescale();
//    void on_dpi_changed(const wxRect &suggested_rect) override { msw_rescale(); }
//};
//
//class CheckUpdateProgressDialog : public DPIDialog
//{
//public:
//    explicit CheckUpdateProgressDialog(wxWindow *parent)
//        : DPIDialog(parent, wxID_ANY, "", wxDefaultPosition, wxDefaultSize, wxNO_BORDER | wxSTAY_ON_TOP, "CheckUpdateProgressDialog")
//    {}
//    ~CheckUpdateProgressDialog() = default;
//    void SetInParent(wxObject *info) { m_info_parent = info; };
//    void Build();
//
//    void CancelEvent(wxCommandEvent &event);
//    void OnIconSizeEvent(wxCommandEvent &event);
//    bool GetBuildIndex() { return m_buildIndex; }
//
//    void SetParInfoToWindow();
//
//    ACButton *GetdownLoadInfoLabel() { return m_downLoadInfoLabel; }
//    wxGauge * GetdownLoadgauge() { return m_downLoadgauge; }
//
//private:
//    bool      m_buildIndex{false};
//    ACButton *m_logButton;
//    ACButton *m_downLoadInfoLabel;
//    ACButton *m_InstallingUpdates;
//    ACButton *m_cancel_btn;
//
//    ACButton *m_btIconize;
//    ACButton *m_btMaximize;
//    ACButton *m_btClose;
//
//
//    wxGauge * m_downLoadgauge;
//
//
//    wxBoxSizer * m_pageSizer;
//    ACStaticBox *m_page;
//    wxBoxSizer * m_mainSizer;
//
//    ACButton *laterBtn;
//    ACButton *updateNowBtn;
//
//    ACButton *m_tipInfo;
//    wxObject *m_info_parent;
//    void msw_rescale();
//    void on_dpi_changed(const wxRect &suggested_rect) override { msw_rescale(); }
//};
//
//class CheckUpdateProgressFinishDialog : public DPIDialog
//{
//public:
//    explicit CheckUpdateProgressFinishDialog(wxWindow *parent)
//        : DPIDialog(parent, wxID_ANY, "", wxDefaultPosition, wxDefaultSize, wxNO_BORDER | wxSTAY_ON_TOP, "CheckUpdateProgressFinishDialog")
//    {}
//    ~CheckUpdateProgressFinishDialog() = default;
//    void SetInParent(wxObject *info) { m_info_parent = info; };
//    void Build();
//
//    void CancelEvent(wxCommandEvent &event);
//    void InstallNowEvent(wxCommandEvent &event);
//    bool GetBuildIndex() { return m_buildIndex; }
//
//    void SetParInfoToWindow();
//
//private:
//    bool      m_buildIndex{false};
//    ACButton *m_logButton;
//    ACButton *m_prepareInfo;
//    ACButton *m_prepareInfo_content;
//    ACButton *m_cancel_btn;
//    ACButton *m_installNow_btn;
//
//
//    wxBoxSizer *m_pageSizer_bottom;
//    wxBoxSizer *m_pageSizer_center;
//    wxBoxSizer *m_pageSizer_center_right;
//    wxBoxSizer *m_mainSizer;
//
//    wxBoxSizer * m_pageSizer;
//    ACStaticBox *m_page;
//
//    ACButton *laterBtn;
//    ACButton *updateNowBtn;
//
//    ACButton *m_tipInfo;
//    wxObject *m_info_parent;
//    void msw_rescale();
//    void on_dpi_changed(const wxRect &suggested_rect) override { msw_rescale(); }
//};
//
//
//class ACCheckUpdate : public wxObject
//{
//public:
//    ACCheckUpdate(wxWindow *parent);
//    ~ACCheckUpdate() = default;
//
//	void CreateCheckVersionDialog();
//	void ShowCheckVersionDialog();
//    void CreateCheckUpdateInfoDiaglog();
//    void ShowCheckUpdateInfoDialog();
//    void CreateCheckUpdateProgressDialog();
//    void ShowCheckUpdateProgressDialog();
//    void CreateCheckUpdateProgressFinishDialog();
//    void ShowCheckUpdateProgressFinishDialog();
//
//    bool GetCheckEnableState();
//    void GetNowSoftwareVersionInfo();
//    void SetCheckEnableState(bool index);
//
//	Semver      GetNowVersion() { 
//        return m_now_version; 
//    }
//    std::string      GetNowVersionString() { 
//
//        std::stringstream version;
//        version << m_now_version;
//        std::string version_s = version.str();
//        return version_s; 
//    }
//    Semver      GetOnlineVersion() {
//        return m_online_version;
//    }
//    Semver           GetOnlineVersionString() { 
//        std::stringstream version;
//        version << m_online_version;
//        std::string version_s = version.str();
//        return version_s; 
//    }
//    void SetOnlineVersion(wxString version) { 
//        m_online_version = *Semver::parse(version.ToStdString());
//    }
//    void SetOnlineVersion(Semver &version) {m_online_version    = version;}
//    wxString    GetOnlineVesionTime() { return m_online_versionTime;}
//
//
//    void     SetOnlineSoftwarePath(wxString path) { m_softwarePath = path; }
//    wxString GetOnlineSoftwarePath() { return m_softwarePath; }
//
//    void     SetOnlineSoftwareSize(int index) { m_softwareSize = index; }
//    void SetOnlineSoftwareSize(std::string index) { 
//        m_softwareSize = std::stoi(index); 
//    }
//    int  GetOnlineSoftwareSize() { return m_softwareSize; }
//    
//    void SetnlineVesionTime(wxString info) { m_online_versionTime = info; }
//
//    wxString    GetOnlineImprove() { return m_online_improve; }
//    void     SetOnlineImprove(wxString info) { m_online_improve = info; }
//
//    wxString    GetOnlineBugFixes() { return m_online_bug_fixes; }
//    void     SetOnlineBugFixes(wxString info) { m_online_bug_fixes = info; }
//
//    wxString GetTipInfo() { return m_tipInfo; }
//    void        SetTipInfo(wxString info) { m_tipInfo = info; }
//
//    bool GetCheckUpdateResult() { return m_UpdateIndex; }
//    void SetCheckUpdateResult(bool index) { m_UpdateIndex = index; }
//    
//    int GetNetWorkTimeOutInfo() { return m_NetWorkTimeOut; }
//
//    bool GetSoftwareIsNewVersion() { return m_IsNewVersion; }
//    void SetSoftwareIsNewVersionIndex(bool index) { m_IsNewVersion = index; }
//
//    bool GetDownLoadIsFinishIndex() { return m_DownLoadIsFinish; }
//    bool SetDownLoadIsFinishIndex(bool index) { m_DownLoadIsFinish = index; }
//
//    int GetNowDownLoadProgressIndex() { return m_NowDownLoadProgress; }
//    wxString GetNowDownLoadProgressIndexString() { return wxString::Format("%d", m_NowDownLoadProgress); }
//    void SetNowDownLoadProgressIndex(int index) { m_NowDownLoadProgress = index; }
//
//    float  GetNowDownFileSize() { return m_NowDownFileSize; }
//    wxString GetNowDownFileSizeString() { return wxString::Format("%.2f", m_NowDownFileSize); }
//    void SetNowDownFileSize(float index) { m_NowDownFileSize = index; }
//
//    float  GetDownFileSize() { return m_DownFileSize; }
//    wxString GetDownFileSizeString() { return wxString::Format("%.2f", m_DownFileSize); }
//    void SetDownFileSize(float index) { m_DownFileSize = index; }
//
//    bool StartCheckForUpdates();
//
//
//    AppConfig *GetAppConfigObj() { return m_app_config; }
//    CheckVersionDialog *             GetCheckVersionDialogObj() { return m_checkVersionDialog; }
//    CheckUpdateInfoDialog *          GetCheckUpdateInfoDialogObj() { return m_checkUpdateInfoDialog; }
//    CheckUpdateProgressDialog *      GetCheckUpdateProgressDialogObj() { return m_checkUpdateProgressDialog; }
//    CheckUpdateProgressFinishDialog *GetCheckUpdateProgressFinishDialogObj() { return m_checkUpdateProgressFinishDialog; }
//
//    wxWindow *GetWindowParent() { return m_parent; }
//
//    void SetImproveInfoZH(std::vector<wxString> info) { improveInfoList_zh = info; }
//    void SetImproveInfoEN(std::vector<wxString> info) { improveInfoList_en = info; }
//
//    void SetBugFixesInfoZH(std::vector<wxString> info) { bufFixesInfoList_zh = info; }
//    void SetBugFixesInfoEN(std::vector<wxString> info) { bufFixesInfoList_en = info; }
//
//    std::vector<wxString> GetImproveInfoZH() { return improveInfoList_zh; }
//    std::vector<wxString> GetImproveInfoEN() { return improveInfoList_en; }
//    std::vector<wxString> GetBugFixesInfoZH() { return bufFixesInfoList_zh; }
//    std::vector<wxString> GetBugFixesInfoEN() { return bufFixesInfoList_en; }
//
//    std::vector<wxString> GetImproveAndBeginInfoList_EN() { return improveAndBeginInfoList_en; }
//    void                  SetImproveAndBeginInfoList_EN(std::vector<wxString> info) { improveAndBeginInfoList_en = info; }
//
//    std::vector<wxString> GetImproveAndBeginInfoList_ZH() { return improveAndBeginInfoList_zh; }
//    void                  SetImproveAndBeginInfoList_ZH(std::vector<wxString> info) { improveAndBeginInfoList_zh = info; }
//
//    void SetIniFileDownInfo(wxString info);
//    void SetDownloadProgressPercentage(float percentage);
//
//    bool SetIniFileInfo(std::vector<std::string> &info, bool showDialog);
//
//    bool SetCheckVersionDialogBtnIco(const wxString &iconName) { m_checkVersionDialog->GeCheckForUpdatesBtn()->SetIcon(iconName); }
//    void SetCheckVersionDialogBtnLabel(const wxString &info)
//    { m_checkVersionDialog->GeCheckForUpdatesBtn()->SetLabel(info);
//    }
//
//    void CloseAllDialog();
//
//    AppConfig *GetAppConfig() { return m_app_config; }
//
//    boost::filesystem::path GetDownFileDestPath() { return m_DownFile_dest_path; }
//    void                    SetDownFileDestPath(std::string value) { m_DownFile_dest_path = (boost::filesystem::path) value; }
//
//
//private:
//    wxWindow *                       m_parent;
//    CheckVersionDialog*              m_checkVersionDialog = nullptr;
//    CheckUpdateInfoDialog *          m_checkUpdateInfoDialog = nullptr;
//    CheckUpdateProgressDialog *      m_checkUpdateProgressDialog = nullptr;
//    CheckUpdateProgressFinishDialog *m_checkUpdateProgressFinishDialog = nullptr;
//
//
//    Semver                          m_now_version;
//    Semver                          m_online_version;
//    int                             m_softwareSize = 0;
//    wxString                        m_softwarePath;
//    wxString                        m_online_versionTime;
//
//    wxString                        m_online_improve;
//    wxString                        m_online_bug_fixes;
//    wxString                        m_tipInfo;
//
//    std::vector<wxString> improveAndBeginInfoList_en;
//    std::vector<wxString> improveAndBeginInfoList_zh;
//
//
//    std::vector<wxString>           improveInfoList_en;
//    std::vector<wxString>           improveInfoList_zh;
//    std::vector<wxString>           bufFixesInfoList_en;
//    std::vector<wxString>           bufFixesInfoList_zh;
//
//    bool                            m_UpdateIndex{false};
//    bool                            m_DownLoadIsFinish{false};
//
//    int                             m_NetWorkTimeOut = 10;
//
//    bool                            m_IsNewVersion{true};
//
//    int                             m_NowDownLoadProgress = 0;
//    float                           m_NowDownFileSize = 0.0f;
//    float                           m_DownFileSize    = 0.0f;
//
//    boost::filesystem::path         m_DownFile_dest_path;
//
//    AppConfig *m_app_config = nullptr;
//
//
//};
//
//} // GUI
//} // Slic3r
//
//#endif /* slic3r_ACCheckUpdate_hpp_ */
