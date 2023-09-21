#include "wxExtensions.hpp"

#include <stdexcept>
#include <cmath>

#include <wx/sizer.h>

#include <boost/algorithm/string/replace.hpp>

#include "BitmapCache.hpp"
#include "GUI.hpp"
#include "GUI_App.hpp"
#include "GUI_ObjectList.hpp"
#include "I18N.hpp"
#include "GUI_Utils.hpp"
#include "Plater.hpp"
#include "../Utils/MacDarkMode.hpp"
#include "BitmapComboBox.hpp"
#include "libslic3r/Utils.hpp"
#include "OG_CustomCtrl.hpp"
#include "format.hpp"
#include "GUI_App.hpp"
#include "libslic3r/Color.hpp"

#ifndef __linux__
// msw_menuitem_bitmaps is used for MSW and OSX
static std::map<int, std::string> msw_menuitem_bitmaps;

ACMenuItem::ACMenuItem(wxMenu *parentMenu,int id,const wxString &name,
                       const wxString &help,wxItemKind kind,wxMenu *subMenu)
    : wxMenuItem(parentMenu, id, name, help, kind, subMenu)
{}
#if wxUSE_OWNER_DRAWN
bool ACMenuItem::OnDrawItem(wxDC &dc, const wxRect &rc, wxODAction act, wxODStatus stat) {
    // wxMenuItem::OnDrawItem(dc, rc, act, stat);
    wxRect   newRc(rc.GetLeft(), rc.GetTop(), rc.width, rc.height);
    wxColour paintColor;
    if ((stat & wxODSelected) || (stat & wxODHasFocus)) {
        paintColor = AC_COLOR_ITEM_HOVER;
    } else {
        // if (stat & wxODChecked) {
        if (getCheckState()) {
            paintColor = AC_COLOR_MLIST_ITEM_SELECT;
        } else {
            paintColor = AC_COLOR_BG_WHITE;
        }
    }
    wxFont font;
    GetFontToUse(font);
    dc.SetFont(font);
    dc.SetBrush(paintColor);
    dc.SetPen(paintColor);
    dc.DrawRectangle(newRc);
    wxString text = GetName();
    wxSize   textSize;
    wxCoord  w, h;
    dc.GetTextExtent(text, &w, &h);
    textSize          = wxSize(w, h);
    wxString new_text = GetItemLabel();
    text              = new_text.BeforeFirst('\t');
    text.Replace("&", "");

    if ((stat & wxODDisabled) && ((stat & wxODGrayed) || (stat & wxODHidePrefix))) {
        dc.SetTextForeground(AC_COLOR_FONT_GRAY);
    } else {
        dc.SetTextForeground(AC_COLOR_BLACK);
    }
    int x = newRc.GetLeft() + 8;
    int y = newRc.GetTop() + (newRc.GetBottom() - newRc.GetTop() - textSize.y) / 2;

    dc.DrawText(text, x, y);

    wxString accel = new_text.AfterFirst('\t');
    if (!accel.empty()) {
        wxSize  accelSize;
        wxCoord w_r, h_r;
        accel.Replace("&", "");
        dc.GetTextExtent(accel, &w_r, &h_r);
        accelSize = wxSize(w_r, h_r);
        x         = newRc.GetRight() - w_r - 9;
        y         = newRc.GetTop() + (newRc.GetBottom() - newRc.GetTop() - accelSize.y) / 2;

        dc.DrawText(accel, x, y);
    }
    return true;
}
bool ACMenuItem::OnMeasureItem(size_t *width, size_t *height) {
    if (IsOwnerDrawn()) {
        wxMemoryDC dc;
        wxFont     font;
        GetFontToUse(font);
        dc.SetFont(font);

        // item name/text without mnemonics
        wxString name = wxStripMenuCodes(GetItemLabel(), wxStrip_Mnemonics);
        wxCoord  w, h;
        dc.GetTextExtent(name, &w, &h);
        double scale = GUI::wxGetApp().em_unit() / 10.0f;
        *width       = 1.3 * w * scale;
        *height      = 1.5 * h * scale;
    } else {
        *width  = 0;
        *height = 0;
    }

    return true;
}
#endif

void sys_color_changed_menu(wxMenu* menu)
{
	struct update_icons {
		static void run(wxMenuItem* item) {
			const auto it = msw_menuitem_bitmaps.find(item->GetId());
			if (it != msw_menuitem_bitmaps.end()) {
				wxBitmapBundle* item_icon = get_bmp_bundle(it->second);
				if (item_icon->IsOk())
					item->SetBitmap(*item_icon);
			}
			if (item->IsSubMenu())
				for (wxMenuItem *sub_item : item->GetSubMenu()->GetMenuItems())
					update_icons::run(sub_item);
		}
	};

	for (wxMenuItem *item : menu->GetMenuItems())
		update_icons::run(item);
}
#endif /* no __linux__ */


void enable_menu_item(wxUpdateUIEvent& evt, std::function<bool()> const cb_condition, wxMenuItem* item, wxWindow* win)
{
    const bool enable = cb_condition();
    evt.Enable(enable);
}

wxMenuItem* append_menu_item(wxMenu* menu, int id, const wxString& string, const wxString& description,
    std::function<void(wxCommandEvent& event)> cb, wxBitmapBundle* icon, wxEvtHandler* event_handler,
    std::function<bool()> const cb_condition, wxWindow* parent, int insert_pos/* = wxNOT_FOUND*/)
{
    if (id == wxID_ANY)
        id = wxNewId();

    //auto *item = new wxMenuItem(menu, id, string, description);
    auto *item = new ACMenuItem(menu, id, string, description);
#ifdef WIN32
    // todo: 这里需要支持macos的特定特性
    item->SetBackgroundColour(AC_COLOR_BG_WHITE);
    item->SetOwnerDrawn(!item->IsSeparator());
    // todo: 这里需要支持macos的特定特性
#endif  //WIN32
    if (icon && icon->IsOk()) {
        item->SetBitmap(*icon);
    }
    if (insert_pos == wxNOT_FOUND)
        menu->Append(item);
    else
        menu->Insert(insert_pos, item);

#ifdef __WXMSW__
    if (event_handler != nullptr && event_handler != menu)
        event_handler->Bind(wxEVT_MENU, cb, id);
    else
#endif // __WXMSW__
        menu->Bind(wxEVT_MENU, cb, id);

    /*auto _event = wxMenuEvent(wxEVT_ENTER_WINDOW, id, menu);
    wxPostEvent(menu, _event);
    auto _event_2 = wxMenuEvent(wxEVT_LEAVE_WINDOW, id, menu);
    wxPostEvent(menu, _event_2);
    menu->Bind(
        wxEVT_ENTER_WINDOW,
        [item](wxMouseEvent &) {
            item->SetBackgroundColour(wxColour(255, 0, 0));
        },
        id);
    menu->Bind(
        wxEVT_LEAVE_WINDOW,
        [item](wxMouseEvent &) {
            item->SetBackgroundColour(wxColour(255, 255,255));
        },
        id);*/


    if (parent) {
        parent->Bind(wxEVT_UPDATE_UI, [cb_condition, item, parent](wxUpdateUIEvent& evt) {
                enable_menu_item(evt, cb_condition, item, parent);
            },id);
    }

    return item;
}

