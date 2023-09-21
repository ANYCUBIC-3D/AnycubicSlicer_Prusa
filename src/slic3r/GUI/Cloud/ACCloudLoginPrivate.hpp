#ifndef AC_CLOUD_LOGIN_PRIVATE_HPP
#define AC_CLOUD_LOGIN_PRIVATE_HPP

#include "ACDrawBackgroundPrivate.hpp"

#include "../ACStaticBox.hpp"
#include "../ACStateColor.hpp"
#include "../ACButton.hpp"
#include "../ACComboBox.hpp"
#include "../GUI.hpp"
#include "../I18N.hpp"
#include "../GUI_Utils.hpp"
#include "../wxExtensions.hpp"
#include "../ACStateHandler.hpp"

#include "libslic3r/Utils.hpp"
#include <boost/filesystem.hpp>

#include <wx/wx.h>
#include <wx/dialog.h>
#include <wx/sizer.h>
#include <wx/stattext.h>
#include <wx/checkbox.h>
#include <wx/bmpcbox.h>
#include <wx/textctrl.h>

#include <wx/combobox.h>
#include <wx/odcombo.h>
#include <wx/bmpcbox.h>
#include <wx/button.h>

#include <wx/popupwin.h>
#include <wx/listctrl.h>


namespace Slic3r {

namespace GUI {

wxDECLARE_EVENT(EVT_ACLOUD_LOGIN_BUTTON_CLOSE, wxCommandEvent);

class ACListViewAccount;

//Icon left comboCombox
/*---------------------------------------------------------------------------------
								ACComboBoxIconLieft
-----------------------------------------------------------------------------------*/
class ACComboBoxIconLeft : public wxOwnerDrawnComboBox
{
public:
	ACComboBoxIconLeft(wxWindow* parent,
		wxWindowID id,
		const std::string& name,
		const wxString& value = wxEmptyString,
		const wxPoint& pos = wxDefaultPosition,
		const wxSize& size = wxDefaultSize,
		const wxArrayString& choices = wxArrayString(),
		long style = 0,
		const wxValidator& validator = wxDefaultValidator,
		const wxString& className = "comboBox")
		: wxOwnerDrawnComboBox(parent, id, value, pos, size, choices, style, validator, className)
	{
		std::string filePath;
		if (boost::filesystem::exists(name + ".png"))
			filePath = name + ".png";
		else
			filePath = Slic3r::var(name + ".png");
		wxImage image(wxString::FromUTF8(filePath.c_str()), wxBITMAP_TYPE_ANY);
		if (image.IsOk())
			bitmap = wxBitmap(image);
	}

	// 重写绘制方法以显示带有图标的文本
	void OnDrawItem(wxDC& dc, const wxRect& rect, int item, int flags) const override
	{
		wxOwnerDrawnComboBox::OnDrawItem(dc, rect, item, flags);

		wxCoord textX, textY;
		dc.GetTextExtent(GetString(item), &textX, &textY);

		wxCoord bitmapX = rect.GetX() + 5;
		wxCoord bitmapY = rect.GetY() + (rect.GetHeight() - bitmap.GetHeight()) / 2;

		dc.DrawBitmap(bitmap, bitmapX, bitmapY, true);
		dc.DrawText(GetString(item), bitmapX + bitmap.GetWidth() + 5, rect.GetY() + (rect.GetHeight() - textY) / 2);
	}

private:
	wxBitmap bitmap;
	
};



/*---------------------------------------------------------------------------------
								ACComboBoxDrawText
-----------------------------------------------------------------------------------*/
class ACComboBoxDrawText : public wxComboBox
{
public:
	ACComboBoxDrawText(wxWindow* parent,
		wxWindowID id,
		const wxString& value = wxEmptyString,
		const wxPoint& pos = wxDefaultPosition,
		const wxSize& size = wxDefaultSize,
		const wxArrayString& choices = wxArrayString(),
		long style = 0,
		const wxValidator& validator = wxDefaultValidator,
		const wxString& className = "comboBox");

protected:
	void OnPaint(wxPaintEvent& event);

	DECLARE_EVENT_TABLE()
};

/*---------------------------------------------------------------------------------
								ACStaticDashLine
-----------------------------------------------------------------------------------*/
class ACStaticDashLine : public ACStaticBox
{
public:
	ACStaticDashLine(wxWindow* parent, wxWindowID id = wxID_ANY, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize,
		long style = wxLI_HORIZONTAL, const wxString& name = wxStaticLineNameStr);

	void OnPaint(wxPaintEvent& event);

	bool isHorizontal{ true };
	wxDECLARE_EVENT_TABLE();
};

/*---------------------------------------------------------------------------------
								ACButtonUnderline
-----------------------------------------------------------------------------------*/
class ACButtonUnderline : public ACButton
{
public:
	ACButtonUnderline(wxWindow* parent, wxString text, wxString icon = "", wxString hover_icon = "", wxString dis_icon = "", long style = 0, wxSize iconSize = wxSize(24, 24));

