#include "ACPrinterContainer.hpp"
#include "ACCloudManger.hpp"

#include "../GUI.hpp"
#include "../GUI_App.hpp"
#include "../GUI_Utils.hpp"
#include "../BitmapCache.hpp"
#include "libslic3r/Utils.hpp"
#include "../ACLabel.hpp"

#include <wx/wx.h>
#include <wx/sizer.h>
#include <wx/panel.h>
#include <wx/font.h>
#include <wx/dcgraph.h>

#include <string>
#include <stdio.h>
#include "../MainFrame.hpp"

//#define GetEditHwnd() ((HWND)(GetEditHWND()))

namespace Slic3r {

namespace GUI {

wxDEFINE_EVENT(EVT_ACLOUD_PRINTER_DELETE, wxCommandEvent);
wxDEFINE_EVENT(EVT_ACLOUD_PRINTER_CHECK_CLICK, wxCommandEvent);
wxDEFINE_EVENT(EVT_ACLOUD_PRINTER_RENAME, wxCommandEvent);
wxDEFINE_EVENT(EVT_OPERAT_PRINTER_RENAME, wxCommandEvent);

/*---------------------------------------------------------------------------------
								ACPrinterContainer
-----------------------------------------------------------------------------------*/
ACPrinterContainer::ACPrinterContainer(wxWindow* parent/*, const std::vector<ACPrinterMeta*>& pm_*/,std::vector<PrinterData> &info,bool checkIndex) :
	wxScrolledWindow(parent, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxVSCROLL),
	//pm				{ pm_ },
    flexSizer{new wxFlexGridSizer(4, 20, 20)}
    , m_info(info)
    , m_checkIndex(checkIndex)
{
	Init();
	Connect();
}
ACPrinterContainer::~ACPrinterContainer()
{
	ReleasePrinterMeta();
}
void ACPrinterContainer::Connect()
{
	Bind(wxEVT_SIZE,                &ACPrinterContainer::OnSize,   this);
	Bind(EVT_ACLOUD_PRINTER_DELETE, &ACPrinterContainer::OnDelete, this);
	Bind(EVT_ACLOUD_PRINTER_RENAME, &ACPrinterContainer::OnRename, this);
}
void ACPrinterContainer::Init()
{
	flexSizer->SetFlexibleDirection(wxVERTICAL | wxHORIZONTAL);
    InitPrinter(m_info);
	SetScrollRate(0, 50);
	SetBackgroundColour(*wxWHITE);

	SetSizer(flexSizer);
	Layout();
	Refresh();
}

std::vector<PrinterData> ACPrinterContainer::GetCloudInfo() {
    std::vector<PrinterData> m_info;
    PrinterData data;
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

void ACPrinterContainer::ResetWindowEvent(std::vector<PrinterData>& printerInfo) {
	InitPrinter(printerInfo);
    ReLayout();
}

void ACPrinterContainer::CheckClickSingleEvent(PrinterData &printerData) 
{


}


void ACPrinterContainer::InitPrinter(std::vector<PrinterData> &cloudInfo)
{
    flexSizer->Clear();
    pm.clear();
	for (int i = 0; i < cloudInfo.size(); i++)
	{
        auto p = new ACPrinterMeta(this, cloudInfo[i]);
		if (p)
		{
			pm.push_back(p);
            if (1 != i && m_checkIndex)
				p->SetChecked(true);
		}
		flexSizer->Add(pm[i], 0, wxALL, 0);
	}
}
void ACPrinterContainer::ReleasePrinterMeta()
{
	//分离云打印机界面 并析构
	wxSizerItemList& children = flexSizer->GetChildren();
	for (wxSizerItem* childItem : children)
	{
		if (childItem->IsWindow())
		{
			wxWindow* w = childItem->GetWindow();
			ACPrinterMeta* c = static_cast<ACPrinterMeta*>(w);
			if (c)
			{
				flexSizer->Detach(c);
				c->Hide();
				delete c;
				c = nullptr;
			}
		}
	}
	pm.clear();
	flexSizer->Clear();//清空布局
}
void ACPrinterContainer::SetPrinters(const std::vector<PrinterData>& printerList)
{
	ReleasePrinterMeta();
	m_info.clear();//清除操作

	m_info = printerList;
	for (int i = 0; i < printerList.size(); i++)
	{
		auto p = new ACPrinterMeta(this, printerList[i]);
		if (p)
			pm.push_back(p);
		flexSizer->Add(pm[i], 0, wxALL, 0);
	}

	Layout();
	Refresh();
}
void ACPrinterContainer::ReLayout()
{
	int min = FromDIP(354 + 20);
	int columns = (GetRect().width - FromDIP(10)) / min;
	if (columns < 1)
		return;
	if (columns != flexSizer->GetCols())
	{
		/*wxSizerItemList& children = flexSizer->GetChildren();
		for (wxSizerItem* childItem : children)
		{
			ACPrinterMeta* child = static_cast<ACPrinterMeta*>(childItem->GetUserData());
			if (child)
			{
				flexSizer->Detach(child);
				child->Hide();
			}
		}
		flexSizer->SetCols(columns);
		for (wxSizerItem* childItem : children)
		{
			ACPrinterMeta* child = static_cast<ACPrinterMeta*>(childItem->GetUserData());
			if (child)
			{
				flexSizer->Add(child, 0, wxALL, 0);
				child->Show();
			}
		}*/
		flexSizer->SetCols(columns);
		flexSizer->Layout();
	}
	Refresh();
}
void ACPrinterContainer::OnSize(wxSizeEvent& event)
{
	ReLayout();
	Layout();
	event.Skip();
}
void ACPrinterContainer::OnDelete(wxCommandEvent& event)
{
	int id = event.GetInt();
	if (id >= 0)
		DeletePrinter(id);
}
void ACPrinterContainer::OnRename(wxCommandEvent& event)
{
	int id = event.GetInt();
	wxString name = event.GetString();
    int             result = 0;
	switch (Cloud->RenamePrinter(uint32_t(id), name.utf8_str().data()))
	{
	case E_FALSE://执行失败
		break;
	case E_TRUE://执行成功
		Cloud->PrinterList();//获取打印机列表
        result = 1;
		break;
	case E_TIMEOUT://超时
		break;
	default:
		break;
	}
    if (wxGetApp().mainframe->GetACCloudSelectMachineObj() != nullptr) {
        wxCommandEvent *op_evt = new wxCommandEvent(EVT_OPERAT_PRINTER_RENAME);
        op_evt->SetInt(result);
        op_evt->SetString("Rename");
        wxPostEvent(wxGetApp().mainframe->GetACCloudSelectMachineObj(), *op_evt);
    }
}
void ACPrinterContainer::AddPrinterSingle(PrinterData& d)
{
    d.id = m_info.size();
    m_info.push_back(d);
	auto p = new ACPrinterMeta(this, d);
	p->SetBackgroundColour(wxColour(240, 240, 240));
	if (p)
	{
		pm.push_back(p);
		p->SetChecked(d.isChecked);
		flexSizer->Add(p, 0, wxALL, 0);
		flexSizer->Layout();
	}
	ReLayout();
}
void ACPrinterContainer::AddPrinterMultiples(std::vector<PrinterData>& d)
{

}
void ACPrinterContainer::DeletePrinter(int id)
{
    
    int             result = 0;
	switch (Cloud->DeletePrinter(uint32_t(id)))
	{
	case E_FALSE://执行失败
		break;
	case E_TRUE://执行成功
		Cloud->PrinterList();//执行刷新操作
        result = 1;
		break;
	case E_TIMEOUT://超时
		break;
	default:
		break;
	}
    if (wxGetApp().mainframe->GetACCloudSelectMachineObj() != nullptr) {
        wxCommandEvent *op_evt = new wxCommandEvent(EVT_OPERAT_PRINTER_RENAME);
        op_evt->SetInt(result);
        op_evt->SetString("Del");
        wxPostEvent(wxGetApp().mainframe->GetACCloudSelectMachineObj(), *op_evt);
    }

	//if (pm.size() < 1)
	//	return;
	//else
	//{
	//	if (id >= 0 && id < pm.size())
	//	{
	//		wxSizerItemList& children = flexSizer->GetChildren();
	//		for (wxSizerItem* childItem : children)
	//		{
	//			if (childItem->IsWindow())
	//			{
	//				wxWindow* w = childItem->GetWindow();
	//				ACPrinterMeta* c = static_cast<ACPrinterMeta*>(w);
	//				if (c && id == c->data.id)
	//				{
	//					flexSizer->Detach(c);
	//					c->Hide();
	//					delete c;
	//					c = nullptr;
	//				}
	//			}
	//			else
	//			{
	//				ACPrinterMeta* child = static_cast<ACPrinterMeta*>(childItem->GetUserData());
	//				if (child && id == child->data.id)
	//				{
	//					//wxLogMessage("find children");
	//					flexSizer->Detach(child);
	//					child->Hide();
	//					delete child;
	//					child = nullptr;
	//				}
	//			}
	//		}
	//		pm.erase(pm.cbegin() + id);
	//		m_info.erase(m_info.cbegin() + id);
	//	}
	//	for (int i = 0; i < pm.size(); i++)
	//	{
	//		m_info[i].id   = i;
	//		pm[i]->data.id = i;
	//	}
	//	ReLayout();
	//	Layout();
	//}
}


/*---------------------------------------------------------------------------------
								ACPrinterMeta
-----------------------------------------------------------------------------------*/
ACPrinterMeta::ACPrinterMeta(wxWindow* parent, const PrinterData& d, const wxString& label, 
	const std::string& model, const std::string& variant) :
	ACStaticBox(parent, wxID_ANY, wxDefaultPosition, wxDefaultSize, 0L, ""),
	//button { new wxButton(this, wxID_ANY, "...", wxDefaultPosition, wxDefaultSize, wxBU_BOTTOM | wxNO_BORDER)}
	button			{ new wxBitmapButton(this, wxID_ANY, *get_bmp_bundle(std::string("icon-etc"), FromDIP(24)), wxDefaultPosition, wxDefaultSize, wxNO_BORDER) },
	popup			{ new wxPopupTransientWindow(this) },
	buttonRename	{ new ACButton(popup, _L("Rename")) },
	buttonDelete	{ new ACButton(popup, _L("Delete")) },
	data			{ d },
	//buttonRename	{ new wxButton(popup, wxID_ANY, _L("Rename"), wxDefaultPosition, wxSize(80, 24), wxBU_LEFT | wxBORDER_NONE) },
	//buttonDelete	{ new wxButton(popup, wxID_ANY, _L("Delete"), wxDefaultPosition, wxSize(80, 24), wxBU_LEFT | wxBORDER_NONE) },
	model			{ model },
	variant			{ variant }
{
	Init();
	Connect();
}
ACPrinterMeta::~ACPrinterMeta()
{
	wxDELETE(button);
	wxDELETE(buttonRename);
	wxDELETE(buttonDelete);
	wxDELETE(popup);
	
}
void ACPrinterMeta::Connect()
{
	Bind(wxEVT_ENTER_WINDOW, &ACPrinterMeta::OnEnter,      this);
	Bind(wxEVT_LEAVE_WINDOW, &ACPrinterMeta::OnLeave,      this);
	Bind(wxEVT_LEFT_DOWN,    &ACPrinterMeta::OnMouseDown,  this);
	Bind(wxEVT_LEFT_UP,      &ACPrinterMeta::OnMouseUp,    this);
	Bind(wxEVT_PAINT,        &ACPrinterMeta::OnPaint,      this);
	Bind(wxEVT_DPI_CHANGED,  &ACPrinterMeta::OnDPIChanged, this);

	button      ->Bind(wxEVT_BUTTON, &ACPrinterMeta::OnButton,       this);
	buttonRename->Bind(wxEVT_BUTTON, &ACPrinterMeta::OnButtonRename, this);
	buttonDelete->Bind(wxEVT_BUTTON, &ACPrinterMeta::OnButtonDelete, this);
}
void ACPrinterMeta::Init()
{
	SetBackgroundColour(wxColour(240, 240, 240));

	//popup window
	wxCursor cursor(wxCURSOR_HAND);
	buttonRename->SetCursor(cursor);
	buttonDelete->SetCursor(cursor);
	button->SetCursor(cursor);

	wxBitmapBundle bl = *get_bmp_bundle(std::string("icon-etc-hover"), FromDIP(24));
	button->SetBitmapHover(bl.GetBitmap(FromDIP(wxSize(24, 24))));

	buttonRename->SetButtonType(ACButton::AC_BUTTON_TYPE::AC_BUTTON_LV3);
	buttonRename->SetCornerRadius(0);
	buttonRename->SetPaddingSize(wxSize(6, 0));
	buttonRename->SetSpacing(0);
	buttonRename->SetMinSize(FromDIP(wxSize(100, 24)));

	buttonDelete->SetButtonType(ACButton::AC_BUTTON_TYPE::AC_BUTTON_LV3);
	buttonDelete->SetCornerRadius(0);
	buttonDelete->SetPaddingSize(wxSize(6, 0));
	buttonDelete->SetSpacing(0);
	buttonDelete->SetMinSize(FromDIP(wxSize(100, 24)));

	ACStateColor bgColor;
	bgColor.append(wxColour(229, 229, 229), ACStateColor::Hovered);
	bgColor.append(wxColour(255, 255, 255), ACStateColor::Normal);
	buttonRename->SetBackgroundColor(bgColor);
	buttonDelete->SetBackgroundColor(bgColor);
	buttonRename->SetAlignCenter(false);
	buttonDelete->SetAlignCenter(false);
	buttonRename->SetTextColorNormal(wxColour(77, 77, 77));
	buttonDelete->SetTextColorNormal(wxColour(77, 77, 77));

	wxFont font = ACLabel::sysFont(12, false);
	buttonRename->SetFont(font);
	buttonDelete->SetFont(font);

	wxBoxSizer* vBox = new wxBoxSizer(wxVERTICAL);
	vBox->AddSpacer(FromDIP(6));
	vBox->Add(buttonRename, 0, wxEXPAND | wxLEFT | wxRIGHT, 1);
	vBox->Add(buttonDelete, 0, wxEXPAND | wxLEFT | wxRIGHT, 1);
	vBox->AddSpacer(FromDIP(6));
	popup->SetSizer(vBox);
	popup->SetMinSize(FromDIP(wxSize(120, 60)));
	popup->Layout();
	AddWindowDrakEdg(popup);
	popup->Hide();

	/*if (data.fileName.empty())
		data.fileName = std::string("Kobra2_thumbnail");
	data.fileName = data.fileName + ".png";
	const fs::path rsrc_dir_path = (fs::path(profiles_anycubic_dir()) / data.fileName).make_preferred();
	data.fileName = rsrc_dir_path.string();*/

	//wxLogMessage(from_u8(data.fileName));
	//SetIcon(data.fileName);
	SetImage(data.fileName);
	SetIconMaskChecked(wxString("icon-mask-nor"), wxString("icon-mask-hover"), 24);


	button->SetMinSize(FromDIP(wxSize(36, 36)));
	button->SetSize(FromDIP(wxSize(36, 36)));
	//button->SetBackgroundColour(wxColour(150, 150, 150));
	button->SetFont(ACLabel::sysFont(20, true));
	SetFont(ACLabel::sysFont(13, false));

	SetCornerRadius(14);
	SetMinSize(FromDIP(wxSize(354, 122)));

	fgColor.append(wxColour(168, 168, 168),    ACStateColor::Disabled);
	fgColor.append(AC_COLOR_WHITEBLUE_PRESSED, ACStateColor::Pressed);
	fgColor.append(wxColour(20, 28, 41),       ACStateColor::Hovered);
	fgColor.append(wxColour(20, 28, 41),       ACStateColor::Normal);


	wxBoxSizer* hBox = new wxBoxSizer(wxHORIZONTAL);
	hBox->AddStretchSpacer(1);
	hBox->Add(button, 0, wxALL, 0);
	hBox->AddSpacer(FromDIP(6));

	wxBoxSizer* sizer = new wxBoxSizer(wxVERTICAL);
	sizer->AddSpacer(FromDIP(6));
	sizer->Add(hBox, 0, wxEXPAND | wxALL, 0);
	sizer->AddStretchSpacer(1);
	SetSizer(sizer);
	Layout();
	Fit();
	Refresh();

}
void ACPrinterMeta::SetIcon(const wxString& iconName)
{
	if (!iconName.IsEmpty())
		iconNormal = ScalableBitmap(this, sizeIcon, iconName.ToStdString());
	else
		iconNormal = ScalableBitmap();
	m_sizeValid = false;
	Refresh();
}
void ACPrinterMeta::SetImage(const std::string& fileFullPath)
{
	wxFileInputStream imageStream(wxString::FromUTF8(fileFullPath.c_str()));//用于加载大文件
	image = wxImage(imageStream, wxBITMAP_TYPE_ANY /*wxBITMAP_TYPE_PNG*/);  //这里不能指定为PNG，否则加载会失败
	wxImage img = image;
	wxSize iconDIP = FromDIP(sizeIcon);

	//高质量缩放
	img.Rescale(iconDIP.GetWidth(), iconDIP.GetHeight(), wxIMAGE_QUALITY_HIGH);
	bitmapImage = wxBitmap(img);

	m_sizeValid = false;
	Refresh();
}
void ACPrinterMeta::SetHoverIcon(const wxString& iconName)
{
	if (!iconName.IsEmpty())
		iconHover = ScalableBitmap(this, sizeIcon, iconName.ToStdString());
	else
		iconHover = ScalableBitmap();
	m_sizeValid = false;
	Refresh();
}
void ACPrinterMeta::SetIconMaskChecked(const wxString& imgNameCheckedOn, const wxString& imgNameCheckedHover, int imgSize)
{
	sizeIconCheck = wxSize(imgSize, imgSize);
	if (!imgNameCheckedOn.IsEmpty())
		iconNormalMaskChecked = ScalableBitmap(this, imgNameCheckedOn.ToStdString(), imgSize);
	else
		iconNormalMaskChecked = ScalableBitmap();
	if (!imgNameCheckedHover.IsEmpty())
		iconHoverMaskChecked = ScalableBitmap(this, imgNameCheckedHover.ToStdString(), imgSize);
	else
		iconHoverMaskChecked = ScalableBitmap();
	m_sizeValid = true;
	Refresh();
}
void ACPrinterMeta::OnButton(wxCommandEvent& event)
{
	popup->SetSize(FromDIP(wxSize(120, 60)));
	popup->SetPosition(button->GetScreenPosition() + wxPoint(0 - FromDIP(82), FromDIP(button->GetSize().GetHeight() / 2 + FromDIP(10))));
	popup->Show();
}
void ACPrinterMeta::OnButtonRename(wxCommandEvent& event)
{
	ACPrinterRenameDialog dlg(nullptr, this);
	dlg.name->SetValue(data.name);
	if (wxID_OK == dlg.ShowModal())
	{
		wxCommandEvent* evt = new wxCommandEvent(EVT_ACLOUD_PRINTER_RENAME);
		evt->SetInt(data.id_printer);
		evt->SetString(dlg.name->GetValue());
		ProcessEvent(*evt);
	}
}
void ACPrinterMeta::OnButtonDelete(wxCommandEvent& event)
{
	ACPrinterDeleteDialog dlg(nullptr, data.name);
	if (wxID_OK == dlg.ShowModal())
	{
		wxCommandEvent* evt = new wxCommandEvent(EVT_ACLOUD_PRINTER_DELETE);
		evt->SetInt(data.id_printer);
		ProcessEvent(*evt);
		//GUI::wxGetApp().QueueEvent(evt);
	}
}
void ACPrinterMeta::SetName(const wxString& t)
{
	data.name = t;
	Refresh();
}
void ACPrinterMeta::SetButtonState(ButtonState bs)
{
	if (buttonState != bs)
	{
		buttonState = bs;
		Refresh();
	}
}
void ACPrinterMeta::SetPrinterData(const PrinterData& d)
{
	data = d;
	if (IsShown())
		Refresh();
}
void ACPrinterMeta::SetChecked(bool b)
{
	isChecked = b;
	if (IsShown())
		Refresh();
}
void ACPrinterMeta::OnEnter(wxMouseEvent& event)
{
	isMouseInside = true;
	SetButtonState(isMousePressed ? ButtonState::Pressed : ButtonState::Hover);
	event.Skip();
}
void ACPrinterMeta::OnLeave(wxMouseEvent& event)
{
	isMouseInside = false;
	SetButtonState(ButtonState::Normal);
	Refresh();
	event.Skip();
}
void ACPrinterMeta::OnMouseDown(wxMouseEvent& event)
{
	isMousePressed = true;
	SetButtonState(ButtonState::Pressed);

	if (!isChecked) {

		wxCommandEvent evt(EVT_ACLOUD_PRINTER_CHECK_CLICK);
        evt.SetInt(data.id_printer);
        wxPostEvent(wxGetApp().mainframe->GetACCloudSelectMachineObj(), evt);
	}

	event.Skip();
}
void ACPrinterMeta::OnMouseUp(wxMouseEvent& event)
{
	isMousePressed = false;
	isChecked = !isChecked;
	SetButtonState(isMouseInside ? ButtonState::Hover : ButtonState::Normal);
	event.Skip();
}
void ACPrinterMeta::OnDPIChanged(wxDPIChangedEvent& event)
{
	wxImage img = image;
	wxSize iconDIP = FromDIP(sizeIcon);
	img.Rescale(iconDIP.GetWidth(), iconDIP.GetHeight(), wxIMAGE_QUALITY_HIGH);
	bitmapImage = wxBitmap(img);
	event.Skip();
}
void ACPrinterMeta::OnPaint(wxPaintEvent& event)
{
	//wxAutoBufferedPaintDC dc(this);
	wxPaintDC dc(this);
	wxGCDC gcdc(dc);
	render(gcdc);
	//render(dc);
}
void ACPrinterMeta::render(wxDC& dc)
{
	if (m_sizeValid == false)
		messureSize();

	wxString text;

	int states = buttonState;

	// background
	ACStaticBox::render(dc);

	dc.SetBrush(*wxTRANSPARENT_BRUSH);

	wxSize iconDIP = FromDIP(sizeIcon);
	//const ScalableBitmap* icon = &iconNormal;
	//bool isIcon = icon->get_bitmap().IsOk();

	wxRect rect = GetClientRect();
	wxPoint pt = GetClientRect().GetLeftTop();

	//draw image (thumbnai)
	if (bitmapImage.IsOk())
	{
		pt.x += FromDIP(6);
		pt.y += (rect.height - iconDIP.y) / 2;
		dc.DrawBitmap(bitmapImage, pt);
	}
	//drawtext
	if (true/*!data.name.IsEmpty()*/)
	{
		dc.SetFont(GetFont());
		//dc.SetPen(wxPen(fgColor.colorForStates(states), 1));
		if (Hover == buttonState)
		{
			dc.SetPen(wxPen(wxColour(150, 150, 150), 1));
			//dc.SetTextForeground(wxColour(150, 150, 150));
		}
		else
		{
			dc.SetPen(wxPen(wxColour(20, 28, 41), 1));
			//dc.SetTextForeground(wxColour(20, 28, 41));
		}
		dc.SetTextForeground(wxColour(20, 28, 41));
		pt.x = FromDIP(70);

		pt.y = FromDIP(16);
		dc.SetFont(ACLabel::sysFont(13, true));
		text = wxControl::Ellipsize(data.name, dc, wxELLIPSIZE_END, FromDIP(250));
		dc.DrawText(text, pt);

		dc.SetFont(ACLabel::sysFont(13, false));
		pt.y = FromDIP(52);
		dc.DrawText(_L("Printer Status"), pt);

		pt.y = FromDIP(82);
		dc.DrawText(_L("Printer Type"), pt);

		pt.x = FromDIP(180);
		//dc.SetTextForeground(wxColour(51, 51, 51));
		dc.SetTextForeground(wxColour(120, 120, 120));
		text = wxControl::Ellipsize(data.type, dc, wxELLIPSIZE_END, FromDIP(165));
		dc.DrawText(text, pt);

		wxSize size = FromDIP(wxSize(64, 24));
		dc.SetPen(wxNullPen);
		// Draw rectangle
		switch (data.status)
		{
		case 1://free
			dc.SetBrush(wxColour(219, 235, 207));
			break;
		case 2://busy
			dc.SetBrush(wxColour(245, 231, 207));
			break;
		case 0://offine
		default:
			dc.SetBrush(wxColour(225, 227, 230));
			break;
		}
		pt.x = FromDIP(180);
		pt.y = FromDIP(52);
		dc.DrawRoundedRectangle(pt, size, FromDIP(4));

		//draw circle
		switch (data.status)
		{
		case 1://free
			dc.SetBrush(wxColour(114, 194, 64));
			break;
		case 2://busy
			dc.SetBrush(wxColour(239, 176, 65));
			break;
		case 0://offine
		default:
			dc.SetBrush(wxColour(128, 128, 128));
			break;
		}
		pt.x = FromDIP(180 + 12);
		pt.y = FromDIP(52 + 12);
		dc.DrawCircle(pt, FromDIP(4));

		//draw text
		dc.SetBrush(*wxTRANSPARENT_BRUSH);
		dc.SetPen(wxPen(wxColour(20, 28, 41), 1));
		pt.x = FromDIP(180 + 24);
		pt.y = FromDIP(52 + 4);
		dc.SetFont(ACLabel::sysFont(11, false));
		switch (data.status)
		{
		case 1://free
			dc.SetTextForeground(wxColour(114, 194, 64));
			dc.DrawText(_L("Free"), pt);
			break;
		case 2://busy
			dc.SetTextForeground(wxColour(239, 176, 65));
			dc.DrawText(_L("Busy"), pt);
			break;
		case 0://offine
		default:
			dc.SetTextForeground(wxColour(128, 128, 128));
			dc.DrawText(_L("Offline"), pt);
			break;
		}

	}
	if (isChecked || (states & (int)ACStateColor::State::Hovered) != 0)
	{
		dc.SetPen(wxPen(wxColour(57, 134, 255), 1));
		//dc.SetTextForeground(wxColour(57, 134, 255));
		dc.SetBrush(*wxTRANSPARENT_BRUSH);
		dc.DrawRoundedRectangle(wxPoint(0, 0), wxSize(GetSize()), FromDIP(14));
	}

	if (isChecked)//选中状态
	{
		ScalableBitmap* sbmp = nullptr;
		if ((states & (int)ACStateColor::State::Hovered) != 0)
			sbmp = &iconHoverMaskChecked;
		else
			sbmp = &iconNormalMaskChecked;
		pt.x = 0;
		pt.y = 0;
		dc.DrawBitmap(sbmp->bmp().GetBitmap(FromDIP(wxSize(24, 24))), pt);

	}
}


/*---------------------------------------------------------------------------------
								ACPrinterRenameDialog
-----------------------------------------------------------------------------------*/
ACPrinterRenameDialog::ACPrinterRenameDialog(wxWindow* parent, ACPrinterMeta* pm) :
	DPIDialog(parent,
		wxID_ANY,
		L"",
		wxDefaultPosition,
		wxSize(460, 200),
		wxNO_BORDER),
	p				{ pm },
	topbar			{ new ACDialogTopbar(this, _L("Rename Printer"), 40) },
	hBox			{ new wxBoxSizer(wxHORIZONTAL) },
	vBox			{ new wxBoxSizer(wxVERTICAL) },
	text			{ new wxStaticText(this, wxID_ANY, _L("Please provide a new name")) },
	textWarning		{ new wxStaticText(this, wxID_ANY, _L("The name connot be empty")) },
	buttonCancel	{ new ACButton(this, _L("Cancel")) },
	buttonRename	{ new ACButton(this, _L("OK")) },
	name			{ new wxTextCtrl(this, wxID_ANY, "", wxDefaultPosition, wxDefaultSize, wxTE_PROCESS_ENTER) }
{
	Init();
	Connect();
}
ACPrinterRenameDialog::~ACPrinterRenameDialog()
{
	p = nullptr;
	wxDELETE(topbar);
	wxDELETE(text);
	wxDELETE(textWarning);
	wxDELETE(buttonCancel);
	wxDELETE(buttonRename);
	wxDELETE(name);
}
void ACPrinterRenameDialog::Init()
{
	SetMinSize(FromDIP(wxSize(460, 200)));
	AddWindowDrakEdg(this);

	wxFont fontBold = ACLabel::sysFont(14, true);
	wxFont font = ACLabel::sysFont(13, false);

	text->SetFont(ACLabel::sysFont(14, false));
	textWarning->SetFont(font);
	textWarning->SetForegroundColour(wxColour(229, 46, 46));
	name->SetMinSize(FromDIP(wxSize(400, 28)));
	name->SetFont(ACLabel::sysFont(14, false));

	topbar->GetTextPtr()->SetFont(fontBold);
	topbar->GetTextPtr()->SetPaddingSize(wxSize(0, 0));
	topbar->GetButtonPtr()->SetPaddingSize(wxSize(0, 0));
	topbar->GetButtonPtr()->SetMinSize(wxSize(36, 36));
	topbar->GetButtonPtr()->SetSpacing(0);

	topbar->LayoutLeft();
	vBox->Add(topbar, 0, wxEXPAND | wxALL, 1);
	vBox->AddSpacer(FromDIP(20));


	//button cancel set stylesheet 
	buttonCancel->SetButtonType(ACButton::AC_BUTTON_TYPE::AC_BUTTON_LV1);
	buttonCancel->SetPaddingSize(wxSize(0, 0));
	buttonCancel->SetMinSize(wxSize(110, 30));
	buttonCancel->SetSpacing(0);
	buttonCancel->SetCornerRadius(4);
	buttonCancel->SetBackgroundColor(ACStateColor(wxColour(244, 248, 254)));
	buttonCancel->SetTextColor(ACStateColor(wxColour(57, 134, 255)));
	buttonCancel->SetBorderColor(ACStateColor(wxColour(57, 134, 255)));
	buttonCancel->SetFont(font);

	//button update now set stylesheet 
	buttonRename->SetButtonType(ACButton::AC_BUTTON_TYPE::AC_BUTTON_LV0);
	buttonRename->SetPaddingSize(wxSize(0, 0));
	buttonRename->SetMinSize(wxSize(110, 30));
	buttonRename->SetSpacing(0);
	buttonRename->SetCornerRadius(4);
	//buttonUpdateNow->SetBackgroundColor(ACStateColor(wxColour(255, 255, 255)));
	buttonRename->SetTextColor(ACStateColor(wxColour(255, 255, 255)));
	buttonRename->SetBorderColor(ACStateColor(wxColour(57, 134, 255)));
	buttonRename->SetFont(font);

	wxCursor cursor(wxCURSOR_HAND);
	buttonCancel->SetCursor(cursor);
	buttonRename->SetCursor(cursor);

	hBox->AddStretchSpacer(1);
	hBox->Add(buttonCancel, 0, wxALL, 0);
	hBox->AddSpacer(FromDIP(20));
	hBox->Add(buttonRename, 0, wxALL, 0);
	hBox->AddSpacer(FromDIP(20));
	vBox->Add(text, 0, wxEXPAND | wxLEFT | wxRIGHT, 20);
	vBox->AddSpacer(FromDIP(10));
	vBox->Add(name, 0, wxEXPAND | wxLEFT | wxRIGHT, 20);
	vBox->Add(textWarning, 0, wxEXPAND | wxLEFT | wxRIGHT, 20);
	vBox->AddStretchSpacer(1);
	
	
	vBox->Add(hBox, 0, wxEXPAND | wxALL, 1);
	vBox->AddSpacer(FromDIP(20));

	SetBackgroundColour(AC_COLOR_WHITE);
	SetSizerAndFit(vBox);
	SetSize(FromDIP(wxSize(460, 200)));
	Layout();
}
void ACPrinterRenameDialog::Connect()
{
	buttonCancel          ->Bind(wxEVT_BUTTON, [this](wxCommandEvent&) { OnClose(); });
	buttonRename          ->Bind(wxEVT_BUTTON, [this](wxCommandEvent&) { OnRename(); });
	topbar->GetButtonPtr()->Bind(wxEVT_BUTTON, [this](wxCommandEvent&) { OnClose(); });

	name->Bind(wxEVT_TEXT, &ACPrinterRenameDialog::OnText, this);
}

void ACPrinterRenameDialog::OnRename()
{
	//wxLogMessage("Rename");
	if (name->GetValue().IsEmpty())
	{
		textWarning->Show();
		Layout();
	}
	else
	{
		if (p)
			p->SetName(name->GetValue());
		EndModal(wxID_OK);
	}
}
void ACPrinterRenameDialog::OnText(wxCommandEvent& event)
{
	if (!name->GetValue().IsEmpty())
	{
		textWarning->Hide();
		Layout();
	}
}
void ACPrinterRenameDialog::OnClose()
{
	Close();
}
void ACPrinterRenameDialog::msw_rescale()
{
	SetMinSize(FromDIP(wxSize(460, 200)));
	Fit();
	Layout();
	Refresh();
}





/*---------------------------------------------------------------------------------
								ACPrinterDeleteDialog
-----------------------------------------------------------------------------------*/
ACPrinterDeleteDialog::ACPrinterDeleteDialog(wxWindow* parent, const wxString& name_) :
	DPIDialog(parent,
		wxID_ANY,
		L"",
		wxDefaultPosition,
		wxSize(460, 200),
		wxNO_BORDER),
	name			{ name_ },
	topbar			{ new ACDialogTopbar(this, _L("Delete Printer"), 40) },
	vBox			{ new wxBoxSizer(wxVERTICAL) },
	hBox			{ new wxBoxSizer(wxHORIZONTAL) },
	text			{ new wxStaticText(this, wxID_ANY, "")},
	buttonCancel	{ new ACButton(this, _L("Cancel")) },
	buttonDelete	{ new ACButton(this, _L("OK")) }
{
	Init();
	Connect();
}
ACPrinterDeleteDialog::~ACPrinterDeleteDialog()
{
	wxDELETE(topbar);
	wxDELETE(text);
	wxDELETE(buttonCancel);
	wxDELETE(buttonDelete);
}
void ACPrinterDeleteDialog::Init()
{
	name = _L("Are you sure to delete Printer") + "\"" + name + "\"?";
	SetMinSize(FromDIP(wxSize(460, 200)));
	AddWindowDrakEdg(this);

	wxFont fontBold = ACLabel::sysFont(14, true);
	wxFont font = ACLabel::sysFont(13, false);

	text->SetLabel(name);
	text->SetFont(ACLabel::sysFont(14, false));

	topbar->GetTextPtr()->SetFont(fontBold);
	topbar->GetTextPtr()->SetPaddingSize(wxSize(0, 0));
	topbar->GetButtonPtr()->SetPaddingSize(wxSize(0, 0));
	topbar->GetButtonPtr()->SetMinSize(wxSize(36, 36));
	topbar->GetButtonPtr()->SetSpacing(0);

	topbar->LayoutLeft();
	vBox->Add(topbar, 0, wxEXPAND | wxALL, 1);
	vBox->AddSpacer(FromDIP(20));


	//button cancel set stylesheet 
	buttonCancel->SetButtonType(ACButton::AC_BUTTON_TYPE::AC_BUTTON_LV1);
	buttonCancel->SetPaddingSize(wxSize(0, 0));
	buttonCancel->SetMinSize(wxSize(110, 30));
	buttonCancel->SetSpacing(0);
	buttonCancel->SetCornerRadius(4);
	buttonCancel->SetBackgroundColor(ACStateColor(wxColour(244, 248, 254)));
	buttonCancel->SetTextColor(ACStateColor(wxColour(57, 134, 255)));
	buttonCancel->SetBorderColor(ACStateColor(wxColour(57, 134, 255)));
	buttonCancel->SetFont(font);

	//button update now set stylesheet 
	buttonDelete->SetButtonType(ACButton::AC_BUTTON_TYPE::AC_BUTTON_LV0);
	buttonDelete->SetPaddingSize(wxSize(0, 0));
	buttonDelete->SetMinSize(wxSize(110, 30));
	buttonDelete->SetSpacing(0);
	buttonDelete->SetCornerRadius(4);
	//buttonUpdateNow->SetBackgroundColor(ACStateColor(wxColour(255, 255, 255)));
	buttonDelete->SetTextColor(ACStateColor(wxColour(255, 255, 255)));
	buttonDelete->SetBorderColor(ACStateColor(wxColour(57, 134, 255)));
	buttonDelete->SetFont(font);

	wxCursor cursor(wxCURSOR_HAND);
	buttonCancel->SetCursor(cursor);
	buttonDelete->SetCursor(cursor);

	hBox->AddStretchSpacer(1);
	hBox->Add(buttonCancel, 0, wxALL, 0);
	hBox->AddSpacer(FromDIP(20));
	hBox->Add(buttonDelete, 0, wxALL, 0);
	hBox->AddSpacer(FromDIP(20));
	vBox->Add(text, 0, wxEXPAND | wxLEFT | wxRIGHT, 20);
	vBox->AddStretchSpacer(1);


	vBox->Add(hBox, 0, wxEXPAND | wxALL, 1);
	vBox->AddSpacer(FromDIP(20));

	SetBackgroundColour(AC_COLOR_WHITE);
	SetSizerAndFit(vBox);
	SetSize(FromDIP(wxSize(460, 200)));
	Layout();
}
void ACPrinterDeleteDialog::Connect()
{
	buttonCancel->Bind(wxEVT_BUTTON, [this](wxCommandEvent&) { OnClose(); });
	buttonDelete->Bind(wxEVT_BUTTON, [this](wxCommandEvent&) { OnDelete(); });
	topbar->GetButtonPtr()->Bind(wxEVT_BUTTON, [this](wxCommandEvent&) { OnClose(); });
}

void ACPrinterDeleteDialog::OnDelete()
{
	EndModal(wxID_OK);
}
void ACPrinterDeleteDialog::OnClose()
{
	Close();
}
void ACPrinterDeleteDialog::msw_rescale()
{
	SetMinSize(FromDIP(wxSize(460, 200)));
	Fit();
	Layout();
	Refresh();
}






/*---------------------------------------------------------------------------------
								ACPrinterAddDialog
-----------------------------------------------------------------------------------*/
ACPrinterAddDialog::ACPrinterAddDialog(wxWindow* parent) :
	DPIDialog(parent,
		wxID_ANY,
		L"",
		wxDefaultPosition,
		wxSize(460, 220),
		wxNO_BORDER),
	topbar			{ new ACDialogTopbar(this, _L("Add Cloud Printer"), 40) },
	text1			{ new wxStaticText(this, wxID_ANY, _L("Device CN")) },
	text2			{ new wxStaticText(this, wxID_ANY, _L("Printer Name")) },
	textWarning		{ new wxStaticText(this, wxID_ANY,  _L("Device CN and Printer Name connot be empty."))},
	buttonCancel	{ new ACButton(this, _L("Cancel")) },
	buttonAdd		{ new ACButton(this, _L("Add")) },
	name			{ new wxTextCtrl(this, wxID_ANY, "", wxDefaultPosition, wxDefaultSize, wxTE_PROCESS_ENTER) },
	deviceCN		{ new wxTextCtrl(this, wxID_ANY, "", wxDefaultPosition, wxDefaultSize, wxTE_PROCESS_ENTER) },
	hBox1			{ new wxBoxSizer(wxHORIZONTAL) },
	hBox2			{ new wxBoxSizer(wxHORIZONTAL) },
	hBox3			{ new wxBoxSizer(wxHORIZONTAL) },
	sizer			{ new wxBoxSizer(wxVERTICAL) }
{
	Init();
	Connect();
}
ACPrinterAddDialog::~ACPrinterAddDialog()
{
	wxDELETE(topbar);
	wxDELETE(text1);
	wxDELETE(text2);
	wxDELETE(textWarning);
	wxDELETE(buttonCancel);
	wxDELETE(buttonAdd);
	wxDELETE(name);
	wxDELETE(deviceCN);
}
void ACPrinterAddDialog::Init()
{
	SetMinSize(FromDIP(wxSize(460, 220)));
	AddWindowDrakEdg(this);

	//buttonAdd->SetEnable(false);

	wxFont fontBold = ACLabel::sysFont(14, true);
	wxFont font = ACLabel::sysFont(13, false);

	text1->SetFont(ACLabel::sysFont(14, false));
	text1->SetMinSize(FromDIP(wxSize(100, 24)));
	text2->SetFont(ACLabel::sysFont(14, false));
	text2->SetMinSize(FromDIP(wxSize(100, 24)));


	textWarning->SetFont(font);
	textWarning->SetForegroundColour(wxColour(229, 46, 46));
	name->SetMinSize(FromDIP(wxSize(400, 28)));
	name->SetFont(ACLabel::sysFont(13, false));
	name->SetHint(_L("Custom printer name"));

	deviceCN->SetMinSize(FromDIP(wxSize(400, 28)));
	deviceCN->SetFont(ACLabel::sysFont(13, false));
	deviceCN->SetHint(_L("Enter a 16 digit CN number"));
	

	topbar->GetTextPtr()->SetFont(fontBold);
	topbar->GetTextPtr()->SetPaddingSize(wxSize(0, 0));
	topbar->GetButtonPtr()->SetPaddingSize(wxSize(0, 0));
	topbar->GetButtonPtr()->SetMinSize(wxSize(36, 36));
	topbar->GetButtonPtr()->SetSpacing(0);

	topbar->LayoutLeft();
	sizer->Add(topbar, 0, wxEXPAND | wxALL, 1);
	sizer->AddSpacer(FromDIP(20));


	//button cancel set stylesheet 
	buttonCancel->SetButtonType(ACButton::AC_BUTTON_TYPE::AC_BUTTON_LV1);
	buttonCancel->SetPaddingSize(wxSize(0, 0));
	buttonCancel->SetMinSize(wxSize(110, 30));
	buttonCancel->SetSpacing(0);
	buttonCancel->SetCornerRadius(4);
	buttonCancel->SetBackgroundColor(ACStateColor(wxColour(244, 248, 254)));
	buttonCancel->SetTextColor(ACStateColor(wxColour(57, 134, 255)));
	buttonCancel->SetBorderColor(ACStateColor(wxColour(57, 134, 255)));
	buttonCancel->SetFont(font);

	//button update now set stylesheet 
	buttonAdd->SetButtonType(ACButton::AC_BUTTON_TYPE::AC_BUTTON_LV0);
	buttonAdd->SetPaddingSize(wxSize(0, 0));
	buttonAdd->SetMinSize(wxSize(110, 30));
	buttonAdd->SetSpacing(0);
	buttonAdd->SetCornerRadius(4);
	buttonAdd->SetTextColor(ACStateColor(wxColour(255, 255, 255)));
	buttonAdd->SetBorderColor(ACStateColor(wxColour(57, 134, 255)));
	buttonAdd->SetFont(font);

	wxCursor cursor(wxCURSOR_HAND);
	buttonCancel->SetCursor(cursor);
	buttonAdd->SetCursor(cursor);

	hBox1->AddSpacer(FromDIP(20));
	hBox1->Add(text1, 0, wxALL | wxEXPAND, 0);
	hBox1->Add(deviceCN,  1, wxALL | wxEXPAND, 0);
	hBox1->AddSpacer(FromDIP(20));
	sizer->Add(hBox1, 0, wxEXPAND | wxALL, 1);

	sizer->AddSpacer(FromDIP(20));

	hBox2->AddSpacer(FromDIP(20));
	hBox2->Add(text2, 0, wxALL | wxEXPAND, 0);
	hBox2->Add(name,  1, wxALL | wxEXPAND, 0);
	hBox2->AddSpacer(FromDIP(20));

	sizer->Add(hBox2, 0, wxEXPAND | wxALL, 1);
	sizer->AddSpacer(FromDIP(10));
	sizer->Add(textWarning, 0, wxEXPAND | wxLEFT | wxRIGHT, 20);
	hBox3->AddStretchSpacer(1);
	hBox3->Add(buttonCancel, 0, wxALL, 0);
	hBox3->AddSpacer(FromDIP(20));
	hBox3->Add(buttonAdd, 0, wxALL, 0);
	hBox3->AddSpacer(FromDIP(20));
	
	sizer->AddStretchSpacer(1);


	sizer->Add(hBox3, 0, wxEXPAND | wxALL, 1);
	sizer->AddSpacer(FromDIP(20));

	textWarning->Hide();

	SetBackgroundColour(AC_COLOR_WHITE);
	SetSizerAndFit(sizer);
	SetSize(FromDIP(wxSize(460, 220)));
	Layout();
}
void ACPrinterAddDialog::Connect()
{
	buttonCancel->Bind(wxEVT_BUTTON, [this](wxCommandEvent&) { OnClose(); });
	buttonAdd   ->Bind(wxEVT_BUTTON, &ACPrinterAddDialog::OnAdd, this);
	topbar->GetButtonPtr()->Bind(wxEVT_BUTTON, [this](wxCommandEvent&) { OnClose(); });

	name    ->Bind(wxEVT_TEXT, &ACPrinterAddDialog::OnText, this);
	deviceCN->Bind(wxEVT_TEXT, &ACPrinterAddDialog::OnText, this);
}

void ACPrinterAddDialog::OnAdd(wxCommandEvent& event)
{
	buttonAdd->SetCursor(wxCursor(wxCURSOR_WAIT));
	if (name->GetValue().IsEmpty() || deviceCN->GetValue().IsEmpty())
	{
		textWarning->Show();
		Layout();
	}
	else
		EndModal(wxID_OK);
	buttonAdd->SetCursor(wxCursor(wxCURSOR_HAND));
}
void ACPrinterAddDialog::OnText(wxCommandEvent& event)
{
	//两者都不为空
	if (!name->GetValue().IsEmpty() && !deviceCN->GetValue().IsEmpty())
	{
		textWarning->Hide();
		Layout();
		//buttonAdd->SetEnable(true);
	}
	else
		;// buttonAdd->SetEnable(true);
}
void ACPrinterAddDialog::OnClose()
{
	Close();
}
void ACPrinterAddDialog::msw_rescale()
{
	SetMinSize(FromDIP(wxSize(460, 220)));
	Fit();
	Layout();
	Refresh();
}

} // GUI
} // Slic3r