wxMenuItem* append_menu_item(wxMenu* menu, int id, const wxString& string, const wxString& description,
    std::function<void(wxCommandEvent& event)> cb, const std::string& icon, wxEvtHandler* event_handler,
    std::function<bool()> const cb_condition, wxWindow* parent, int insert_pos/* = wxNOT_FOUND*/)
{
    if (id == wxID_ANY)
        id = wxNewId();

    wxBitmapBundle* bmp = icon.empty() ? nullptr : get_bmp_bundle(icon);

#ifndef __linux__
    if (bmp && bmp->IsOk())
        msw_menuitem_bitmaps[id] = icon;
#endif /* no __linux__ */

    return append_menu_item(menu, id, string, description, cb, bmp, event_handler, cb_condition, parent, insert_pos);
}

wxMenuItem* append_submenu(wxMenu* menu, wxMenu* sub_menu, int id, const wxString& string, const wxString& description, const std::string& icon,
    std::function<bool()> const cb_condition, wxWindow* parent)
{
    if (id == wxID_ANY)
        id = wxNewId();

    //wxMenuItem* item = new wxMenuItem(menu, id, string, description);
    wxMenuItem *item = new ACMenuItem(menu, id, string, description);
#ifdef WIN32
    item->SetBackgroundColour(AC_COLOR_BG_WHITE);
    item->SetOwnerDrawn(!item->IsSeparator());
#endif //WIN32
    if (!icon.empty()) {
        item->SetBitmap(*get_bmp_bundle(icon));

#ifndef __linux__
        msw_menuitem_bitmaps[id] = icon;
#endif // no __linux__
    }

    item->SetSubMenu(sub_menu);
    menu->Append(item);

    if (parent) {
        parent->Bind(wxEVT_UPDATE_UI, [cb_condition, item, parent](wxUpdateUIEvent& evt) {
            enable_menu_item(evt, cb_condition, item, parent); }, id);
    }

    return item;
}

wxMenuItem* append_menu_radio_item(wxMenu* menu, int id, const wxString& string, const wxString& description,
    std::function<void(wxCommandEvent& event)> cb, wxEvtHandler* event_handler)
{
    if (id == wxID_ANY)
        id = wxNewId();

    //wxMenuItem* item = menu->AppendRadioItem(id, string, description);
    auto *item = new ACMenuItem(menu, id, string, description, wxITEM_RADIO);
#ifdef WIN32
    item->SetBackgroundColour(AC_COLOR_BG_WHITE);
    item->SetOwnerDrawn(!item->IsSeparator());
#endif //WIN32
    
    menu->Append(item);
#ifdef __WXMSW__
    if (event_handler != nullptr && event_handler != menu)
        event_handler->Bind(wxEVT_MENU, cb, id);
    else
#endif // __WXMSW__
        menu->Bind(wxEVT_MENU, cb, id);

    return item;
}

wxMenuItem* append_menu_check_item(wxMenu* menu, int id, const wxString& string, const wxString& description,
    std::function<void(wxCommandEvent & event)> cb, wxEvtHandler* event_handler,
    std::function<bool()> const enable_condition, std::function<bool()> const check_condition, wxWindow* parent)
{
    if (id == wxID_ANY)
        id = wxNewId();

    //wxMenuItem* item = menu->AppendCheckItem(id, string, description);
    //auto *item = new ACMenuItem(menu, id, string, description, wxITEM_CHECK);
    auto *item = new ACMenuItem(menu, id, string, description);
#ifdef WIN32
    item->SetBackgroundColour(AC_COLOR_BG_WHITE);
    item->SetOwnerDrawn(!item->IsSeparator());
#endif //WIN32
    menu->Append(item);
#ifdef __WXMSW__
    if (event_handler != nullptr && event_handler != menu)
        event_handler->Bind(wxEVT_MENU, cb, id);
    else
#endif // __WXMSW__
        menu->Bind(wxEVT_MENU, cb, id);

    if (parent)
        parent->Bind(
            wxEVT_UPDATE_UI,
            [enable_condition, check_condition, item, parent](wxUpdateUIEvent &evt)
            {
                item->setCheckState(check_condition());
                //evt.Enable(enable_condition());
                enable_menu_item(evt, enable_condition, item, parent);
                evt.Check(check_condition());
            }, id);

    return item;
}

const unsigned int wxCheckListBoxComboPopup::DefaultWidth = 200;
const unsigned int wxCheckListBoxComboPopup::DefaultHeight = 200;

bool wxCheckListBoxComboPopup::Create(wxWindow* parent)
{
    return wxCheckListBox::Create(parent, wxID_HIGHEST + 1, wxPoint(0, 0));
}

wxWindow* wxCheckListBoxComboPopup::GetControl()
{
    return this;
}

void wxCheckListBoxComboPopup::SetStringValue(const wxString& value)
{
    m_text = value;
}

wxString wxCheckListBoxComboPopup::GetStringValue() const
{
    return m_text;
}

wxSize wxCheckListBoxComboPopup::GetAdjustedSize(int minWidth, int prefHeight, int maxHeight)
{
    // set width dinamically in dependence of items text
    // and set height dinamically in dependence of items count

    wxComboCtrl* cmb = GetComboCtrl();
    if (cmb != nullptr) {
        wxSize size = GetComboCtrl()->GetSize();

        unsigned int count = GetCount();
        if (count > 0) {
            int max_width = size.x;
            for (unsigned int i = 0; i < count; ++i) {
                max_width = std::max(max_width, 60 + GetTextExtent(GetString(i)).x);
            }
            size.SetWidth(max_width);
            size.SetHeight(count * cmb->GetCharHeight());
        }
        else
            size.SetHeight(DefaultHeight);

        return size;
    }
    else
        return wxSize(DefaultWidth, DefaultHeight);
}