	void render(wxDC& dc) override;

	bool isSelect{ false };
	void SetSelect(bool s) { isSelect = s; isSelect ? SetTextColorNormal(wxColour(57, 134, 255)) : SetTextColorNormal(wxColour(20, 28, 41)); }
	bool GetSelect() { return isSelect; }
};

/*---------------------------------------------------------------------------------
								ACTextCtrlIcon
-----------------------------------------------------------------------------------*/
class ACTextCtrlIcon : public wxTextCtrl
{
public:
	ACTextCtrlIcon(wxWindow* parent,
		wxWindowID id,
		const std::string& name,
		const wxString& value = wxEmptyString,
		const wxPoint& pos = wxDefaultPosition,
		const wxSize& size = wxDefaultSize,
		long style = 0,
		const wxValidator& validator = wxDefaultValidator,
		const wxString& className = "comboBox");

	virtual ~ACTextCtrlIcon();

	void OnText(wxCommandEvent& event) { Refresh(); }
	void OnFocus(wxFocusEvent& event) { event.Skip(); Refresh(); }
	//void OnText1(wxMouseEvent& event) { Refresh(); }
protected:
	void OnPaint(wxPaintEvent& event);
	

	DECLARE_EVENT_TABLE()
private:
	wxBitmap bitmap;
};

/*---------------------------------------------------------------------------------
								ACTextCtrlAccount
-----------------------------------------------------------------------------------*/
class ACTextCtrlAccount : public wxTextCtrl
{
public:
	ACTextCtrlAccount(wxWindow* parent,
		wxWindowID id,
		const wxString& value = wxEmptyString,
		const wxPoint& pos = wxDefaultPosition,
		const wxSize& size = wxDefaultSize,
		long style = 0,
		const wxValidator& validator = wxDefaultValidator,
		const wxString& className = "textctrl");

	virtual ~ACTextCtrlAccount();

	void Init();
	void Connect();
	void LayoutReset();
	void OnButtonClicked();
public:
	enum ModelType
	{
		AC_EMAIL,//Email
		AC_PHONE //Phone Number
	};
	ModelType GetModelType() { return model; }
	void SetModelType(const ModelType& m);


	//wxButton* buttonEmail;//Email
	wxButton* button;     //Phone Number
	ACListViewAccount* view;
protected:
	//void OnPaint(wxPaintEvent& event);

private:
	ModelType model{ ModelType::AC_EMAIL };
	wxBoxSizer* hBox;
};

/*---------------------------------------------------------------------------------
								ACListViewAccount
-----------------------------------------------------------------------------------*/
class ACListViewAccount : public wxPopupTransientWindow
{
public:
	ACListViewAccount(wxWindow* parent, const wxPoint& pos, const wxSize& size);
	~ACListViewAccount();

	void Init();
	//void OnLeftClicked(wxListEvent& event);
	//void OnDoubleClicked(wxListEvent& event);

	wxListCtrl* GetListCtrl() { return l; }


	//wxDECLARE_EVENT_TABLE();
private:
	wxListCtrl* l;
	wxBoxSizer* sizer;
};

/*---------------------------------------------------------------------------------
								ACTextCtrlPassword
-----------------------------------------------------------------------------------*/
class ACTextCtrlPassword : public wxTextCtrl
{
public:
	ACTextCtrlPassword(wxWindow* parent,
		wxWindowID id,
		const wxString& value = wxEmptyString,
		const wxPoint& pos = wxDefaultPosition,
		const wxSize& size = wxDefaultSize,
		long style = 0,
		const wxValidator& validator = wxDefaultValidator,
		const wxString& className = "textctrl");

	virtual ~ACTextCtrlPassword();

	void Init();
	void Connect();
	void OnButtonClicked();
	void SetPasswordShow(bool s = true);
public:
	//wxButton* buttonIcon;
	wxButton* button;
private:
	wxBoxSizer* hBox;
	bool isPasswordShow{ false };
	wxBitmap bitmapShow;
	wxBitmap bitmapHide;
};


/*---------------------------------------------------------------------------------
								ACButtonRoundUser
-----------------------------------------------------------------------------------*/
class ACButtonRoundUser : public wxButton
{
public:
	ACButtonRoundUser(wxWindow* parent, wxWindowID id, const wxString& label = wxEmptyString, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize, long style = 0, const wxValidator& validator = wxDefaultValidator, const wxString& name = wxButtonNameStr);

