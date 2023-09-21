#ifndef slic3r_GUI_ACDIALOG_TOP_BAR1_HPP
#define slic3r_GUI_ACDIALOG_TOP_BAR1_HPP

// ----------------------------------------------------------------------------
// ACDialogTopbar1 第一个版本是为了在线更新界面的ACUpdateDialogInfo作为Top可移动界
// 用，基于ACDialogTopbar的变更，为保持一致性，这里的成员对象昵称不做改动
// ----------------------------------------------------------------------------

#include "wx/wx.h"

#include "ACLabel.hpp"
#include "ACButton.hpp"
#include "GUI_Utils.hpp"

namespace Slic3r
{

namespace GUI
{

class ACDialogTopbar1 : public wxWindow
{
public:
	explicit ACDialogTopbar1(wxWindow* parent, const wxString& title, int toolbarW, int toolbarH = 62);
	~ACDialogTopbar1(void);

	void Init(wxWindow* parent);

	void OnMouseLeftDown(wxMouseEvent& event);
	void OnMouseLeftUp(wxMouseEvent& event);
	void OnMouseMotion(wxMouseEvent& event);

	void SetTitle(const wxString& title);
	void SetSubTitle(const wxString& subTitle);
	void SetShowCloseButton(bool show);
	void msw_rescale();
	void SetToolBarH(int h);
	//void LayoutLeft();// Only call once
	void SetCustomBackgroundColour(const wxColour& color);
public:
	//void OnClose(wxEvent& event);
private:
	wxWindow* m_frame;

	wxString    m_title;
	int         m_toolbar_w;
	int         m_toolbar_h;

	wxBoxSizer* m_mainSizer;
	wxBoxSizer* m_v_Sizer;//vertical sizer

	ACButton* m_title_item;//title
	ACButton* m_subtitle_item;//subtitle

	ACButton* m_logo_button;//logo
public:
	ACButton* m_close_button;
private:

	// drag move
	wxPoint m_delta;
	wxPoint m_point;
};




}

}


#endif //!slic3r_GUI_ACDIALOG_TOP_BAR1_HPP