void wxCheckListBoxComboPopup::OnKeyEvent(wxKeyEvent& evt)
{
    // filters out all the keys which are not working properly
    switch (evt.GetKeyCode())
    {
    case WXK_LEFT:
    case WXK_UP:
    case WXK_RIGHT:
    case WXK_DOWN:
    case WXK_PAGEUP:
    case WXK_PAGEDOWN:
    case WXK_END:
    case WXK_HOME:
    case WXK_NUMPAD_LEFT:
    case WXK_NUMPAD_UP:
    case WXK_NUMPAD_RIGHT:
    case WXK_NUMPAD_DOWN:
    case WXK_NUMPAD_PAGEUP:
    case WXK_NUMPAD_PAGEDOWN:
    case WXK_NUMPAD_END:
    case WXK_NUMPAD_HOME:
    {
        break;
    }
    default:
    {
        evt.Skip();
        break;
    }
    }
}

void wxCheckListBoxComboPopup::OnCheckListBox(wxCommandEvent& evt)
{
    // forwards the checklistbox event to the owner wxComboCtrl

    if (m_check_box_events_status == OnCheckListBoxFunction::FreeToProceed )
    {
        wxComboCtrl* cmb = GetComboCtrl();
        if (cmb != nullptr) {
            wxCommandEvent event(wxEVT_CHECKLISTBOX, cmb->GetId());
            event.SetEventObject(cmb);
            cmb->ProcessWindowEvent(event);
        }
    }

    evt.Skip();

    #ifndef _WIN32  // events are sent differently on OSX+Linux vs Win (more description in header file)
        if ( m_check_box_events_status == OnCheckListBoxFunction::RefuseToProceed )
            // this happens if the event was resent by OnListBoxSelection - next call to OnListBoxSelection is due to user clicking the text, so the function should
            // explicitly change the state on the checkbox
            m_check_box_events_status = OnCheckListBoxFunction::WasRefusedLastTime;
        else
            // if the user clicked the checkbox square, this event was sent before OnListBoxSelection was called, so we don't want it to resend it
            m_check_box_events_status = OnCheckListBoxFunction::RefuseToProceed;
    #endif
}

void wxCheckListBoxComboPopup::OnListBoxSelection(wxCommandEvent& evt)
{
    // transforms list box item selection event into checklistbox item toggle event 

    int selId = GetSelection();
    if (selId != wxNOT_FOUND)
    {
        #ifndef _WIN32
            if (m_check_box_events_status == OnCheckListBoxFunction::RefuseToProceed)
        #endif
                Check((unsigned int)selId, !IsChecked((unsigned int)selId));

        m_check_box_events_status = OnCheckListBoxFunction::FreeToProceed; // so the checkbox reacts to square-click the next time

        SetSelection(wxNOT_FOUND);
        wxCommandEvent event(wxEVT_CHECKLISTBOX, GetId());
        event.SetInt(selId);
        event.SetEventObject(this);
        ProcessEvent(event);
    }
}


// ***  wxDataViewTreeCtrlComboPopup  ***

const unsigned int wxDataViewTreeCtrlComboPopup::DefaultWidth = 270;
const unsigned int wxDataViewTreeCtrlComboPopup::DefaultHeight = 200;
const unsigned int wxDataViewTreeCtrlComboPopup::DefaultItemHeight = 22;

bool wxDataViewTreeCtrlComboPopup::Create(wxWindow* parent)
{
	return wxDataViewTreeCtrl::Create(parent, wxID_ANY/*HIGHEST + 1*/, wxPoint(0, 0), wxDefaultSize/*wxSize(270, -1)*/, wxDV_NO_HEADER);
}
/*
wxSize wxDataViewTreeCtrlComboPopup::GetAdjustedSize(int minWidth, int prefHeight, int maxHeight)
{
	// matches owner wxComboCtrl's width
	// and sets height dinamically in dependence of contained items count
	wxComboCtrl* cmb = GetComboCtrl();
	if (cmb != nullptr)
	{
		wxSize size = GetComboCtrl()->GetSize();
		if (m_cnt_open_items > 0)
			size.SetHeight(m_cnt_open_items * DefaultItemHeight);
		else
			size.SetHeight(DefaultHeight);

		return size;
	}
	else
		return wxSize(DefaultWidth, DefaultHeight);
}
*/
void wxDataViewTreeCtrlComboPopup::OnKeyEvent(wxKeyEvent& evt)
{
	// filters out all the keys which are not working properly
	if (evt.GetKeyCode() == WXK_UP)
	{
		return;
	}
	else if (evt.GetKeyCode() == WXK_DOWN)
	{
		return;
	}
	else
	{
		evt.Skip();
		return;
	}
}

void wxDataViewTreeCtrlComboPopup::OnDataViewTreeCtrlSelection(wxCommandEvent& evt)
{
	wxComboCtrl* cmb = GetComboCtrl();
	auto selected = GetItemText(GetSelection());
	cmb->SetText(selected);
}
void AddWindowDrakEdg(wxWindow *child_win, wxColour penColor)
{
    child_win->Bind(wxEVT_PAINT, [child_win, penColor](wxPaintEvent &evt) {
        wxPaintDC dc(child_win);
        dc.SetPen(wxPen(penColor));
        dc.DrawRectangle(wxPoint(0, 0), child_win->GetClientSize());
        evt.Skip();
    });
}

