#include "MainFrame.hpp"

#include <wx/panel.h>
#include <wx/notebook.h>
#include <wx/listbook.h>
#include <wx/simplebook.h>
#include <wx/icon.h>
#include <wx/sizer.h>
#include <wx/menu.h>
#include <wx/progdlg.h>
#include <wx/tooltip.h>
//#include <wx/glcanvas.h>
#include <wx/filename.h>
#include <wx/debug.h>
#include <wx/glcanvas.h>

#include <boost/algorithm/string/predicate.hpp>
#include <boost/algorithm/string/replace.hpp>
#include <boost/log/trivial.hpp>

#include "libslic3r/Print.hpp"
#include "libslic3r/Preset.hpp"
#include "libslic3r/Polygon.hpp"
#include "libslic3r/SLAPrint.hpp"
#include "libslic3r/PresetBundle.hpp"

#include "Tab.hpp"
#include "ProgressStatusBar.hpp"
#include "3DScene.hpp"
#include "PrintHostDialogs.hpp"
#include "wxExtensions.hpp"
#include "GUI_ObjectList.hpp"
#include "Mouse3DController.hpp"
#include "RemovableDriveManager.hpp"
#include "InstanceCheck.hpp"
#include "I18N.hpp"
#include "GLCanvas3D.hpp"
#include "Plater.hpp"
#include "../Utils/Process.hpp"
#include "format.hpp"

#include <fstream>
#include <string_view>

#include "GUI_App.hpp"
#include "UnsavedChangesDialog.hpp"
#include "MsgDialog.hpp"
#include "Notebook.hpp"
#include "GUI_Factories.hpp"
#include "GalleryDialog.hpp"
#include "NotificationManager.hpp"
#include "ACPreferences.hpp"
#include "wx/graphics.h"

#include <wx/txtstrm.h>

#include "ACWebDialog.hpp"
#include "Cloud/ACCloudSelectMachine.hpp"
#include "Cloud/ACCloudLogin.hpp"
#include "Cloud/ACCloudManger.hpp"

#ifdef _WIN32
#include <dbt.h>
#include <shlobj.h>
#endif // _WIN32

