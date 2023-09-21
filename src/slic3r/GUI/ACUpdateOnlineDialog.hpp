#ifndef slic3r_ACUpdateOnlineDialog_hpp_
#define slic3r_ACUpdateOnlineDialog_hpp_

#include "GUI_Utils.hpp"
#include "ACButton.hpp"
#include "ACLabel.hpp"
#include "ACCheckBox.hpp"
#include "ACStaticBox.hpp"
#include "ACDialogTopbar.hpp"
#include "ACDialogTopbar1.hpp"
#include "ACScaleBitmapHighQuality.hpp"

#include <wx/panel.h>
#include <wx/wx.h>
#include <wx/gauge.h>
#include <wx/sizer.h>
#include <wx/textctrl.h>
#include <wx/animate.h>
#include <vector>

#include "GUI.hpp"
#include "GUI_App.hpp"
#include "MainFrame.hpp"
#include "wxExtensions.hpp"
#include "libslic3r/Semver.hpp"
#include "ACDefines.h"
#include "libslic3r/AppConfig.hpp"
#include "I18N.hpp"
#include <wx/dialog.h>
#include <wx/timer.h>
#include <map>
#include "MsgDialog.hpp"

class wxPanel;
class wxTextCtrl;
class wxStaticText;
class wxBoxSizer;
class wxFlexGridSizer;
class wxCheckBox;
class wxButton;
class wxStaticBitmap;
class wxDialog;

namespace Slic3r {

namespace GUI {

class RoundTextCtrl;
class ACUpdateManger;

class PrivateDrawBackground : public DPIPanelEX
{
public:
	PrivateDrawBackground(wxWindow* parent);
	PrivateDrawBackground(const PrivateDrawBackground& obj) = delete;
	
	void OnMouseLeftDown(wxMouseEvent& event);
	void OnMouseLeftUp(wxMouseEvent& event);
	void OnMouseMove(wxMouseEvent& event);
protected:
	void msw_rescale();
	void on_dpi_changed(const wxRect& suggested_rect) override { msw_rescale(); };
	void OnPaint(wxPaintEvent& event);
private:
	wxBitmap bitmap;
	wxImage imageHigh;
	wxImage image;
	std::string filePath;

	wxPoint m_delta;

	wxWindow* m_parent{nullptr};
};

class ACUpdateOnlineDialog : public DPIDialog
{
public:
	ACUpdateOnlineDialog(wxWindow* parent);
	ACUpdateOnlineDialog(const ACUpdateOnlineDialog& obj) = delete;
	~ACUpdateOnlineDialog();

	void Build();
	void Init();
	void Connect();

	void OnUpdateCheck();//slot button update check
    void OnUpdateCheckEvent();
	void OnCheckBox();//slot checkbox
	void OnClose();

	void SetText2(const wxString& str) { text2->SetLabel(str); Layout(); }
	void SetText4(const wxString& str) { text4->SetLabel(str); Layout(); }//warning
	void SetCheckBoxChecked(bool isChecked) { checkBox->SetValue(isChecked); checkBox->Refresh(); }
	void SetACUpdateManger(ACUpdateManger* p) { parent = p; }
	void SetText4Color(bool r) { text4->SetTextColor(r ? ACStateColor(wxColour(220, 0, 0)) : ACStateColor(wxColour(87, 167, 28))); text4->Refresh(); }
	void SetWaitGIF(bool isEnable);
	ACButton* GetButton(){ return button; }
protected:
	void msw_rescale();
	void on_dpi_changed(const wxRect& suggested_rect) override { msw_rescale(); };

private:
	wxWindow* origin;//for mask
	PrivateDrawBackground* panel;
	ACButton* text1;
	ACButton* text2;
	ACCheckBox* checkBox;
	ACButton* text3;//checkbox text

	ACButton* button;//update check
	ACButton* buttonClose;

	ACButton* text4;//warning
	ACButton* buttonWarning;
	
	wxBoxSizer* sizer;
	wxBoxSizer* sizerMain;
	wxBoxSizer* hBox;
	wxBoxSizer* hBox1;
	wxBoxSizer* vBox;

	wxAnimationCtrl* animationCtrl;
	ACUpdateManger* parent;
};

class ACInstallWizardDialog : public DPIDialog
{
public:
	ACInstallWizardDialog(wxWindow* parent);
	ACInstallWizardDialog(const ACInstallWizardDialog& obj) = delete;
	~ACInstallWizardDialog();

	void Build();
	void Init();
    void Connect();