wxDialog *setMarkWindow(wxWindow *m_parent, wxWindow *child_win, bool bind, bool activateIndex)
{
    /*
    * m_parent: parent window
    * child_win: dialog window
    * bind: bind event or not ? deafult bind=true
    * activateIndex:parent window get activate event? deafult activateIndex=false
    */
    wxDialog* markDialog = new wxDialog(m_parent, wxID_ANY, "", wxDefaultPosition, m_parent->GetSize(), wxNO_BORDER);
    markDialog->SetExtraStyle(markDialog->GetExtraStyle() | wxWS_EX_VALIDATE_RECURSIVELY);
#ifdef __APPLE__
    markDialog->SetWindowStyleFlag(markDialog->GetWindowStyleFlag() | wxSTAY_ON_TOP);
#endif
    markDialog->SetParent(m_parent);
#ifdef __WXMSW__
    HWND hwnd = (HWND) markDialog->GetHandle();
    ::SetWindowLong(hwnd, GWL_EXSTYLE, ::GetWindowLong(hwnd, GWL_EXSTYLE) | WS_EX_TOOLWINDOW);
#endif
    markDialog->SetBackgroundColour(wxColor(20, 28, 41));
    markDialog->SetTransparent(66);
    markDialog->Enable(false);
    if (bind) {
        int m_em = em_unit(m_parent);
        //if (activateIndex) {
        m_parent->Bind(wxEVT_ACTIVATE, [markDialog, m_parent, activateIndex, child_win](wxActivateEvent &event) {
            if (activateIndex && child_win != nullptr) {
                if ( event.GetActive() && child_win != nullptr && !child_win->IsBeingDeleted() && child_win->IsShown()) {
                    if (markDialog->IsShown())
                        markDialog->Hide();
                    markDialog->Show();
            }
                if (!child_win->HasFlag(wxSTAY_ON_TOP))
                    child_win->SetWindowStyleFlag(child_win->GetWindowStyleFlag() | wxSTAY_ON_TOP);
            }
            event.Skip();
        });
        m_parent->Bind(wxEVT_SIZE, [markDialog, m_parent, child_win](wxSizeEvent &event) {
            if (child_win != nullptr && !child_win->IsBeingDeleted() && child_win->IsShown()) {
                if (markDialog->IsShown())
                    markDialog->Hide();
                markDialog->Show();
            }
            event.Skip();
        });
        //}
        child_win->Bind(wxEVT_ACTIVATE, [markDialog, m_parent, child_win](wxActivateEvent &event) { 
            if (event.GetActive()) {
                if (!child_win->HasFlag(wxSTAY_ON_TOP))
                    child_win->SetWindowStyleFlag(child_win->GetWindowStyleFlag() | wxSTAY_ON_TOP);
            } else {
                if (child_win->HasFlag(wxSTAY_ON_TOP))
                    child_win->SetWindowStyleFlag(child_win->GetWindowStyleFlag() & ~wxSTAY_ON_TOP);
            }
        });
        child_win->Bind(wxEVT_SHOW, [m_parent, m_em, markDialog, child_win](wxShowEvent &evt) {
            if (evt.IsShown()) {
                wxSize markDialogSize = m_parent->GetSize();
                wxPoint markPosition = m_parent->GetPosition();
                if (dynamic_cast<wxFrame *>(m_parent)) {
                    float m_indexSize = 0.035f;
#if _WIN32
                    m_indexSize = 0.04f;
#endif
                    if (dynamic_cast<wxFrame *>(m_parent)->IsFullScreen())
                        m_indexSize = 0.03f;
                    if (dynamic_cast<wxFrame *>(m_parent)->IsMaximized())
                        m_indexSize = 0.035f;
                    float m_gap = m_indexSize * markDialogSize.y;
                    float m_index = 0.0f;
#if _WIN32
                    if (dynamic_cast<wxFrame *>(m_parent) && !dynamic_cast<wxFrame *>(m_parent)->IsFullScreen()) {
                        m_index = 0.7f;
                    }
#endif
                    float   sizeGap            = m_index * m_em;
                    wxPoint new_markPosition   = wxPoint(markPosition.x + sizeGap, markPosition.y + m_gap);
                    wxSize  new_markDialogSize = wxSize(markDialogSize.x - (2 * sizeGap), markDialogSize.y - m_gap - sizeGap);
                    markPosition               = new_markPosition;
                    markDialogSize             = new_markDialogSize;
                } 
                markDialog->SetPosition(markPosition);
                markDialog->SetSize(markDialogSize);
                if (!markDialog->IsShown()) {
                    markDialog->Show();
                }
            } else {
                if (markDialog->IsShown()) {
                    markDialog->Hide();
                }
            }
            //child_win->SetWindowStyleFlag(wxSTAY_ON_TOP);
            evt.Skip();
        });
        child_win->Bind(wxEVT_CLOSE_WINDOW, [markDialog, m_parent, child_win](wxCloseEvent &evt) {
            if (dynamic_cast<wxFrame *>(m_parent)) {
                if (dynamic_cast<wxFrame *>(m_parent)->IsIconized()) {
                    dynamic_cast<wxFrame *>(m_parent)->Iconize(false);
                    dynamic_cast<wxFrame *>(m_parent)->Restore();
                }
            }
            if (markDialog->IsShown()) {
                markDialog->Hide();
            }
            evt.Skip();
        });
        child_win->Bind(wxEVT_UPDATE_UI, [markDialog, m_em,m_parent](wxUpdateUIEvent &event) {
            if (markDialog) {
                wxSize  markDialogSize = m_parent->GetSize();
                wxPoint markPosition   = m_parent->GetPosition();
                if (dynamic_cast<wxFrame *>(m_parent)) {
                    float m_indexSize = 0.035f;
#if _WIN32
                    m_indexSize = 0.04f;
#endif
                    if (dynamic_cast<wxFrame *>(m_parent)->IsFullScreen())
                        m_indexSize = 0.03f;
                    if (dynamic_cast<wxFrame *>(m_parent)->IsMaximized())
                        m_indexSize = 0.035f;
                    float m_gap   = m_indexSize * markDialogSize.y;
                    float m_index = 0.0f;
#if _WIN32
                    if (dynamic_cast<wxFrame *>(m_parent) && !dynamic_cast<wxFrame *>(m_parent)->IsFullScreen()) {
                        m_index = 0.7f;
                    }
#endif
                    float   sizeGap            = m_index * m_em;
                    wxPoint new_markPosition   = wxPoint(markPosition.x + sizeGap, markPosition.y + m_gap);
                    wxSize  new_markDialogSize = wxSize(markDialogSize.x - (2 * sizeGap), markDialogSize.y - m_gap - sizeGap);
                    markPosition               = new_markPosition;
                    markDialogSize             = new_markDialogSize;
                }
                markDialog->SetPosition(markPosition);
                markDialog->SetSize(markDialogSize);
            }
            event.Skip();
        });
        
    }
    return markDialog;
}

// edit tooltip : change Slic3r to SLIC3R_APP_KEY
// Temporary workaround for localization
void edit_tooltip(wxString& tooltip)
{
    tooltip.Replace("Slic3r", Slic3r::GUI::wxGetApp().appName(), true);
}

