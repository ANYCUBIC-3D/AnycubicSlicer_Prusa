#include "ACToolbar.hpp"
#include "I18N.hpp"
#include "GUI_App.hpp"
#include "GUI.hpp"
#include "wxExtensions.hpp"
#include "Plater.hpp"
#include "MainFrame.hpp"
#include "wx/defs.h"

#include "ACDefines.h"
#include "ACSplitLine.hpp"
#include "ACButton.hpp"
#include "ACGauge.hpp"
#include "Cloud/ACCloudLogin.hpp"

using namespace Slic3r;


ACToolBar::ACToolBar(wxFrame* parent) 
    : ACStaticBox(parent, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxNO_BORDER)
    , m_toolbar_h(48)
{ 
    Init(parent);

	SetCornerRadius(0);//修复MacOS下呈圆角边，会有黑底效果
}

void ACToolBar::Init(wxFrame* parent) 
{
    m_frame = parent;

    int client_w = parent->GetClientSize().GetWidth();
    this->SetSize(client_w, (m_toolbar_h));

    SetBackgroundColour(AC_COLOR_WHITE);

    wxString m_iconName_import   = "icon-import";

    wxString m_iconName_save_nor = "icon-save-nor";
    wxString m_iconName_save_hov = "icon-save-hover";
    wxString m_iconName_save_dis = "icon-save-disable";
    wxString m_iconName_undo_nor = "icon-undo-nor"    ;
    wxString m_iconName_undo_hov = "icon-undo-hover"  ;
    wxString m_iconName_undo_dis = "icon-undo-disable";
    wxString m_iconName_redo_nor = "icon-redo-nor"    ;
    wxString m_iconName_redo_hov = "icon-redo-hover"  ;
    wxString m_iconName_redo_dis = "icon-redo-disable";

    wxString m_iconName_PA_nor = "pressureAdvance-nor"    ;
    wxString m_iconName_PA_hover = "pressureAdvance-hover";
    wxString m_iconName_PA_dis = "pressureAdvance-disable";

    wxString m_iconName_open_nor = "icon-configuration_manage-nor"    ;
    wxString m_iconName_open_dis = "icon-configuration_manage-disable";

    ACSplitLine* splitLine = new ACSplitLine(this, ACSplitLine::SplitHorizontal, 20, 2);
    splitLine->setLineColour(wxColour(197, 205, 219));
    splitLine->setLinePadding(30);

    //splitLine->SetCornerRadius(0);
    //splitLine->SetBorderColor(ACStateColor(
    //    std::make_pair(AC_COLOR_BLACK_DISABLE, (int) ACStateColor::Disabled), 
    //    //std::make_pair(AC_COLOR_MAIN_BLUE_HOVER, (int) ACStateColor::Hovered), 
    //    std::make_pair(AC_COLOR_BLACK, (int) ACStateColor::Normal))
    //);

    m_btImport    = new ACButton(this, _L("Import"),  m_iconName_import ,  m_iconName_import , m_iconName_import , wxNO_BORDER, wxSize(20,20));
    m_btSave      = new ACButton(this, "",  m_iconName_save_nor ,  m_iconName_save_hov , m_iconName_save_dis , wxNO_BORDER, wxSize(32,32));
    m_undo_item   = new ACButton(this, "",  m_iconName_undo_nor ,  m_iconName_undo_hov , m_iconName_undo_dis , wxNO_BORDER, wxSize(32,32));
    m_redo_item   = new ACButton(this, "",  m_iconName_redo_nor ,  m_iconName_redo_hov , m_iconName_redo_dis , wxNO_BORDER, wxSize(32,32));
	m_cloud       = new ACButton(this, _L("Login to begin remote print"), "icon-cloud-login", "icon-cloud-login", "icon-cloud-login", wxNO_BORDER, wxSize(16, 16));
    m_paTest_item = new ACButton(this, "", m_iconName_PA_nor, m_iconName_PA_hover, m_iconName_PA_dis, wxNO_BORDER, wxSize(32, 32));
    m_gauge       = new ACGauge(this, wxID_ANY, wxDefaultPosition, wxSize(32, 32));
    m_config_item = new ACButton(this, _L("Configuration Manage"),  m_iconName_open_nor ,  m_iconName_open_nor , m_iconName_open_dis , 0, wxSize(20,20));


	m_line        = new wxStaticLine(this, wxID_ANY, wxDefaultPosition, { wxDefaultCoord, 255 }, wxLI_VERTICAL);
	m_line->SetBackgroundColour(wxColour(200, 200, 200));
	m_line->SetMinSize(FromDIP(wxSize(3, 30)));


    m_btImport   ->SetPaddingSize(wxSize(10,6));
    m_btSave     ->SetPaddingSize(wxSize(0,0));
    m_undo_item  ->SetPaddingSize(wxSize(0,0));
    m_redo_item  ->SetPaddingSize(wxSize(0,0));
	m_cloud      ->SetPaddingSize(wxSize(6, 0));
    m_paTest_item->SetPaddingSize(wxSize(5,5));
    m_config_item->SetPaddingSize(wxSize(10,6));
    
    m_btImport   ->SetSpacing(8);
    m_config_item->SetSpacing(8);

    m_btImport   ->SetButtonType(ACButton::AC_BUTTON_LV0);
    m_config_item->SetButtonType(ACButton::AC_BUTTON_LV2);

    //m_btSave   ->clearButtonColor();
    //m_undo_item->clearButtonColor();
    //m_redo_item->clearButtonColor();

    m_btSave   ->SetButtonType(ACButton::AC_BUTTON_ICON);
    m_undo_item->SetButtonType(ACButton::AC_BUTTON_ICON);
    m_redo_item->SetButtonType(ACButton::AC_BUTTON_ICON);
	m_cloud    ->SetButtonType(ACButton::AC_BUTTON_TYPE::AC_BUTTON_LV0);

    m_paTest_item->SetButtonType(ACButton::AC_BUTTON_ICON);

    //ACStateColor background_color (
    //    std::make_pair(AC_COLOR_BT_L0_BG_PRE, (int) ACStateColor::Checked|ACStateColor::Pressed),
    //    std::make_pair(AC_COLOR_BT_L0_BG_HOV, (int) ACStateColor::Hovered), 
    //    std::make_pair(AC_COLOR_BT_L0_BG_NOR, (int) ACStateColor::Normal));

    //ACStateColor boader_color = background_color;
    //ACStateColor text_color(
    //    std::make_pair(AC_COLOR_WHITE, (int) ACStateColor::Normal));

    //m_btImport->SetBackgroundColor(background_color);
    //m_btImport->SetBorderColor(boader_color);
    //m_btImport->SetTextColor(text_color);

    //background_color = ACStateColor (
    //    std::make_pair(AC_COLOR_BT_L2_BG_PRE, (int) ACStateColor::Checked|ACStateColor::Pressed),
    //    std::make_pair(AC_COLOR_BT_L2_BG_HOV, (int) ACStateColor::Hovered), 
    //    std::make_pair(AC_COLOR_BT_L2_BG_NOR, (int) ACStateColor::Normal));

    //boader_color = ACStateColor (
    //    std::make_pair(AC_COLOR_BT_L2_BD_PRE, (int) ACStateColor::Checked|ACStateColor::Pressed),
    //    std::make_pair(AC_COLOR_BT_L2_BD_HOV, (int) ACStateColor::Hovered), 
    //    std::make_pair(AC_COLOR_BT_L2_BD_NOR, (int) ACStateColor::Normal));
    //text_color = ACStateColor (
    //    std::make_pair(AC_COLOR_BLACK, (int) ACStateColor::Normal));

    //m_config_item->SetBackgroundColor(background_color);
    //m_config_item->SetBorderColor(boader_color);
    //m_config_item->SetTextColor(text_color);
	m_cloud->SetMinSize(wxSize(204, 30));
	//m_cloud->SetAlignCenter(false);
	ACStateColor fgColor;
	fgColor.append(wxColour(35, 39, 44),    ACStateColor::Pressed);
	fgColor.append(wxColour(255, 255, 255), ACStateColor::Hovered);
	fgColor.append(wxColour(255, 255, 255), ACStateColor::Normal);
	m_cloud->SetTextColor(fgColor);
	m_cloud->SetCornerRadius(6);
	ACStateColor bgColor;
	bgColor.append(AC_COLOR_BT_L0_BG_NOR,   ACStateColor::Disabled);
	bgColor.append(wxColour(235, 239, 244), ACStateColor::Pressed);
	bgColor.append(wxColour(242, 196, 34),  ACStateColor::Hovered);
	bgColor.append(wxColour(242, 196, 34),  ACStateColor::Normal);
	m_cloud->SetBackgroundColor(bgColor);
	


	wxCursor cursor(wxCURSOR_HAND);
	m_cloud->SetCursor(cursor);

    wxBoxSizer* mainSizer = new wxBoxSizer(wxHORIZONTAL);
    mainSizer->AddSpacer(10);
    mainSizer->Add(m_btImport , 0, wxALIGN_CENTRE_VERTICAL|wxTOP|wxBOTTOM,12);

    mainSizer->Add(splitLine, 0, wxALIGN_CENTRE_VERTICAL);

    mainSizer->Add(m_btSave   , 0, wxALIGN_CENTRE_VERTICAL);
    mainSizer->AddSpacer(16);
    mainSizer->Add(m_undo_item, 0, wxALIGN_CENTRE_VERTICAL);
    mainSizer->AddSpacer(16);
    mainSizer->Add(m_redo_item, 0, wxALIGN_CENTRE_VERTICAL);
    mainSizer->AddSpacer(32);

    mainSizer->Add(m_paTest_item, 0, wxALIGN_CENTRE_VERTICAL);


	//mainSizer->AddSpacer(100);

    mainSizer->AddStretchSpacer();

	

    mainSizer->Add(m_gauge, 0, wxALIGN_CENTRE_VERTICAL);
    mainSizer->AddSpacer(10);
    mainSizer->Add(m_config_item, 0, wxALIGN_CENTRE_VERTICAL);
    mainSizer->AddSpacer(10);
	mainSizer->Add(m_line, 0, wxALIGN_CENTRE_VERTICAL);
	mainSizer->AddSpacer(6);
	mainSizer->Add(m_cloud, 0, wxALIGN_CENTRE_VERTICAL);
	mainSizer->AddSpacer(6);

	//先隐藏跟云相关按钮
	m_line->Hide();
	m_cloud->Hide();

    SetSizer(mainSizer);
    Layout();

    m_btImport   ->Bind(wxEVT_BUTTON, &ACToolBar::OnAddToPlate      , this);
    m_btSave     ->Bind(wxEVT_BUTTON, &ACToolBar::OnSaveProject     , this);
    m_undo_item  ->Bind(wxEVT_BUTTON, &ACToolBar::OnUndo            , this);
    m_redo_item  ->Bind(wxEVT_BUTTON, &ACToolBar::OnRedo            , this);
	m_cloud      ->Bind(wxEVT_BUTTON, &ACToolBar::OnCloud           , this);
    m_paTest_item->Bind(wxEVT_BUTTON, &ACToolBar::OnPressureAdvanceDialog, this);
    m_config_item->Bind(wxEVT_BUTTON, &ACToolBar::OnOpenConfigDialog, this);

    m_undo_item->Bind(wxEVT_KEY_DOWN, [](wxKeyEvent &evt) {
        if (evt.GetKeyCode() == WXK_SPACE || evt.GetKeyCode() == WXK_RETURN){
            return;
        }
        evt.Skip();
    });
    m_redo_item->Bind(wxEVT_KEY_DOWN, [](wxKeyEvent &evt) {
        if (evt.GetKeyCode() == WXK_SPACE || evt.GetKeyCode() == WXK_RETURN) {
            return;
        }
        evt.Skip();
    });

    this->Bind(wxEVT_MOTION, &ACToolBar::OnMouseMotion, this);
    this->Bind(wxEVT_LEFT_DOWN, &ACToolBar::OnMouseLeftDown, this);
    this->Bind(wxEVT_LEFT_UP, &ACToolBar::OnMouseLeftUp, this);
}

