#ifndef AC_CLOUD_MACHINE_PRIVATE_HPP
#define AC_CLOUD_MACHINE_PRIVATE_HPP

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
#include <wx/textctrl.h>
#include <wx/button.h>

#include <wx/popupwin.h>
#include <wx/listctrl.h>


namespace Slic3r {

namespace GUI {


class ACQuickResponseDialog : public wxPopupTransientWindow
{
public:
	ACQuickResponseDialog(wxWindow* parent, const wxSize& size = wxDefaultSize);
	~ACQuickResponseDialog();

	void Init();
	void Connect();

	void OnButtonAndriod(wxCommandEvent& event);//安卓
	void OnButtonIOS(wxCommandEvent& event);    //IOS
	void OnButtonLogout(wxCommandEvent& event); //账号登出

	//wxDECLARE_EVENT_TABLE();
private:
	ACButton* buttonAndriod;
	ACButton* buttonIOS;
	ACButton* buttonLogout;
	wxStaticText* title;
	wxStaticText* textAndriod;
	wxStaticText* textIOS;

	wxBoxSizer* sizer;
};

} // GUI
} // Slic3r

#endif //!AC_CLOUD_MACHINE_PRIVATE_HPP