/* Function for rescale of buttons in Dialog under MSW if dpi is changed.
 * btn_ids - vector of buttons identifiers
 */
void msw_buttons_rescale(wxDialog* dlg, const int em_unit, const std::vector<int>& btn_ids)
{
    const wxSize& btn_size = wxSize(-1, int(2.5f * em_unit + 0.5f));

    for (int btn_id : btn_ids) {
        // There is a case [FirmwareDialog], when we have wxControl instead of wxButton
        // so let casting everything to the wxControl
        wxControl* btn = static_cast<wxControl*>(dlg->FindWindowById(btn_id, dlg));
        if (btn)
            btn->SetMinSize(btn_size);
    }
}

/* Function for getting of em_unit value from correct parent.
 * In most of cases it is m_em_unit value from GUI_App,
 * but for DPIDialogs it's its own value. 
 * This value will be used to correct rescale after moving between 
 * Displays with different HDPI */
int em_unit(wxWindow* win)
{
    if (win)
    {
        wxTopLevelWindow *toplevel = Slic3r::GUI::find_toplevel_parent(win);
        Slic3r::GUI::DPIDialog* dlg = dynamic_cast<Slic3r::GUI::DPIDialog*>(toplevel);
        if (dlg)
            return dlg->em_unit();
        Slic3r::GUI::DPIFrame* frame = dynamic_cast<Slic3r::GUI::DPIFrame*>(toplevel);
        if (frame)
            return frame->em_unit();
    }
    
    return Slic3r::GUI::wxGetApp().em_unit();
}

int mode_icon_px_size()
{
#ifdef __APPLE__
    return 10;
#else
    return 12;
#endif
}

#ifdef __WXGTK2__
static int scale() 
{
    return int(em_unit(nullptr) * 0.1f + 0.5f);
}
#endif // __WXGTK2__
wxBitmapBundle* get_bmp_bundle(const std::string& bmp_name_in, wxSize px_cnt, const std::string& new_color )
{
#ifdef __WXGTK2__
    px_cnt *= scale();
#endif // __WXGTK2__

    static Slic3r::GUI::BitmapCache cache;

    std::string bmp_name = bmp_name_in;
    boost::replace_last(bmp_name, ".png", "");

    // Try loading an SVG first, then PNG if SVG is not found:
    wxBitmapBundle* bmp = cache.from_svg(bmp_name, px_cnt.x, px_cnt.y, Slic3r::GUI::wxGetApp().dark_mode(), new_color);
    if (bmp == nullptr) {
        bmp = cache.from_png(bmp_name, px_cnt.x, px_cnt.y);
        if (!bmp)
            // Neither SVG nor PNG has been found, raise error
            throw Slic3r::RuntimeError("Could not load bitmap: " + bmp_name);
    }
    return bmp;
}

wxBitmapBundle* get_bmp_bundle(const std::string& bmp_name_in, int px_cnt/* = 16*/, const std::string& new_color/* = std::string()*/)
{
    return get_bmp_bundle(bmp_name_in, wxSize(px_cnt, px_cnt), new_color );
}

wxBitmapBundle* get_empty_bmp_bundle(int width, int height)
{
    static Slic3r::GUI::BitmapCache cache;
#ifdef __WXGTK2__
    return cache.mkclear_bndl(width * scale(), height * scale());
#else
    return cache.mkclear_bndl(width, height);
#endif // __WXGTK2__
}

wxBitmapBundle* get_solid_bmp_bundle(int width, int height, const std::string& color )
{
    static Slic3r::GUI::BitmapCache cache;
#ifdef __WXGTK2__
    return cache.mksolid_bndl(width * scale(), height * scale(), color, 1, Slic3r::GUI::wxGetApp().dark_mode());
#else
    return cache.mksolid_bndl(width, height, color, 1, Slic3r::GUI::wxGetApp().dark_mode());
#endif // __WXGTK2__
}

std::vector<wxBitmapBundle*> get_extruder_color_icons(bool thin_icon/* = false*/)
{
    // Create the bitmap with color bars.
    std::vector<wxBitmapBundle*> bmps;
    std::vector<std::string> colors = Slic3r::GUI::wxGetApp().plater()->get_extruder_colors_from_plater_config();

    if (colors.empty())
        return bmps;

    for (const std::string& color : colors)
        bmps.emplace_back(get_solid_bmp_bundle(thin_icon ? 16 : 32, 16, color));

    return bmps;
}


void apply_extruder_selector(Slic3r::GUI::BitmapComboBox** ctrl, 
                             wxWindow* parent,
                             const std::string& first_item/* = ""*/, 
                             wxPoint pos/* = wxDefaultPosition*/,
                             wxSize size/* = wxDefaultSize*/,
                             bool use_thin_icon/* = false*/)
{
    std::vector<wxBitmapBundle*> icons = get_extruder_color_icons(use_thin_icon);

    if (!*ctrl) {
        *ctrl = new Slic3r::GUI::BitmapComboBox(parent, wxID_ANY, wxEmptyString, pos, size, 0, nullptr, wxCB_READONLY);
        Slic3r::GUI::wxGetApp().UpdateDarkUI(*ctrl);
    }
    else
    {
        (*ctrl)->SetPosition(pos);
        (*ctrl)->SetMinSize(size);
        (*ctrl)->SetSize(size);
        (*ctrl)->Clear();
    }
    if (first_item.empty())
        (*ctrl)->Hide();    // to avoid unwanted rendering before layout (ExtruderSequenceDialog)

    if (icons.empty() && !first_item.empty()) {
        (*ctrl)->Append(_(first_item), wxNullBitmap);
        return;
    }

    // For ObjectList we use short extruder name (just a number)
    const bool use_full_item_name = dynamic_cast<Slic3r::GUI::ObjectList*>(parent) == nullptr;

    int i = 0;
    wxString str = _(L("Extruder"));
    for (wxBitmapBundle* bmp : icons) {
        if (i == 0) {
            if (!first_item.empty())
                (*ctrl)->Append(_(first_item), *bmp);
            ++i;
        }

        (*ctrl)->Append(use_full_item_name
                        ? Slic3r::GUI::from_u8((boost::format("%1% %2%") % str % i).str())
                        : wxString::Format("%d", i), *bmp);
        ++i;
    }
    (*ctrl)->SetSelection(0);
}


