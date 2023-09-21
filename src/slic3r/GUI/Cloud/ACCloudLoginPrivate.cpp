#include "ACCloudLoginPrivate.hpp"

#include "../GUI.hpp"
#include "../GUI_App.hpp"
#include "../GUI_Utils.hpp"
#include "../BitmapCache.hpp"
#include "../ACLabel.hpp"
#include "libslic3r/Utils.hpp"
#include <boost/filesystem.hpp>

#include <wx/wx.h>
#include <wx/sizer.h>
#include <wx/panel.h>
#include <wx/font.h>
#include <wx/graphics.h>
#include <wx/dcgraph.h>
#include <wx/bitmap.h>

#include <string>
#include <stdio.h>

//#define GetEditHwnd() ((HWND)(GetEditHWND()))

namespace Slic3r {

namespace GUI {


BEGIN_EVENT_TABLE(ACComboBoxIcon, wxComboBox)
EVT_PAINT(ACComboBoxIcon::OnPaint)
END_EVENT_TABLE()

BEGIN_EVENT_TABLE(ACComboBoxDrawText, wxComboBox)
EVT_PAINT(ACComboBoxDrawText::OnPaint)
END_EVENT_TABLE()

BEGIN_EVENT_TABLE(ACStaticDashLine, ACStaticBox)
//EVT_PAINT(ACStaticDashLine::OnPaint)
END_EVENT_TABLE()

BEGIN_EVENT_TABLE(ACTextCtrlIcon, wxTextCtrl)
//EVT_PAINT(ACTextCtrlIcon::OnPaint)
END_EVENT_TABLE()


wxDEFINE_EVENT(EVT_ACLOUD_LOGIN_BUTTON_CLOSE, wxCommandEvent);
//BEGIN_EVENT_TABLE(ACListViewAccount, wxPopupTransientWindow)
//EVT_LIST_ITEM_SELECTED(wxID_ANY, ACListViewAccount::OnLeftClicked)
//EVT_LIST_ITEM_ACTIVATED(wxID_ANY, ACListViewAccount::OnDoubleClicked)
//END_EVENT_TABLE()




/*---------------------------------------------------------------------------------
								ACComboBoxDrawText
-----------------------------------------------------------------------------------*/
ACComboBoxDrawText::ACComboBoxDrawText(wxWindow* parent,
	wxWindowID id,
	const wxString& value,
	const wxPoint& pos,
	const wxSize& size,
	const wxArrayString& choices,
	long style,
	const wxValidator& validator,
	const wxString& className) :
	wxComboBox(parent, id, value, pos, size, choices, style, validator, className)
{

}
void ACComboBoxDrawText::OnPaint(wxPaintEvent& event)
{
	wxPaintDC dc(this);

#ifdef _WIN32
    // 绘制文本
    wxComboBox::OnPaint(event);
#endif // _WIN32
	

	wxRect rc = GetClientRect();
	dc.SetBrush(*wxWHITE_BRUSH);
	dc.SetPen(*wxTRANSPARENT_PEN);
	rc.Deflate(1);//保留边框
	rc.width = rc.width - FromDIP(20);
	dc.DrawRectangle(rc);

	
	wxString text = GetValue();
	if (!text.IsEmpty())
	{
		wxCoord textWidth, textHeight;
		dc.GetTextExtent(text, &textWidth, &textHeight);

		//bitmap default 28px
		int textX = FromDIP(4);
		int textY = (GetSize().GetHeight() - textHeight) / 2;

		dc.DrawText(text, textX, textY);
	}

}

/*---------------------------------------------------------------------------------
								ACTextCtrlAccount
-----------------------------------------------------------------------------------*/
ACStaticDashLine::ACStaticDashLine(wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style, const wxString& name) :
	ACStaticBox(parent, id, pos, size, style, name)
{

}

void ACStaticDashLine::OnPaint(wxPaintEvent& event)
{
	wxPaintDC dc(this);

	wxRect rc = GetClientRect();
	dc.SetBrush(*wxWHITE_BRUSH);
	dc.SetPen(*wxTRANSPARENT_PEN);
	dc.Clear();
	//dc.DrawRectangle(rc);

	wxPen pen(wxColour(200, 200, 200), 2, wxPENSTYLE_SHORT_DASH);
	dc.SetPen(pen);

	if (!isHorizontal)
	{
		int y1 = 0;
		int y2 = GetSize().GetHeight();
		int x = GetSize().GetWidth() / 2;
		dc.DrawLine(x, y1, x, y2);
	}
	else
	{
		int x1 = 0;
		int x2 = GetSize().GetWidth();
		int y = GetSize().GetHeight() / 2;
		dc.DrawLine(x1, y, x2, y);
	}
}

/*---------------------------------------------------------------------------------
								ACButtonUnderline
-----------------------------------------------------------------------------------*/
ACButtonUnderline::ACButtonUnderline(wxWindow* parent, wxString text, wxString icon, wxString hover_icon, wxString dis_icon, long style, wxSize iconSize)
	: ACButton(parent, text, icon, hover_icon, dis_icon, style, iconSize)
{
	SetPaddingSize(wxSize(8, 8));
	SetSpacing(0);
	SetButtonType(ACButton::AC_BUTTON_LV3);
	SetChecked(true);
	SetCornerRadius(4);
}
void ACButtonUnderline::render(wxDC& dc)
{
	ACButton::render(dc);

	isSelect ? dc.SetPen(wxPen(wxColour(57, 134, 255), 2)) : dc.SetPen(wxPen(wxColour(195, 204, 217), 2));

	dc.SetBrush(wxNullBrush);
	//if (isSelect)
		dc.DrawLine(wxPoint(0, GetClientRect().GetHeight() - 1), wxPoint(GetClientRect().GetWidth(), GetClientRect().GetHeight() - 1));
}

/*---------------------------------------------------------------------------------
								ACTextCtrlIcon
-----------------------------------------------------------------------------------*/
ACTextCtrlIcon::ACTextCtrlIcon(wxWindow* parent,
	wxWindowID id,
	const std::string& name,
	const wxString& value,
	const wxPoint& pos,
	const wxSize& size,
	long style,
	const wxValidator& validator,
	const wxString& className) :
	wxTextCtrl(parent, id, value, pos, size, style, validator, className)
{
//	SetBackgroundStyle(wxBG_STYLE_TRANSPARENT);
//#if defined(__WXMSW__)
//	int extendedStyle = GetWindowLong(GetHWND(), GWL_EXSTYLE);
//	SetWindowLong(GetHWND(), GWL_EXSTYLE, extendedStyle | WS_EX_TRANSPARENT);
//#endif
	std::string filePath;
	if (boost::filesystem::exists(name + ".png"))
		filePath = name + ".png";
	else
		filePath = Slic3r::var(name + ".png");
	wxImage image(wxString::FromUTF8(filePath.c_str()), wxBITMAP_TYPE_ANY);
	if (image.IsOk())
		bitmap = wxBitmap(image);

	Bind(wxEVT_TEXT/*wxEVT_TEXT_ENTER*/, &ACTextCtrlIcon::OnText, this);
	Bind(wxEVT_SET_FOCUS , &ACTextCtrlIcon::OnFocus, this);
	//Bind(wxEVT_LEFT_DOWN/*wxEVT_TEXT_ENTER*/, &ACTextCtrlIcon::OnText1, this);
}
ACTextCtrlIcon::~ACTextCtrlIcon()
{

}
void ACTextCtrlIcon::OnPaint(wxPaintEvent& event)
{
	wxPaintDC dc(this);

	
#ifdef _WIN32
    // 绘制文本
    wxTextCtrl::OnPaint(event);
#endif // _WIN32

	// 绘制图标
	if (!bitmap.IsNull())
	{
		int iconWidth = bitmap.GetWidth();
		int iconHeight = bitmap.GetHeight();

		int textWidth, textHeight;
		GetTextExtent(GetValue(), &textWidth, &textHeight);

		int x = FromDIP(8);// 起始绘制位置
		int y = (GetSize().GetHeight() - iconHeight) / 2;

		dc.DrawBitmap(bitmap, x, y, true);
	}

}


/*---------------------------------------------------------------------------------
								ACTextCtrlAccount
-----------------------------------------------------------------------------------*/
ACTextCtrlAccount::ACTextCtrlAccount(wxWindow* parent,
	wxWindowID id,
	const wxString& value,
	const wxPoint& pos,
	const wxSize& size,
	long style,
	const wxValidator& validator,
	const wxString& className) :
	wxTextCtrl(parent, id, value, pos, size, style, validator, className),
	//buttonEmail			{ new wxButton(this, wxID_ANY, "", wxDefaultPosition, wxSize(16,16), wxBORDER_NONE) },
	button				{ new wxButton(this, wxID_ANY, "CN +86", wxDefaultPosition, wxSize(86,24), wxBU_LEFT | wxBORDER_NONE) },
	view				{ new ACListViewAccount(this, wxDefaultPosition, wxSize(150,300)) },
	hBox				{ new wxBoxSizer(wxHORIZONTAL) }
{
	Init();
	Connect();
	button->Bind(wxEVT_BUTTON, [this](wxCommandEvent&) { OnButtonClicked(); });
	
	//Bind(wxEVT_SET_FOCUS,  &ACTextCtrlAccount::OnFocus, this);
	//Bind(wxEVT_KILL_FOCUS, &ACTextCtrlAccount::OnKillFocus, this);
}
ACTextCtrlAccount::~ACTextCtrlAccount()
{
	//wxDELETE(buttonEmail);
	wxDELETE(button);
	wxDELETE(view);
}
void ACTextCtrlAccount::Connect()
{
	view->Bind(wxEVT_LIST_ITEM_SELECTED, [&](wxListEvent& event)
		{
			int selectItemIndex = event.GetIndex();
			wxString s = view->GetListCtrl()->GetItemText(selectItemIndex);
			button->SetLabel(s);
			view->Hide();
			event.Skip();
		});
	view->Bind(wxEVT_LIST_ITEM_ACTIVATED, [&](wxListEvent& event)
		{
			int selectItemIndex = event.GetIndex();
			wxString s = view->GetListCtrl()->GetItemText(selectItemIndex);
			button->SetLabel(s);
			view->Hide();
			event.Skip();
		});
	//view->Bind(wxEVT_SET_FOCUS, &ACTextCtrlIcon::OnFocus, this);
}
void ACTextCtrlAccount::Init()
{
	SetMaxLength(100);//

	wxCursor cursor(wxCURSOR_HAND);
	button->SetCursor(cursor);

	button->SetBackgroundColour(*wxWHITE);
	button->SetBitmap(*get_bmp_bundle("icon_angle_brackets", 16), wxRIGHT);
	button->SetBitmapMargins(0, 0);
	
	SetMinSize(FromDIP(wxSize(150, 28)));
	//buttonEmail->SetMinSize(FromDIP(wxSize(28, 28)));
	//buttonEmail->SetBackgroundColour(*wxWHITE);
	//buttonEmail->SetBitmap(*get_bmp_bundle("icon_cloud_account", 28));
	button->SetMinSize(FromDIP(wxSize(/*94*/86, 24)));
	button->SetFont(ACLabel::sysFont(13, false));
	SetSizer(hBox);//no fit


	//View
	view->SetMinSize(wxSize(GetSize().GetWidth(), 300));
	view->Show(false);
}
void ACTextCtrlAccount::SetModelType(const ModelType& m)
{
	model = m;
	if (model == ModelType::AC_EMAIL)
		SetHint(_L("Please enter email"));
	else
		SetHint(_L("Please enter phone number"));
	LayoutReset();
}
void ACTextCtrlAccount::LayoutReset()
{
	hBox->Clear();
	if (model == ModelType::AC_EMAIL)
	{
		//buttonEmail->Show(true);
		button->Show(false);
		//hBox->AddSpacer(FromDIP(4));
		//hBox->Add(buttonEmail, 0, wxALIGN_CENTER | wxALL, 0);
		hBox->AddStretchSpacer(1);
//#if defined(__WXMSW__)
//		::SendMessage((HWND)this->GetHandle(), EM_SETMARGINS,
//			EC_LEFTMARGIN | EC_RIGHTMARGIN,
//			MAKELONG(FromDIP(32), 0));
//#else
//		SetMargins(wxPoint(FromDIP(32), 1));
//#endif
		SetMargins(wxPoint(FromDIP(2), 1));
	}
	else
	{
		//buttonEmail->Show(true);
		button->Show(true);
		//hBox->AddSpacer(FromDIP(4));
		//hBox->Add(buttonEmail, 0, wxALIGN_CENTER | wxALL, 0);
		hBox->AddSpacer(FromDIP(4));
		hBox->Add(button, 0, wxALIGN_CENTER | wxALL, 0);
		hBox->AddStretchSpacer(1);
#if defined(__WXMSW__)
		::SendMessage((HWND)this->GetHandle(), EM_SETMARGINS,
			EC_LEFTMARGIN | EC_RIGHTMARGIN,
			MAKELONG(FromDIP(/*32 + */86 + 8), 0));
#else
		SetMargins(wxPoint(FromDIP(/*32 + */86 + 8), 20));
#endif
		
	}
	Layout();
}
void ACTextCtrlAccount::OnButtonClicked()
{
	//w->SetPosition(wxGetApp().mainframe->GetPosition() + wxPoint(x, y));
	view->SetPosition(this->GetScreenPosition() + wxPoint(-2, GetSize().GetHeight()));
	view->SetSize(wxSize(GetSize().GetWidth(), 300));
	view->Show();
}

/*---------------------------------------------------------------------------------
								ACListViewAccount
-----------------------------------------------------------------------------------*/
ACListViewAccount::ACListViewAccount(wxWindow* parent, const wxPoint& pos, const wxSize& size) :
	wxPopupTransientWindow(parent, wxBORDER_SIMPLE),
	l			{ new wxListCtrl(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxLC_REPORT |
		/*wxLC_HRULES |*/ wxLC_NO_HEADER | wxLC_SINGLE_SEL) },
	sizer		{ new wxBoxSizer(wxVERTICAL) }
{
	Init();
}
ACListViewAccount::~ACListViewAccount()
{
	wxDELETE(l);
}
void ACListViewAccount::Init()
{
	sizer->Add(l, 1, wxEXPAND | wxALL, 0);
	SetSizer(sizer);

	const int w = 300;
	l->InsertColumn(0, "regions", wxLIST_FORMAT_LEFT, w);
	for (int i = 1; i <= 20; i++)
	{
		wxString itemText = wxString::Format("CN +%d", i);
		wxListItem listItem;
		listItem.SetId(i - 1);
		listItem.SetText(itemText);
		l->InsertItem(listItem);
	}

	// 添加分隔符
	wxListItem separator;
	separator.SetId(20);
	separator.SetText("--------------Anycubic Slicer-------------");
	separator.SetState(wxLIST_STATE_DONTCARE);
	l->InsertItem(separator);
	//l->SetItemState(20, 0, wxLIST_STATE_DONTCARE);

	for (int i = 21; i <= 40; i++)
	{
		wxString itemText = wxString::Format("US +%d", i);
		wxListItem listItem;
		listItem.SetId(i);
		listItem.SetText(itemText);
		l->InsertItem(listItem);
	}
}
//void ACListViewAccount::OnLeftClicked(wxListEvent& event)
//{
//	Hide();
//	int selectItemIndex = event.GetIndex();
//	wxString s = l->GetItemText(selectItemIndex);
//	wxLogMessage("Left Clicked: %s", s);
//	event.Skip();
//}
//
//void ACListViewAccount::OnDoubleClicked(wxListEvent& event)
//{
//	Hide();
//	int selectItemIndex = event.GetIndex();
//	wxString s = l->GetItemText(selectItemIndex);
//	wxLogMessage("Double Clicked: %s", s);
//	event.Skip();
//}



/*---------------------------------------------------------------------------------
								ACTextCtrlPassword
-----------------------------------------------------------------------------------*/
ACTextCtrlPassword::ACTextCtrlPassword(wxWindow* parent,
	wxWindowID id,
	const wxString& value,
	const wxPoint& pos,
	const wxSize& size,
	long style,
	const wxValidator& validator,
	const wxString& className) :
	wxTextCtrl(parent, id, value, pos, size, style, validator, className),
	//buttonIcon	{ new wxButton(this, wxID_ANY, "", wxDefaultPosition, wxSize(16,16), wxBORDER_NONE) },
	button		{ new wxButton(this, wxID_ANY, "", wxDefaultPosition, wxSize(16,16), wxBU_LEFT | wxBORDER_NONE) },
	hBox		{ new wxBoxSizer(wxHORIZONTAL) }
{
	Init();
	Connect();
}
ACTextCtrlPassword::~ACTextCtrlPassword()
{
	//wxDELETE(buttonIcon);
	wxDELETE(button);
}
void ACTextCtrlPassword::Connect()
{
	button->Bind(wxEVT_BUTTON, [this](wxCommandEvent&) { OnButtonClicked(); });
}
void ACTextCtrlPassword::OnButtonClicked()
{
	isPasswordShow = !isPasswordShow;
	SetPasswordShow(isPasswordShow);
#if defined(__WXMSW__)
	HWND hWnd = (HWND)this->GetHandle();
	::SendMessage(hWnd, EM_SETPASSWORDCHAR, isPasswordShow ? 0 : 0x25cf, 0); // 0x25cf is ● character
	Refresh();
#endif
}
void ACTextCtrlPassword::SetPasswordShow(bool isShow)
{
	isPasswordShow = isShow;
	button->SetBitmap(isPasswordShow ? bitmapShow : bitmapHide);
}
void ACTextCtrlPassword::Init()
{
	//SetMaxLength(100);//
	wxBitmapBundle b = *get_bmp_bundle("icon_cloud_password_show", 28);
	bitmapShow = b.GetBitmap(wxSize(28, 28));
	b = *get_bmp_bundle("icon_cloud_password_hide", 28);
	bitmapHide = b.GetBitmap(wxSize(28, 28));

	wxCursor cursor(wxCURSOR_HAND);
	button->SetCursor(cursor);

	button->SetBackgroundColour(*wxWHITE);
	button->SetBitmap(isPasswordShow ? bitmapShow : bitmapHide);
	button->SetBitmapMargins(0, 0);
	button->SetMinSize(wxSize(28, 28));


	SetMinSize(wxSize(150, 28));
	
	//buttonIcon->SetBackgroundColour(*wxWHITE);
	//buttonIcon->SetBitmap(*get_bmp_bundle("icon_cloud_password", 28));
	//buttonIcon->SetBitmapMargins(0, 0);
	//buttonIcon->SetMinSize(wxSize(28, 28));



	//buttonIcon->Show(true);
	button->Show(true);
	//hBox->Add(buttonIcon, 0, wxALIGN_CENTER | wxALL, 0);
	hBox->AddStretchSpacer(1);
	hBox->Add(button, 0, wxALIGN_CENTER | wxALL, 0);

	SetSizer(hBox);//no fit
}





















/*---------------------------------------------------------------------------------
								ACButtonRoundUser
-----------------------------------------------------------------------------------*/



ACButtonRoundUser::ACButtonRoundUser(wxWindow* parent, wxWindowID id, const wxString& label, const wxPoint& pos, const wxSize& size,
	long style, const wxValidator& validator, const wxString& name) :
	wxButton(parent, id, label, pos, size, style, validator, name)
{
	wxFont font = ACLabel::sysFont(18, false);
	SetFont(font);
	SetBackgroundColour(wxColour(235, 237, 240));

	/*Bind(wxEVT_PAINT,      &ACButtonRoundUser::OnPaint,     this);
	Bind(wxEVT_SHOW,       &ACButtonRoundUser::OnShow,      this);
	Bind(wxEVT_SET_FOCUS,  &ACButtonRoundUser::OnSetFocus,  this);
	Bind(wxEVT_KILL_FOCUS, &ACButtonRoundUser::OnKillFocus, this);*/
}

void ACButtonRoundUser::OnPaint(wxPaintEvent& event)
{
	//wxPaintDC dc(this);

	//int centerX = GetRect().GetWidth() / 2;
	//int centerY = GetRect().GetHeight() / 2;
	//int radius = std::min(centerX, centerY);

	////draw circle
	//wxRect rc = GetClientRect();
	//dc.SetBrush(wxColour(57, 134, 255));
	//dc.SetPen(*wxTRANSPARENT_PEN);
	//dc.DrawCircle(centerX, centerY, radius);


	////draw text
	////wxString text("morecpp");
	//wxString text = GetLabel();
	//wxSize textSize = dc.GetTextExtent(text);
	//dc.SetTextForeground(*wxBLACK);
	//dc.DrawText(text, centerX - textSize.GetWidth() / 2, centerY - textSize.GetHeight() / 2);

	wxPaintDC dc(this);

	wxGraphicsContext* gc = wxGraphicsContext::Create(dc);
	if (gc)
	{
		wxSize buttonSize = GetClientSize();

		int centerX = buttonSize.GetWidth() / 2;
		int centerY = buttonSize.GetHeight() / 2;
		int radius = std::min(centerX, centerY);

		gc->SetBrush(wxColour(57, 134, 255));
		gc->SetPen(wxNullPen);
		gc->DrawEllipse(centerX - radius, centerY - radius, radius * 2, radius * 2);


		wxString text = GetLabel();
		
		wxSize textSize = dc.GetTextExtent(text);
		int textX = centerX - textSize.GetWidth() / 2;
		int textY = centerY - textSize.GetHeight() / 2 - 2;

		gc->SetFont(GetFont(), *wxWHITE);
		gc->DrawText(text, textX, textY);

		delete gc;
	}
}
void ACButtonRoundUser::OnShow(wxShowEvent& event)
{
	Refresh();
}
void ACButtonRoundUser::OnKillFocus(wxFocusEvent& event)
{
	Refresh();
}
void ACButtonRoundUser::OnSetFocus(wxFocusEvent& event)
{
	Refresh();
}


/*---------------------------------------------------------------------------------
								ACTransparentPanel
-----------------------------------------------------------------------------------*/
ACTransparentPanel::ACTransparentPanel(wxWindow* parent) :
	wxPanel(parent)
{
	SetBackgroundStyle(wxBG_STYLE_TRANSPARENT);
	//SetBackgroundColour(wxColour(0, 0, 0, 0));
	Bind(wxEVT_PAINT, &ACTransparentPanel::OnPaint, this);
	//Bind(wxEVT_ERASE_BACKGROUND, &ACTransparentPanel::OnEraseBackground, this);
#if defined(__WXMSW__)
	int extendedStyle = GetWindowLong(GetHWND(), GWL_EXSTYLE);
	SetWindowLong(GetHWND(), GWL_EXSTYLE, extendedStyle | WS_EX_TRANSPARENT);
#endif
}
//void ACTransparentPanel::OnEraseBackground(wxEraseEvent& event)
//{
//	//event.Skip();
//}
void ACTransparentPanel::OnPaint(wxPaintEvent& event)
{
	//wxPaintDC dc(this);
	double scale = GetDPIScaleFactor();
	wxBitmap bitmap;
	bitmap.CreateWithDIPSize(GetClientRect().GetSize() * GetContentScaleFactor(), scale, 32);
	bitmap.UseAlpha();
	
	wxMemoryDC dc(bitmap);
	dc.SetUserScale(scale, scale);

	std::unique_ptr<wxGraphicsContext> gc{ wxGraphicsContext::Create(dc) };
	if (gc)
	{
		DrawOnContext(*gc);
		//gc->DrawBitmap(bitmap);
		//gc->DrawRoundedRectangle(0, 0, GetSize().GetWidth(), GetSize().GetHeight(), 6);
	}

	//wxGCDC gcdc(dc);
	//render(gcdc);
}
void ACTransparentPanel::DrawOnContext(wxGraphicsContext& gc)
{
	wxColor color{ "#EA3B2D" };

	gc.SetBrush(wxBrush(color));

	auto buttonRect = this->GetClientRect();

	gc.DrawRoundedRectangle(buttonRect.GetLeft(),
		buttonRect.GetTop(),
		buttonRect.GetWidth(),
		buttonRect.GetHeight(),
		buttonRect.GetHeight() / 6);

	wxFont font(wxFontInfo({ 0, buttonRect.GetHeight() / 2 }).FaceName("Arial"));

	gc.SetFont(font, *wxWHITE);

	//double textWidth, textHeight;
	//gc.GetTextExtent(this->text, &textWidth, &textHeight);

	/*gc.Clip(buttonRect.GetLeft(),
		buttonRect.GetTop(),
		buttonRect.GetWidth(),
		buttonRect.GetHeight());*/
	
	gc.SetPen(wxPen(wxColour(20, 28, 41), 1));
	gc.SetBrush(wxBrush(wxColour(255, 255, 255, 230)));
	gc.DrawRoundedRectangle(0, 0, GetRect().GetWidth(), GetRect().GetHeight(), 14);

	/*gc.DrawText(this->text,
		(buttonRect.GetWidth() - textWidth) / 2.0,
		(buttonRect.GetHeight() - textHeight) / 2.0);*/
}
void ACTransparentPanel::render(wxDC& dc)
{
	dc.SetBackground(wxBrush(wxColour(0, 0, 0, 1)));
	dc.Clear();
	//dc.SetPen(wxNullPen);
	//dc.SetBrush(wxBrush(wxColour(0, 0, 0, 0)));
	//dc.DrawRectangle(GetRect());


	dc.SetPen(wxPen(wxColour(20, 28, 41), 1));
	dc.SetBrush(wxBrush(wxColour(255, 255, 255, 230)));
	dc.DrawRoundedRectangle(wxPoint(0, 0), wxSize(GetSize()), FromDIP(14));
}

/*---------------------------------------------------------------------------------
								ACBackgroundRounded
-----------------------------------------------------------------------------------*/
ACBackgroundRounded::ACBackgroundRounded(wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style, const wxString& name) : wxWindow()
{
	SetBackgroundStyle(wxBG_STYLE_TRANSPARENT);
	wxWindow::Create(parent, id, pos, size, style, name);

#if defined(__WXMSW__)
	int extendedStyle = GetWindowLong(GetHWND(), GWL_EXSTYLE);
	SetWindowLong(GetHWND(), GWL_EXSTYLE, extendedStyle | WS_EX_TRANSPARENT);
#endif

	Bind(wxEVT_PAINT, &ACBackgroundRounded::OnPaint, this);
}
ACBackgroundRounded::~ACBackgroundRounded()
{

}
void ACBackgroundRounded::OnPaint(wxPaintEvent& event)
{
	wxPaintDC dc(this);

	std::unique_ptr<wxGraphicsContext> gc{ wxGraphicsContext::Create(dc) };

	if (gc)
	{
		gc->SetBrush(wxBrush(wxColour(255, 255, 255, 255/*230*/)));
		gc->SetPen(wxNullPen);
		gc->DrawRoundedRectangle(0, 0, GetRect().GetWidth(), GetRect().GetHeight(), FromDIP(8));
	}
}

/*---------------------------------------------------------------------------------
								ACButtonTransparent
-----------------------------------------------------------------------------------*/
ACButtonTransparent::ACButtonTransparent(wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style, const wxString& name)
	: wxWindow()
{
	SetBackgroundStyle(wxBG_STYLE_TRANSPARENT);
	wxWindow::Create(parent, id, pos, size, style, name);

#if defined(__WXMSW__)
	int extendedStyle = GetWindowLong(GetHWND(), GWL_EXSTYLE);
	SetWindowLong(GetHWND(), GWL_EXSTYLE, extendedStyle | WS_EX_TRANSPARENT);
#endif

	wxInitAllImageHandlers();
	wxBitmapBundle b = *get_bmp_bundle("icon-close-full-normal", FromDIP(50));
	iconNormal = b.GetBitmap(FromDIP(wxSize(50, 50)));
	wxBitmapBundle c = *get_bmp_bundle("icon-close-full-hover", FromDIP(50));
	iconHover = c.GetBitmap(FromDIP(wxSize(50, 50)));

	Bind(wxEVT_PAINT,        &ACButtonTransparent::OnPaint,     this);
	Bind(wxEVT_ENTER_WINDOW, &ACButtonTransparent::OnEnter,     this);
	Bind(wxEVT_LEAVE_WINDOW, &ACButtonTransparent::OnLeave,     this);
	Bind(wxEVT_LEFT_DOWN,    &ACButtonTransparent::OnMouseDown, this);
	Bind(wxEVT_LEFT_UP,      &ACButtonTransparent::OnMouseUp,   this);
	Bind(wxEVT_DPI_CHANGED,  &ACButtonTransparent::OnDPI,       this);

}
void ACButtonTransparent::SetButtonState(ButtonState bs)
{
	if (buttonState != bs)
	{
		buttonState = bs;
		Refresh();
	}
}
void ACButtonTransparent::OnEnter(wxMouseEvent& event)
{
	isMouseInside = true;
	SetButtonState(isMousePressed ? ButtonState::Pressed : ButtonState::Hover);
	event.Skip();
}
void ACButtonTransparent::OnLeave(wxMouseEvent& event)
{
	isMouseInside = false;
	SetButtonState(ButtonState::Normal);
	Refresh();
	event.Skip();
}
void ACButtonTransparent::OnMouseDown(wxMouseEvent& event)
{
	isMousePressed = true;
	SetButtonState(ButtonState::Pressed);
	event.Skip();
}
void ACButtonTransparent::OnMouseUp(wxMouseEvent& event)
{
	isMousePressed = false;
	SetButtonState(isMouseInside ? ButtonState::Hover : ButtonState::Normal);
	event.Skip();

	//发生关闭信号
	wxCommandEvent* evt = new wxCommandEvent(EVT_ACLOUD_LOGIN_BUTTON_CLOSE);
	ProcessEvent(*evt);
}
void ACButtonTransparent::OnPaint(wxPaintEvent& event)
{
	wxPaintDC dc(this);

	std::unique_ptr<wxGraphicsContext> gc{ wxGraphicsContext::Create(dc) };

	if (gc)
	{
		gc->SetBrush(wxNullBrush);
		gc->SetPen(wxNullPen);
		gc->DrawBitmap(buttonState == ButtonState::Normal ? iconNormal : iconHover, 0, 0, GetRect().GetWidth(), GetRect().GetHeight());
	}
}
void ACButtonTransparent::OnDPI(wxDPIChangedEvent& event)
{
	wxBitmapBundle b = *get_bmp_bundle("icon-close-full-normal", FromDIP(50));
	iconNormal = b.GetBitmap(FromDIP(wxSize(50, 50)));
	wxBitmapBundle c = *get_bmp_bundle("icon-close-full-hover", FromDIP(50));
	iconHover = c.GetBitmap(FromDIP(wxSize(50, 50)));
	Refresh();
}

//以下为未使用

/*---------------------------------------------------------------------------------
								ACButtonUnderline
-----------------------------------------------------------------------------------*/
static int acPushButtonCounter = 0;

wxButtonUnderline::~wxButtonUnderline()
{
	printf("ACButton Desdroyed.... %d \n", --acPushButtonCounter);
}


wxButtonUnderline::wxButtonUnderline(wxWindow* parent, wxWindowID id, const wxString& label, const wxPoint& pos, const wxSize& size,
	long style, const wxValidator& validator, const wxString& name) :
	wxButton(parent, id, label, pos, size, style, validator, name)
{
	acPushButtonCounter++;

	bgColor.push_back(AC_COLOR_BT_L1_BG_DIS);
	bgColor.push_back(AC_COLOR_BT_L1_BG_PRE);
	bgColor.push_back(AC_COLOR_BT_L1_BG_HOV);
	bgColor.push_back(AC_COLOR_BT_L1_BG_NOR);

	bdColor.push_back(AC_COLOR_BT_L1_BG_DIS);
	bdColor.push_back(AC_COLOR_BT_L1_BG_PRE);
	bdColor.push_back(AC_COLOR_BT_L1_BG_HOV);
	bdColor.push_back(AC_COLOR_BT_L1_BG_NOR);

	fgColor.push_back(AC_COLOR_BT_L1_BG_DIS);
	fgColor.push_back(AC_COLOR_BT_L1_BG_PRE);
	fgColor.push_back(AC_COLOR_BT_L1_BG_HOV);
	fgColor.push_back(AC_COLOR_BT_L1_BG_NOR);

	Bind(wxEVT_ENTER_WINDOW, &wxButtonUnderline::OnEnter,     this);
	Bind(wxEVT_LEAVE_WINDOW, &wxButtonUnderline::OnLeave,     this);
	Bind(wxEVT_LEFT_DOWN,    &wxButtonUnderline::OnMouseDown, this);
	Bind(wxEVT_LEFT_UP,      &wxButtonUnderline::OnMouseUp,   this);
	Bind(wxEVT_PAINT,        &wxButtonUnderline::OnPaint,     this);
}

void wxButtonUnderline::SetButtonState(ButtonState bs)
{
	if (buttonState != bs)
	{
		buttonState = bs;
		Refresh();
	}
}

void wxButtonUnderline::OnEnter(wxMouseEvent& event)
{
	isMouseInside = true;
	SetButtonState(isMousePressed ? ButtonState::Pressed : ButtonState::Hover);
	Refresh();
	event.Skip();
}
void wxButtonUnderline::OnLeave(wxMouseEvent& event)
{
	isMouseInside = false;
	SetButtonState(ButtonState::Normal);
	Refresh();
	event.Skip();
}
void wxButtonUnderline::OnMouseDown(wxMouseEvent& event)
{
	isMousePressed = true;
	SetButtonState(ButtonState::Pressed);
	Refresh();
	event.Skip();
}
void wxButtonUnderline::OnMouseUp(wxMouseEvent& event)
{
	isMousePressed = false;
	SetButtonState(isMouseInside ? ButtonState::Hover : ButtonState::Normal);
	Refresh();
	event.Skip();
}
void wxButtonUnderline::OnPaint(wxPaintEvent& event)
{
	wxAutoBufferedPaintDC dc(this);
	DrawButton(dc);
}
void wxButtonUnderline::DrawButton(wxDC& dc)
{
	wxSize buttonSize = GetClientSize();

	wxColour bg_color;
	wxColour bd_color;
	wxColour fg_color;
	//Disable Pressed Hover Normal
	switch (buttonState)
	{
	case ButtonState::Normal:
		bg_color = bgColor[3];
		bd_color = bdColor[3];
		fg_color = fgColor[3];
		break;
	case ButtonState::Hover:
		bg_color = bgColor[2];
		bd_color = bdColor[2];
		fg_color = fgColor[2];
		break;
	case ButtonState::Pressed:
		bg_color = bgColor[1];
		bd_color = bdColor[1];
		fg_color = fgColor[1];
		break;
	case ButtonState::Disabled:
		bg_color = bgColor[0];
		bd_color = bdColor[0];
		fg_color = fgColor[0];
		break;
	}

	dc.SetBackground(*wxWHITE_BRUSH);
	dc.Clear();
	dc.SetBrush(wxBrush(bg_color));
	dc.SetPen(wxNullPen);
	dc.DrawRectangle(wxPoint(0, 0), buttonSize);
	dc.SetPen(wxPen(bd_color, 2));
	dc.SetBrush(wxNullBrush);
	dc.DrawLine(wxPoint(0, GetClientRect().GetHeight() - 2), wxPoint(GetClientRect().GetWidth(), GetClientRect().GetHeight() - 2));


	dc.SetTextForeground(wxColor(55, 55, 55));
	dc.DrawLabel(GetLabel(), GetBitmap(), GetClientRect(), wxALIGN_CENTER | wxALIGN_CENTER_VERTICAL);
}

/*---------------------------------------------------------------------------------
								ACComboBoxIcon
-----------------------------------------------------------------------------------*/
ACComboBoxIcon::ACComboBoxIcon(wxWindow* parent,
	wxWindowID id,
	const std::string& name,
	const wxString& value,
	const wxPoint& pos,
	const wxSize& size,
	const wxArrayString& choices,
	long style,
	const wxValidator& validator,
	const wxString& className) :
	wxComboBox(parent, id, value, pos, size, choices, style, validator, className)
{
	/*std::string filePath;
	if (boost::filesystem::exists(name + ".png"))
		filePath = name + ".png";
	else
		filePath = Slic3r::var(name + ".png");
	wxImage image(wxString::FromUTF8(filePath.c_str()), wxBITMAP_TYPE_ANY);
	if (image.IsOk())
		bitmap = wxBitmap(image);*/
	wxBitmapBundle bl = *get_bmp_bundle(name, FromDIP(28));
	bitmap = bl.GetBitmap(FromDIP(wxSize(28, 28)));
}
void ACComboBoxIcon::OnPaint(wxPaintEvent& event)
{
	wxPaintDC dc(this);

#ifdef _WIN32
    // 绘制文本
    wxComboBox::OnPaint(event);
#endif // _WIN32

	wxRect rc = GetClientRect();
	dc.SetBrush(*wxWHITE_BRUSH);
	dc.SetPen(*wxTRANSPARENT_PEN);
	rc.Deflate(1);//保留边框
	rc.width = rc.width - FromDIP(20);
	dc.DrawRectangle(rc);

	// 绘制图标
	if (!bitmap.IsNull())
	{
		int iconWidth = bitmap.GetWidth();
		int iconHeight = bitmap.GetHeight();

		int textWidth, textHeight;
		GetTextExtent(GetValue(), &textWidth, &textHeight);

		int x = 0/*FromDIP(8)*/;// 起始绘制位置
		int y = (GetSize().GetHeight() - iconHeight) / 2;

		dc.DrawBitmap(bitmap, x, y, true);
	}
	wxString text = GetValue();
	if (!text.IsEmpty())
	{
		wxCoord textWidth, textHeight;
		dc.GetTextExtent(text, &textWidth, &textHeight);

		//bitmap default 28px
		int textX = FromDIP(4) + FromDIP(bitmap.GetWidth()); // 适当的间距
		int textY = (GetSize().GetHeight() - textHeight) / 2;

		dc.DrawText(text, textX, textY);
	}

}
//void ACButtonUnderline::SetButtonType(ACPushButton::AC_BUTTON_TYPE type)
//{
//	ACStateColor bgColor;
//	ACStateColor bdColor;
//	ACStateColor fgColor;
//
//	switch (type)
//	{
//	case ACButton::AC_BUTTON_LV0:
//		bgColor.append(AC_COLOR_BT_L0_BG_DIS, ACStateColor::Disabled);
//		bgColor.append(AC_COLOR_BT_L0_BG_PRE, ACStateColor::Pressed);
//		bgColor.append(AC_COLOR_BT_L0_BG_HOV, ACStateColor::Hovered);
//		bgColor.append(AC_COLOR_BT_L0_BG_NOR, ACStateColor::Normal);
//
//		fgColor.append(AC_COLOR_BT_L0_FG_DIS, ACStateColor::Disabled);
//		fgColor.append(AC_COLOR_BT_L0_FG_PRE, ACStateColor::Pressed);
//		fgColor.append(AC_COLOR_BT_L0_FG_HOV, ACStateColor::Hovered);
//		fgColor.append(AC_COLOR_BT_L0_FG_NOR, ACStateColor::Normal);
//		break;
//	case ACButton::AC_BUTTON_LV1:
//		bgColor.append(AC_COLOR_BT_L1_BG_DIS, ACStateColor::Disabled);
//		bgColor.append(AC_COLOR_BT_L1_BG_PRE, ACStateColor::Pressed);
//		bgColor.append(AC_COLOR_BT_L1_BG_HOV, ACStateColor::Hovered);
//		bgColor.append(AC_COLOR_BT_L1_BG_NOR, ACStateColor::Normal);
//
//		bdColor.append(AC_COLOR_BT_L1_BD_DIS, ACStateColor::Disabled);
//		bdColor.append(AC_COLOR_BT_L1_BD_PRE, ACStateColor::Pressed);
//		bdColor.append(AC_COLOR_BT_L1_BD_HOV, ACStateColor::Hovered);
//		bdColor.append(AC_COLOR_BT_L1_BD_NOR, ACStateColor::Normal);
//
//		fgColor.append(AC_COLOR_BT_L1_FG_DIS, ACStateColor::Disabled);
//		fgColor.append(AC_COLOR_BT_L1_FG_PRE, ACStateColor::Pressed);
//		fgColor.append(AC_COLOR_BT_L1_FG_HOV, ACStateColor::Hovered);
//		fgColor.append(AC_COLOR_BT_L1_FG_NOR, ACStateColor::Normal);
//		break;
//	case ACButton::AC_BUTTON_LV2:
//		bgColor.append(AC_COLOR_BT_L2_BG_DIS, ACStateColor::Disabled);
//		bgColor.append(AC_COLOR_BT_L2_BG_PRE, ACStateColor::Pressed);
//		bgColor.append(AC_COLOR_BT_L2_BG_HOV, ACStateColor::Hovered);
//		bgColor.append(AC_COLOR_BT_L2_BG_NOR, ACStateColor::Normal);
//		bdColor.append(AC_COLOR_BT_L2_BD_DIS, ACStateColor::Disabled);
//		bdColor.append(AC_COLOR_BT_L2_BD_PRE, ACStateColor::Pressed);
//		bdColor.append(AC_COLOR_BT_L2_BD_HOV, ACStateColor::Hovered);
//		bdColor.append(AC_COLOR_BT_L2_BD_NOR, ACStateColor::Normal);
//		fgColor.append(AC_COLOR_BT_L2_FG_DIS, ACStateColor::Disabled);
//		fgColor.append(AC_COLOR_BT_L2_FG_PRE, ACStateColor::Pressed);
//		fgColor.append(AC_COLOR_BT_L2_FG_HOV, ACStateColor::Hovered);
//		fgColor.append(AC_COLOR_BT_L2_FG_NOR, ACStateColor::Normal);
//		break;
//	case ACButton::AC_BUTTON_LV3:
//		bgColor.append(AC_COLOR_BT_L3_BG_DIS, ACStateColor::Disabled);
//		bgColor.append(AC_COLOR_BT_L3_BG_PRE, ACStateColor::Pressed);
//		bgColor.append(AC_COLOR_BT_L3_BG_HOV, ACStateColor::Hovered);
//		bgColor.append(AC_COLOR_BT_L3_BG_NOR, ACStateColor::Normal);
//		fgColor.append(AC_COLOR_BT_L3_FG_DIS, ACStateColor::Disabled);
//		fgColor.append(AC_COLOR_BT_L3_FG_PRE, ACStateColor::Pressed);
//		fgColor.append(AC_COLOR_BT_L3_FG_HOV, ACStateColor::Hovered);
//		fgColor.append(AC_COLOR_BT_L3_FG_NOR, ACStateColor::Normal);
//		break;
//	case ACButton::AC_BUTTON_LABEL:
//		bgColor.append(AC_COLOR_WHITE, ACStateColor::Normal);
//		fgColor.append(AC_COLOR_BLACK, ACStateColor::Normal);
//		break;
//	default:
//		break;
//	}
//	SetBackgroundColor(bgColor);
//	SetBorderColor(bdColor);
//	SetTextColor(fgColor);
//
//	Refresh();
//}
//
//void ACButtonUnderline::SetTextColor(ACStateColor const& color)
//{
//	fgColor = color;
//	//state_handler.update_binds();
//	Refresh();
//}
//
//void ACButtonUnderline::clearColor()
//{
//	bgColor.clear();
//	bdColor.clear();
//	fgColor.clear();
//
//	//state_handler.update_binds();
//	Refresh();
//}


} // GUI
} // Slic3r
