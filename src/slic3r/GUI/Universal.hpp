#ifndef AC_UNIVERSAL_HPP
#define AC_UNIVERSAL_HPP

/**************************************************
*此容器类似Plater，属于同级，镶嵌在主界面。 2023/08/04
*当显示Plater，此界面隐藏；反之显示此界面，Plater隐藏
***************************************************/


#include "ACButton.hpp"
#include "MainFrame.hpp"
#include "slic3r/GUI/Cloud/ACCloudMachinePrivate.hpp"


#include <wx/wx.h>
#include <wx/panel.h>
#include <wx/bookctrl.h>
#include <wx/stattext.h>



namespace Slic3r { 
namespace GUI {

class ACButtonRoundUser;
class ACCloudMachine;

class UniversalToolBar : public wxPanel
{
public:
	UniversalToolBar(wxWindow* parent, MainFrame* main_frame);
	UniversalToolBar(UniversalToolBar&&) = delete;
	UniversalToolBar(const UniversalToolBar&) = delete;
	UniversalToolBar& operator=(UniversalToolBar&&) = delete;
	UniversalToolBar& operator=(const UniversalToolBar&) = delete;
	~UniversalToolBar();

	void OnButtonReturn(wxCommandEvent& event);
	void OnButtonUser(wxCommandEvent& event);

private:
	ACButtonRoundUser* buttonUser;
	ACButton* buttonReturn;
	wxStaticText* text;
	MainFrame* p;
	ACQuickResponseDialog* popup;
};

class Universal : public wxPanel
{
public:
	Universal(wxWindow* parent, MainFrame* main_frame);
	Universal(Universal&&) = delete;
	Universal(const Universal&) = delete;
	Universal& operator=(Universal&&) = delete;
	Universal& operator=(const Universal&) = delete;
	~Universal();

	void set_focus();

	void UpdatePrinterList();

	wxPanel* m_space_panel;
	UniversalToolBar* m_tool_bar;//对应mainframe的toolbar栏
	ACCloudMachine* m_cloud_machine;
	wxBookCtrlBase* m_book_ctrl{ nullptr };//controls all panel;


};


} // namespace GUI
} // namespace Slic3r

#endif //!AC_UNIVERSAL_HPP