// ----------------------------------------------------------------------------
// LockButton
// ----------------------------------------------------------------------------

LockButton::LockButton( wxWindow *parent, 
                        wxWindowID id, 
                        const wxPoint& pos /*= wxDefaultPosition*/, 
                        const wxSize& size /*= wxDefaultSize*/):
                        wxButton(parent, id, wxEmptyString, pos, size, wxBU_EXACTFIT | wxNO_BORDER)
{
    m_bmp_lock_closed   = ScalableBitmap(this, "ACEmpty");
    m_bmp_lock_closed_f = ScalableBitmap(this, "ACEmpty");
    m_bmp_lock_open     = ScalableBitmap(this, "ACEmpty");
    m_bmp_lock_open_f   = ScalableBitmap(this, "ACEmpty");

    Slic3r::GUI::wxGetApp().UpdateDarkUI(this);
    SetBitmap(m_bmp_lock_open.bmp());
    SetBitmapDisabled(m_bmp_lock_open.bmp());
    SetBitmapCurrent(m_bmp_lock_closed_f.bmp());

    //button events
    Bind(wxEVT_BUTTON, &LockButton::OnButton, this);
}

void LockButton::OnButton(wxCommandEvent& event)
{
    if (m_disabled)
        return;

    SetLock(!m_is_pushed);
    event.Skip();
}

void LockButton::SetLock(bool lock)
{
    if (m_is_pushed != lock) {
        m_is_pushed = lock;
        update_button_bitmaps();
    }
}

void LockButton::sys_color_changed()
{
    Slic3r::GUI::wxGetApp().UpdateDarkUI(this);

    m_bmp_lock_closed.sys_color_changed();
    m_bmp_lock_closed_f.sys_color_changed();
    m_bmp_lock_open.sys_color_changed();
    m_bmp_lock_open_f.sys_color_changed();

    update_button_bitmaps();
}

void LockButton::update_button_bitmaps()
{
    SetBitmap(m_is_pushed ? m_bmp_lock_closed.bmp() : m_bmp_lock_open.bmp());
    SetBitmapCurrent(m_is_pushed ? m_bmp_lock_closed_f.bmp() : m_bmp_lock_open_f.bmp());

    Refresh();
    Update();
}



// ----------------------------------------------------------------------------
// ModeButton
// ----------------------------------------------------------------------------

ModeButton::ModeButton( wxWindow *          parent,
                        wxWindowID          id,
                        const std::string&  icon_name   /* = ""*/,
                        const wxString&     mode        /* = wxEmptyString*/,
                        const wxSize&       size        /* = wxDefaultSize*/,
                        const wxPoint&      pos         /* = wxDefaultPosition*/) :
    ScalableButton(parent, id, icon_name, mode, size, pos, wxBU_EXACTFIT)
{
    Init(mode);
}

ModeButton::ModeButton( wxWindow*           parent,
                        const wxString&     mode/* = wxEmptyString*/,
                        const std::string&  icon_name/* = ""*/,
                        int                 px_cnt/* = 16*/) :
    ScalableButton(parent, wxID_ANY, icon_name, mode, wxDefaultSize, wxDefaultPosition, wxBU_EXACTFIT, px_cnt)
{
    Init(mode);
}

ModeButton::ModeButton( wxWindow*           parent,
                        int                 mode_id,/*ConfigOptionMode*/
                        const wxString&     mode /*= wxEmptyString*/,
                        int                 px_cnt /*= = 16*/) :
    ScalableButton(parent, wxID_ANY, "", mode, wxDefaultSize, wxDefaultPosition, wxBU_EXACTFIT, px_cnt),
    m_mode_id(mode_id)
{
    update_bitmap();
    Init(mode);
}

void ModeButton::Init(const wxString &mode)
{
    m_tt_focused  = Slic3r::GUI::format_wxstr(_L("Switch to the %s mode"), mode);
    m_tt_selected = Slic3r::GUI::format_wxstr(_L("Current mode is %s"),    mode);

    SetBitmapMargins(3, 0);

    //button events
    Bind(wxEVT_BUTTON,          &ModeButton::OnButton, this);
    Bind(wxEVT_ENTER_WINDOW,    &ModeButton::OnEnterBtn, this);
    Bind(wxEVT_LEAVE_WINDOW,    &ModeButton::OnLeaveBtn, this);
}

void ModeButton::OnButton(wxCommandEvent& event)
{
    m_is_selected = true;
    focus_button(m_is_selected);

    event.Skip();
}

void ModeButton::SetState(const bool state)
{
    m_is_selected = state;
    focus_button(m_is_selected);
    SetToolTip(state ? m_tt_selected : m_tt_focused);
}

void ModeButton::update_bitmap()
{
    m_bmp = *get_bmp_bundle("ACEmpty", m_pxSize, Slic3r::GUI::wxGetApp().get_mode_btn_color(m_mode_id));

    SetBitmap(m_bmp);
    SetBitmapCurrent(m_bmp);
    SetBitmapPressed(m_bmp);
}

void ModeButton::focus_button(const bool focus)
{
    const wxFont& new_font = focus ? 
                             Slic3r::GUI::wxGetApp().bold_font() : 
                             Slic3r::GUI::wxGetApp().normal_font();

    SetFont(new_font);
#ifdef _WIN32
    GetParent()->Refresh(); // force redraw a background of the selected mode button
#else
    SetForegroundColour(wxSystemSettings::GetColour(focus ? wxSYS_COLOUR_BTNTEXT : 
#if defined (__linux__) && defined (__WXGTK3__)
        wxSYS_COLOUR_GRAYTEXT
#elif defined (__linux__) && defined (__WXGTK2__)
        wxSYS_COLOUR_BTNTEXT
#else 
        wxSYS_COLOUR_BTNSHADOW
#endif    
    ));
#endif /* no _WIN32 */

    Refresh();
    Update();
}

void ModeButton::sys_color_changed()
{
    Slic3r::GUI::wxGetApp().UpdateDarkUI(this, m_has_border);
    update_bitmap();
}

ACModeButtons::ACModeButtons(wxWindow *parent)
    : ACSwitchButton(parent, _L("Simple Mode"), _L("Advanced Mode"), 0)
{
    /*
    enum ConfigOptionMode {
    comSimple = 0,
    comAdvanced,
    comExpert,
    comUndef
    };*/
    auto modebtnfn = [this](wxCommandEvent &event) {
        int mode_id = event.GetInt();
        if (Slic3r::GUI::wxGetApp().save_mode(mode_id))
            event.Skip();
        else
            SetMode(Slic3r::GUI::wxGetApp().get_mode());
    };

    this->Bind(wxEVT_AC_TOGGLEBUTTON, modebtnfn);
}