ACToolBar::~ACToolBar()
{
}
//
//void ACToolBar::OnOpenProject(wxCommandEvent& event)
//{
//    MainFrame* main_frame = dynamic_cast<MainFrame*>(m_frame);
//    Plater* plater = main_frame->plater();
//    plater->load_project();
//}

void ACToolBar::OnAddToPlate(wxCommandEvent& event)
{
    MainFrame* main_frame = dynamic_cast<MainFrame*>(m_frame);
    Plater* plater = main_frame->plater();
    plater->add_model();
}

void ACToolBar::OnSaveProject(wxCommandEvent& event)
{
    MainFrame* main_frame = dynamic_cast<MainFrame*>(m_frame);
    Plater* plater = main_frame->plater();
    plater->save_project_if_dirty("",false);
}

void ACToolBar::OnUndo(wxCommandEvent& event)
{
    MainFrame* main_frame = dynamic_cast<MainFrame*>(m_frame);
    Plater* plater = main_frame->plater();
    plater->undo();
}

void ACToolBar::OnRedo(wxCommandEvent& event)
{
    MainFrame* main_frame = dynamic_cast<MainFrame*>(m_frame);
    Plater* plater = main_frame->plater();
    plater->redo();
}

void ACToolBar::OnCloud(wxCommandEvent& event)
{
	if (GUI::wxGetApp().is_cloud_login())//账号已登录
	{
		MainFrame* main_frame = dynamic_cast<MainFrame*>(m_frame);
		if (main_frame)
			main_frame->select_main_panel(1);
	}
	else//账号未登录
	{
		ACCloudLoginDialog dlg(nullptr);
		dlg.ShowModal();
	}
	
}