namespace Slic3r {
namespace GUI {

enum class ERescaleTarget
{
    Mainframe,
    SettingsDialog
};

#ifdef __APPLE__
class PrusaSlicerTaskBarIcon : public wxTaskBarIcon
{
public:
    PrusaSlicerTaskBarIcon(wxTaskBarIconType iconType = wxTBI_DEFAULT_TYPE) : wxTaskBarIcon(iconType) {}
    wxMenu *CreatePopupMenu() override {
        wxMenu *menu = new wxMenu;
        if(wxGetApp().app_config->get("single_instance") == "0") {
            // Only allow opening a new Anycubic Slicer instance on OSX if "single_instance" is disabled,
            // as starting new instances would interfere with the locking mechanism of "single_instance" support.
            append_menu_item(menu, wxID_ANY, _L("Open new instance"), _L("Open a new AnycubicSlicer instance"),
            [](wxCommandEvent&) { start_new_slicer(); }, "", nullptr);
        }
        append_menu_item(menu, wxID_ANY, _L("G-code preview") + dots, _L("Open G-code viewer"),
            [](wxCommandEvent&) { start_new_gcodeviewer_open_file(); }, "", nullptr);
        return menu;
    }
};
class GCodeViewerTaskBarIcon : public wxTaskBarIcon
{
public:
    GCodeViewerTaskBarIcon(wxTaskBarIconType iconType = wxTBI_DEFAULT_TYPE) : wxTaskBarIcon(iconType) {}
    wxMenu *CreatePopupMenu() override {
        wxMenu *menu = new wxMenu;
        append_menu_item(menu, wxID_ANY, _L("Open AnycubicSlicer"), _L("Open a new AnycubicSlicer instance"),
            [](wxCommandEvent&) { start_new_slicer(nullptr, true); }, "", nullptr);
        append_menu_item(menu, wxID_ANY, _L("G-code preview") + dots, _L("Open new G-code viewer"),
            [](wxCommandEvent&) { start_new_gcodeviewer_open_file(); }, "", nullptr);
        return menu;
    }
};
#endif // __APPLE__

// Load the icon either from the exe, or from the ico file.
static wxIcon main_frame_icon(GUI_App::EAppMode app_mode)
{
#if _WIN32
    std::wstring path(size_t(MAX_PATH), wchar_t(0));
    int len = int(::GetModuleFileName(nullptr, path.data(), MAX_PATH));
    if (len > 0 && len < MAX_PATH) {
        path.erase(path.begin() + len, path.end());
        if (app_mode == GUI_App::EAppMode::GCodeViewer) {
            // Only in case the slicer was started with --gcodeviewer parameter try to load the icon from AnycubicSlicer-gcodeviewer.exe
            // Otherwise load it from the exe.
            for (const std::wstring_view exe_name : { std::wstring_view(L"Anycubic-Slicer.exe"), std::wstring_view(L"Anycubic-Slicer-console.exe") })
                if (boost::iends_with(path, exe_name)) {
                    path.erase(path.end() - exe_name.size(), path.end());
                    path += L"AnycubicSlicer-gcodeviewer.exe";
                    break;
                }
        }
    }
    return wxIcon(path, wxBITMAP_TYPE_ICO);
#else // _WIN32
    return wxIcon(Slic3r::var(app_mode == GUI_App::EAppMode::Editor ? "logo_128px.png" : "logo_128px.png"), wxBITMAP_TYPE_PNG);
#endif // _WIN32
}

MainFrame::~MainFrame()
{
    delete preferences_dialog;
    delete checkUpdate_dialog;
    if (m_ACSelectDialog != nullptr)
        delete m_ACSelectDialog;
    if (m_ACSAddDialog != nullptr)
        delete m_ACSAddDialog;
	wxDELETE(m_universal);
}

#ifndef __APPLE__
#define BORDERLESS_FRAME_STYLE (wxRESIZE_BORDER | wxMINIMIZE_BOX | wxMAXIMIZE_BOX | wxCLOSE_BOX)
#else
#define BORDERLESS_FRAME_STYLE (wxMINIMIZE_BOX | wxMAXIMIZE_BOX | wxCLOSE_BOX)
#endif

MainFrame::MainFrame(const int font_point_size) :
DPIFrame(NULL, wxID_ANY, "", wxDefaultPosition, wxDefaultSize, BORDERLESS_FRAME_STYLE, "mainframe", font_point_size),
    m_printhost_queue_dlg(new PrintHostQueueDialog(this))
    , m_recent_projects(9)
    , m_settings_dialog(this)
    , diff_dialog(this)
{
    // Fonts were created by the DPIFrame constructor for the monitor, on which the window opened.
    wxGetApp().update_fonts(this);
/*
#ifndef __WXOSX__ // Don't call SetFont under OSX to avoid name cutting in ObjectList 
    this->SetFont(this->normal_font());
#endif
    // Font is already set in DPIFrame constructor
*/

#ifdef __APPLE__
    auto panel_topbar = new wxPanel(this, wxID_ANY);
    panel_topbar->SetBackgroundColour(wxColour(38, 46, 48));
    auto sizer_tobar = new wxBoxSizer(wxVERTICAL);
    panel_topbar->SetSizer(sizer_tobar);
    panel_topbar->Layout();
#else
    m_topbar         = new ACTopbar(this);
#endif

    if (wxGetApp().is_editor())
        m_toolbar = new ACToolBar(this);

#ifdef __APPLE__
    // Initialize the docker task bar icon.
    switch (wxGetApp().get_app_mode()) {
    default:
    case GUI_App::EAppMode::Editor:
        m_taskbar_icon = std::make_unique<PrusaSlicerTaskBarIcon>(wxTBI_DOCK);
        m_taskbar_icon->SetIcon(wxIcon(Slic3r::var("logo_mac_128px.png"), wxBITMAP_TYPE_PNG), "AnycubicSlicer");
        break;
    case GUI_App::EAppMode::GCodeViewer:
        m_taskbar_icon = std::make_unique<GCodeViewerTaskBarIcon>(wxTBI_DOCK);
        m_taskbar_icon->SetIcon(wxIcon(Slic3r::var("logo_mac_128px.png"), wxBITMAP_TYPE_PNG), "G-code Viewer");
        break;
    }
#endif // __APPLE__

    // Load the icon either from the exe, or from the ico file.
    SetIcon(main_frame_icon(wxGetApp().get_app_mode()));

	// initialize status bar
//    m_statusbar = std::make_shared<ProgressStatusBar>(this);
//    m_statusbar->set_font(GUI::wxGetApp().normal_font());
//    if (wxGetApp().is_editor())
//        m_statusbar->embed(this);
//    m_statusbar->set_status_text(_L("Version") + " " +
//        SLIC3R_VERSION + " - " +
//       _L("Remember to check for updates at https://github.com/ANYCUBIC-3D/AnycubicSlicer/releases"));

    // initialize tabpanel and menubar

    init_tabpanel();

    m_recent_projects.SetMenuPathStyle(wxFH_PATH_SHOW_ALWAYS);
    if (wxGetApp().is_gcode_viewer())
        init_menubar_as_gcodeviewer();
    else
        init_menubar_as_editor();

#if _WIN32
    // This is needed on Windows to fake the CTRL+# of the window menu when using the numpad
    wxAcceleratorEntry entries[32];
    entries[0].Set(wxACCEL_CTRL, WXK_NUMPAD1, wxID_HIGHEST + 1);
    entries[1].Set(wxACCEL_CTRL, WXK_NUMPAD2, wxID_HIGHEST + 2);
    entries[2].Set(wxACCEL_CTRL, WXK_NUMPAD3, wxID_HIGHEST + 3);
    entries[3].Set(wxACCEL_CTRL, WXK_NUMPAD4, wxID_HIGHEST + 4);
    entries[4].Set(wxACCEL_CTRL, WXK_NUMPAD5, wxID_HIGHEST + 5);
    entries[5].Set(wxACCEL_CTRL, WXK_NUMPAD6, wxID_HIGHEST + 6);
    entries[6].Set(wxACCEL_CTRL, (int) 'N', wxID_HIGHEST + 7);
    entries[7].Set(wxACCEL_CTRL, (int) 'O', wxID_HIGHEST + 8);
    entries[8].Set(wxACCEL_CTRL, (int) 'S', wxID_HIGHEST + 9);
    entries[9].Set(wxACCEL_CTRL | wxACCEL_SHIFT, (int) 'S', wxID_HIGHEST + 10);
    entries[10].Set(wxACCEL_CTRL, (int) 'I', wxID_HIGHEST + 11);
    entries[11].Set(wxACCEL_CTRL, (int) 'L', wxID_HIGHEST + 12);
    entries[12].Set(wxACCEL_CTRL, (int) 'E', wxID_HIGHEST + 13);
    entries[13].Set(wxACCEL_CTRL, (int) 'T', wxID_HIGHEST + 14);
    entries[14].Set(wxACCEL_CTRL, (int) 'A', wxID_HIGHEST + 15);
    entries[15].Set(wxMOD_NONE, (int) WXK_ESCAPE, wxID_HIGHEST + 16);
    entries[16].Set(wxMOD_NONE, (int) WXK_DELETE, wxID_HIGHEST + 17);
    entries[17].Set(wxACCEL_CTRL, (int) WXK_DELETE, wxID_HIGHEST + 18);
    entries[18].Set(wxACCEL_CTRL, (int) 'Z', wxID_HIGHEST + 19);
    entries[19].Set(wxACCEL_CTRL, (int) 'Y', wxID_HIGHEST + 20);
    entries[20].Set(wxACCEL_CTRL, (int) 'C', wxID_HIGHEST + 21);
    entries[21].Set(wxACCEL_CTRL, (int) 'V', wxID_HIGHEST + 22);
    entries[22].Set(wxACCEL_CTRL, (int) 'P', wxID_HIGHEST + 23);
    entries[23].Set(wxMOD_NONE, (int) WXK_NUMPAD0, wxID_HIGHEST + 24);
    entries[24].Set(wxMOD_NONE, (int) WXK_NUMPAD1, wxID_HIGHEST + 25);
    entries[25].Set(wxMOD_NONE, (int) WXK_NUMPAD2, wxID_HIGHEST + 26);
    entries[26].Set(wxMOD_NONE, (int) WXK_NUMPAD3, wxID_HIGHEST + 27);
    entries[27].Set(wxMOD_NONE, (int) WXK_NUMPAD4, wxID_HIGHEST + 28);
    entries[28].Set(wxMOD_NONE, (int) WXK_NUMPAD5, wxID_HIGHEST + 29);
    entries[29].Set(wxMOD_NONE, (int) WXK_NUMPAD6, wxID_HIGHEST + 30);
    entries[30].Set(wxMOD_NONE, (int) 'E', wxID_HIGHEST + 31);
    entries[31].Set(wxMOD_NONE, (int) WXK_F11, wxID_HIGHEST + 32);
    wxAcceleratorTable accel(32, entries);
    SetAcceleratorTable(accel);
#endif // _WIN32

    // set default tooltip timer in msec
    // SetAutoPop supposedly accepts long integers but some bug doesn't allow for larger values
    // (SetAutoPop is not available on GTK.)
    wxToolTip::SetAutoPop(32767);

    m_loaded = true;

    // initialize layout
    m_main_sizer = new wxBoxSizer(wxVERTICAL);
    wxSizer* sizer = new wxBoxSizer(wxVERTICAL);
#ifdef __APPLE__
    sizer->Add(panel_topbar, 0, wxEXPAND);
#else
    sizer->Add(m_topbar, 0, wxEXPAND|wxTOP,1);
#endif // __WINDOWS__
    if (wxGetApp().is_editor())
        sizer->Add(m_toolbar, 0, wxEXPAND);
    sizer->Add(m_main_sizer, 1, wxEXPAND);
    SetSizer(sizer);
    // initialize layout from config
    update_layout();
    sizer->SetSizeHints(this);
    Fit();

    const wxSize min_size = wxGetApp().get_min_size(); //wxSize(76*wxGetApp().em_unit(), 49*wxGetApp().em_unit());
#ifdef __APPLE__
    // Using SetMinSize() on Mac messes up the window position in some cases
    // cf. https://groups.google.com/forum/#!topic/wx-users/yUKPBBfXWO0
    SetSize(min_size/*wxSize(760, 490)*/);
#else
    SetMinSize(min_size/*wxSize(760, 490)*/);
    SetSize(GetMinSize());
#endif
    Layout();

    update_title();
    Bind(wxEVT_SIZE, [this](wxSizeEvent& event) {
#ifdef __APPLE__
        wxRect screenSize = wxGetClientDisplayRect();
        wxSize windowSize = event.GetSize();
        if (windowSize.GetWidth() > screenSize.GetWidth() || windowSize.GetHeight() > screenSize.GetHeight()) {
            int maxWidth  = wxMin(windowSize.GetWidth(), screenSize.GetWidth());
            int maxHeight = wxMin(windowSize.GetHeight(), screenSize.GetHeight());
            SetSize(maxWidth, maxHeight);
            Layout();
        }
#endif

        m_plater->canvas3D()->on_size(event);
        
        event.Skip();
    });
    // declare events
    Bind(wxEVT_CLOSE_WINDOW, [this](wxCloseEvent& event) {
        if (event.CanVeto() && m_plater->canvas3D()->get_gizmos_manager().is_in_editing_mode(true)) {
            // prevents to open the save dirty project dialog
            event.Veto();
            if (!isRunDownFileIndex)
                return;
        }

        if (m_plater != nullptr) {
            int saved_project;
            if (m_online_ForceUpgrade) {
                saved_project = m_plater->save_project_forceupgrade_if_dirty(
                    format_wxstr(_L("Closing %1%. Current project is modified."), wxGetApp().appName()),m_online_ForceUpgrade);
            } else {
                saved_project = m_plater->save_project_if_dirty(
                    format_wxstr(_L("Closing %1%. Current project is modified."), wxGetApp().appName()));
            }
            if (saved_project == wxID_CANCEL && !m_online_ForceUpgrade) {
                event.Veto();
                if (!isRunDownFileIndex)
                    return;
            }
            // check unsaved changes only if project wasn't saved
            else if (!m_online_ForceUpgrade&&plater()->is_project_dirty() && saved_project == wxID_NO && event.CanVeto() &&
                     (plater()->is_presets_dirty() && !wxGetApp().check_and_save_current_preset_changes(format_wxstr(_L("%1% is closing"), wxGetApp().appName()), format_wxstr(_L("Closing %1% while some presets are modified."), wxGetApp().appName())))) {
                event.Veto();
                if (!isRunDownFileIndex)
                    return;
            }
        }

        if (event.CanVeto() && !wxGetApp().check_print_host_queue() && !m_online_ForceUpgrade) {
            event.Veto();
            if (!isRunDownFileIndex)
                return;
        }
        wxGetApp().CloseWizardObj();
        if (isRunDownFileIndex)
            wxGetApp().runUpdateFileEvent();
        this->shutdown();
        // propagate event
        event.Skip();
    });

    //FIXME it seems this method is not called on application start-up, at least not on Windows. Why?
    // The same applies to wxEVT_CREATE, it is not being called on startup on Windows.
    Bind(wxEVT_ACTIVATE, [this](wxActivateEvent& event) {
        if (m_plater != nullptr && event.GetActive())
            m_plater->on_activate();
        wxWindowList &children = this->GetChildren();
        for (wxWindowList::iterator it = children.begin(); it != children.end(); ++it) {
            wxWindow *child = *it;
            if (child->IsKindOf(wxClassInfo::FindClass(wxT("wxDialog")))) {
                wxDialog *dialog = wxDynamicCast(child, wxDialog);
                if (dialog && dialog->IsShown())
                    dialog->Raise();
            }
        }
        event.Skip();
    });

	Bind(EVT_ACCLOUD_LOGIN_SUCCESS, &MainFrame::OnCloudLoginSuccess, this);
	Bind(EVT_ACCLOUD_LOGOUT,        &MainFrame::OnCloudLogout,       this);
	Bind(EVT_ACCLOUD_PRINTER_LIST,  &MainFrame::OnCloudPrinterList,  this);
// OSX specific issue:
// When we move application between Retina and non-Retina displays, The legend on a canvas doesn't redraw
// So, redraw explicitly canvas, when application is moved
//FIXME maybe this is useful for __WXGTK3__ as well?
#if __APPLE__
    Bind(wxEVT_MOVE, [](wxMoveEvent& event) {
        wxGetApp().plater()->get_current_canvas3D()->set_as_dirty();
        wxGetApp().plater()->get_current_canvas3D()->request_extra_frame();
        event.Skip();
    });

    Bind(wxEVT_SIZE, [this](wxSizeEvent& event) {
        wxPoint curPos = this->GetPosition();
        bool titleIsOutofScreen = curPos.y < 0;
        if (titleIsOutofScreen)
            this->SetPosition(wxPoint(curPos.x, 20));
        event.Skip();
    });

#endif

    wxGetApp().persist_window_geometry(this, true);
    //wxGetApp().persist_window_geometry(&m_settings_dialog, false);

    update_ui_from_settings();    // FIXME (?)

    if (m_plater != nullptr) {
        //m_plater->get_collapse_toolbar().set_enabled(wxGetApp().app_config->get_bool("show_collapse_button"));
        //m_plater->show_action_buttons(true);

    }
    preferences_dialog = new ACPreferencesDialog(this);
    //checkUpdate_dialog = new ACCheckUpdate(this);
	checkUpdate_dialog =  new ACUpdateManger(this);
    // bind events from DiffDlg

    bind_diff_dialog();
#if _WIN32
    Bind(wxEVT_MENU, &MainFrame::OnShortcut, this, wxID_HIGHEST + 7, wxID_HIGHEST + 32);
    Bind(wxEVT_MENU, &MainFrame::OnShortcut, this, wxID_EXIT);
#endif // _WIN32
}

void MainFrame::CheckRunNumEvent(int num) {
    wxKeyEvent evt(wxEVT_CHAR);
    evt.m_keyCode = WXK_NUMPAD1;
    if (num == 0) {
        evt.m_uniChar = '0';
    } else if (num == 1) {
        evt.m_uniChar = '1';
    } else if (num == 2) {
        evt.m_uniChar = '2';
    } else if (num == 3) {
        evt.m_uniChar = '3';
    } else if (num == 4) {
        evt.m_uniChar = '4';
    } else if (num == 5) {
        evt.m_uniChar = '5';
    } else if (num == 6) {
        evt.m_uniChar = '6';
    }
    auto imgui    = wxGetApp().imgui();
    if (!imgui->update_key_data(evt)) {
        if (num == 0) {
            this->select_view("iso");
        } else if (num == 1) {
            this->select_view("top");
        } else if (num == 2) {
            this->select_view("bottom");
        } else if (num == 3) {
            this->select_view("front");
        } else if (num == 4) {
            this->select_view("rear");
        } else if (num == 5) {
            this->select_view("left");
        } else if (num == 6) {
            this->select_view("right");
        }

    }
}


#if _WIN32

void MainFrame::OnShortcut(wxCommandEvent &event)
{

    int id = event.GetId();

    switch (id) {
    case wxID_HIGHEST + 7:
        if (m_plater)
            m_plater->new_project();
        break;
    case wxID_HIGHEST + 8:
        if (m_plater)
            m_plater->load_project();
        break;
    case wxID_HIGHEST + 9: save_project();
        break;
    case wxID_HIGHEST + 10: save_project_as();
        break;
    case wxID_HIGHEST + 11:
        if (m_plater)
            m_plater->add_model();
        break;
    case wxID_HIGHEST + 12: load_config_file();
        break;
    case wxID_HIGHEST + 13: export_config();
        break;
    /*case wxID_HIGHEST + 14:
        if (m_plater)
            m_plater->eject_drive();
        break;*/
    case wxID_HIGHEST + 15: m_plater->select_all();
        break;
    case wxID_HIGHEST + 16: m_plater->deselect_all();
        break;
    case wxID_HIGHEST + 17: m_plater->remove_selected();
        break;
    case wxID_HIGHEST + 18: m_plater->reset_with_confirm();
        break;
    case wxID_HIGHEST + 19: m_plater->undo();
        break;
    case wxID_HIGHEST + 20: m_plater->redo();
        break;
    case wxID_HIGHEST + 21: {
        if (m_plater->canvas3D()->get_gizmos_manager().get_current_type() != GLGizmosManager::EType::Undefined) {
            wxKeyEvent evt(wxEVT_CHAR);
            int        keyIndex = 67;
            evt.m_keyCode       = 'C';
            evt.m_uniChar       = '\x3';
            auto imgui          = wxGetApp().imgui();
            wxGetApp().imgui()->SetKeyEvent(keyIndex, true);
            imgui->update_key_data(evt);
            m_plater->canvas3D()->render();
            wxGetApp().imgui()->SetKeyEvent(keyIndex, false);
        } else {
            m_plater->copy_selection_to_clipboard();
        }
        break;
    }
    case wxID_HIGHEST + 22: {
        if (m_plater->canvas3D()->get_gizmos_manager().get_current_type() != GLGizmosManager::EType::Undefined) {
            wxKeyEvent evt(wxEVT_CHAR);
            int        keyIndex = 86;
            evt.m_keyCode       = 'V';
            evt.m_uniChar       = '\x16';
            auto imgui          = wxGetApp().imgui();
            wxGetApp().imgui()->SetKeyEvent(keyIndex, true);
            imgui->update_key_data(evt);
            m_plater->canvas3D()->render();
            wxGetApp().imgui()->SetKeyEvent(keyIndex, false);
        } else {
            m_plater->paste_from_clipboard();
        }
        break;
    }
    case wxID_HIGHEST + 23:wxGetApp().open_preferences();
        break;
    case wxID_HIGHEST + 24: {
        CheckRunNumEvent(0);
        break;
    }
    case wxID_HIGHEST + 25: {
        CheckRunNumEvent(1);
        break;
    }
    case wxID_HIGHEST + 26: {
        CheckRunNumEvent(2);
        break;
    }
    case wxID_HIGHEST + 27: {
        CheckRunNumEvent(3);
        break;
    }
    case wxID_HIGHEST + 28: {
        CheckRunNumEvent(4);
        break;
    }
    case wxID_HIGHEST + 29: {
        CheckRunNumEvent(5);
        break;
    }
    case wxID_HIGHEST + 30: {
        CheckRunNumEvent(6);
        break;
    }
    case wxID_HIGHEST + 31: {
        m_plater->show_view3D_labels(!m_plater->are_view3D_labels_shown());
        m_plater->update();
        break;
    }
    /*case wxID_HIGHEST + 32: {
        bool opIndex = !this->IsFullScreen() && !this->IsMaximized();
        if (!opIndex && !this->IsFullScreen()) {
            this->Maximize(false);
        }else {
            this->ShowFullScreen(!this->IsFullScreen() && !this->IsMaximized(),
                                 wxFULLSCREEN_NOSTATUSBAR | wxFULLSCREEN_NOBORDER | wxFULLSCREEN_NOCAPTION);
        }
        this->m_topbar->ChangeIconStyle();
        break;
    }*/
    case wxID_EXIT: Close(false);
    default: break;
    }
    wxGetApp().imgui()->releaseCtrlKey();
}
#endif // _WIN32
void MainFrame::bind_diff_dialog()
{
    auto get_tab = [](Preset::Type type) {
        Tab* null_tab = nullptr;
        for (Tab* tab : wxGetApp().tabs_list)
            if (tab->type() == type)
                return tab;
        return null_tab;
    };

    auto transfer = [this, get_tab](Preset::Type type) {
        get_tab(type)->transfer_options(diff_dialog.get_left_preset_name(type),
                                        diff_dialog.get_right_preset_name(type),
                                        diff_dialog.get_selected_options(type));
    };

    auto update_presets = [this, get_tab](Preset::Type type) {
        get_tab(type)->update_preset_choice();
        //m_plater->sidebar().update_presets(type);
    };

    auto process_options = [this](std::function<void(Preset::Type)> process) {
        const Preset::Type diff_dlg_type = diff_dialog.view_type();
        if (diff_dlg_type == Preset::TYPE_INVALID) {
            for (const Preset::Type& type : diff_dialog.types_list() )
                process(type);
        }
        else
            process(diff_dlg_type);
    };

    diff_dialog.Bind(EVT_DIFF_DIALOG_TRANSFER,      [process_options, transfer](SimpleEvent&)         { process_options(transfer); });

    diff_dialog.Bind(EVT_DIFF_DIALOG_UPDATE_PRESETS,[process_options, update_presets](SimpleEvent&)   { process_options(update_presets); });
}

void MainFrame::OnCloudLoginSuccess(wxCommandEvent& event)
{
	select_main_panel(1);//show cloud printer...
}
void MainFrame::OnCloudLogout(wxCommandEvent& event)
{
	if (100 == event.GetInt())//succeed
	{
		select_main_panel(0);
		ACCloudLoginDialog dlg(nullptr);
		dlg.ShowModal();
	}
	else//failed put some log
	{
		wxString log = event.GetString();
	}
}
void MainFrame::OnCloudPrinterList(wxCommandEvent& event)
{
	if (100 == event.GetInt())
	{
		m_universal->UpdatePrinterList();//刷新打印机列表
	}
	else//failed
	{
		wxString log = event.GetString();
	}
}
// #ifdef _MSW_DARK_MODE
// static wxString pref() { return " [ "; }
// static wxString suff() { return " ] "; }
// static void append_tab_menu_items_to_menubar(wxMenuBar* bar, PrinterTechnology pt, bool is_mainframe_menu)
// {
//     if (is_mainframe_menu)
//         bar->Append(new wxMenu(), pref() + _L("Plater") + suff());
//     for (const wxString& title : { is_mainframe_menu    ? _L("Print Settings")       : pref() + _L("Print Settings") + suff(),
//                                    pt == ptSLA          ? _L("Material Settings")    : _L("Filament"),
//                                    _L("Printer") })
//         bar->Append(new wxMenu(), title);
// }

// // update markers for selected/unselected menu items
// static void update_marker_for_tabs_menu(wxMenuBar* bar, const wxString& title, bool is_mainframe_menu)
// {
//     if (!bar)
//         return;
//     size_t items_cnt = bar->GetMenuCount();
//     for (size_t id = items_cnt - (is_mainframe_menu ? 4 : 3); id < items_cnt; id++) {
//         wxString label = bar->GetMenuLabel(id);
//         if (label.First(pref()) == 0) {
//             if (label == pref() + title + suff())
//                 return;
//             label.Remove(size_t(0), pref().Len());
//             label.RemoveLast(suff().Len());
//             bar->SetMenuLabel(id, label);
//             break;
//         }
//     }
//     if (int id = bar->FindMenu(title); id != wxNOT_FOUND)
//         bar->SetMenuLabel(id, pref() + title + suff());
// }

// static void add_tabs_as_menu(wxMenuBar* bar, MainFrame* main_frame, wxWindow* bar_parent)
// {
//     PrinterTechnology pt = main_frame->plater() ? main_frame->plater()->printer_technology() : ptFFF;

//     bool is_mainframe_menu = bar_parent == main_frame;
//     if (!is_mainframe_menu)
//         append_tab_menu_items_to_menubar(bar, pt, is_mainframe_menu);

//     bar_parent->Bind(wxEVT_MENU_OPEN, [main_frame, bar, is_mainframe_menu](wxMenuEvent& event) {
//         wxMenu* const menu = event.GetMenu();
//         if (!menu || menu->GetMenuItemCount() > 0) {
//             // If we are here it means that we open regular menu and not a tab used as a menu
//             event.Skip(); // event.Skip() is verry important to next processing of the wxEVT_UPDATE_UI by this menu items.
//                           // If wxEVT_MENU_OPEN will not be pocessed in next event queue then MenuItems of this menu will never caught wxEVT_UPDATE_UI
//                           // and, as a result, "check/radio value" will not be updated
//             return;
//         }

//         // update tab selection

//         const wxString& title = menu->GetTitle();
//         if (title == _L("Plater"))
//             main_frame->select_tab(size_t(0));
//         else if (title == _L("Print Settings"))
//             main_frame->select_tab(wxGetApp().get_tab(main_frame->plater()->printer_technology() == ptFFF ? Preset::TYPE_PRINT : Preset::TYPE_SLA_PRINT));
//         else if (title == _L("Filament"))
//             main_frame->select_tab(wxGetApp().get_tab(Preset::TYPE_FILAMENT));
//         else if (title == _L("Material Settings"))
//             main_frame->select_tab(wxGetApp().get_tab(Preset::TYPE_SLA_MATERIAL));
//         else if (title == _L("Printer"))
//             main_frame->select_tab(wxGetApp().get_tab(Preset::TYPE_PRINTER));

//         // update markers for selected/unselected menu items
//         update_marker_for_tabs_menu(bar, title, is_mainframe_menu);
//     });
// }

// void MainFrame::show_tabs_menu(bool show)
// {
//     if (show)
//         append_tab_menu_items_to_menubar(m_menubar, plater() ? plater()->printer_technology() : ptFFF, true);
//     else
//         while (m_menubar->GetMenuCount() >= 8) {
//             if (wxMenu* menu = m_menubar->Remove(7))
//                 delete menu;
//         }
// }
// #endif // _MSW_DARK_MODE

void MainFrame::update_layout()
{
    auto restore_to_creation = [this]() {
        auto clean_sizer = [](wxSizer* sizer) {
            while (!sizer->GetChildren().IsEmpty()) {
                sizer->Detach(0);
            }
        };

        // On Linux m_plater needs to be removed from m_tabpanel before to reparent it
        int plater_page_id = m_tabpanel->FindPage(m_plater);
        if (plater_page_id != wxNOT_FOUND)
            m_tabpanel->RemovePage(plater_page_id);

        if (m_plater->GetParent() != this)
            m_plater->Reparent(this);

        if (m_tabpanel->GetParent() != this)
            m_tabpanel->Reparent(this);

        plater_page_id = (m_plater_page != nullptr) ? m_tabpanel->FindPage(m_plater_page) : wxNOT_FOUND;
        if (plater_page_id != wxNOT_FOUND) {
            m_tabpanel->DeletePage(plater_page_id);
            m_plater_page = nullptr;
        }

        clean_sizer(m_main_sizer);
        clean_sizer(m_settings_dialog.GetPanelSizer());

        /*if (m_settings_dialog.IsShown())
            m_settings_dialog.Close();*/
        m_settings_dialog.ShowModal();

        m_tabpanel->Hide();
        m_plater->Hide();

        Layout();
    };

    ESettingsLayout layout = wxGetApp().is_gcode_viewer()|| wxGetApp().plater()->acgcode_mode() == ACGcodeMode::ACGcode_Show ? ESettingsLayout::GCodeViewer : ESettingsLayout::Dlg;
    //    (wxGetApp().app_config->get("old_settings_layout_mode") == "1" ? ESettingsLayout::Old :
    //        wxGetApp().app_config->get("new_settings_layout_mode") == "1" ? ( wxGetApp().tabs_as_menu() ? ESettingsLayout::Old : ESettingsLayout::New) :
    //        wxGetApp().app_config->get("dlg_settings_layout_mode") == "1" ? ESettingsLayout::Dlg : ESettingsLayout::Old);
    //ESettingsLayout layout = ESettingsLayout::Dlg;

    if (m_layout == layout)
        return;

    wxBusyCursor busy;

    Freeze();

    // Remove old settings
    if (m_layout != ESettingsLayout::Unknown)
        restore_to_creation();

#ifdef __WXMSW__
    enum class State {
        noUpdate,
        fromDlg,
        toDlg
    };
    State update_scaling_state = m_layout == ESettingsLayout::Unknown   ? State::noUpdate   : // don't scale settings dialog from the application start
                                 m_layout == ESettingsLayout::Dlg       ? State::fromDlg    :
                                 layout   == ESettingsLayout::Dlg       ? State::toDlg      : State::noUpdate;
#endif //__WXMSW__

    ESettingsLayout old_layout = m_layout;
    m_layout = layout;

    // From the very beginning the Print settings should be selected
    m_last_selected_tab = m_layout == ESettingsLayout::Dlg ? 0 : 1;

    // Set new settings
    switch (m_layout)
    {
    case ESettingsLayout::Unknown:
    {
        break;
    }
//     case ESettingsLayout::Old:
//     {
//         m_plater->Reparent(m_tabpanel);
// // #ifdef _MSW_DARK_MODE
//         m_plater->Layout();
//         // if (!wxGetApp().tabs_as_menu())
//             dynamic_cast<Notebook*>(m_tabpanel)->InsertPage(0, m_plater, _L("Plater"), std::string("plater"), true);
// //         else
// // #endif
//         // m_tabpanel->InsertPage(0, m_plater, _L("Plater"));
//         m_main_sizer->Add(m_tabpanel, 1, wxEXPAND | wxTOP, 1);
//         m_plater->Show();
//         m_tabpanel->Show();
//         // update Tabs
//         if (old_layout == ESettingsLayout::Dlg)
//             if (int sel = m_tabpanel->GetSelection(); sel != wxNOT_FOUND)
//                 m_tabpanel->SetSelection(sel+1);// call SetSelection to correct layout after switching from Dlg to Old mode
// // #ifdef _MSW_DARK_MODE
// //         if (wxGetApp().tabs_as_menu())
// //             show_tabs_menu(true);
// // #endif
//         break;
//     }
//     case ESettingsLayout::New:
//     {
//         m_main_sizer->Add(m_plater, 1, wxEXPAND);
//         m_tabpanel->Hide();
//         m_main_sizer->Add(m_tabpanel, 1, wxEXPAND);
//         m_plater_page = new wxPanel(m_tabpanel);
// // #ifdef _MSW_DARK_MODE
// //         if (!wxGetApp().tabs_as_menu())
//             dynamic_cast<Notebook*>(m_tabpanel)->InsertPage(0, m_plater_page, _L("Plater"), std::string("plater"), true);
// //         else
// // #endif
// //         m_tabpanel->InsertPage(0, m_plater_page, _L("Plater")); // empty panel just for Plater tab */
//         m_plater->Show();
//         break;
//     }
    case ESettingsLayout::Dlg:
    {
		m_main_sizer->Add(m_universal, 1, wxEXPAND);
        m_main_sizer->Add(m_plater, 1, wxEXPAND);
        m_tabpanel->Reparent(&m_settings_dialog);

        m_settings_dialog.GetPanelSizer()->Add(m_tabpanel, 1, wxEXPAND);

        m_tabpanel->Show();
		m_universal->Hide();
        m_plater->Show();

        //m_settings_dialog.GetPanelSizer()->Layout();

        //m_settings_dialog.Fit();

        int win_width  = 99 * em_unit();
        int win_height = 66 * em_unit();
        m_settings_dialog.SetSize(wxSize(win_width, win_height));
        m_settings_dialog.Layout();



// #ifdef _MSW_DARK_MODE
//         if (wxGetApp().tabs_as_menu())
//             show_tabs_menu(false);
// #endif
        break;
    }
    case ESettingsLayout::GCodeViewer:
    {
        m_main_sizer->Add(m_plater, 1, wxEXPAND);
        m_plater->set_default_bed_shape();
        //m_plater->get_collapse_toolbar().set_enabled(false);
        //m_plater->collapse_sidebar(true);
        m_plater->Show();
        break;
    }
    }

// #ifdef _MSW_DARK_MODE
    // Sizer with buttons for mode changing
    // m_plater->sidebar().show_mode_sizer(wxGetApp().tabs_as_menu() || m_layout != ESettingsLayout::Old);
    //m_plater->sidebar().show_mode_sizer(m_layout != ESettingsLayout::Old);
// #endif

#ifdef __WXMSW__
    if (update_scaling_state != State::noUpdate)
    {
        int mainframe_dpi   = get_dpi_for_window(this);
        int dialog_dpi      = get_dpi_for_window(&m_settings_dialog);
        if (mainframe_dpi != dialog_dpi) {
            wxSize oldDPI = update_scaling_state == State::fromDlg ? wxSize(dialog_dpi, dialog_dpi) : wxSize(mainframe_dpi, mainframe_dpi);
            wxSize newDPI = update_scaling_state == State::toDlg   ? wxSize(dialog_dpi, dialog_dpi) : wxSize(mainframe_dpi, mainframe_dpi);

            if (update_scaling_state == State::fromDlg)
                this->enable_force_rescale();
            else
                (&m_settings_dialog)->enable_force_rescale();

            wxWindow* win { nullptr };
            if (update_scaling_state == State::fromDlg)
                win = this;
            else
                win = &m_settings_dialog;

#if wxVERSION_EQUAL_OR_GREATER_THAN(3,1,3)
            m_tabpanel->MSWUpdateOnDPIChange(oldDPI, newDPI);
            win->GetEventHandler()->AddPendingEvent(wxDPIChangedEvent(oldDPI, newDPI));
#else
            win->GetEventHandler()->AddPendingEvent(DpiChangedEvent(EVT_DPI_CHANGED_SLICER, newDPI, win->GetRect()));
#endif // wxVERSION_EQUAL_OR_GREATER_THAN
        }
    }
#endif //__WXMSW__

//#ifdef __APPLE__
//    // Using SetMinSize() on Mac messes up the window position in some cases
//    // cf. https://groups.google.com/forum/#!topic/wx-users/yUKPBBfXWO0
//    // So, if we haven't possibility to set MinSize() for the MainFrame, 
//    // set the MinSize() as a half of regular  for the m_plater and m_tabpanel, when settings layout is in slNew mode
//    // Otherwise, MainFrame will be maximized by height
//    if (m_layout == ESettingsLayout::New) {
//        wxSize size = wxGetApp().get_min_size();
//        size.SetHeight(int(0.5 * size.GetHeight()));
//        m_plater->SetMinSize(size);
//        m_tabpanel->SetMinSize(size);
//    }
//#endif

//#ifdef __APPLE__
//    m_plater->sidebar().change_top_border_for_mode_sizer(m_layout != ESettingsLayout::Old);
//#endif
    
    Layout();
    Thaw();
}

// Called when closing the application and when switching the application language.
void MainFrame::shutdown()
{
#ifdef _WIN32
	if (m_hDeviceNotify) {
		::UnregisterDeviceNotification(HDEVNOTIFY(m_hDeviceNotify));
		m_hDeviceNotify = nullptr;
	}
 	if (m_ulSHChangeNotifyRegister) {
        SHChangeNotifyDeregister(m_ulSHChangeNotifyRegister);
        m_ulSHChangeNotifyRegister = 0;
 	}
#endif // _WIN32

    if (m_plater != nullptr) {
        m_plater->get_ui_job_worker().cancel_all();

        // Unbinding of wxWidgets event handling in canvases needs to be done here because on MAC,
        // when closing the application using Command+Q, a mouse event is triggered after this lambda is completed,
        // causing a crash
        m_plater->unbind_canvas_event_handlers();

        // Cleanup of canvases' volumes needs to be done here or a crash may happen on some Linux Debian flavours
        // see: https://github.com/prusa3d/PrusaSlicer/issues/3964
        m_plater->reset_canvas_volumes();
    }

    // Weird things happen as the Paint messages are floating around the windows being destructed.
    // Avoid the Paint messages by hiding the main window.
    // Also the application closes much faster without these unnecessary screen refreshes.
    // In addition, there were some crashes due to the Paint events sent to already destructed windows.
    this->Show(false);

    if (m_settings_dialog.IsShown())
        // call Close() to trigger call to lambda defined into GUI_App::persist_window_geometry()
        m_settings_dialog.Close();

    if (m_plater != nullptr) {
        // Stop the background thread (Windows and Linux).
        // Disconnect from a 3DConnextion driver (OSX).
        m_plater->get_mouse3d_controller().shutdown();
        // Store the device parameter database back to appconfig.
        m_plater->get_mouse3d_controller().save_config(*wxGetApp().app_config);
    }

    // Stop the background thread of the removable drive manager, so that no new updates will be sent to the Plater.
    wxGetApp().removable_drive_manager()->shutdown();
	//stop listening for messages from other instances
	wxGetApp().other_instance_message_handler()->shutdown(this);
    // Save the slic3r.ini.Usually the ini file is saved from "on idle" callback,
    // but in rare cases it may not have been called yet.
    if (wxGetApp().app_config->dirty())
        wxGetApp().app_config->save();
//         if (m_plater)
//             m_plater->print = undef;
//         Slic3r::GUI::deregister_on_request_update_callback();

    // set to null tabs and a plater
    // to avoid any manipulations with them from App->wxEVT_IDLE after of the mainframe closing 
    wxGetApp().tabs_list.clear();
    wxGetApp().plater_ = nullptr;
}

GalleryDialog* MainFrame::gallery_dialog()
{
    if (!m_gallery_dialog)
        m_gallery_dialog = new GalleryDialog(this);
    return m_gallery_dialog;
}

void MainFrame::AC_ReplaceFirst(wxString &str, const wxString &searchStr, const wxString &replaceStr) 
{
    size_t pos = str.find(searchStr);
    if (pos != wxString::npos) {
        str.replace(pos, searchStr.length(), replaceStr);
    }
}

void MainFrame::update_title()
{
    wxString title = wxEmptyString;
    if (m_plater != nullptr) {
        // m_plater->get_project_filename() produces file name including path, but excluding extension.
        // Don't try to remove the extension, it would remove part of the file name after the last dot!
        wxString project = from_path(into_path(m_plater->get_project_filename()).filename());
//        wxString dirty_marker = (!m_plater->model().objects.empty() && m_plater->is_project_dirty()) ? "*" : "";
        wxString dirty_marker = m_plater->is_project_dirty() ? "*" : "";
        if (!dirty_marker.empty() || !project.empty()) {
            if (!dirty_marker.empty() && project.empty())
                project = _L("Untitled");
            title = dirty_marker + project + " - ";
        }
    }

    //std::string build_id = wxGetApp().buildID();
    //std::string build_id = from_u8(wxGetApp().buildID()).ToStdString();
    wxString build_id = format_wxstr(wxGetApp().buildID());
    if (!wxGetApp().is_editor())
        AC_ReplaceFirst(build_id, wxGetApp().is_region_CN() ? SLIC3R_APP_NAME_CN : SLIC3R_APP_NAME, wxGetApp().appName());
        /*boost::replace_first(
            build_id,
            wxString(wxGetApp().is_region_CN() ? SLIC3R_APP_NAME_CN : SLIC3R_APP_NAME).ToStdString(),
            wxString(wxGetApp().appName()).ToStdString()
        );*/
    size_t 		idx_plus = build_id.find('+');
    if (idx_plus != build_id.npos) {
    	// Parse what is behind the '+'. If there is a number, then it is a build number after the label, and full build ID is shown.
    	int commit_after_label;
        if (/*! boost::starts_with(build_id.data() + idx_plus + 1, "UNKNOWN")*/ !build_id.Mid(idx_plus + 1).StartsWith("UNKNOWN") && 
            (build_id.at(idx_plus + 1) == '-' || sscanf(build_id.data() + idx_plus + 1, "%d-", &commit_after_label) == 0)) {
    		// It is a release build.
    		build_id.erase(build_id.begin() + idx_plus, build_id.end());    		
#if defined(_WIN32) && ! defined(_WIN64)
    		// People are using 32bit slicer on a 64bit machine by mistake. Make it explicit.
            build_id += " 32 bit";
#endif
    	}
    }

    title += wxString(build_id);


#ifdef __APPLE__
    SetTitle(title);
#else
    m_topbar->SetTitle(title);
#endif // __WINDOWS__

}

void MainFrame::init_tabpanel()
{
    wxGetApp().update_ui_colours_from_appconfig();

    // wxNB_NOPAGETHEME: Disable Windows Vista theme for the Notebook background. The theme performance is terrible on Windows 10
    // with multiple high resolution displays connected.
// #ifdef _MSW_DARK_MODE
//     if (wxGetApp().tabs_as_menu()) {
//         m_tabpanel = new wxSimplebook(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxNB_TOP | wxTAB_TRAVERSAL | wxNB_NOPAGETHEME);
//         wxGetApp().UpdateDarkUI(m_tabpanel);
//     }
//     else
        m_tabpanel = new Notebook(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxNB_TOP | wxTAB_TRAVERSAL | wxNB_NOPAGETHEME);
// #else
//     m_tabpanel = new wxNotebook(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxNB_TOP | wxTAB_TRAVERSAL | wxNB_NOPAGETHEME);
// #endif

#ifndef __WXOSX__ // Don't call SetFont under OSX to avoid name cutting in ObjectList
    m_tabpanel->SetFont(Slic3r::GUI::wxGetApp().normal_font());
#endif
    m_tabpanel->Hide();
    m_settings_dialog.set_tabpanel(m_tabpanel);

#ifdef __WXMSW__
    m_tabpanel->Bind(wxEVT_BOOKCTRL_PAGE_CHANGED, [this](wxBookCtrlEvent& e) {
#else
    m_tabpanel->Bind(wxEVT_NOTEBOOK_PAGE_CHANGED, [this](wxBookCtrlEvent& e) {
#endif
        if (int old_selection = e.GetOldSelection();
            old_selection != wxNOT_FOUND && old_selection < static_cast<int>(m_tabpanel->GetPageCount())) {
            Tab* old_tab = dynamic_cast<Tab*>(m_tabpanel->GetPage(old_selection));
            if (old_tab)
                old_tab->validate_custom_gcodes();
        }

        wxWindow* panel = m_tabpanel->GetCurrentPage();
        Tab* tab = dynamic_cast<Tab*>(panel);

        // There shouldn't be a case, when we try to select a tab, which doesn't support a printer technology
        if (panel == nullptr || (tab != nullptr && !tab->supports_printer_technology(m_plater->printer_technology())))
            return;

        auto& tabs_list = wxGetApp().tabs_list;
        if (tab && std::find(tabs_list.begin(), tabs_list.end(), tab) != tabs_list.end()) {
            // On GTK, the wxEVT_NOTEBOOK_PAGE_CHANGED event is triggered
            // before the MainFrame is fully set up.
            tab->OnActivate();
            m_last_selected_tab = m_tabpanel->GetSelection();
// #ifdef _MSW_DARK_MODE
//             if (wxGetApp().tabs_as_menu())
//                 tab->SetFocus();
// #endif
        }
        else
            select_tab(size_t(0)); // select Plater
    });

    m_plater = new Plater(this, this);
    m_plater->Hide();

    wxGetApp().plater_ = m_plater;

	m_universal = new Universal(this, this);
	m_universal->Hide();
	wxGetApp().universal_ = m_universal;

    if (wxGetApp().is_editor())
        create_preset_tabs();

    if (m_plater) {
        // load initial config
        auto full_config = wxGetApp().preset_bundle->full_config();
        m_plater->on_config_change(full_config);

        // Show a correct number of filament fields.
        // nozzle_diameter is undefined when SLA printer is selected
        //if (full_config.has("nozzle_diameter")) {
        //    m_plater->on_extruders_change(full_config.option<ConfigOptionFloats>("nozzle_diameter")->values.size());
        //}
    }
}

#ifdef WIN32
void MainFrame::register_win32_callbacks()
{
    //static GUID GUID_DEVINTERFACE_USB_DEVICE  = { 0xA5DCBF10, 0x6530, 0x11D2, 0x90, 0x1F, 0x00, 0xC0, 0x4F, 0xB9, 0x51, 0xED };
    //static GUID GUID_DEVINTERFACE_DISK        = { 0x53f56307, 0xb6bf, 0x11d0, 0x94, 0xf2, 0x00, 0xa0, 0xc9, 0x1e, 0xfb, 0x8b };
    //static GUID GUID_DEVINTERFACE_VOLUME      = { 0x71a27cdd, 0x812a, 0x11d0, 0xbe, 0xc7, 0x08, 0x00, 0x2b, 0xe2, 0x09, 0x2f };
    static GUID GUID_DEVINTERFACE_HID           = { 0x4D1E55B2, 0xF16F, 0x11CF, 0x88, 0xCB, 0x00, 0x11, 0x11, 0x00, 0x00, 0x30 };

    // Register USB HID (Human Interface Devices) notifications to trigger the 3DConnexion enumeration.
    DEV_BROADCAST_DEVICEINTERFACE NotificationFilter = { 0 };
    NotificationFilter.dbcc_size = sizeof(DEV_BROADCAST_DEVICEINTERFACE);
    NotificationFilter.dbcc_devicetype = DBT_DEVTYP_DEVICEINTERFACE;
    NotificationFilter.dbcc_classguid = GUID_DEVINTERFACE_HID;
    m_hDeviceNotify = ::RegisterDeviceNotification(this->GetHWND(), &NotificationFilter, DEVICE_NOTIFY_WINDOW_HANDLE);

// or register for file handle change?
//      DEV_BROADCAST_HANDLE NotificationFilter = { 0 };
//      NotificationFilter.dbch_size = sizeof(DEV_BROADCAST_HANDLE);
//      NotificationFilter.dbch_devicetype = DBT_DEVTYP_HANDLE;

    // Using Win32 Shell API to register for media insert / removal events.
    LPITEMIDLIST ppidl;
    if (SHGetSpecialFolderLocation(this->GetHWND(), CSIDL_DESKTOP, &ppidl) == NOERROR) {
        SHChangeNotifyEntry shCNE;
        shCNE.pidl       = ppidl;
        shCNE.fRecursive = TRUE;
        // Returns a positive integer registration identifier (ID).
        // Returns zero if out of memory or in response to invalid parameters.
        m_ulSHChangeNotifyRegister = SHChangeNotifyRegister(this->GetHWND(),        // Hwnd to receive notification
            SHCNE_DISKEVENTS,                                                       // Event types of interest (sources)
            SHCNE_MEDIAINSERTED | SHCNE_MEDIAREMOVED,
            //SHCNE_UPDATEITEM,                                                     // Events of interest - use SHCNE_ALLEVENTS for all events
            WM_USER_MEDIACHANGED,                                                   // Notification message to be sent upon the event
            1,                                                                      // Number of entries in the pfsne array
            &shCNE);                                                                // Array of SHChangeNotifyEntry structures that 
                                                                                    // contain the notifications. This array should 
                                                                                    // always be set to one when calling SHChnageNotifyRegister
                                                                                    // or SHChangeNotifyDeregister will not work properly.
        assert(m_ulSHChangeNotifyRegister != 0);    // Shell notification failed
    } else {
        // Failed to get desktop location
        assert(false); 
    }

    {
        static constexpr int device_count = 1;
        RAWINPUTDEVICE devices[device_count] = { 0 };
        // multi-axis mouse (SpaceNavigator, etc.)
        devices[0].usUsagePage = 0x01;
        devices[0].usUsage = 0x08;
        if (! RegisterRawInputDevices(devices, device_count, sizeof(RAWINPUTDEVICE)))
            BOOST_LOG_TRIVIAL(error) << "RegisterRawInputDevices failed";
    }
}
#endif // _WIN32

void MainFrame::create_preset_tabs()
{
    //add_created_tab(new TabPrint(m_tabpanel), "icon-setting-click");

    //// add_created_tab(new TabPrint(m_tabpanel), "icon-machine-click");
    //add_created_tab(new TabFilament(m_tabpanel), "icon-material-click");
    //add_created_tab(new TabPrinter(m_tabpanel), wxGetApp().preset_bundle->printers.get_edited_preset().printer_technology() == ptFFF ?
    //                                                "icon-machine-click" :
    //                                                "ACEmpty");

    add_created_tab(new TabPrint(m_tabpanel),"icon-tab_setting-off", "icon-tab_setting-hover_or_sel");

    add_created_tab(new TabFilament(m_tabpanel),"icon-tab_material-off", "icon-tab_material-hover_or_sel");
    add_created_tab(new TabPrinter(m_tabpanel),
                    wxGetApp().preset_bundle->printers.get_edited_preset().printer_technology() == ptFFF ? "icon-tab_machine-off" : "ACEmpty",
                    wxGetApp().preset_bundle->printers.get_edited_preset().printer_technology() == ptFFF ? "icon-tab_machine-hover_or_sel" : "ACEmpty");
    // add_created_tab(new TabSLAPrint(m_tabpanel), "ACEmpty");
    // add_created_tab(new TabSLAMaterial(m_tabpanel), "ACEmpty");
    // add_created_tab(new TabPrinter(m_tabpanel), wxGetApp().preset_bundle->printers.get_edited_preset().printer_technology() == ptFFF ?
    // "Icon-setting-click" : "ACEmpty");

}
void MainFrame::add_created_tab(Tab *panel, const std::string &bmp_name /*= ""*/, const std::string &click_bmp_name /*= ""*/)
{
    panel->create_preset_tab();

    const auto printer_tech = wxGetApp().preset_bundle->printers.get_edited_preset().printer_technology();

    if (panel->supports_printer_technology(printer_tech))
// #ifdef _MSW_DARK_MODE
//         if (!wxGetApp().tabs_as_menu())
            dynamic_cast<Notebook *>(m_tabpanel)->AddPage(panel, panel->title(), bmp_name, click_bmp_name);
//         else
// #endif
//             m_tabpanel->AddPage(panel, panel->title());
    m_tabpanel->Layout();
}
void MainFrame::add_created_tab(Tab* panel,  const std::string& bmp_name /*= ""*/)
{
    panel->create_preset_tab();

    const auto printer_tech = wxGetApp().preset_bundle->printers.get_edited_preset().printer_technology();

    if (panel->supports_printer_technology(printer_tech))
// #ifdef _MSW_DARK_MODE
//         if (!wxGetApp().tabs_as_menu())
            dynamic_cast<Notebook*>(m_tabpanel)->AddPage(panel, panel->title(), bmp_name);
//         else
// #endif
//         m_tabpanel->AddPage(panel, panel->title());
}

bool MainFrame::is_active_and_shown_tab(Tab* tab)
{
    int page_id = m_tabpanel->FindPage(tab);

    if (m_tabpanel->GetSelection() != page_id)
        return false;

    if (m_layout == ESettingsLayout::Dlg)
        return m_settings_dialog.IsShown();

    if (m_layout == ESettingsLayout::New)
        return m_main_sizer->IsShown(m_tabpanel);
    
    return true;
}

bool MainFrame::can_start_new_project() const
{
    return m_plater && (!m_plater->get_project_filename(".3mf").IsEmpty() || 
                        GetTitle().StartsWith('*')||
                        wxGetApp().has_current_preset_changes() || 
                        !m_plater->model().objects.empty() );
}

bool MainFrame::can_save() const
{
    return (m_plater != nullptr) &&
        !m_plater->canvas3D()->get_gizmos_manager().is_in_editing_mode(false) &&
        m_plater->is_project_dirty();
}

bool MainFrame::can_save_as() const
{
    return (m_plater != nullptr) &&
        !m_plater->canvas3D()->get_gizmos_manager().is_in_editing_mode(false);
}

void MainFrame::save_project()
{
    save_project_as(m_plater->get_project_filename(".3mf"));
}

bool MainFrame::save_project_as(const wxString& filename)
{
    bool ret = (m_plater != nullptr) ? m_plater->export_3mf(into_path(filename)) : false;
    if (ret) {
        // Make a copy of the active presets for detecting changes in preset values.
        wxGetApp().update_saved_preset_from_current_preset();
        // Save the names of active presets and project specific config into ProjectDirtyStateManager.
        // Reset ProjectDirtyStateManager's state as saved, mark active UndoRedo step as saved with project.
        m_plater->reset_project_dirty_after_save();
    }
    return ret;
}

bool MainFrame::can_export_model() const
{
    return (m_plater != nullptr) && !m_plater->model().objects.empty();
}

bool MainFrame::can_export_toolpaths() const
{
    return (m_plater != nullptr) && (m_plater->printer_technology() == ptFFF) && m_plater->is_preview_shown() && m_plater->is_preview_loaded() && m_plater->has_toolpaths_to_export();
}

bool MainFrame::can_export_supports() const
{
    if ((m_plater == nullptr) || (m_plater->printer_technology() != ptSLA) || m_plater->model().objects.empty())
        return false;

    bool can_export = false;
    const PrintObjects& objects = m_plater->sla_print().objects();
    for (const SLAPrintObject* object : objects)
    {
        if (!object->support_mesh().empty() || !object->pad_mesh().empty())
        {
            can_export = true;
            break;
        }
    }
    return can_export;
}

bool MainFrame::can_export_gcode() const
{
    if (m_plater == nullptr)
        return false;

    if (m_plater->model().objects.empty())
        return false;

    if (m_plater->is_export_gcode_scheduled())
        return false;

    // TODO:: add other filters

    return true;
}

bool MainFrame::can_send_gcode() const
{
    if (m_plater && ! m_plater->model().objects.empty())
        if (const DynamicPrintConfig *cfg = wxGetApp().preset_bundle->physical_printers.get_selected_printer_config(); cfg)
            if (const auto *print_host_opt = cfg->option<ConfigOptionString>("print_host"); print_host_opt)
                return ! print_host_opt->value.empty();
    return false;
}

bool MainFrame::can_export_gcode_sd() const
{
	if (m_plater == nullptr)
		return false;

	if (m_plater->model().objects.empty())
		return false;

	if (m_plater->is_export_gcode_scheduled())
		return false;

	// TODO:: add other filters

	return wxGetApp().removable_drive_manager()->status().has_removable_drives;
}

bool MainFrame::can_eject() const
{
	return wxGetApp().removable_drive_manager()->status().has_eject;
}

bool MainFrame::can_slice() const
{
    bool bg_proc = wxGetApp().app_config->get_bool("background_processing");
    return (m_plater != nullptr) ? !m_plater->model().objects.empty() && !bg_proc : false;
}

bool MainFrame::can_change_view() const
{
    switch (m_layout)
    {
    default:                   { return false; }
    case ESettingsLayout::New: { return m_plater->IsShown(); }
    case ESettingsLayout::Dlg: { return true; }
    case ESettingsLayout::Old: { 
        int page_id = m_tabpanel->GetSelection();
        return page_id != wxNOT_FOUND && dynamic_cast<const Slic3r::GUI::Plater*>(m_tabpanel->GetPage((size_t)page_id)) != nullptr;
    }
    case ESettingsLayout::GCodeViewer: { return true; }
    }
}

bool MainFrame::can_select() const
{
    return (m_plater != nullptr) && !m_plater->model().objects.empty();
}

bool MainFrame::can_deselect() const
{
    return (m_plater != nullptr) && !m_plater->is_selection_empty();
}

bool MainFrame::can_delete() const
{
    return (m_plater != nullptr) && !m_plater->is_selection_empty();
}

bool MainFrame::can_delete_all() const
{
    return (m_plater != nullptr) && !m_plater->model().objects.empty();
}

bool MainFrame::can_reslice() const
{
    return (m_plater != nullptr) && !m_plater->model().objects.empty();
}

void MainFrame::on_dpi_changed(const wxRect& suggested_rect)
{
    wxGetApp().update_fonts(this);
    this->SetFont(this->normal_font());

// #ifdef _MSW_DARK_MODE
//     // update common mode sizer
//     if (!wxGetApp().tabs_as_menu())
        dynamic_cast<Notebook*>(m_tabpanel)->Rescale();
// #endif

//#ifndef __APPLE__
//    // BBS
    if (m_topbar)
        m_topbar->Rescale();
    if (m_toolbar)
        m_toolbar->Rescale();
    //#endif

    // update Plater
    wxGetApp().plater()->msw_rescale();

    // update Tabs
    if (m_layout != ESettingsLayout::Dlg) // Do not update tabs if the Settings are in the separated dialog
        for (auto tab : wxGetApp().tabs_list)
            tab->msw_rescale();

    // Workarounds for correct Window rendering after rescale

    /* Even if Window is maximized during moving,
     * first of all we should imitate Window resizing. So:
     * 1. cancel maximization, if it was set
     * 2. imitate resizing
     * 3. set maximization, if it was set
     */
    const bool is_maximized = this->IsMaximized();
    if (is_maximized)
        this->Maximize(false);

    /* To correct window rendering (especially redraw of a status bar)
     * we should imitate window resizing.
     */
    const wxSize& sz = this->GetSize();
    this->SetSize(sz.x + 1, sz.y + 1);
    this->SetSize(sz);

    this->Maximize(is_maximized);
}

void MainFrame::on_sys_color_changed()
{
    wxBusyCursor wait;

    // update label colors in respect to the system mode
    wxGetApp().init_ui_colours();
    // but if there are some ui colors in appconfig, they have to be applied
    wxGetApp().update_ui_colours_from_appconfig();
#ifdef __WXMSW__
    wxGetApp().UpdateDarkUI(m_tabpanel);
 //   m_statusbar->update_dark_ui();
// #ifdef _MSW_DARK_MODE
//     // update common mode sizer
//     if (!wxGetApp().tabs_as_menu())
        dynamic_cast<Notebook*>(m_tabpanel)->OnColorsChanged();
// #endif
#endif

    // update Plater
    wxGetApp().plater()->sys_color_changed();

    // update Tabs
    for (auto tab : wxGetApp().tabs_list)
        tab->sys_color_changed();

    MenuFactory::sys_color_changed(m_menubar);

    this->Refresh();
}

void MainFrame::update_mode_markers()
{
    // update mode markers on side_bar
    //wxGetApp().sidebar().update_mode_markers();

    // update mode markers in tabs
    for (auto tab : wxGetApp().tabs_list)
        tab->update_mode_markers();
}

#ifdef _MSC_VER
    // \xA0 is a non-breaking space. It is entered here to spoil the automatic accelerators,
    // as the simple numeric accelerators spoil all numeric data entry.
//static const wxString sep = "\t\xA0";
static const wxString sep       = "\t";
static const wxString sep_space = "\xA0";
#else
//static const wxString sep = " - ";
static const wxString sep = "\t";
static const wxString sep_space = "";
#endif

static void run_wizard(ACConfigWizard::StartPage sp)
{
    wxGetApp().run_wizard(ACConfigWizard::RR_USER, sp);
}

static wxMenu *generate_help_menu(wxWindow *parent)
{
    wxMenu* helpMenu = new wxMenu();
    /*append_menu_item(helpMenu, wxID_ANY, _L("Prusa 3D &Drivers"), _L("Open the Prusa3D drivers download page in your browser"),
        [](wxCommandEvent&) { wxGetApp().open_web_page_localized("https://www.xxxxxxx.com/downloads"); });
    append_menu_item(helpMenu, wxID_ANY, _L("Software &Releases"), _L("Open the software releases page in your browser"),
        [](wxCommandEvent&) { wxGetApp().open_browser_with_warning_dialog("https://github.com/ANYCUBIC-3D/AnycubicSlicer/releases", nullptr, false); });*/
//#        my $versioncheck = $self->_append_menu_item($helpMenu, "Check for &Updates...", "Check for new Slic3r versions", sub{
//#            wxTheApp->check_version(1);
//#        });
//#        $versioncheck->Enable(wxTheApp->have_version_check);
    /*append_menu_item(helpMenu, wxID_ANY, wxString::Format(_L("%s &Website"), SLIC3R_APP_NAME),
        format_wxstr(_L("Open the %s website in your browser"), SLIC3R_APP_NAME),
        [](wxCommandEvent&) { wxGetApp().open_web_page_localized("https://www.xxxxxxx.com/slicerweb"); });*/
//        append_menu_item(helpMenu, wxID_ANY, wxString::Format(_L("%s &Manual"), SLIC3R_APP_NAME),
//                                             wxString::Format(_L("Open the %s manual in your browser"), SLIC3R_APP_NAME),
//            [this](wxCommandEvent&) { wxGetApp().open_browser_with_warning_dialog("http://manual.slic3r.org/"); });
    /*helpMenu->AppendSeparator();
    append_menu_item(helpMenu, wxID_ANY, _L("System &Info"), _L("Show system information"),
        [](wxCommandEvent&) { wxGetApp().system_info(); });
    append_menu_item(helpMenu, wxID_ANY, _L("Show &Configuration Folder"), _L("Show user configuration folder (datadir)"),
        [](wxCommandEvent&) { Slic3r::GUI::desktop_open_datadir_folder(); });
    append_menu_item(helpMenu, wxID_ANY, _L("Report an I&ssue"), wxString::Format(_L("Report an issue on %s"), SLIC3R_APP_NAME),
        [](wxCommandEvent&) { wxGetApp().open_browser_with_warning_dialog("https://github.com/xxxxxxx/slic3r/issues/new", nullptr, false); });*/


    const auto config_wizard_name = _(ACConfigWizard::name(true));
    const auto config_wizard_tooltip = from_u8((boost::format(_u8L(L("Run %s"))) % config_wizard_name).str());

    append_menu_item(helpMenu, wxID_ANY, config_wizard_name + dots, config_wizard_tooltip,
                     [](wxCommandEvent &) { wxGetApp().run_wizard(ACConfigWizard::RR_USER); });
    if (!wxGetApp().is_gcode_viewer()) {
        append_menu_item(helpMenu, wxID_ANY, _L("Check Update"), _L("Check Update"),
                         [](wxCommandEvent &) { wxGetApp().mainframe->checkUpdate_dialog->ShowCheckVersionDialog(); });
    }

	/*append_menu_item(helpMenu, wxID_ANY, wxString::Format(_L("Check Update")), _L("Show about dialog"),
		             [](wxCommandEvent&) { Slic3r::GUI::update(); });*/
    
    append_menu_item(helpMenu, wxID_ANY, format_wxstr(_L("About %1%"), wxGetApp().appName()), _L("Show about dialog"),
                     [parent](wxCommandEvent &) { Slic3r::GUI::about(parent); });
    //    append_menu_item(helpMenu, wxID_ANY, _L("Show Tip of the Day")
//#if 0//debug
//        + "\tCtrl+Shift+T"
//#endif
//        ,_L("Opens Tip of the day notification in bottom right corner or shows another tip if already opened."),
//        [](wxCommandEvent&) { wxGetApp().plater()->get_notification_manager()->push_hint_notification(false); });
//    helpMenu->AppendSeparator();
//    append_menu_item(helpMenu, wxID_ANY, _L("Keyboard Shortcuts") + sep + "&?", _L("Show the list of the keyboard shortcuts"),
//        [](wxCommandEvent&) { wxGetApp().keyboard_shortcuts(); });
//#if ENABLE_THUMBNAIL_GENERATOR_DEBUG
//    helpMenu->AppendSeparator();
//    append_menu_item(helpMenu, wxID_ANY, "DEBUG gcode thumbnails", "DEBUG ONLY - read the selected gcode file and generates png for the contained thumbnails",
//        [](wxCommandEvent&) { wxGetApp().gcode_thumbnails_debug(); });
//#endif // ENABLE_THUMBNAIL_GENERATOR_DEBUG

    return helpMenu;
}
bool        MainFrame::OnMenuSelection() 
{ 
 #ifdef __APPLE__
    return true;
#endif // __APPLE__

    return !plater()->canvas3D()->get_gizmos_manager().is_running() && IsActive(); 
}
static void add_common_view_menu_items(wxMenu *view_menu, MainFrame *mainFrame, std::function<bool(void)> onMenuSelection)
{

#ifdef _MSC_VER
    static const wxString sepInfo       = "\t&";
#else
    static const wxString sepInfo = "\t";
#endif
    // The camera control accelerators are captured by GLCanvas3D::on_char().
    wxMenuItem *menuItem_Iso = append_menu_item(
        view_menu, wxID_HIGHEST+24, _L("Iso") + sepInfo + "0", _L("Iso View"), [mainFrame](wxCommandEvent &) { mainFrame->CheckRunNumEvent(0); },
        "", nullptr, [onMenuSelection]() { return onMenuSelection(); }, mainFrame);
    view_menu->AppendSeparator();
    //TRN To be shown in the main menu View->Top 
    append_menu_item(
        view_menu, wxID_HIGHEST+25, _L("Top") + sepInfo + "1", _L("Top View"), [mainFrame](wxCommandEvent &) { mainFrame->CheckRunNumEvent(1); },
        "", nullptr, [onMenuSelection]() { return onMenuSelection(); }, mainFrame);
    //TRN To be shown in the main menu View->Bottom 
    append_menu_item(
        view_menu, wxID_HIGHEST +26, _L("Bottom") + sepInfo + "2", _L("Bottom View"),
        [mainFrame](wxCommandEvent &) { mainFrame->CheckRunNumEvent(2); },
        "", nullptr, [onMenuSelection]() { return onMenuSelection(); }, mainFrame);
    append_menu_item(
        view_menu, wxID_HIGHEST +27, _L("Front") + sepInfo + "3", _L("Front View"),
        [mainFrame](wxCommandEvent &) { mainFrame->CheckRunNumEvent(3); },
        "", nullptr, [onMenuSelection]() { return onMenuSelection(); }, mainFrame);
    append_menu_item(
        view_menu, wxID_HIGHEST + 28, _L("Rear") + sepInfo + "4", _L("Rear View"),
        [mainFrame](wxCommandEvent &) { mainFrame->CheckRunNumEvent(4); },
        "", nullptr, [onMenuSelection]() { return onMenuSelection(); }, mainFrame);
    append_menu_item(
        view_menu, wxID_HIGHEST + 29, _L("Left") + sepInfo + "5", _L("Left View"),
        [mainFrame](wxCommandEvent &) { mainFrame->CheckRunNumEvent(5); },
        "", nullptr, [onMenuSelection]() { return onMenuSelection(); }, mainFrame);
    append_menu_item(
        view_menu, wxID_HIGHEST + 30, _L("Right") + sepInfo + "6", _L("Right View"),
        [mainFrame](wxCommandEvent &) { mainFrame->CheckRunNumEvent(6); },
        "", nullptr, [onMenuSelection]() { return onMenuSelection(); }, mainFrame);
}

void MainFrame::init_menubar_as_editor()
{
#ifdef __APPLE__
    wxMenuBar::SetAutoWindowMenu(false);
    m_menubar = new wxMenuBar();
#endif

    // File menu
    wxMenu* fileMenu = new wxMenu;
    {
        append_menu_item(
            fileMenu,wxID_HIGHEST + 7, _L("&New Project") + "\tCtrl+N", _L("Start a new project "),
            [this](wxCommandEvent&) { if (m_plater) m_plater->new_project(); }, "", nullptr,
            [this](){return m_plater != nullptr && can_start_new_project(); }, this);

        append_menu_item(
            fileMenu, wxID_HIGHEST + 8, _L("&Open Project") + "\tCtrl+O", _L("Open a project file "),
            [this](wxCommandEvent&) { if (m_plater) m_plater->load_project(); }, "", nullptr,
            [this](){return m_plater != nullptr && !wxGetApp().notification_manager()->is_in_preview(); }, this);

        wxMenu* recent_projects_menu = new wxMenu();
        wxMenuItem *recent_projects_submenu = append_submenu(fileMenu, recent_projects_menu, wxID_ANY, _L("Recent projects"), "");
        m_recent_projects.UseMenu(recent_projects_menu);
        Bind(wxEVT_MENU, [this](wxCommandEvent& evt) {
            size_t file_id = evt.GetId() - wxID_FILE1;
            wxString filename = m_recent_projects.GetHistoryFile(file_id);
            if (wxFileExists(filename)) {
                if (wxGetApp().can_load_project())
                    m_plater->load_project(filename);
            }
            else
            {
                //wxMessageDialog msg(this, _L("The selected project is no longer available.\nDo you want to remove it from the recent projects list?"), _L("Error"), wxYES_NO | wxYES_DEFAULT);
                MessageDialog msg(this, _L("The selected project is no longer available.\nDo you want to remove it from the recent projects list?"), _L("Error"), wxYES_NO | wxYES_DEFAULT);
                if (msg.ShowModal() == wxID_YES)
                {
                    m_recent_projects.RemoveFileFromHistory(file_id);
                        std::vector<std::string> recent_projects;
                        size_t count = m_recent_projects.GetCount();
                        for (size_t i = 0; i < count; ++i)
                        {
                            recent_projects.push_back(into_u8(m_recent_projects.GetHistoryFile(i)));
                        }
                    wxGetApp().app_config->set_recent_projects(recent_projects);
                }
            }
            }, wxID_FILE1, wxID_FILE9);

        std::vector<std::string> recent_projects = wxGetApp().app_config->get_recent_projects();
        std::reverse(recent_projects.begin(), recent_projects.end());
        for (const std::string& project : recent_projects)
        {
            m_recent_projects.AddFileToHistory(from_u8(project));
        }

        Bind(wxEVT_UPDATE_UI, [this](wxUpdateUIEvent& evt) { evt.Enable(m_recent_projects.GetCount() > 0 && m_plater != nullptr && !wxGetApp().notification_manager()->is_in_preview()); }, recent_projects_submenu->GetId());

        append_menu_item(
            fileMenu, wxID_HIGHEST+9, _L("&Save Project") + "\tCtrl+S", _L("Save current project file"),
            [this](wxCommandEvent&) { save_project(); }, "", nullptr,
            [this](){return m_plater != nullptr && can_save(); }, this);
#ifdef __APPLE__
        append_menu_item(
            fileMenu, wxID_ANY,
            _L("Save Project &as") + dots + "\tCtrl+Shift+S", _L(" Save current project file as "),
#else
        append_menu_item(
            fileMenu, wxID_HIGHEST + 10, _L("Save Project &as") + dots + "\tCtrl+Shift+S", _L("Save current project file as"),
#endif // __APPLE__
            [this](wxCommandEvent&) { save_project_as(); }, "", nullptr,
            [this](){return m_plater != nullptr && can_save_as(); }, this);

        fileMenu->AppendSeparator();

        wxMenu* import_menu = new wxMenu();
        append_menu_item(
            import_menu, wxID_HIGHEST+11, _L("Import STL/3MF/STEP/OBJ/AM&F") + dots + "\tCtrl+I",
            _L("Load a model"),
            [this](wxCommandEvent&) { if (m_plater) m_plater->add_model(); }, "", nullptr,
            [this](){return m_plater != nullptr; }, this);
        
        /*append_menu_item(import_menu, wxID_ANY, _L("Import STL (Imperial Units)"), _L("Load an model saved with imperial units"),
            [this](wxCommandEvent&) { if (m_plater) m_plater->add_model(true); }, "", nullptr,
            [this](){return m_plater != nullptr; }, this);
        
        append_menu_item(import_menu, wxID_ANY, _L("Import SLA Archive") + dots, _L("Load an SLA archive"),
            [this](wxCommandEvent&) { if (m_plater) m_plater->import_sl1_archive(); }, "", nullptr,
            [this](){return m_plater != nullptr && m_plater->get_ui_job_worker().is_idle(); }, this);
    
        append_menu_item(import_menu, wxID_ANY, _L("Import ZIP Archive") + dots, _L("Load a ZIP archive"),
            [this](wxCommandEvent&) { if (m_plater) m_plater->import_zip_archive(); }, "ACEmpty", nullptr,
            [this]() {return m_plater != nullptr; }, this);*/

        //import_menu->AppendSeparator();
        append_menu_item(
            import_menu, wxID_HIGHEST+12, _L("Import &Config") + dots + +"\tCtrl+L",
            _L("Load exported configuration file"),
            [this](wxCommandEvent&) { load_config_file(); }, "", nullptr,
            []() {return true; }, this);
        /*append_menu_item(import_menu, wxID_ANY, _L("Import Config from &Project") + dots +"\tCtrl+Alt+L", _L("Load configuration from project file"),
            [this](wxCommandEvent&) { if (m_plater) m_plater->extract_config_from_project(); }, "", nullptr,
            []() {return true; }, this);
        import_menu->AppendSeparator();
        append_menu_item(import_menu, wxID_ANY, _L("Import Config &Bundle") + dots, _L("Load presets from a bundle"),
            [this](wxCommandEvent&) { load_configbundle(); }, "", nullptr,
            []() {return true; }, this);*/
        append_submenu(fileMenu, import_menu, wxID_ANY, _L("&Import"), "");

        wxMenu* export_menu = new wxMenu();
        /*wxMenuItem* item_export_gcode = append_menu_item(export_menu, wxID_ANY, _L("Export &G-code") + dots + "\tCtrl+G", _L("Export current plate as G-code"),
            [this](wxCommandEvent&) { if (m_plater) m_plater->export_gcode(false); }, "", nullptr,
            [this](){return can_export_gcode(); }, this);
        m_changeable_menu_items.push_back(item_export_gcode);
        wxMenuItem* item_send_gcode = append_menu_item(export_menu, wxID_ANY, _L("S&end G-code") + dots + "\tCtrl+Shift+G", _L("Send to print current plate as G-code"),
            [this](wxCommandEvent&) { if (m_plater) m_plater->send_gcode(); }, "", nullptr,
            [this](){return can_send_gcode(); }, this);
        m_changeable_menu_items.push_back(item_send_gcode);
		append_menu_item(export_menu, wxID_ANY, _L("Export G-code to SD Card / Flash Drive") + dots + "\tCtrl+U", _L("Export current plate as G-code to SD card / Flash drive"),
			[this](wxCommandEvent&) { if (m_plater) m_plater->export_gcode(true); }, "", nullptr,
			[this]() {return can_export_gcode_sd(); }, this);
        export_menu->AppendSeparator();*/
        append_menu_item(
            export_menu, wxID_ANY, _L("Export Plate as &STL/OBJ") + dots, _L("Export current plate as STL/OBJ"),
            [this](wxCommandEvent&) { if (m_plater) m_plater->export_stl_obj(); }, "", nullptr,
            [this](){return can_export_model(); }, this);
//        append_menu_item(export_menu, wxID_ANY, _L("Export Plate as STL/OBJ &Including Supports") + dots, _L("Export current plate as STL/OBJ including supports"),
//            [this](wxCommandEvent&) { if (m_plater) m_plater->export_stl_obj(true); }, "", nullptr,
//            [this](){return can_export_supports(); }, this);
//// Deprecating AMF export. Let's wait for user feedback.
////        append_menu_item(export_menu, wxID_ANY, _L("Export Plate as &AMF") + dots, _L("Export current plate as AMF"),
////            [this](wxCommandEvent&) { if (m_plater) m_plater->export_amf(); }, "ACEmpty", nullptr,
////            [this](){return can_export_model(); }, this);
//        //export_menu->AppendSeparator();
//        append_menu_item(export_menu, wxID_ANY, _L("Export &Toolpaths as OBJ") + dots, _L("Export toolpaths as OBJ"),
//            [this](wxCommandEvent&) { if (m_plater) m_plater->export_toolpaths_to_obj(); }, "", nullptr,
//            [this]() {return can_export_toolpaths(); }, this);
//        export_menu->AppendSeparator();
        append_menu_item(
            export_menu, wxID_HIGHEST+13, _L("Export &Config") + dots + +"\tCtrl+E",
            _L("Export current configuration to file"),
            [this](wxCommandEvent&) { export_config(); }, "", nullptr,
            []() {return true; }, this);
        /*append_menu_item(export_menu, wxID_ANY, _L("Export Config &Bundle") + dots, _L("Export all presets to file"),
            [this](wxCommandEvent&) { export_configbundle(); }, "", nullptr,
            []() {return true; }, this);
        append_menu_item(export_menu, wxID_ANY, _L("Export Config Bundle With Physical Printers") + dots, _L("Export all presets including physical printers to file"),
            [this](wxCommandEvent&) { export_configbundle(true); }, "", nullptr,
            []() {return true; }, this);*/
        append_submenu(fileMenu, export_menu, wxID_ANY, _L("&Export"), "");

		/*append_menu_item(
            fileMenu, wxID_HIGHEST+14, _L("Ejec&t SD Card / Flash Drive") + dots + "\tCtrl+T",
            _L("Eject SD card / Flash drive after the G-code was exported to it."),
			[this](wxCommandEvent&) { if (m_plater) m_plater->eject_drive(); }, "", nullptr,
			[this]() {return can_eject(); }, this);*/

        fileMenu->AppendSeparator();

#if 0
        m_menu_item_repeat = nullptr;
        append_menu_item(fileMenu, wxID_ANY, _L("Quick Slice") +dots+ "\tCtrl+U", _L("Slice a file into a G-code"),
            [this](wxCommandEvent&) {
                wxTheApp->CallAfter([this]() {
                    quick_slice();
                    m_menu_item_repeat->Enable(is_last_input_file());
                }); }, "");
        append_menu_item(fileMenu, wxID_ANY, _L("Quick Slice and Save As") +dots +"\tCtrl+Alt+U", _L("Slice a file into a G-code, save as"),
            [this](wxCommandEvent&) {
            wxTheApp->CallAfter([this]() {
                    quick_slice(qsSaveAs);
                    m_menu_item_repeat->Enable(is_last_input_file());
                }); }, "");
        m_menu_item_repeat = append_menu_item(fileMenu, wxID_ANY, _L("Repeat Last Quick Slice") +"\tCtrl+Shift+U", _L("Repeat last quick slice"),
            [this](wxCommandEvent&) {
            wxTheApp->CallAfter([this]() {
                quick_slice(qsReslice);
            }); }, "");
        m_menu_item_repeat->Enable(false);
        fileMenu->AppendSeparator();
#endif
        /*m_menu_item_reslice_now = append_menu_item(fileMenu, wxID_ANY, _L("(Re)Slice No&w") + "\tCtrl+R", _L("Start new slicing process"),
            [this](wxCommandEvent&) { reslice_now(); }, "", nullptr,
            [this]() { return m_plater != nullptr && can_reslice(); }, this);
        fileMenu->AppendSeparator();
        append_menu_item(fileMenu, wxID_ANY, _L("&Repair STL file") + dots, _L("Automatically repair an STL file"),
            [this](wxCommandEvent&) { repair_stl(); }, "", nullptr,
            []() { return true; }, this);
        fileMenu->AppendSeparator();*/
        //append_menu_item(fileMenu, wxID_ANY, _L("Preview &G-code File") + dots, _L("Open G-code viewer"),
        //    [this](wxCommandEvent &) {
        //        if (m_plater != nullptr)m_plater->load_gcode() /*start_new_gcodeviewer_open_file(this)*/; },"", nullptr, 
        //    [this]() { return m_plater != nullptr && m_plater->acgcode_mode() == ACGcodeMode::ACGcode_None; }, this);
        //fileMenu->AppendSeparator();
        #ifdef _WIN32
            append_menu_item(fileMenu, wxID_EXIT, _L("E&xit"), format_wxstr(_L("Exit %1%"), wxGetApp().appName()),
        #else
            append_menu_item(fileMenu, wxID_EXIT, _L("&Quit"), format_wxstr(_L("Quit %1%"), wxGetApp().appName()),
        #endif
            [this](wxCommandEvent&) { Close(false); }, "");
    }

    // Edit menu
    wxMenu* editMenu = nullptr;
    if (m_plater != nullptr)
    {
        editMenu = new wxMenu();
        wxString hotkey_delete = "Del";
    //#ifdef __APPLE__
    //    // Backspace sign
    //    wxString hotkey_delete = "\u232b";
    //#else
    //    wxString hotkey_delete = "Del";
    //#endif
        append_menu_item(
            editMenu, wxID_HIGHEST+15, _L("&Select All") + sep + GUI::shortkey_ctrl_prefix() + sep_space + "A",
            _L("Selects all objects"), [this](wxCommandEvent&) { m_plater->select_all(); },
            "", nullptr, [this](){return can_select(); }, this);
        append_menu_item(
            editMenu, wxID_HIGHEST+16, _L("D&eselect All") + sep + "Esc",
            _L("Deselects all objects"), [this](wxCommandEvent&) { m_plater->deselect_all(); },
            "", nullptr, [this](){return can_deselect(); }, this);
        editMenu->AppendSeparator();
        append_menu_item(
            editMenu, wxID_HIGHEST +17, _L("&Delete Selected") + sep + hotkey_delete,
            _L("Deletes the current selection"),[this](wxCommandEvent&) { m_plater->remove_selected(); },
            "", nullptr, [this](){return can_delete(); }, this);
        append_menu_item(
            editMenu, wxID_HIGHEST +18, _L("Delete &All") + sep + GUI::shortkey_ctrl_prefix() + sep_space + hotkey_delete,
            _L("Deletes all objects"), [this](wxCommandEvent&) { m_plater->reset_with_confirm(); },
            "", nullptr, [this](){return can_delete_all(); }, this);

        editMenu->AppendSeparator();
        append_menu_item(
            editMenu, wxID_HIGHEST +19, _L("&Undo") + sep + GUI::shortkey_ctrl_prefix() + sep_space + "Z",
            _L("Undo"), [this](wxCommandEvent&) { m_plater->undo(); },
            "", nullptr, [this](){return m_plater->can_undo(); }, this);
        append_menu_item(
            editMenu, wxID_HIGHEST +20, _L("&Redo") + sep + GUI::shortkey_ctrl_prefix() + sep_space + "Y",
            _L("Redo"), [this](wxCommandEvent&) { m_plater->redo(); },
            "", nullptr, [this](){return m_plater->can_redo(); }, this);

        editMenu->AppendSeparator();
        append_menu_item(
            editMenu, wxID_HIGHEST +21, _L("&Copy") + sep + GUI::shortkey_ctrl_prefix() + sep_space + "C",
            _L("Copy selection to clipboard"), [this](wxCommandEvent&) { m_plater->copy_selection_to_clipboard(); },
            "", nullptr, [this](){return m_plater->can_copy_to_clipboard(); }, this);
        append_menu_item(
            editMenu, wxID_HIGHEST +22, _L("&Paste") + sep + GUI::shortkey_ctrl_prefix() + sep_space + "V",
            _L("Paste clipboard"), [this](wxCommandEvent&) { m_plater->paste_from_clipboard(); },
            "", nullptr, [this](){return m_plater->can_paste_from_clipboard(); }, this);
        
//        editMenu->AppendSeparator();
//
//#ifdef __APPLE__
//        append_menu_item(editMenu, wxID_ANY, _L("Re&load from Disk") + dots + "\tCtrl+Shift+R",
//            _L("Reload the plater from disk"), [this](wxCommandEvent&) { m_plater->reload_all_from_disk(); },
//            "", nullptr, [this]() {return !m_plater->model().objects.empty(); }, this);
//#else
//        append_menu_item(editMenu, wxID_ANY, _L("Re&load from Disk") + sep + "F5",
//            _L("Reload the plater from disk"), [this](wxCommandEvent&) { m_plater->reload_all_from_disk(); },
//            "", nullptr, [this]() {return !m_plater->model().objects.empty(); }, this);
//#endif // __APPLE__
//
//        editMenu->AppendSeparator();
//        append_menu_item(editMenu, wxID_ANY, _L("Searc&h") + "\tCtrl+F",
//            _L("Search in settings"), [this](wxCommandEvent&) { m_plater->search(m_plater->IsShown()); },
//            "", nullptr, []() {return true; }, this);
    }

    // Window menu
    // auto windowMenu = new wxMenu();
    // {
    //     if (m_plater) {
    //         append_menu_item(windowMenu, wxID_HIGHEST + 1, _L("&Plater Tab") + "\tCtrl+1", _L("Show the plater"),
    //             [this](wxCommandEvent&) { select_tab(size_t(0)); }, "", nullptr,
    //             []() {return true; }, this);
    //         windowMenu->AppendSeparator();
    //     }
    //     append_menu_item(windowMenu, wxID_HIGHEST + 2, _L("P&rint Settings Tab") + "\tCtrl+2", _L("Show the print settings"),
    //         [this/*, tab_offset*/](wxCommandEvent&) { select_tab(1); }, "", nullptr,
    //         []() {return true; }, this);
    //     wxMenuItem* item_material_tab = append_menu_item(windowMenu, wxID_HIGHEST + 3, _L("&Filament Settings Tab") + "\tCtrl+3", _L("Show the filament settings"),
    //         [this/*, tab_offset*/](wxCommandEvent&) { select_tab(2); }, "", nullptr,
    //         []() {return true; }, this);
    //     m_changeable_menu_items.push_back(item_material_tab);
    //     wxMenuItem* item_printer_tab = append_menu_item(windowMenu, wxID_HIGHEST + 4, _L("Print&er Settings Tab") + "\tCtrl+4", _L("Show the printer settings"),
    //         [this/*, tab_offset*/](wxCommandEvent&) { select_tab(3); }, "", nullptr,
    //         []() {return true; }, this);
    //     m_changeable_menu_items.push_back(item_printer_tab);
    //     if (m_plater) {
    //         windowMenu->AppendSeparator();
    //         append_menu_item(windowMenu, wxID_HIGHEST + 5, _L("3&D") + "\tCtrl+5", _L("Show the 3D editing view"),
    //             [this](wxCommandEvent&) { m_plater->select_view_3D("3D"); }, "", nullptr,
    //             [this](){return can_change_view(); }, this);
    //         append_menu_item(windowMenu, wxID_HIGHEST + 6, _L("Pre&view") + "\tCtrl+6", _L("Show the 3D slices preview"),
    //             [this](wxCommandEvent&) { m_plater->select_view_3D("Preview"); }, "", nullptr,
    //             [this](){return can_change_view(); }, this);
    //     }

    //     windowMenu->AppendSeparator();
    //     append_menu_item(windowMenu, wxID_ANY, _L("Shape Gallery"), _L("Open the dialog to modify shape gallery"),
    //         [this](wxCommandEvent&) {
    //             if (gallery_dialog()->show(true) == wxID_OK) {
    //                 wxArrayString input_files;
    //                 m_gallery_dialog->get_input_files(input_files);
    //                 if (!input_files.IsEmpty())
    //                     m_plater->sidebar().obj_list()->load_shape_object_from_gallery(input_files);
    //             }
    //         }, "", nullptr, []() {return true; }, this);
        
    //     windowMenu->AppendSeparator();
    //     append_menu_item(windowMenu, wxID_ANY, _L("Print &Host Upload Queue") + "\tCtrl+J", _L("Display the Print Host Upload Queue window"),
    //         [this](wxCommandEvent&) { m_printhost_queue_dlg->Show(); }, "", nullptr, []() {return true; }, this);
        
    //     windowMenu->AppendSeparator();
    //     append_menu_item(windowMenu, wxID_ANY, _L("Open New Instance") + "\tCtrl+Shift+I", _L("Open a new AnycubicSlicer instance"),
    //         [](wxCommandEvent&) { start_new_slicer(); }, "", nullptr, [this]() {return m_plater != nullptr && !wxGetApp().app_config->get_bool("single_instance"); }, this);

    //     windowMenu->AppendSeparator();
    //     append_menu_item(windowMenu, wxID_ANY, _L("Compare Presets")/* + "\tCtrl+F"*/, _L("Compare presets"),
    //         [this](wxCommandEvent&) { diff_dialog.show();}, "", nullptr, []() {return true; }, this);
    // }

    // View menu
    wxMenu* viewMenu = nullptr;
    if (m_plater) {
        viewMenu = new wxMenu();
        add_common_view_menu_items(viewMenu, this, std::bind(&MainFrame::OnMenuSelection, this));
        viewMenu->AppendSeparator();
        append_menu_check_item(
            viewMenu, wxID_HIGHEST +31, _L("Show &Labels") + sep + "E", _L("Show object/instance labels in 3D scene"),
            [this](wxCommandEvent&) { m_plater->show_view3D_labels(!m_plater->are_view3D_labels_shown()); }, this,
            [this]() { return m_plater->is_view3D_shown(); }, [this]() { return m_plater->are_view3D_labels_shown(); }, this);
        /*append_menu_check_item(viewMenu, wxID_ANY, _L("Show Legen&d") + sep + "L", _L("Show legend in preview"),
            [this](wxCommandEvent&) { m_plater->show_legend(!m_plater->is_legend_shown()); }, this,
            [this]() { return m_plater->is_preview_shown(); }, [this]() { return m_plater->is_legend_shown(); }, this);
        append_menu_check_item(viewMenu, wxID_ANY, _L("&Collapse Sidebar") + sep + "Shift+" + sep_space + "Tab", _L("Collapse sidebar"),
            [this](wxCommandEvent&) { m_plater->collapse_sidebar(!m_plater->is_sidebar_collapsed()); }, this,
            []() { return true; }, [this]() { return m_plater->is_sidebar_collapsed(); }, this);*/
//#ifndef __APPLE__
//        // OSX adds its own menu item to toggle fullscreen.
//        append_menu_check_item(
//            viewMenu, wxID_HIGHEST +32, _L("&Fullscreen") + sep + "F11", _L("Fullscreen"),
//            [this](wxCommandEvent &) {
//                bool opIndex = !this->IsFullScreen() && !this->IsMaximized();
//                if (!opIndex && !this->IsFullScreen()) {
//                    this->Maximize(false);
//                } else {
//                    this->ShowFullScreen(!this->IsFullScreen() && !this->IsMaximized(),
//                                         // wxFULLSCREEN_ALL: wxFULLSCREEN_NOMENUBAR | wxFULLSCREEN_NOTOOLBAR | wxFULLSCREEN_NOSTATUSBAR |
//                                         // wxFULLSCREEN_NOBORDER | wxFULLSCREEN_NOCAPTION
//                                         wxFULLSCREEN_NOSTATUSBAR | wxFULLSCREEN_NOBORDER | wxFULLSCREEN_NOCAPTION);
//                }
//                this->m_topbar->ChangeIconStyle();
//            },
//            this, []() { return true; }, [this]() { return this->IsFullScreen()||this->IsMaximized(); }, this);
//#endif // __APPLE__
    }

    // Help menu
    auto helpMenu = generate_help_menu(this);

#ifdef __APPLE__
    // menubar
    // assign menubar to frame after appending items, otherwise special items
    // will not be handled correctly
   // m_menubar = new wxMenuBar(); -- todo:新代码里出现的
    m_menubar->SetFont(this->normal_font());
    m_menubar->Append(fileMenu, _L("&File"));
    if (editMenu) m_menubar->Append(editMenu, _L("&Edit"));
    // m_menubar->Append(windowMenu, _L("&Window"));
    if (viewMenu) m_menubar->Append(viewMenu, _L("&View"));
    // Add additional menus from C++
    m_menubar->Append( wxGetApp().add_config_menu(), _L("&Configuration"));
    m_menubar->Append(helpMenu, _L("&Help"));

// #ifdef _MSW_DARK_MODE
//     if (wxGetApp().tabs_as_menu()) {
//         // Add separator
//         m_menubar->Append(new wxMenu(), "          ");
//         add_tabs_as_menu(m_menubar, this, this);
//     }
// #endif
    SetMenuBar(m_menubar);

// #ifdef _MSW_DARK_MODE
//     if (wxGetApp().tabs_as_menu())
//         m_menubar->EnableTop(6, false);
// #endif

    // This fixes a bug on Mac OS where the quit command doesn't emit window close events
    // wx bug: https://trac.wxwidgets.org/ticket/18328
    wxMenu* apple_menu = m_menubar->OSXGetAppleMenu();
    if (apple_menu != nullptr) {
        apple_menu->Bind(wxEVT_MENU, [this](wxCommandEvent &) {
            Close();
        }, wxID_EXIT);
    }
#else
        // Add additional menus from C++
    wxMenu* configMenu = wxGetApp().add_config_menu();

    m_topbar->SetFileMenu(fileMenu);
    m_topbar->SetEditMenu(editMenu);
    m_topbar->SetViewMenu(viewMenu);
    m_topbar->SetHelpMenu(helpMenu);
    m_topbar->SetSetsMenu(configMenu);
#endif // __APPLE__


    if (plater()->printer_technology() == ptSLA)
        update_menubar();
}

void MainFrame::open_menubar_item(const wxString& menu_name,const wxString& item_name)
{
    if (m_menubar == nullptr)
        return;
    // Get menu object from menubar
    int     menu_index = m_menubar->FindMenu(menu_name);
    wxMenu* menu       = m_menubar->GetMenu(menu_index);
    if (menu == nullptr) {
        BOOST_LOG_TRIVIAL(error) << "Mainframe open_menubar_item function couldn't find menu: " << menu_name;
        return;
    }
    // Get item id from menu
    int     item_id   = menu->FindItem(item_name);
    if (item_id == wxNOT_FOUND)
    {
        // try adding three dots char
        item_id = menu->FindItem(item_name + dots);
    }
    if (item_id == wxNOT_FOUND)
    {
        BOOST_LOG_TRIVIAL(error) << "Mainframe open_menubar_item function couldn't find item: " << item_name;
        return;
    }
    // wxEVT_MENU will trigger item
    wxPostEvent((wxEvtHandler*)menu, wxCommandEvent(wxEVT_MENU, item_id));
}

void MainFrame::init_menubar_as_gcodeviewer()
{
    wxMenu* fileMenu = new wxMenu;
    {
        append_menu_item(fileMenu, wxID_ANY, _L("&Open G-code") + dots + "\tCtrl+O", _L("Open a G-code file"),
            [this](wxCommandEvent&) { if (m_plater != nullptr) m_plater->load_gcode(); }, "", nullptr,
            [this]() {return m_plater != nullptr; }, this);
#ifdef __APPLE__
        append_menu_item(fileMenu, wxID_ANY, _L("Re&load from Disk") + dots + "\tCtrl+Shift+R",
            _L("Reload the plater from disk"), [this](wxCommandEvent&) { m_plater->reload_gcode_from_disk(); },
            "", nullptr, [this]() { return !m_plater->get_last_loaded_gcode().empty(); }, this);
#else
        append_menu_item(fileMenu, wxID_ANY, _L("Re&load from Disk") + sep + "F5",
            _L("Reload the plater from disk"), [this](wxCommandEvent&) { m_plater->reload_gcode_from_disk(); },
            "", nullptr, [this]() { return !m_plater->get_last_loaded_gcode().empty(); }, this);
#endif // __APPLE__
        fileMenu->AppendSeparator();
        append_menu_item(fileMenu, wxID_ANY, _L("Export &Toolpaths as OBJ") + dots, _L("Export toolpaths as OBJ"),
            [this](wxCommandEvent&) { if (m_plater != nullptr) m_plater->export_toolpaths_to_obj(); }, "ACEmpty", nullptr,
            [this]() {return can_export_toolpaths(); }, this);
        append_menu_item(fileMenu, wxID_ANY, _L("Open &AnycubicSlicer") + dots, _L("Open AnycubicSlicer"),
            [](wxCommandEvent&) { start_new_slicer(); }, "", nullptr,
            []() {return true; }, this);
        fileMenu->AppendSeparator();
        append_menu_item(fileMenu, wxID_EXIT, _L("&Quit"), format_wxstr(_L("Quit %s"), wxGetApp().appName()),
            [this](wxCommandEvent&) { Close(false); });
    }

    // View menu
    wxMenu* viewMenu = nullptr;
    if (m_plater != nullptr) {
        viewMenu = new wxMenu();
        add_common_view_menu_items(viewMenu, this, std::bind(&MainFrame::OnMenuSelection, this));
        viewMenu->AppendSeparator();
        append_menu_check_item(viewMenu, wxID_ANY, _L("Show Legen&d") + sep + "L", _L("Show legend"),
            [this](wxCommandEvent&) { m_plater->show_legend(!m_plater->is_legend_shown()); }, this,
            [this]() { return m_plater->is_preview_shown(); }, [this]() { return m_plater->is_legend_shown(); }, this);
    }

    // helpmenu
    auto helpMenu = generate_help_menu(this);
    wxMenu* configMenu = wxGetApp().add_config_menu();

#ifdef __APPLE__
    m_menubar = new wxMenuBar();
    m_menubar->Append(fileMenu, _L("&File"));
    if (viewMenu != nullptr) m_menubar->Append(viewMenu, _L("&View"));
    // Add additional menus from C++
    m_menubar->Append( wxGetApp().add_config_menu(), _L("&Configuration"));
    //m_menubar->Append(helpMenu, _L("&Help"));
    SetMenuBar(m_menubar);
#else
    m_topbar->SetFileMenu(fileMenu);
    //m_topbar->SetEditMenu(editMenu);
    m_topbar->SetViewMenu(viewMenu);
    //m_topbar->SetHelpMenu(helpMenu);
    m_topbar->SetSetsMenu(configMenu);

    m_topbar->SetEditHide();
#endif // __APPLE__

#ifdef __APPLE__
    // This fixes a bug on Mac OS where the quit command doesn't emit window close events
    // wx bug: https://trac.wxwidgets.org/ticket/18328
    wxMenu* apple_menu = m_menubar->OSXGetAppleMenu();
    if (apple_menu != nullptr) {
        apple_menu->Bind(wxEVT_MENU, [this](wxCommandEvent&) {
            Close();
            }, wxID_EXIT);
    }
#endif // __APPLE__
}

void MainFrame::update_menubar()
{
    if (wxGetApp().is_gcode_viewer())
        return;

    const bool is_fff = plater()->printer_technology() == ptFFF;

    /*m_changeable_menu_items[miExport]       ->SetItemLabel((is_fff ? _L("Export &G-code")         : _L("E&xport"))        + dots    + "\tCtrl+G");
    m_changeable_menu_items[miSend]         ->SetItemLabel((is_fff ? _L("S&end G-code")           : _L("S&end to print")) + dots    + "\tCtrl+Shift+G");

    m_changeable_menu_items[miMaterialTab]  ->SetItemLabel((is_fff ? _L("&Filament Settings Tab") : _L("Mate&rial Settings Tab"))   + "\tCtrl+3");
    m_changeable_menu_items[miMaterialTab]  ->SetBitmap(*get_bmp_bundle(is_fff ? "ACEmpty"   : "ACEmpty"));

    m_changeable_menu_items[miPrinterTab]   ->SetBitmap(*get_bmp_bundle(is_fff ? "ACEmpty" : "ACEmpty"));*/


    // m_changeable_menu_items[miMaterialTab]->SetItemLabel((is_fff ? _L("&Filament Settings Tab") : _L("Mate&rial Settings Tab")) +
    //                                                      "\tCtrl+3");
    // m_changeable_menu_items[miMaterialTab]->SetBitmap(*get_bmp_bundle(is_fff ? "ACEmpty" : "ACEmpty"));

    // m_changeable_menu_items[miPrinterTab]->SetBitmap(*get_bmp_bundle(is_fff ? "ACEmpty" : "ACEmpty"));
}

#if 0
// To perform the "Quck Slice", "Quick Slice and Save As", "Repeat last Quick Slice" and "Slice to SVG".
void MainFrame::quick_slice(const int qs)
{
//     my $progress_dialog;
    wxString input_file;
//  eval
//     {
    // validate configuration
    auto config = wxGetApp().preset_bundle->full_config();
    auto valid = config.validate();
    if (! valid.empty()) {
        show_error(this, valid);
        return;
    }

    // select input file
    if (!(qs & qsReslice)) {
        wxFileDialog dlg(this, _L("Choose a file to slice (STL/OBJ/AMF/3MF/PRUSA):"),
            wxGetApp().app_config->get_last_dir(), "",
            file_wildcards(FT_MODEL), wxFD_OPEN | wxFD_FILE_MUST_EXIST);
        if (dlg.ShowModal() != wxID_OK)
            return;
        input_file = dlg.GetPath();
        if (!(qs & qsExportSVG))
            m_qs_last_input_file = input_file;
    }
    else {
        if (m_qs_last_input_file.IsEmpty()) {
            //wxMessageDialog dlg(this, _L("No previously sliced file."),
            MessageDialog dlg(this, _L("No previously sliced file."),
                _L("Error"), wxICON_ERROR | wxOK);
            dlg.ShowModal();
            return;
        }
        if (std::ifstream(m_qs_last_input_file.ToUTF8().data())) {
            //wxMessageDialog dlg(this, _L("Previously sliced file (")+m_qs_last_input_file+_L(") not found."),
            MessageDialog dlg(this, _L("Previously sliced file (")+m_qs_last_input_file+_L(") not found."),
                _L("File Not Found"), wxICON_ERROR | wxOK);
            dlg.ShowModal();
            return;
        }
        input_file = m_qs_last_input_file;
    }
    auto input_file_basename = get_base_name(input_file);
    wxGetApp().app_config->update_skein_dir(get_dir_name(input_file));

    auto bed_shape = Slic3r::Polygon::new_scale(config.option<ConfigOptionPoints>("bed_shape")->values);
//     auto print_center = Slic3r::Pointf->new_unscale(bed_shape.bounding_box().center());
// 
//     auto sprint = new Slic3r::Print::Simple(
//         print_center = > print_center,
//         status_cb = > [](int percent, const wxString& msg) {
//         m_progress_dialog->Update(percent, msg+"…");
//     });

    // keep model around
    auto model = Slic3r::Model::read_from_file(input_file.ToUTF8().data());

//     sprint->apply_config(config);
//     sprint->set_model(model);

    // Copy the names of active presets into the placeholder parser.
//     wxGetApp().preset_bundle->export_selections(sprint->placeholder_parser);

    // select output file
    wxString output_file;
    if (qs & qsReslice) {
        if (!m_qs_last_output_file.IsEmpty())
            output_file = m_qs_last_output_file;
    } 
    else if (qs & qsSaveAs) {
        // The following line may die if the output_filename_format template substitution fails.
        wxFileDialog dlg(this, format_wxstr(_L("Save %s file as:"), ((qs & qsExportSVG) ? _L("SVG") : _L("G-code"))),
            wxGetApp().app_config->get_last_output_dir(get_dir_name(output_file)), get_base_name(input_file), 
            qs & qsExportSVG ? file_wildcards(FT_SVG) : file_wildcards(FT_GCODE),
            wxFD_SAVE | wxFD_OVERWRITE_PROMPT);
        if (dlg.ShowModal() != wxID_OK)
            return;
        output_file = dlg.GetPath();
        if (!(qs & qsExportSVG))
            m_qs_last_output_file = output_file;
        wxGetApp().app_config->update_last_output_dir(get_dir_name(output_file));
    } 
    else if (qs & qsExportPNG) {
        wxFileDialog dlg(this, _L("Save ZIP file as:"),
            wxGetApp().app_config->get_last_output_dir(get_dir_name(output_file)),
            get_base_name(output_file), "*.sl1", wxFD_SAVE | wxFD_OVERWRITE_PROMPT);
        if (dlg.ShowModal() != wxID_OK)
            return;
        output_file = dlg.GetPath();
    }

    // show processbar dialog
    m_progress_dialog = new wxProgressDialog(_L("Slicing") + dots,
        // TRN ProgressDialog on reslicing: "input file basename"
        format_wxstr(_L("Processing %s"), (input_file_basename + dots)),
        100, nullptr, wxPD_AUTO_HIDE);
    m_progress_dialog->Pulse();
    {
//         my @warnings = ();
//         local $SIG{ __WARN__ } = sub{ push @warnings, $_[0] };

//         sprint->output_file(output_file);
//         if (export_svg) {
//             sprint->export_svg();
//         }
//         else if(export_png) {
//             sprint->export_png();
//         }
//         else {
//             sprint->export_gcode();
//         }
//         sprint->status_cb(undef);
//         Slic3r::GUI::warning_catcher($self)->($_) for @warnings;
    }
    m_progress_dialog->Destroy();
    m_progress_dialog = nullptr;

    auto message = format(_L("%1% was successfully sliced."), input_file_basename);
//     wxTheApp->notify(message);
    //wxMessageDialog(this, message, _L("Slicing Done!"), wxOK | wxICON_INFORMATION).ShowModal();
    MessageDialog(this, message, _L("Slicing Done!"), wxOK | wxICON_INFORMATION).ShowModal();
//     };
//     Slic3r::GUI::catch_error(this, []() { if (m_progress_dialog) m_progress_dialog->Destroy(); });
}
#endif

void MainFrame::reslice_now()
{
    if (m_plater)
        m_plater->reslice();
}

void MainFrame::repair_stl()
{
    wxString input_file;
    {
        wxFileDialog dlg(this, _L("Select the STL file to repair:"),
            wxGetApp().app_config->get_last_dir(), "",
            file_wildcards(FT_STL), wxFD_OPEN | wxFD_FILE_MUST_EXIST);
        if (dlg.ShowModal() != wxID_OK)
            return;
        input_file = dlg.GetPath();
    }

    wxString output_file = input_file;
    {
        wxFileDialog dlg( this, L("Save OBJ file (less prone to coordinate errors than STL) as:"),
                                        get_dir_name(output_file), get_base_name(output_file, ".obj"),
                                        file_wildcards(FT_OBJ), wxFD_SAVE | wxFD_OVERWRITE_PROMPT);
        if (dlg.ShowModal() != wxID_OK)
            return;
        output_file = dlg.GetPath();
    }

    Slic3r::TriangleMesh tmesh;
    tmesh.ReadSTLFile(input_file.ToUTF8().data());
    tmesh.WriteOBJFile(output_file.ToUTF8().data());
    Slic3r::GUI::show_info(this, L("Your file was repaired."), L("Repair"));
}

bool MainFrame::showWebDialog() { 
    bool isRun = wxGetApp().checkURLNetWorkStatic();
    if (!isRun) {
        WarningDialog dialog(wxGetApp().mainframe, _L("Please check the network and try again."), _L("Warning"),wxYES);
        dialog.ShowModal();
        return false;
    }
        
    ACWebDialog *m_web_dialog = new ACWebDialog(wxGetApp().m_acCaptchaURL);
    m_web_dialog->Bind(wxEVT_CLOSE_WINDOW, [&m_web_dialog](wxCloseEvent &event) { m_web_dialog->EndModal(wxID_NO); });

    if (m_web_dialog->ShowModal() == wxID_YES) {
        delete m_web_dialog;
        return true;
    }
    delete m_web_dialog;
    return false;
}
wxString MainFrame::showAddPrinterDialog()
{
	wxString rv;
	if (wxGetApp().is_cloud_login())
	{
		m_ACSAddDialog = new ACCloudAddMachine(this);

		m_ACSAddDialog->Bind(wxEVT_CLOSE_WINDOW, [&](wxCloseEvent& event) { m_ACSAddDialog->EndModal(wxID_NO); });

		if (m_ACSAddDialog->ShowModal() == wxID_YES)
			rv = m_ACSAddDialog->GetCNInfo();

		delete m_ACSAddDialog;
		m_ACSAddDialog = nullptr;
	}
	return rv;
}


bool MainFrame::showSelectPrinterDialog()
{
    bool isLoginStatus = true;
    if (isLoginStatus) {
        m_ACSelectDialog = new ACCloudSelectMachine(this);

        m_ACSelectDialog->Bind(wxEVT_CLOSE_WINDOW, [&](wxCloseEvent &event) { m_ACSelectDialog->EndModal(wxID_NO); });

        if (m_ACSelectDialog->ShowModal() == wxID_YES) {
            PrinterData              printerInfo   = m_ACSelectDialog->GetPrinterInfo();
            PrinterPresetCollection &printerPreset = wxGetApp().preset_bundle->printers;
            bool                     isInclude     = false;
            for (Preset info : printerPreset.get_presets()) {
                if (info.name == printerInfo.name) {
                    isInclude = true;
                    break;
                }
            }
            // include name
            if (!isInclude) {
                wxString info = _L("The printer type used in G-code is not detected，make sure the g-code is suitable for your printer "
                                   "before you printing it .");
                RichMessageDialog dialog(NULL, info, _L("Remote Printing"), wxYES_NO);
                dialog.Bind(wxEVT_CLOSE_WINDOW, [&dialog](wxCloseEvent &event) { dialog.EndModal(wxID_NO); });
#ifdef _WIN32
                dialog.SetYesBtnLabel(_L("Yes, start print"));
                dialog.SetNoBtnLabel(_L("Cancel"));
#else
                dialog.SetYesNoLabels(_L("Yes, start print"), _L("Cancel"));
#endif
                if (dialog.ShowModal() == wxID_NO) {
                    m_ACSelectDialog = nullptr;
                    delete m_ACSelectDialog;
                    return false;
                }

            } else {
                Preset prest = printerPreset.get_selected_preset();
                if (printerInfo.name != prest.name) {
                    wxString info = format_wxstr(_L("The Printer you select (%1%) is not consistent with the printer used in G-code "
                                                    "(%2%). Do you want to continue?"),
                                                 printerInfo.name, prest.name);
                    RichMessageDialog dialog(NULL, info, _L("Remote Printing"), wxYES_NO);
                    dialog.Bind(wxEVT_CLOSE_WINDOW, [&dialog](wxCloseEvent &event) { dialog.EndModal(wxID_NO); });
#ifdef _WIN32
                    dialog.SetYesBtnLabel(_L("Continue"));
                    dialog.SetNoBtnLabel(_L("Cancel"));
#else
                    dialog.SetYesNoLabels(_L("Continue"), _L("Cancel"));
#endif
                    if (dialog.ShowModal() == wxID_NO) {
                        m_ACSelectDialog = nullptr;
                        delete m_ACSelectDialog;
                        return false;
                    }
                }
            }
            m_ACSelectDialog = nullptr;
            delete m_ACSelectDialog;
            return true;
        }
        m_ACSelectDialog = nullptr;
        delete m_ACSelectDialog;
    }
    return false;
}

void MainFrame::export_config()
{
    // Generate a cummulative configuration for the selected print, filaments and printer.
    auto config = wxGetApp().preset_bundle->full_config();
    // Validate the cummulative configuration.
    auto valid = config.validate();
    if (! valid.empty()) {
        show_error(this, valid);
        return;
    }
    // Ask user for the file name for the config file.
    wxFileDialog dlg(this, _L("Save configuration as:"),
        !m_last_config.IsEmpty() ? get_dir_name(m_last_config) : wxGetApp().app_config->get_last_dir(),
        !m_last_config.IsEmpty() ? get_base_name(m_last_config) : "config.ini",
        file_wildcards(FT_INI), wxFD_SAVE | wxFD_OVERWRITE_PROMPT);
    wxString file;
    if (dlg.ShowModal() == wxID_OK)
        file = dlg.GetPath();
    if (!file.IsEmpty()) {
        wxGetApp().app_config->update_config_dir(get_dir_name(file));
        m_last_config = file;
        config.save(file.ToUTF8().data());
    }
}

// Load a config file containing a Print, Filament & Printer preset.
void MainFrame::load_config_file()
{
    if (!wxGetApp().check_and_save_current_preset_changes(_L("Loading of a configuration file"), "", false))
        return;
    wxFileDialog dlg(this, _L("Select configuration to load:"),
        !m_last_config.IsEmpty() ? get_dir_name(m_last_config) : wxGetApp().app_config->get_last_dir(),
        "config.ini", "INI files (*.ini, *.gcode)|*.ini;*.INI;*.gcode;*.g", wxFD_OPEN | wxFD_FILE_MUST_EXIST);
	wxString file;
    if (dlg.ShowModal() == wxID_OK)
        file = dlg.GetPath();
	if (!check_file_from_acslicer(file))
	{
		//导入的配置文件非ACSlicer生成的，继续导入可能存在兼容性问题，是否要继续？
		MessageDialog dlg = MessageDialog(nullptr, format_wxstr( _L("The imported configuration file is not generated by %s, \nthere may be compatibility issues if you continue to import, \ndo you want to continue?"), wxGetApp().appName()), _L("Warning!"), wxYES | wxCANCEL);
		if (dlg.ShowModal() != wxID_YES)
			return;
	}
    if (! file.IsEmpty() && this->load_config_file(file.ToUTF8().data())) {
        wxGetApp().app_config->update_config_dir(get_dir_name(file));
        m_last_config = file;
    }
}

// Load a config file containing a Print, Filament & Printer preset from command line.
bool MainFrame::load_config_file(const std::string &path)
{
    std::vector<std::string> errInfoList = {
        _u8L("The Config Bundle cannot be loaded: %1%\n\tReason: %2%"),
        _u8L("Failed loading the Config Bundle \"%1%\": %2% at line %3%"),
        _u8L("Failed loading the preset file: %1%\n\tReason: %2%"),
        _u8L("Unknown configuration file type: %1%"),
        _u8L("Invalid configuration file: %1%. This is an application config file."),
        _u8L("Invalid configuration file %1%: %2%"),
        _u8L("Not a Slic3r base generated g-code."),
        _u8L("Configuration block closing tag \"; AnycubicSlicer_config = end\" not found when reading %1%"),
        _u8L("Configuration block opening tag \"; AnycubicSlicer_config = begin\" not found when reading %1%"),
        _u8L("Suspiciously low number of configuration values extracted from %1%: %2%"),
    };
    try {
        ConfigSubstitutions config_substitutions = wxGetApp().preset_bundle->load_config_file(path, ForwardCompatibilitySubstitutionRule::Enable,errInfoList);
        if (!config_substitutions.empty())
            show_substitutions_info(config_substitutions, path);
    } catch (const std::exception &ex) {
        show_error(this, ex.what());
        return false;
    }

    m_plater->check_selected_presets_visibility(ptFFF);
    wxGetApp().load_current_presets();
    return true;
}

// Check a config file is Anycubic ACSlicer config file.
bool MainFrame::check_file_from_acslicer(const wxString& path)
{
	if (path.IsEmpty())
		return true;
	wxFileName file(path);

	if (!file.FileExists())
		return true;

	wxFileInputStream fileStream(path);
	if (!fileStream.IsOk())
		return true;

	std::string searchStr = "generated by AnycubicSlicer";
	wxTextInputStream textStream(fileStream);
    // par provisional 30 ----
    for (int i = 0; i < 30; i++) {
        wxString lineInfo = textStream.ReadLine();
        if (!lineInfo.empty() && lineInfo.ToStdString().find(searchStr) != std::string::npos)
            return true;
    }
    return false;
}

void MainFrame::export_configbundle(bool export_physical_printers /*= false*/)
{
    if (!wxGetApp().check_and_save_current_preset_changes(_L("Exporting configuration bundle"),
                                                          _L("Some presets are modified and the unsaved changes will not be exported into configuration bundle."), false, true))
        return;
    // validate current configuration in case it's dirty
    auto err = wxGetApp().preset_bundle->full_config().validate();
    if (! err.empty()) {
        show_error(this, err);
        return;
    }
    // Ask user for a file name.
    wxFileDialog dlg(this, _L("Save presets bundle as:"),
        !m_last_config.IsEmpty() ? get_dir_name(m_last_config) : wxGetApp().app_config->get_last_dir(),
        SLIC3R_APP_KEY "_config_bundle.ini",
        file_wildcards(FT_INI), wxFD_SAVE | wxFD_OVERWRITE_PROMPT);
    wxString file;
    if (dlg.ShowModal() == wxID_OK)
        file = dlg.GetPath();
    if (!file.IsEmpty()) {
        // Export the config bundle.
        wxGetApp().app_config->update_config_dir(get_dir_name(file));
        try {
            wxGetApp().preset_bundle->export_configbundle(file.ToUTF8().data(), false, export_physical_printers);
        } catch (const std::exception &ex) {
			show_error(this, ex.what());
        }
    }
}

// Loading a config bundle with an external file name used to be used
// to auto - install a config bundle on a fresh user account,
// but that behavior was not documented and likely buggy.
void MainFrame::load_configbundle(wxString file/* = wxEmptyString, const bool reset_user_profile*/)
{
    if (!wxGetApp().check_and_save_current_preset_changes(_L("Loading of a configuration bundle"), "", false))
        return;
    if (file.IsEmpty()) {
        wxFileDialog dlg(this, _L("Select configuration to load:"),
            !m_last_config.IsEmpty() ? get_dir_name(m_last_config) : wxGetApp().app_config->get_last_dir(),
            "config.ini", file_wildcards(FT_INI), wxFD_OPEN | wxFD_FILE_MUST_EXIST);
        if (dlg.ShowModal() != wxID_OK)
            return;
        file = dlg.GetPath();
	}

    wxGetApp().app_config->update_config_dir(get_dir_name(file));

    size_t presets_imported = 0;
    PresetsConfigSubstitutions config_substitutions;
    try {
        // Report all substitutions.
        std::tie(config_substitutions, presets_imported) = wxGetApp().preset_bundle->load_configbundle(
            file.ToUTF8().data(), PresetBundle::LoadConfigBundleAttribute::SaveImported, ForwardCompatibilitySubstitutionRule::Enable);
    } catch (const std::exception &ex) {
        show_error(this, ex.what());
        return;
    }

    if (! config_substitutions.empty())
        show_substitutions_info(config_substitutions);

    // Load the currently selected preset into the GUI, update the preset selection box.
	wxGetApp().load_current_presets();

    const auto message = wxString::Format(_L("%d presets successfully imported."), presets_imported);
    Slic3r::GUI::show_info(this, message, wxString("Info"));
}

// Load a provied DynamicConfig into the Print / Filament / Printer tabs, thus modifying the active preset.
// Also update the plater with the new presets.
void MainFrame::load_config(const DynamicPrintConfig& config)
{
	PrinterTechnology printer_technology = wxGetApp().preset_bundle->printers.get_edited_preset().printer_technology();
	const auto       *opt_printer_technology = config.option<ConfigOptionEnum<PrinterTechnology>>("printer_technology");
	if (opt_printer_technology != nullptr && opt_printer_technology->value != printer_technology) {
		printer_technology = opt_printer_technology->value;
		this->plater()->set_printer_technology(printer_technology);
	}
#if 0
	for (auto tab : wxGetApp().tabs_list)
		if (tab->supports_printer_technology(printer_technology)) {
			if (tab->type() == Slic3r::Preset::TYPE_PRINTER)
				static_cast<TabPrinter*>(tab)->update_pages();
			tab->load_config(config);
		}
    if (m_plater)
        m_plater->on_config_change(config);
#else
	// Load the currently selected preset into the GUI, update the preset selection box.
    //FIXME this is not quite safe for multi-extruder printers,
    // as the number of extruders is not adjusted for the vector values.
    // (see PresetBundle::update_multi_material_filament_presets())
    // Better to call PresetBundle::load_config() instead?
    for (auto tab : wxGetApp().tabs_list)
        if (tab->supports_printer_technology(printer_technology)) {
            // Only apply keys, which are present in the tab's config. Ignore the other keys.
			for (const std::string &opt_key : tab->get_config()->diff(config))
				// Ignore print_settings_id, printer_settings_id, filament_settings_id etc.
				if (! boost::algorithm::ends_with(opt_key, "_settings_id"))
					tab->get_config()->option(opt_key)->set(config.option(opt_key));
        }
    
    wxGetApp().load_current_presets();
#endif
}

void MainFrame::select_tab(Tab* tab)
{
    if (!tab)
        return;
    int page_idx = m_tabpanel->FindPage(tab);
    if (page_idx != wxNOT_FOUND && m_layout == ESettingsLayout::Dlg)
        page_idx++;
    select_tab(size_t(page_idx));
}

void MainFrame::ShowPressureAdvanceDialog()
{
    ACPressureAdvanceDialog pressureAdvance_dialog(this);

    if (pressureAdvance_dialog.ShowModal() == wxID_OK) {
        if (wxGetApp().plater()->new_project() == false)
            return;

        wxString paStlPath = from_u8(Slic3r::resources_dir() + "/calib/PressureAdvance/pressure_advance_test.stl");
        wxGetApp().plater()->add_model(false, paStlPath, false);

        Calib_Params params;
        params.start         = pressureAdvance_dialog.get_start_value();
        params.end           = pressureAdvance_dialog.get_end_value()  ;
        params.step          = pressureAdvance_dialog.get_step_value() ;
        params.mode          = CalibMode::Calib_PA_Line;
        params.print_numbers = true;
        // set model
        m_plater->fff_print().set_calib_params(params);

        // start slicer
        m_plater->doSlicer();
    }
}

void MainFrame::select_tab(size_t tab/* = size_t(-1)*/)
{
    bool tabpanel_was_hidden = false;
    /*m_settings_dialog.setDialogObj(setMarkWindow(this, &m_settings_dialog,false));*/
    // Controls on page are created on active page of active tab now.
    // We should select/activate tab before its showing to avoid an UI-flickering
    auto select = [this, tab](bool was_hidden) {
        // when tab == -1, it means we should show the last selected tab
        size_t new_selection = tab == (size_t)(-1) ? m_last_selected_tab : (m_layout == ESettingsLayout::Dlg && tab != 0) ? tab - 1 : tab;

        if (m_tabpanel->GetSelection() != (int)new_selection)
            m_tabpanel->SetSelection(new_selection);
// #ifdef _MSW_DARK_MODE
//         if (wxGetApp().tabs_as_menu()) {
//             if (Tab* cur_tab = dynamic_cast<Tab*>(m_tabpanel->GetPage(new_selection)))
//                 update_marker_for_tabs_menu((m_layout == ESettingsLayout::Old ? m_menubar : m_settings_dialog.menubar()), cur_tab->title(), m_layout == ESettingsLayout::Old);
//             else if (tab == 0 && m_layout == ESettingsLayout::Old)
//                 m_plater->get_current_canvas3D()->render();
//         }
// #endif
        if (tab == 0 && m_layout == ESettingsLayout::Old)
            m_plater->canvas3D()->render();
        else if (was_hidden) {
            Tab* cur_tab = dynamic_cast<Tab*>(m_tabpanel->GetPage(new_selection));
            if (cur_tab)
                cur_tab->OnActivate();
        }
    };

    if (m_layout == ESettingsLayout::Dlg) {
        if (tab==0) {
            if (m_settings_dialog.IsShown())
                this->SetFocus();
            // plater should be focused for correct navigation inside search window
            //if (m_plater->canvas3D()->is_search_pressed())
            //    m_plater->SetFocus();
            return;
        }
        // Show/Activate Settings Dialog
#ifdef __WXOSX__ // Don't call SetFont under OSX to avoid name cutting in ObjectList
        if (m_settings_dialog.IsShown())
            m_settings_dialog.Hide();
        else
            tabpanel_was_hidden = true;
        auto nowSelection = m_tabpanel->GetSelection();
        select(tabpanel_was_hidden);
        m_tabpanel->Show();
        //m_settings_dialog.Show();
        if (nowSelection == 2) {
            Tab *cur_tab = dynamic_cast<Tab *>(m_tabpanel->GetPage(2));
            if (cur_tab->GetModeButtonObj() != nullptr) {
                ConfigOptionMode nowMode = wxGetApp().get_mode();
                if (nowMode == comExpert) {
                    cur_tab->GetModeButtonObj()->SetSwitchState((ACSwitchButton::SwitchState) comSimple, true);
                } else {
                    cur_tab->GetModeButtonObj()->SetSwitchState((ACSwitchButton::SwitchState) comExpert, true);
                }
                cur_tab->GetModeButtonObj()->SetSwitchState((ACSwitchButton::SwitchState) nowMode, true);
            }
        }
        m_settings_dialog.ShowModal();
#else
        if (m_settings_dialog.IsShown()) {
            select(false);
            m_settings_dialog.SetFocus();
        }
        else {
            tabpanel_was_hidden = true;
            select(tabpanel_was_hidden);
            m_tabpanel->Show();
            m_settings_dialog.ShowModal();
        }
#endif
        if (m_settings_dialog.IsIconized())
            m_settings_dialog.Iconize(false);
    }
    else if (m_layout == ESettingsLayout::New) {
        m_main_sizer->Show(m_plater, tab == 0);
        tabpanel_was_hidden = !m_main_sizer->IsShown(m_tabpanel);
        select(tabpanel_was_hidden);
        m_main_sizer->Show(m_tabpanel, tab != 0);

        // plater should be focused for correct navigation inside search window
        if (tab == 0)
            m_plater->SetFocus();
        Layout();
    }
    else {
        select(false);
// #ifdef _MSW_DARK_MODE
//         if (wxGetApp().tabs_as_menu() && tab == 0)
//             m_plater->SetFocus();
// #endif
    }

    // When we run application in ESettingsLayout::New or ESettingsLayout::Dlg mode, tabpanel is hidden from the very beginning
    // and as a result Tab::update_changed_tree_ui() function couldn't update m_is_nonsys_values values,
    // which are used for update TreeCtrl and "revert_buttons".
    // So, force the call of this function for Tabs, if tab panel was hidden
    if (tabpanel_was_hidden)
        for (auto cur_tab : wxGetApp().tabs_list)
            cur_tab->update_changed_tree_ui();

    //// when tab == -1, it means we should show the last selected tab
    //size_t new_selection = tab == (size_t)(-1) ? m_last_selected_tab : (m_layout == ESettingsLayout::Dlg && tab != 0) ? tab - 1 : tab;
    //if (m_tabpanel->GetSelection() != new_selection)
    //    m_tabpanel->SetSelection(new_selection);
    //if (tabpanel_was_hidden)
    //    static_cast<Tab*>(m_tabpanel->GetPage(new_selection))->OnActivate();
}

// Set a camera direction, zoom to all objects.
void MainFrame::select_view(const std::string& direction)
{
     if (m_plater)
         m_plater->select_view(direction);
}

// Select Plater or Universal show
void MainFrame::select_main_panel(int index)
{
	if (0 == index)// m_plater
	{
		m_toolbar->Show();
		m_plater->Show();
		m_universal->Hide();
	}
	else
	{
		m_toolbar->Hide();
		m_plater->Hide();
		m_universal->Show();
	}
	Layout();
	Refresh();
}

// #ys_FIXME_to_delete
void MainFrame::on_presets_changed(SimpleEvent &event)
{
    auto *tab = dynamic_cast<Tab*>(event.GetEventObject());
    wxASSERT(tab != nullptr);
    if (tab == nullptr) {
        return;
    }

    // Update preset combo boxes(Print settings, Filament, Material, Printer) from their respective tabs.
    auto presets = tab->get_presets();
    if (m_plater != nullptr && presets != nullptr) {

        // FIXME: The preset type really should be a property of Tab instead
        Slic3r::Preset::Type preset_type = tab->type();
        if (preset_type == Slic3r::Preset::TYPE_INVALID) {
            wxASSERT(false);
            return;
        }

        m_plater->on_config_change(*tab->get_config());
        //m_plater->sidebar().update_presets(preset_type);
    }
}

// #ys_FIXME_to_delete
void MainFrame::on_value_changed(wxCommandEvent& event)
{
    auto *tab = dynamic_cast<Tab*>(event.GetEventObject());
    wxASSERT(tab != nullptr);
    if (tab == nullptr)
        return;

    auto opt_key = event.GetString();
    if (m_plater) {
        m_plater->on_config_change(*tab->get_config()); // propagate config change events to the plater
        //if (opt_key == "extruders_count") {
        //    auto value = event.GetInt();
        //    m_plater->on_extruders_change(value);
        //}
    }
}

void MainFrame::on_config_changed(DynamicPrintConfig* config) const
{
    if (m_plater)
        m_plater->on_config_change(*config); // propagate config change events to the plater
}

void MainFrame::add_to_recent_projects(const wxString& filename)
{
    if (wxFileExists(filename))
    {
        m_recent_projects.AddFileToHistory(filename);
        std::vector<std::string> recent_projects;
        size_t count = m_recent_projects.GetCount();
        for (size_t i = 0; i < count; ++i)
        {
            recent_projects.push_back(into_u8(m_recent_projects.GetHistoryFile(i)));
        }
        wxGetApp().app_config->set_recent_projects(recent_projects);
    }
}

void MainFrame::technology_changed()
{
    // update menu titles
    PrinterTechnology pt = plater()->printer_technology();
    if (int id = m_menubar->FindMenu(pt == ptFFF ? _L("Material Settings") : _L("Filament Settings")); id != wxNOT_FOUND)
        m_menubar->SetMenuLabel(id , pt == ptSLA ? _L("Material Settings") : _L("Filament Settings"));

    //if (wxGetApp().tab_panel()->GetSelection() != wxGetApp().tab_panel()->GetPageCount() - 1)
    //    wxGetApp().tab_panel()->SetSelection(wxGetApp().tab_panel()->GetPageCount() - 1);

}

//
// Called after the Preferences dialog is closed and the program settings are saved.
// Update the UI based on the current preferences.
void MainFrame::update_ui_from_settings()
{
//    const bool bp_on = wxGetApp().app_config->get_bool("background_processing");
//     m_menu_item_reslice_now->Enable(!bp_on);
//    m_plater->sidebar().show_reslice(!bp_on);
//    m_plater->sidebar().show_export(bp_on);
//    m_plater->sidebar().Layout();

    if (m_plater)
        m_plater->update_ui_from_settings();
    for (auto tab: wxGetApp().tabs_list)
        tab->update_ui_from_settings();
}

std::string MainFrame::get_base_name(const wxString &full_name, const char *extension) const 
{
    boost::filesystem::path filename = boost::filesystem::path(full_name.wx_str()).filename();
    if (extension != nullptr)
		filename = filename.replace_extension(extension);
    return filename.string();
}

std::string MainFrame::get_dir_name(const wxString &full_name) const 
{
    return boost::filesystem::path(full_name.wx_str()).parent_path().string();
}


// ----------------------------------------------------------------------------
// SettingsDialog
// ----------------------------------------------------------------------------

SettingsDialog::SettingsDialog(MainFrame* mainframe)
    : DPIDialog(mainframe,
               wxID_ANY,
                format_wxstr(wxGetApp().appName()) + " - " + _L("Settings"),
               wxDefaultPosition,
               wxDefaultSize,
               wxFRAME_SHAPED | wxNO_BORDER ,
               "settings_dialog", mainframe->normal_font().GetPointSize())
    ,
//: DPIDialog(mainframe, wxID_ANY, wxString(SLIC3R_APP_NAME) + " - " + _L("Settings"), wxDefaultPosition, wxDefaultSize,
//        wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER | wxMINIMIZE_BOX | wxMAXIMIZE_BOX, "settings_dialog"),
    m_main_frame(mainframe)
{
    AddWindowDrakEdg(this);
    if (wxGetApp().is_gcode_viewer())
        return;

#if defined(__WXMSW__)
    // ys_FIXME! temporary workaround for correct font scaling
    // Because of from wxWidgets 3.1.3 auto rescaling is implemented for the Fonts,
    // From the very beginning set dialog font to the wxSYS_DEFAULT_GUI_FONT
    this->SetFont(wxSystemSettings::GetFont(wxSYS_DEFAULT_GUI_FONT));
#else
    this->SetFont(wxGetApp().normal_font());
    this->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_WINDOW));
#endif // __WXMSW__

    // Load the icon either from the exe, or from the ico file.
#if _WIN32
    {
        TCHAR szExeFileName[MAX_PATH];
        GetModuleFileName(nullptr, szExeFileName, MAX_PATH);
        SetIcon(wxIcon(szExeFileName, wxBITMAP_TYPE_ICO));
    }
#else
    SetIcon(wxIcon(var("logo_128px.png"), wxBITMAP_TYPE_PNG));
#endif // _WIN32
    float m_em = em_unit();
    this->Bind(wxEVT_SHOW, [this, mainframe, m_em](wxShowEvent &evt) {

        auto key_up_handker = [this](wxKeyEvent& evt) {
            if ((evt.GetModifiers() & wxMOD_CONTROL) != 0) {
                switch (evt.GetKeyCode()) {
                case '1': { m_main_frame->select_tab(size_t(0)); break; }
                case '2': { m_main_frame->select_tab(1); break; }
                case '3': { m_main_frame->select_tab(2); break; }
                case '4': { m_main_frame->select_tab(3); break; }
#ifdef __APPLE__
                case 'f':
#else /* __APPLE__ */
                case WXK_CONTROL_F:
#endif /* __APPLE__ */
                case 'F': { m_main_frame->plater()->search(false); break; }
                default:break;
                }
            }
        };

        if (evt.IsShown()) {
            m_isVisible = true;
            if (m_tabpanel != nullptr)
                m_tabpanel->Bind(wxEVT_KEY_UP, key_up_handker);
        }
        else {
            if (m_tabpanel != nullptr)
                m_tabpanel->Unbind(wxEVT_KEY_UP, key_up_handker);
        }
        //this->SetWindowStyleFlag(wxSTAY_ON_TOP);
        });
    //just hide the Frame on closing
    /*this->Bind(wxEVT_CLOSE_WINDOW, [this, mainframe](wxCloseEvent &evt) {
        this->Hide();
        m_isVisible = false;
        });*/


#ifdef _MSW_DARK_MODE
    //if (wxGetApp().tabs_as_menu()) {
    //    // menubar
    //    m_menubar = new wxMenuBar();
    //    add_tabs_as_menu(m_menubar, mainframe, this);
    //    this->SetMenuBar(m_menubar);
    //}
#endif

    //this->SetBackgroundColour(wxColour(255,0,0));

    const wxSize min_size = wxSize(99 * m_em, 66 * m_em);

    m_dialogTopbar = new ACDialogTopbar(this, _L("Configuration Manage"),99);

    m_panelSizer = new wxBoxSizer(wxVERTICAL);
    // initialize layout
    auto sizer = new wxBoxSizer(wxVERTICAL);
    sizer->SetSizeHints(this);
    sizer->Add(m_dialogTopbar, 0, wxEXPAND|wxTOP|wxLEFT|wxRIGHT,1);
    sizer->Add(m_panelSizer, 1, wxEXPAND | wxBOTTOM | wxLEFT | wxRIGHT, 1);
    Bind(wxEVT_CLOSE_WINDOW, [this](wxCloseEvent &event) {
        this->EndModal(wxID_NO);
        event.Skip();
    });
    SetSizer(sizer);
    //Fit();
    int screenwidth  = wxSystemSettings::GetMetric(wxSYS_SCREEN_X, NULL);
    int screenheight = wxSystemSettings::GetMetric(wxSYS_SCREEN_Y, NULL);
    SetPosition(wxPoint((screenwidth - 99 * m_em) / 2, (screenheight - 66 * m_em) / 2));
    //wxGraphicsPath path = wxGraphicsRenderer::GetDefaultRenderer()->CreatePath();
    //path.AddRoundedRectangle(0, 0, min_size.x, min_size.y, 2 * em_unit());
    //SetShape(path);

#ifdef __APPLE__
    // Using SetMinSize() on Mac messes up the window position in some cases
    // cf. https://groups.google.com/forum/#!topic/wx-users/yUKPBBfXWO0
    SetSize(min_size);
#else
    SetMinSize(min_size);
    SetSize(GetMinSize());
#endif
    Layout();
}

void SettingsDialog::on_dpi_changed(const wxRect& suggested_rect)
{
    if (wxGetApp().is_gcode_viewer())
        return;

    const int& em = em_unit();
    const wxSize& size = wxSize(99 * em, 66 * em);

// #ifdef _MSW_DARK_MODE
//     // update common mode sizer
//     if (!wxGetApp().tabs_as_menu())
        dynamic_cast<Notebook*>(m_tabpanel)->Rescale();
// #endif

    // update Tabs
    for (auto tab : wxGetApp().tabs_list)
        tab->msw_rescale();

    SetMinSize(size);
    Fit();
    Refresh();
}


} // GUI
} // Slic3r