void ACModeButtons::SetMode(const /*ConfigOptionMode*/int mode)
{
    SetSwitchState((ACSwitchButton::SwitchState)mode, false);
}

void ACModeButtons::sys_color_changed()
{
    // AC : TODO
}


// ----------------------------------------------------------------------------
// ModeSizer
// ----------------------------------------------------------------------------

ModeSizer::ModeSizer(wxWindow *parent, int hgap/* = 0*/) :
    wxFlexGridSizer(3, 0, hgap),
    m_hgap_unscaled((double)(hgap)/em_unit(parent))
{
    SetFlexibleDirection(wxHORIZONTAL);

    auto modebtnfn = [this](wxCommandEvent &event, int mode_id) {
        if (Slic3r::GUI::wxGetApp().save_mode(mode_id))
            event.Skip();
        else
            SetMode(Slic3r::GUI::wxGetApp().get_mode());
    };
    
    m_mode_btns.reserve(3);
    int mode_id = 0;
    for (const wxString& label : {_L("Simple"), _CTX(L_CONTEXT("Advanced", "Mode"), "Mode"),_L("Expert")}) {
        m_mode_btns.push_back(new ModeButton(parent, mode_id++, label, mode_icon_px_size()));

        m_mode_btns.back()->Bind(wxEVT_BUTTON, std::bind(modebtnfn, std::placeholders::_1, int(m_mode_btns.size() - 1)));
        Add(m_mode_btns.back());
    }
}

void ModeSizer::SetMode(const int mode)
{
    for (size_t m = 0; m < m_mode_btns.size(); m++)
        m_mode_btns[m]->SetState(int(m) == mode);
}

void ModeSizer::set_items_flag(int flag)
{
    for (wxSizerItem* item : this->GetChildren())
        item->SetFlag(flag);
}

void ModeSizer::set_items_border(int border)
{
    for (wxSizerItem* item : this->GetChildren())
        item->SetBorder(border);
}

void ModeSizer::sys_color_changed()
{
    for (ModeButton* btn : m_mode_btns)
        btn->sys_color_changed();
}

void ModeSizer::update_mode_markers()
{
    for (ModeButton* btn : m_mode_btns)
        btn->update_bitmap();
}

// ----------------------------------------------------------------------------
// MenuWithSeparators
// ----------------------------------------------------------------------------

void MenuWithSeparators::DestroySeparators()
{
    if (m_separator_frst) {
        Destroy(m_separator_frst);
        m_separator_frst = nullptr;
    }

    if (m_separator_scnd) {
        Destroy(m_separator_scnd);
        m_separator_scnd = nullptr;
    }
}

void MenuWithSeparators::SetFirstSeparator()
{
    m_separator_frst = this->AppendSeparator();
}

void MenuWithSeparators::SetSecondSeparator()
{
    m_separator_scnd = this->AppendSeparator();
}

// ----------------------------------------------------------------------------
// PrusaBitmap
// ----------------------------------------------------------------------------


ScalableBitmap::ScalableBitmap( wxWindow *parent, 
                                const int px_cnt/* = 16*/, 
                                const std::string& icon_name/* = ""*/,
                                const bool grayscale/* = false*/):
    m_parent(parent), m_icon_name(icon_name),
    m_pxSize(wxSize(px_cnt, px_cnt))
{
    m_bmp = *get_bmp_bundle(icon_name, m_pxSize);
    m_bitmap = m_bmp.GetBitmap(m_pxSize);
}

ScalableBitmap::ScalableBitmap(wxWindow *         parent,
                               const std::string &icon_name /* = ""*/,
                               const int          px_cnt /* = 16*/,
                               const bool         grayscale /* = false*/)
    : ScalableBitmap(parent, wxSize(px_cnt, px_cnt), icon_name, grayscale)
{}

ScalableBitmap::ScalableBitmap( wxWindow *parent, 
                                const std::string& icon_name/* = ""*/,
                                const bool grayscale/* = false*/):
    ScalableBitmap(parent, 16, icon_name, grayscale)
{
}
ScalableBitmap::ScalableBitmap( wxWindow *parent, 
                                const wxSize pxSize/* = 16*/, 
                                const std::string& icon_name/* = ""*/,
                                const bool grayscale/* = false*/):
    m_parent(parent), m_icon_name(icon_name),
    m_pxSize(pxSize)
{
    m_bmp = *get_bmp_bundle(icon_name, m_pxSize);
    m_bitmap = m_bmp.GetBitmap(m_pxSize);
}
ScalableBitmap::ScalableBitmap(wxWindow *parent, int width /*0*/, int height /*0*/, const bool grayscale /* = false*/)
    : m_parent(parent), m_pxSize(width, height)
{
    m_bmp    = *get_empty_bmp_bundle(width, height);
    m_bitmap = m_bmp.GetBitmap(m_pxSize);
}

void ScalableBitmap::sys_color_changed()
{
    m_bmp = *get_bmp_bundle(m_icon_name, m_pxSize);
    m_bitmap = m_bmp.GetBitmap(m_pxSize);
}


// ----------------------------------------------------------------------------
// PrusaButton
// ----------------------------------------------------------------------------

ScalableButton::ScalableButton( wxWindow *          parent,
                                wxWindowID          id,
                                const std::string&  icon_name /*= ""*/,
                                const wxString&     label /* = wxEmptyString*/,
                                const wxSize&       size /* = wxDefaultSize*/,
                                const wxPoint&      pos /* = wxDefaultPosition*/,
                                long                style /*= wxBU_EXACTFIT | wxNO_BORDER*/,
                                int                 bmp_px_cnt/* = 16*/) :
    m_parent(parent),
    m_current_icon_name(icon_name),
    m_pxSize(wxSize(bmp_px_cnt, bmp_px_cnt)),
    m_has_border(!(style & wxNO_BORDER))
{
    Create(parent, id, label, pos, size, style);
    Slic3r::GUI::wxGetApp().UpdateDarkUI(this);

    if (!icon_name.empty()) {
        SetBitmap(*get_bmp_bundle(icon_name, m_pxSize));
        if (!label.empty())
            SetBitmapMargins(int(0.5* em_unit(parent)), 0);
    }

    if (size != wxDefaultSize)
    {
        const int em = em_unit(parent);
        m_width = size.x/em;
        m_height= size.y/em;
    }
}