	void OnCancel();
    void OnCancelWin();
	void OnIcon();

	void SetProgressText(const wxString& str) { progressText->SetLabelText(str); }
	void SetProgressValue(int value);//percentage
    void SetACUpdateManger(ACUpdateManger *p) { parent = p; }
    RichMessageDialog *GetShowModelDialog() { return m_dialog; }
    void               SetShowModelDialogNull() { m_dialog = nullptr; }

protected:
	void msw_rescale();
	void on_dpi_changed(const wxRect& suggested_rect) override { msw_rescale(); };

	void OnMouseLeftDown(wxMouseEvent& event);
	void OnMouseLeftUp(wxMouseEvent& event);
	void OnMouseMove(wxMouseEvent& event);
private:
	wxWindow* origin;//for mask
	wxGauge* progressBar;
	
	//wxStaticBitmap* logoBitmap;
	ACButton* logoBitmap;
	wxStaticText* text;
	wxStaticText* installingText;
	wxStaticText* progressText;
	ACButton* buttonCancel;

	ACButton* buttonIcon;
	ACButton* buttonMax;
	ACButton* buttonClose;

	wxBoxSizer* vBox;
	wxBoxSizer* hBox;
    ACUpdateManger *parent;
    RichMessageDialog *m_dialog = nullptr;

	wxPoint m_delta;
};


class ACUpdateOnlineInfoDialog : public DPIDialog
{
public:
	explicit ACUpdateOnlineInfoDialog(wxWindow* parent);
	ACUpdateOnlineInfoDialog(const ACUpdateOnlineInfoDialog& obj) = delete;
	~ACUpdateOnlineInfoDialog();

	void Build();
	void Init();
	void Connect();//bind
	void OnClose();//Close
	void OnUpdate();//update
    void OnCloseWin(); // update
	void SetContext(const wxString& context);

	void SetLatestVersion(const wxString& str) { topBar->SetTitle(str); }
	void SetCurrentVersion(const wxString& str) { topBar->SetSubTitle(str); }
	void SetOnlineVersion(const wxString& str) { onlineVersion->SetLabelText(str); }
	void SetTimeText(const wxString& str) { textTime->SetLabelText(str); }
	void SetACUpdateManger(ACUpdateManger* p) { parent = p; }

protected:
	void msw_rescale();
	void on_dpi_changed(const wxRect& suggested_rect) override { msw_rescale(); };
private:
	wxWindow* origin;//for mask
	wxBoxSizer* hBottomBox;
	wxBoxSizer* vBox;
	wxBoxSizer* vContext;
	//ACScaleBitmapHighQuality* logoBitmap;

	ACDialogTopbar1* topBar;

	wxStaticText* onlineVersion; //online update version
	wxStaticText* textTime;      //time
	
	ACButton* buttonLater; //later
	ACButton* buttonUpdate;//install now
	ACStaticBox* box;
	wxTextCtrl* text;

	ACUpdateManger* parent;
};


class ACHint1Dialog : public DPIDialog
{
public:
	explicit ACHint1Dialog(wxWindow* parent);
	ACHint1Dialog(const  ACHint1Dialog& obj) = delete;
	~ACHint1Dialog();

	void Build();
	void Init();
	void Connect();

	//Slots
	void OnClose();
	void OnUpdateNow();
protected:
	void msw_rescale();
	void on_dpi_changed(const wxRect& suggested_rect) override { msw_rescale(); };
private:
	wxWindow* origin;//for mask
	ACDialogTopbar* topbar;
	wxBoxSizer* hBox1;
	wxBoxSizer* hBox2;
	wxBoxSizer* vBox1;
	wxBoxSizer* vBox2;

	ACButton* buttonCancel;    //Cancel
	ACButton* buttonUpdateNow; //Update Now

	wxStaticText* textTop;     //Prepare to install the update
	wxStaticText* textBottom;  //Update content is download. Do you want to install now?
	ACButton* logoBitmap;
	//wxStaticBitmap* logoBitmap;//logo
};


class Hint2Dialog : public DPIDialog
{
public:
	explicit Hint2Dialog(wxWindow* parent);
	Hint2Dialog(const  Hint2Dialog& obj) = delete;
	~Hint2Dialog();

	void Init();
	void Connect();

	void SetContext(const wxString& text);//text