void ACToolBar::OnOpenConfigDialog(wxCommandEvent& event)
{
    MainFrame* main_frame = dynamic_cast<MainFrame*>(m_frame);
    main_frame->select_tab(size_t(1));
}

void ACToolBar::OnPressureAdvanceDialog(wxCommandEvent &event)
{
    MainFrame *main_frame = dynamic_cast<MainFrame *>(m_frame);

    main_frame->ShowPressureAdvanceDialog();
}

void ACToolBar::UpdateToolbarWidth(int width)
{
    this->SetSize(width, (m_toolbar_h));
}

void ACToolBar::Rescale() {

    m_btImport   ->Rescale();
    m_btSave     ->Rescale();
    m_undo_item  ->Rescale();
    m_redo_item  ->Rescale();
    m_paTest_item->Rescale();
    m_config_item->Rescale();
    m_gauge->Rescale();

}
void ACToolBar::OnMouseLeftDown(wxMouseEvent &event)
{
    wxPoint mouse_pos = ::wxGetMousePosition();
    wxPoint frame_pos = m_frame->GetScreenPosition();
    m_delta           = mouse_pos - frame_pos;

    if (!m_frame->IsMaximized()) {
        CaptureMouse();
    }

    event.Skip();
}

void ACToolBar::OnMouseLeftUp(wxMouseEvent &event)
{
    wxPoint mouse_pos = ::wxGetMousePosition();
    if (HasCapture()) {
        ReleaseMouse();
    }

    event.Skip();
}

void ACToolBar::OnMouseMotion(wxMouseEvent &event)
{
    wxPoint mouse_pos = ::wxGetMousePosition();

    if (!HasCapture()) {
        // m_frame->OnMouseMotion(event);
        event.Skip();
        return;
    }

    if (event.Dragging() && event.LeftIsDown()) {
        m_frame->Move(mouse_pos - m_delta);
    }
    event.Skip();
}