ScalableButton::ScalableButton( wxWindow *          parent, 
                                wxWindowID          id,
                                const ScalableBitmap&  bitmap,
                                const wxString&     label /*= wxEmptyString*/, 
                                long                style /*= wxBU_EXACTFIT | wxNO_BORDER*/) :
    m_parent(parent),
    m_current_icon_name(bitmap.name()),
    m_pxSize(bitmap.pxSize()),
    m_has_border(!(style& wxNO_BORDER))
{
    Create(parent, id, label, wxDefaultPosition, wxDefaultSize, style);
    Slic3r::GUI::wxGetApp().UpdateDarkUI(this);

    SetBitmap(bitmap.bmp());
}

void ScalableButton::SetBitmap_(const ScalableBitmap& bmp)
{
    SetBitmap(bmp.bmp());
    m_current_icon_name = bmp.name();
}
void ScalableButton::setScalableButtonImg_hover(const std::string &bmp_name, const std::string &hover_bmp_name) { 
    this->Bind(wxEVT_ENTER_WINDOW, [this, hover_bmp_name](wxMouseEvent &event) {
        this->SetBitmap_(hover_bmp_name);

    });
    this->Bind(wxEVT_LEAVE_WINDOW, [this,bmp_name](wxMouseEvent &event) {
        this->SetBitmap_(bmp_name);
    });
}

bool ScalableButton::SetBitmap_(const std::string &bmp_name)
{
    m_current_icon_name = bmp_name;
    if (m_current_icon_name.empty())
        return false;

    wxBitmapBundle bmp = *get_bmp_bundle(m_current_icon_name, m_pxSize);
    SetBitmap(bmp);
    SetBitmapCurrent(bmp);
    SetBitmapPressed(bmp);
    SetBitmapFocus(bmp);
    SetBitmapDisabled(bmp);
    return true;
}

void ScalableButton::SetBitmapDisabled_(const ScalableBitmap& bmp)
{
    SetBitmapDisabled(bmp.bmp());
    m_disabled_icon_name = bmp.name();
}

int ScalableButton::GetBitmapHeight()
{
#ifdef __APPLE__
    return GetBitmap().GetScaledHeight();
#else
    return GetBitmap().GetHeight();
#endif
}

void ScalableButton::sys_color_changed()
{
    Slic3r::GUI::wxGetApp().UpdateDarkUI(this, m_has_border);

    wxBitmapBundle bmp = *get_bmp_bundle(m_current_icon_name, m_pxSize);
    SetBitmap(bmp);
    SetBitmapCurrent(bmp);
    SetBitmapPressed(bmp);
    SetBitmapFocus(bmp);
    if (!m_disabled_icon_name.empty())
        SetBitmapDisabled(*get_bmp_bundle(m_disabled_icon_name, m_pxSize));
    if (!GetLabelText().IsEmpty())
        SetBitmapMargins(int(0.5 * em_unit(m_parent)), 0);
}

// ----------------------------------------------------------------------------
// BlinkingBitmap
// ----------------------------------------------------------------------------

BlinkingBitmap::BlinkingBitmap(wxWindow* parent, const std::string& icon_name) :
    wxStaticBitmap(parent, wxID_ANY, wxNullBitmap, wxDefaultPosition, wxSize(int(1.6 * Slic3r::GUI::wxGetApp().em_unit()), -1))
{
    bmp = ScalableBitmap(parent, icon_name);
}

void BlinkingBitmap::invalidate()
{
    this->SetBitmap(wxNullBitmap);
}

void BlinkingBitmap::activate()
{
    this->SetBitmap(bmp.bmp());
    show = true;
}

void BlinkingBitmap::blink()
{
    show = !show;
    this->SetBitmap(show ? bmp.bmp() : wxNullBitmap);
}

namespace Slic3r {
namespace GUI {

void Highlighter::set_timer_owner(wxWindow* owner, int timerid/* = wxID_ANY*/)
{
    m_timer.SetOwner(owner, timerid);
    bind_timer(owner);
}

bool Highlighter::init(bool input_failed)
{
    if (input_failed)
        return false;

    m_timer.Start(300, false);
    return true;
}
void Highlighter::invalidate()
{
    if (m_timer.IsRunning())
        m_timer.Stop();
    m_blink_counter = 0;
}

void Highlighter::blink()
{
    if ((++m_blink_counter) == 11)
        invalidate();
}

// HighlighterForWx

void HighlighterForWx::bind_timer(wxWindow* owner)
{
    owner->Bind(wxEVT_TIMER, [this](wxTimerEvent&) {
        blink();
    });
}

// using OG_CustomCtrl where arrow will be rendered and flag indicated "show/hide" state of this arrow
void HighlighterForWx::init(std::pair<OG_CustomCtrl*, bool*> params)
{
    invalidate();
    if (!Highlighter::init(!params.first && !params.second))
        return;

    m_custom_ctrl = params.first;
    m_show_blink_ptr = params.second;

    *m_show_blink_ptr = true;
    m_custom_ctrl->Refresh();
}

// - using a BlinkingBitmap. Change state of this bitmap
void HighlighterForWx::init(BlinkingBitmap* blinking_bmp)
{
    invalidate();
    if (!Highlighter::init(!blinking_bmp))
        return;

    m_blinking_bitmap = blinking_bmp;
    m_blinking_bitmap->activate();
}

void HighlighterForWx::invalidate()
{
    Highlighter::invalidate();

    if (m_custom_ctrl && m_show_blink_ptr) {
        *m_show_blink_ptr = false;
        m_custom_ctrl->Refresh();
        m_show_blink_ptr = nullptr;
        m_custom_ctrl = nullptr;
    }
    else if (m_blinking_bitmap) {
        m_blinking_bitmap->invalidate();
        m_blinking_bitmap = nullptr;
    }
}

void HighlighterForWx::blink()
{
    if (m_custom_ctrl && m_show_blink_ptr) {
        *m_show_blink_ptr = !*m_show_blink_ptr;
        m_custom_ctrl->Refresh();
    }
    else if (m_blinking_bitmap)
        m_blinking_bitmap->blink();
    else
        return;

    Highlighter::blink();
}

}// GUI
}//Slicer