	//Slots
	void OnClose();
	void OnCancel();
	void OnDiscard();
	void OnSave();
private:
	void on_dpi_changed(const wxRect& suggested_rect)
	{
		int em = em_unit();
		BOOST_LOG_TRIVIAL(error) << "on_dpi_changed";
		const wxSize& size = wxSize(45 * em, 40 * em);
		SetSize(size);
		Fit();
		Refresh();
	}
private:
	ACDialogTopbar* topbar;
	wxBoxSizer* hBox1;
	wxBoxSizer* hBox2;
	wxBoxSizer* vBox;

	ACButton* buttonCancel;  //Cancel
	ACButton* buttonDiscard; //Discard
	ACButton* buttonSave;    //Save

	wxStaticText* text;     //Prepare to install the update
	wxStaticBitmap* logoBitmap;//logo
};

class ACUpdateManger : public wxObject
{
public:
	ACUpdateManger(wxWindow* parent);
	~ACUpdateManger()=default;

	void ShowCheckVersionDialog();
	void ShowCheckUpdateInfoDialog();
	void ShowCheckUpdateProgressDialog();
	void ShowCheckUpdateProgressFinishDialog();

	void SetShowCenter(wxWindow* w);

	bool GetCheckEnableState();
	void GetNowSoftwareVersionInfo();
    std::string GetNowSoftwareLanguageInfo();
	void SetCheckEnableState(bool index);

	//Set infomation
	void SetUpdateOnlineDialog();
	void SetUpdateOnlineInfoDialog();
	void SetInstallWizardDialog();
	void SetHint1Dialog();

	Semver      GetNowVersion()
	{
		return m_now_version;
	}
	std::string      GetNowVersionString()
	{

		std::stringstream version;
		version << m_now_version;
		std::string version_s = version.str();
		return version_s;
	}
	Semver      GetOnlineVersion()
	{
		return m_online_version;
	}
	Semver           GetOnlineVersionString()
	{
		std::stringstream version;
		version << m_online_version;
		std::string version_s = version.str();
		return version_s;
	}
	void SetOnlineVersion(wxString version)
	{
		m_online_version = *Semver::parse(version.ToStdString());
	}
	void SetOnlineVersion(Semver & version) { m_online_version = version; }
	wxString    GetOnlineVesionTime() { return m_online_versionTime; }


	void SetOnlineForceUpgradeStyle(wxString info) { m_online_ForceUpgrade = info == "1" ? true:false; }
    bool GetOnlineForceUpgradeStyle() { return m_online_ForceUpgrade; }


	void     SetOnlineSoftwarePath(wxString path) { m_softwarePath = path; }
	wxString GetOnlineSoftwarePath() { return m_softwarePath; }

	void     SetOnlineSoftwareSize(int index) { m_softwareSize = index; }
	void SetOnlineSoftwareSize(std::string index)
	{
		m_softwareSize = std::stoi(index);
	}
	int  GetOnlineSoftwareSize() { return m_softwareSize; }

	void SetnlineVesionTime(wxString info) { m_online_versionTime = info; }

	wxString    GetOnlineImprove() { return m_online_improve; }
	void     SetOnlineImprove(wxString info) { m_online_improve = info; }

	wxString    GetOnlineBugFixes() { return m_online_bug_fixes; }
	void     SetOnlineBugFixes(wxString info) { m_online_bug_fixes = info; }

	wxString GetTipInfo() { return m_tipInfo; }
	void        SetTipInfo(wxString info) { m_tipInfo = info; }

	bool GetCheckUpdateResult() { return m_UpdateIndex; }
	void SetCheckUpdateResult(bool index) { m_UpdateIndex = index; }

	int GetNetWorkTimeOutInfo() { return m_NetWorkTimeOut; }

	bool GetSoftwareIsNewVersion() { return m_IsNewVersion; }
	void SetSoftwareIsNewVersionIndex(bool index) { m_IsNewVersion = index; }

	bool GetDownLoadIsFinishIndex() { return m_DownLoadIsFinish; }
	//bool SetDownLoadIsFinishIndex(bool index) { m_DownLoadIsFinish = index; }

	int GetNowDownLoadProgressIndex() { return m_NowDownLoadProgress; }
	wxString GetNowDownLoadProgressIndexString() { return wxString::Format("%d", m_NowDownLoadProgress); }
	void SetNowDownLoadProgressIndex(int index) { m_NowDownLoadProgress = index; }

	float  GetNowDownFileSize() { return m_NowDownFileSize; }
	wxString GetNowDownFileSizeString() { return wxString::Format("%.2f", m_NowDownFileSize); }
	void SetNowDownFileSize(float index) { m_NowDownFileSize = index; }

