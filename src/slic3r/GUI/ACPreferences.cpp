#include "ACPreferences.hpp"
#include "GUI_App.hpp"
#include "OptionsGroup.hpp"
#include "libslic3r/AppConfig.hpp"
#include "GUI.hpp"
#include "MsgDialog.hpp"
#include "Plater.hpp"
#include <wx/evtloop.h>
#include "MainFrame.hpp"
//#include "MsgDialog.hpp"
//#include "I18N.hpp"
//#include "libslic3r/AppConfig.hpp"
//#include <wx/notebook.h>
//#include "Notebook.hpp"
//#include "ButtonsDescription.hpp"
//#include "OG_CustomCtrl.hpp"
//#include "GLCanvas3D.hpp"
//#include "ACConfigWizard.hpp"
#include <wx/intl.h>
//#include <boost/dll/runtime_symbol_info.hpp>

//#ifdef WIN32
//#include <wx/msw/registry.h>
//#endif // WIN32
//#ifdef __linux__
//#include "DesktopIntegrationDialog.hpp"
//#endif //__linux__

#include "ACDialogTopbar.hpp"
#include "ACDefines.h"

namespace Slic3r {

namespace GUI {

wxDEFINE_EVENT(EVT_ACSLICER_APP_CHANGE_LANGUAGE, wxCommandEvent);

ACPreferencesDialog::ACPreferencesDialog(wxWindow* parent)
    : DPIDialog(parent, wxID_ANY, _L("Preferences"), wxDefaultPosition, wxDefaultSize, wxNO_BORDER, "ACPreferencesDialog")
{
	create();
}

void ACPreferencesDialog::create()
{
    AddWindowDrakEdg(this);
	this->SetBackgroundColour(AC_COLOR_WHITE);

	const int &em = em_unit();
    wxSize     _size(76 * em, 32 * em);
    SetSize(_size);

	m_pageSizer = new wxBoxSizer(wxVERTICAL);

	ACDialogTopbar* topbar = new ACDialogTopbar(this,  _L("Preferences"),76);

	m_page = new ACStaticBox(this);
	m_page->SetBackgroundColour(AC_COLOR_PANEL_BG);
	m_page->SetCornerRadius(14);
	m_page->SetSizer(m_pageSizer);
    m_page->SetFocus();
	m_mainSizer = new wxBoxSizer(wxVERTICAL);
    m_mainSizer->Add(topbar, 0, wxEXPAND | wxTOP | wxLEFT | wxRIGHT, 1);
	m_mainSizer->Add(m_page, 1, wxEXPAND|wxALL, 14);
	SetSizer(m_mainSizer);

	build();
    Refresh();
	Layout();
}

static std::shared_ptr<ConfigOptionsGroup> create_options_group(const wxString& title, wxWindow* parent)
{
	std::shared_ptr<ConfigOptionsGroup> optgroup = std::make_shared<ConfigOptionsGroup>(parent, title);
	optgroup->label_width = 40;
	optgroup->set_config_category_and_type(title, int(Preset::TYPE_PREFERENCES));
	return optgroup;
}


static void ac_append_bool_option( std::shared_ptr<ConfigOptionsGroup> optgroup,
								const std::string& opt_key,
								const std::string& label,
								const std::string& tooltip,
								bool def_val,
								ConfigOptionMode mode = comSimple)
{
	ConfigOptionDef def = {opt_key, coBool};
	def.label = label;
	def.tooltip = tooltip;
	def.mode = mode;
	def.set_default_value(new ConfigOptionBool{ def_val });
	Option option(def, opt_key);
	optgroup->append_single_option_line(option);

	//// fill data to the Search Dialog
	//wxGetApp().plater()->get_searcher().add_key(opt_key, Preset::TYPE_PREFERENCES, optgroup->config_category(), L("Preferences"));
}

static void ac_append_enum_option( std::shared_ptr<ConfigOptionsGroup> optgroup,
								const std::string& opt_key,
								const std::string& label,
								const std::string& tooltip,
								const ConfigOption* def_val,
								const std::vector<std::string>& enum_labels,//std::initializer_list<std::pair<std::string_view, std::string_view>> enum_values, 
								ConfigOptionMode mode = comSimple)
{
	//
	ConfigOptionDef def = {opt_key, coStrings };
	def.label = label;
	def.tooltip = tooltip;
	def.mode = mode;
    def.gui_flags = "show_value";
    def.set_enum_values(ConfigOptionDef::GUIType::select_close, enum_labels);
	def.set_default_value(def_val);

	Option option(def, opt_key);
	optgroup->append_single_option_line(option);

	// fill data to the Search Dialog
	//wxGetApp().sidebar().get_searcher().add_key(opt_key, Preset::TYPE_PREFERENCES, optgroup->config_category(), L("Preferences"));
}


void ACPreferencesDialog::build()
{
//#ifdef _WIN32
//	wxGetApp().UpdateDarkUI(this);
//#else
//	SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_WINDOW));
//#endif

	auto app_config = get_app_config();

	std::vector<const wxLanguageInfo*> language_infos;
	std::vector<std::string> languageNames;
	int defaultIndex;
	int curIndex;
	wxGetApp().get_language_info(language_infos, languageNames, defaultIndex, curIndex);

	if (curIndex == -1)
		curIndex = defaultIndex;

	std::string defaultSelName = curIndex == -1 ? "" : languageNames[curIndex];

	m_optgroup_language = create_options_group(L("General"), m_page);

	static bool isLanguageSetting = false;

	m_optgroup_language->m_on_change = [this, defaultSelName,languageNames, language_infos,
                                        curIndex](t_config_option_key opt_key, boost::any value) {
		if (isLanguageSetting)
			return;
		isLanguageSetting = true;
        const std::string& selection = boost::any_cast<std::string>(value);
		auto itSel = std::find(languageNames.begin(), languageNames.end(), selection);
		if (itSel == languageNames.end()) {
			isLanguageSetting = false;
			return;
		}
		int selIndex = int(itSel - languageNames.begin());

		if (curIndex != -1 && selIndex == curIndex) {
			isLanguageSetting = false;
			return;
		}
		
		ChangeLanguageInfo* info = new ChangeLanguageInfo();
		info->currentLanguage = defaultSelName;
		info->targetLanguage  = language_infos[selIndex];

		wxCommandEvent evt(EVT_ACSLICER_APP_CHANGE_LANGUAGE);
		evt.SetClientData(info);
		evt.SetEventObject(this);
		wxPostEvent(this, evt);
	};

	Bind(EVT_ACSLICER_APP_CHANGE_LANGUAGE, [this](const wxCommandEvent &evt) {
		ChangeLanguageInfo* ptr = static_cast<ChangeLanguageInfo*>(evt.GetClientData());
		ChangeLanguageInfo info = *ptr;
		delete ptr;

		int id = -1;
		{

			MessageDialog dialog( this, _L("Attention: Switching languages requires a reboot and the current operation will not be saved!"),
                                 _L("Switching Language"), wxYES_NO | wxNO_DEFAULT | wxICON_QUESTION);
            dialog.SetName("select_switching_warning_dialog");
#ifdef _WIN32
            dialog.SetYesBtnLabel(_L("YES"));
            dialog.SetNoBtnLabel(_L("NO"));
#else

            dialog.SetYesNoLabels(_L("YES"), _L("NO"));
#endif
			id = dialog.ShowModal();
		}

		if (id == wxID_YES) {
            this->EndModal(wxID_YES);
            wxGetApp().switch_language(info.targetLanguage);
        } else {
			{
				wxWindowUpdateLocker locker(this);
				dynamic_cast<Choice *>(m_optgroup_language->get_field("language"))->set_value(info.currentLanguage, false);
			}
            this->Raise();
        }

		isLanguageSetting = false;
    }); 

	ac_append_enum_option(m_optgroup_language, "language",
		L("Language"),
		L("Select the language"),
		new ConfigOptionStrings{ defaultSelName },
		languageNames);

	m_optgroup_language->activate([](){}, wxALIGN_RIGHT);
	m_optgroup_language->Show(true);
	m_pageSizer->Add(m_optgroup_language->sizer, 0, wxEXPAND | wxLEFT|wxTOP|wxRIGHT, 12);


	// Add "Camera" tab
	m_optgroup_camera = create_options_group(L("Camera"), m_page);
	m_optgroup_camera->m_on_change = [this, app_config](t_config_option_key opt_key, boost::any value) {
		app_config->set(opt_key, boost::any_cast<bool>(value) ? "1" : "0");
		wxGetApp().update_ui_from_settings();
	};

	ac_append_bool_option(m_optgroup_camera, "use_perspective_camera",
		L("Use perspective camera"),
		L("If enabled, use perspective camera. If not enabled, use orthographic camera."),
		app_config->get_bool("use_perspective_camera"));

	ac_append_bool_option(m_optgroup_camera, "use_free_camera",
		L("Use free camera"),
		L("If enabled, use free camera. If not enabled, use constrained camera."),
		app_config->get_bool("use_free_camera"));

	//ac_append_bool_option(m_optgroup_camera, "reverse_mouse_wheel_zoom",
	//	L("Reverse direction of zoom with mouse wheel"),
	//	L("If enabled, reverses the direction of zoom with mouse wheel"),
	//	app_config->get_bool("reverse_mouse_wheel_zoom"));

	m_optgroup_camera->activate([](){}, wxALIGN_RIGHT);
	m_optgroup_camera->Show(true);

	//optgroup->update_visibility(comSimple);
	m_pageSizer->Add(m_optgroup_camera->sizer, 0, wxEXPAND | wxLEFT|wxTOP|wxRIGHT, 12);

	m_pageSizer->AddSpacer(12);

	wxSize minSize = m_mainSizer->CalcMin();

	this->SetMinSize(minSize);
	this->SetSize(this->GetSize().x, minSize.y);
    int screenwidth  = wxSystemSettings::GetMetric(wxSYS_SCREEN_X, NULL);
    int screenheight = wxSystemSettings::GetMetric(wxSYS_SCREEN_Y, NULL);
    SetPosition(wxPoint((screenwidth - minSize.x) / 2, (screenheight - minSize.y) / 2));
}

std::vector<ConfigOptionsGroup*> ACPreferencesDialog::optgroups()
{
	std::vector<ConfigOptionsGroup*> out;
	out.reserve(2);
	for (ConfigOptionsGroup* opt : { m_optgroup_language.get(), m_optgroup_camera.get()	})
		if (opt)
			out.emplace_back(opt);
	return out;
}

void ACPreferencesDialog::msw_rescale()
{
	for (ConfigOptionsGroup* og : this->optgroups())
		og->msw_rescale();

	//update_ctrls_alignment();

    //msw_buttons_rescale(this, em_unit(), { wxID_OK, wxID_CANCEL });

	wxSize minSize = m_mainSizer->CalcMin();

	this->SetMinSize(minSize);

	Fit();

	this->Layout();

    Refresh();
}

void ACPreferencesDialog::show(const std::string& highlight_opt_key /*= std::string()*/, const std::string& tab_name/*= std::string()*/)
{
	//int selected_tab = 0;
	//for ( ; selected_tab < int(tabs->GetPageCount()); selected_tab++)
	//	if (tabs->GetPageText(selected_tab) == _(tab_name))
	//		break;
	//if (selected_tab < int(tabs->GetPageCount()))
	//	tabs->SetSelection(selected_tab);

	//if (!highlight_opt_key.empty())
	//	init_highlighter(highlight_opt_key);

	//// cache input values for custom toolbar size
	//m_custom_toolbar_size		= atoi(get_app_config()->get("custom_toolbar_size").c_str());
	//m_use_custom_toolbar_size	= get_app_config()->get_bool("use_custom_toolbar_size");

	//// set Field for notify_release to its value
	//if (m_optgroup_gui && m_optgroup_gui->get_field("notify_release") != nullptr) {
	//	boost::any val = s_keys_map_NotifyReleaseMode.at(wxGetApp().app_config->get("notify_release"));
	//	m_optgroup_gui->get_field("notify_release")->set_value(val, false);
	//}
	//

	//if (wxGetApp().is_editor()) {
	//	auto app_config = get_app_config();

	//	downloader->set_path_name(app_config->get("url_downloader_dest"));
	//	downloader->allow(!app_config->has("downloader_url_registered") || app_config->get_bool("downloader_url_registered"));

	//	for (const std::string& opt_key : {"suppress_hyperlinks", "downloader_url_registered"})
	//		m_optgroup_other->set_value(opt_key, app_config->get_bool(opt_key));

	//	// update colors for color pickers of the labels
	//	update_color(m_sys_colour, wxGetApp().get_label_clr_sys());
	//	update_color(m_mod_colour, wxGetApp().get_label_clr_modified());

	//	// update color pickers for mode palette
	//	const auto palette = wxGetApp().get_mode_palette(); 
	//	std::vector<wxColourPickerCtrl*> color_pickres = {m_mode_simple, m_mode_advanced, m_mode_expert};
	//	for (size_t mode = 0; mode < color_pickres.size(); ++mode)
	//		update_color(color_pickres[mode], palette[mode]);
	//}

    /*if (markDialog == nullptr)
        setDialogObj(setMarkWindow(this->GetParent(), this));*/

	/*this->Show();
	wxGetApp().mainframe->Raise();
	this->Raise();*/
    this->ShowModal();

}


} // GUI
} // Slic3r


