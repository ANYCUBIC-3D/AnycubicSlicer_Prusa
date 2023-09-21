#include "ACCloudMachine.hpp"
#include "ACCloudManger.hpp"

#include "../GUI.hpp"
#include "../GUI_App.hpp"
#include "../GUI_Utils.hpp"
#include "../BitmapCache.hpp"
#include "libslic3r/Utils.hpp"
#include "../ACLabel.hpp"
#include "../MainFrame.hpp"

#include <wx/wx.h>
#include <wx/sizer.h>
#include <wx/panel.h>
#include <wx/font.h>
#include <wx/dcgraph.h>

#include <string>
#include <stdio.h>
#include <thread>

//#define GetEditHwnd() ((HWND)(GetEditHWND()))

namespace Slic3r {

namespace GUI {

ACCloudMachine::ACCloudMachine(wxWindow* parent, MainFrame* main_frame) :
	wxPanel(parent),
	mf				{ main_frame },
	box				{ new ACStaticBox(this) },
	panel			{ new wxPanel(box, wxID_ANY) },
	title			{ new wxStaticText(panel, wxID_ANY, "", wxDefaultPosition, wxDefaultSize, wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL)},
	printer			{ nullptr },
	buttonRefresh	{ new ACButton(panel, _L("Refresh"), "", "", "") },
	buttonAddNew	{ new ACButton(panel, _L("Add New"), "", "", "") },
	line			{ new wxStaticLine(panel, wxID_ANY, wxDefaultPosition, { 255, wxDefaultCoord }, wxLI_HORIZONTAL) },
	sizer			{ new wxBoxSizer(wxVERTICAL) },
	vBox			{ new wxBoxSizer(wxVERTICAL) }
{ 
	std::vector<PrinterData> info;
	printer = new ACPrinterContainer(panel, info, false);
	Init();
	Connect();
}
ACCloudMachine::~ACCloudMachine()
{
	wxDELETE(title);
	wxDELETE(buttonRefresh);
	wxDELETE(buttonAddNew);
	wxDELETE(line);

	//wxDELETE(printer);

	wxDELETE(panel);
	wxDELETE(box);
}
void ACCloudMachine::Init()
{
	title->SetLabel(_L("Cloud Printers Added from your Account"));

	wxCursor cursor(wxCURSOR_HAND);
	buttonRefresh->SetCursor(cursor);
	buttonAddNew->SetCursor(cursor);

	wxFont font = ACLabel::sysFont(13, false);
	title->SetFont(font);

	buttonRefresh->SetFont(font);
	buttonAddNew->SetFont(font);


	buttonRefresh->SetButtonType(ACButton::AC_BUTTON_TYPE::AC_BUTTON_LV2);
	buttonRefresh->SetCornerRadius(4);
	buttonRefresh->SetPaddingSize(wxSize(0, 0));
	buttonRefresh->SetSpacing(0);
	buttonRefresh->SetMinSize(wxSize(90, 30));//不需要使用FromDIP

	buttonAddNew->SetButtonType(ACButton::AC_BUTTON_TYPE::AC_BUTTON_LV2);
	buttonAddNew->SetCornerRadius(4);
	buttonAddNew->SetPaddingSize(wxSize(0, 0));
	buttonAddNew->SetSpacing(0);
	buttonAddNew->SetMinSize(wxSize(90, 30));//不需要使用FromDIP


	line->SetBackgroundColour(wxColour(200, 200, 200));
	//line->SetMinSize(FromDIP(wxSize(100, 2)));



	vBox->AddSpacer(FromDIP(70));
	vBox->Add(title, 0, wxEXPAND | wxLEFT | wxRIGHT, FromDIP(12));
	vBox->AddSpacer(FromDIP(12));

	wxBoxSizer* hBox = new wxBoxSizer(wxHORIZONTAL);
	hBox->Add(buttonRefresh, 0, wxALL | wxEXPAND, 0);
	hBox->AddSpacer(FromDIP(20));
	hBox->Add(buttonAddNew, 0, wxALL | wxEXPAND, 0);
	hBox->AddStretchSpacer(1);

	vBox->Add(hBox, 0, wxEXPAND | wxLEFT | wxRIGHT, FromDIP(12));
	vBox->AddSpacer(FromDIP(20));
	vBox->Add(line, 0, wxEXPAND | wxLEFT | wxRIGHT, FromDIP(12));
	vBox->AddSpacer(FromDIP(20));

	//PrinterContainer
	vBox->Add(printer, 1, wxEXPAND | wxLEFT, FromDIP(12));

	panel->SetSizer(vBox);


	box->SetCornerRadius(12);
	box->SetBackgroundColor(*wxWHITE);
	panel->SetBackgroundColour(*wxWHITE);
	
	sizer->Add(panel, 1, wxEXPAND | wxALL, 6);
	box->SetSizer(sizer);



	wxBoxSizer* main = new wxBoxSizer(wxVERTICAL);
	main->Add(box, 1, wxEXPAND | wxALL, 0);
	SetSizer(main);
	Layout();
	Refresh();
}
void ACCloudMachine::Connect()
{
	buttonRefresh->Bind(wxEVT_BUTTON, &ACCloudMachine::OnButtonRefresh, this);
	buttonAddNew ->Bind(wxEVT_BUTTON, &ACCloudMachine::OnButtonAddNew,  this);

	//Bind(wxEVT_SIZE, &ACCloudMachine::OnSize, this);
}
void ACCloudMachine::OnButtonRefresh(wxCommandEvent& event)
{
	buttonRefresh->SetCursor(wxCursor(wxCURSOR_WAIT));
	wxLogMessage(Cloud->PrinterList() ? "succeed" : "failed");
	buttonRefresh->SetCursor(wxCursor(wxCURSOR_HAND));
}
void ACCloudMachine::OnButtonAddNew(wxCommandEvent& event)
{
	wxString cn = wxGetApp().mainframe->showAddPrinterDialog();
	

	ACPrinterAddDialog dlg(nullptr);
	if (wxID_OK == dlg.ShowModal())
	{
		PrinterData data;
		data.name = dlg.name->GetValue();//打印机名
		wxString deviceCN = dlg.deviceCN->GetValue();//CN码

		//其实执行的是两条命令，首先执行添加打印机命令，然后执行Rename命令
		switch (Cloud->AddPrinter(deviceCN.utf8_str().data()))//添加打印机
		{
		case E_FALSE://执行失败
			wxLogMessage("failed");
			break;
		case E_TRUE://执行成功
			Cloud->PrinterList();//执行刷新
			break;
		case E_TIMEOUT://超时
			wxLogMessage("time out");
			break;
		default:
			break;
		}

		//printer->AddPrinterSingle(data);
		//Layout();
		//Refresh();
	}
}

void ACCloudMachine::UpdatePrinterList()
{
	std::vector<PrinterData>printerList = Cloud->GetPrinterInfo();
	if (printer)
		printer->SetPrinters(printerList);
}
//void ACCloudMachine::OnSize(wxSizeEvent& event)
//{
//	ReLayoutPrinter();
//	Layout();
//	//Refresh();
//	event.Skip();
//}
} // GUI
} // Slic3r