	float  GetDownFileSize() { return m_DownFileSize; }
	wxString GetDownFileSizeString() { return wxString::Format("%.2f", m_DownFileSize); }
	void SetDownFileSize(float index) { m_DownFileSize = index; }

	bool StartCheckForUpdates();


	AppConfig* GetAppConfigObj() { return m_app_config; }

	void SetImproveInfoZH(std::vector<wxString> info) { improveInfoList_zh = info; }
	void SetImproveInfoEN(std::vector<wxString> info) { improveInfoList_en = info; }

	void SetBugFixesInfoZH(std::vector<wxString> info) { bufFixesInfoList_zh = info; }
	void SetBugFixesInfoEN(std::vector<wxString> info) { bufFixesInfoList_en = info; }

	std::vector<wxString> GetImproveInfoZH() { return improveInfoList_zh; }
	std::vector<wxString> GetImproveInfoEN() { return improveInfoList_en; }
	std::vector<wxString> GetBugFixesInfoZH() { return bufFixesInfoList_zh; }
	std::vector<wxString> GetBugFixesInfoEN() { return bufFixesInfoList_en; }

	std::vector<wxString> GetImproveAndBeginInfoList_EN() { return improveAndBeginInfoList_en; }
	void                  SetImproveAndBeginInfoList_EN(std::vector<wxString> info) { improveAndBeginInfoList_en = info; }

	std::vector<wxString> GetImproveAndBeginInfoList_ZH() { return improveAndBeginInfoList_zh; }
	void                  SetImproveAndBeginInfoList_ZH(std::vector<wxString> info) { improveAndBeginInfoList_zh = info; }

	void SetIniFileDownInfo(wxString info);
	void SetTexe4Color(bool red = true);
	void SetDownloadProgressPercentage(float percentage);

	bool SetIniFileInfo(std::vector<std::string> &info,const std::string &downLoadUrl,bool showDialog);

	//bool SetCheckVersionDialogBtnIco(const wxString & iconName) { dlg->GetButton()->SetIcon(iconName); }
	void SetCheckVersionDialogBtnLabel(const wxString & info) { dlg->GetButton()->SetLabel(info); }
	void SetCheckVersionDialogGIF() { dlg->SetWaitGIF(false); }

	void CloseAllDialog();
    void SetWindowPosEvent(wxWindow* win);

	RichMessageDialog* ShowForceUpgradeDialog(wxString content, wxString title, wxString OKLable, wxString NOLable);

	AppConfig* GetAppConfig() { return m_app_config; }

	boost::filesystem::path GetDownFileDestPath() { return m_DownFile_dest_path; }
	void                    SetDownFileDestPath(std::string value) { m_DownFile_dest_path = (boost::filesystem::path)value; }

	ACInstallWizardDialog* GetCheckUpdateProgressDialogObj() { return dlgProgress; }
    ACUpdateOnlineDialog * GetUpdateOnlineDialogObj() { return dlg; }
	void SetOriginParent(wxWindow* parent);
private:
	ACUpdateOnlineDialog* dlg;
	ACUpdateOnlineInfoDialog* dlgInfo;
	ACInstallWizardDialog* dlgProgress;
	ACHint1Dialog* dlgFinish;


	Semver                          m_now_version;
	Semver                          m_online_version;
	int                             m_softwareSize = 0;
	wxString                        m_softwarePath;
	wxString                        m_online_versionTime;

	bool							m_online_ForceUpgrade = false;

	wxString                        m_online_improve;
	wxString                        m_online_bug_fixes;
	wxString                        m_tipInfo;

	std::vector<wxString> improveAndBeginInfoList_en;
	std::vector<wxString> improveAndBeginInfoList_zh;


	std::vector<wxString>           improveInfoList_en;
	std::vector<wxString>           improveInfoList_zh;
	std::vector<wxString>           bufFixesInfoList_en;
	std::vector<wxString>           bufFixesInfoList_zh;

	bool                            m_UpdateIndex{ false };
	bool                            m_DownLoadIsFinish{ false };

	int                             m_NetWorkTimeOut = 10;

	bool                            m_IsNewVersion{ true };

	int                             m_NowDownLoadProgress = 0;
	float                           m_NowDownFileSize = 0.0f;
	float                           m_DownFileSize = 0.0f;

	boost::filesystem::path         m_DownFile_dest_path;

	AppConfig* m_app_config = nullptr;
};

}
}

#endif //slic3r_GUI_ACUPDATE_ONLINE_DIALOG_HPP