	~ACButtonRoundUser() = default;

protected:
	virtual void OnPaint(wxPaintEvent& event);
	virtual void OnShow(wxShowEvent& event);
	virtual void OnKillFocus(wxFocusEvent& event);
	virtual void OnSetFocus(wxFocusEvent& event);
};

/*---------------------------------------------------------------------------------
								ACTransparentPanel
-----------------------------------------------------------------------------------*/
class ACTransparentPanel : public wxPanel
{
public:
	ACTransparentPanel(wxWindow* parent);
	ACTransparentPanel(ACTransparentPanel&&) = delete;
	ACTransparentPanel(const ACTransparentPanel&) = delete;
	ACTransparentPanel& operator=(ACTransparentPanel&&) = delete;
	ACTransparentPanel& operator=(const ACTransparentPanel&) = delete;
	~ACTransparentPanel() = default;

protected:
	virtual void OnPaint(wxPaintEvent& event);
	//virtual void OnEraseBackground(wxEraseEvent& event);
	virtual void render(wxDC& dc);
	void DrawOnContext(wxGraphicsContext& gc);
};


/*---------------------------------------------------------------------------------
								ACBackgroundRounded
-----------------------------------------------------------------------------------*/
class ACBackgroundRounded : public wxWindow
{
public:
	ACBackgroundRounded(wxWindow* parent, wxWindowID id = wxID_ANY, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize, long style = 0, const wxString& name = wxPanelNameStr);
	~ACBackgroundRounded();
protected:
	virtual void OnPaint(wxPaintEvent& event);
};

/*---------------------------------------------------------------------------------
								ACButtonTransparent
-----------------------------------------------------------------------------------*/
class ACButtonTransparent : public wxWindow
{
public:
	ACButtonTransparent(wxWindow* parent, wxWindowID id = wxID_ANY, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize, long style = 0, const wxString& name = wxPanelNameStr);
	~ACButtonTransparent() = default;

	enum ButtonState
	{
		Normal = 0,
		//Checked = 2,
		Hover = 8,
		Pressed = 16,
		Disabled = 1 << 16
	};

	void SetButtonState(ButtonState state);
protected:
	virtual void OnEnter(wxMouseEvent& event);
	virtual void OnLeave(wxMouseEvent& event);
	virtual void OnMouseDown(wxMouseEvent& event);
	virtual void OnMouseUp(wxMouseEvent& event);
	virtual void OnPaint(wxPaintEvent& event);
	virtual void OnDPI(wxDPIChangedEvent& event);
private:
	bool isMouseInside{ false };
	bool isMousePressed{ false };
	ButtonState buttonState{ ButtonState::Normal };
	wxBitmap iconHover;
	wxBitmap iconNormal;
};

/// <summary>
/// 未使用
/// </summary>

/*---------------------------------------------------------------------------------
								wxButtonUnderline
-----------------------------------------------------------------------------------*/
class wxButtonUnderline : public wxButton
{
public:
	wxButtonUnderline(wxWindow* parent, wxWindowID id, const wxString& label = wxEmptyString, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize, long style = 0, const wxValidator& validator = wxDefaultValidator, const wxString& name = wxButtonNameStr);

	~wxButtonUnderline();

	enum ButtonState
	{
		Normal,
		Hover,
		Pressed,
		Disabled
	};
	enum AC_BUTTON_TYPE
	{
		AC_BUTTON_LV0,
		AC_BUTTON_LV1,
		AC_BUTTON_LV2,
		AC_BUTTON_LV3,
		AC_BUTTON_LABEL
	};

	void SetButtonState(ButtonState state);

	//void SetButtonType(AC_BUTTON_TYPE type);
	//void SetBackgroundColor(ACStateColor const& color);
	//void SetBorderColor(ACStateColor const& color);
	//void SetTextColor(ACStateColor const& color);
	//void clearColor();
protected:
	virtual void OnEnter(wxMouseEvent& event);
	virtual void OnLeave(wxMouseEvent& event);
	virtual void OnMouseDown(wxMouseEvent& event);
	virtual void OnMouseUp(wxMouseEvent& event);
	virtual void OnPaint(wxPaintEvent& event);
	virtual void DrawButton(wxDC& dc);

	//DECLARE_EVENT_TABLE()
private:
	bool isMouseInside{ false };
	bool isMousePressed{ false };
	std::vector<wxColour> bgColor;
	std::vector<wxColour> bdColor;
	std::vector<wxColour> fgColor;
	ButtonState buttonState;

	//private:
	//	ACStateColor bgColor;
	//	ACStateColor bdColor;
	//	ACStateColor fgColor;
};

/*---------------------------------------------------------------------------------
								ACComboBoxIcon
-----------------------------------------------------------------------------------*/
class ACComboBoxIcon : public wxComboBox
{
public:
	ACComboBoxIcon(wxWindow* parent,
		wxWindowID id,
		const std::string& name,
		const wxString& value = wxEmptyString,
		const wxPoint& pos = wxDefaultPosition,
		const wxSize& size = wxDefaultSize,
		const wxArrayString& choices = wxArrayString(),
		long style = 0,
		const wxValidator& validator = wxDefaultValidator,
		const wxString& className = "comboBox");

protected:
	void OnPaint(wxPaintEvent& event);

	DECLARE_EVENT_TABLE()
private:
	wxBitmap bitmap;
};

} // GUI
} // Slic3r

#endif //!AC_CLOUD_LOGIN_PRIVATE_HPP
