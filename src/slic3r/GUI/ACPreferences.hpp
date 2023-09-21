#ifndef slic3r_ACPreferences_hpp_
#define slic3r_ACPreferences_hpp_

#include "GUI.hpp"
#include "GUI_Utils.hpp"
#include "wxExtensions.hpp"

#include <wx/dialog.h>
#include <wx/timer.h>
#include <vector>
#include <map>


namespace Slic3r {

namespace GUI {

class ConfigOptionsGroup;

class ACPreferencesDialog : public DPIDialog
{
public:
	explicit ACPreferencesDialog(wxWindow* parent);
	~ACPreferencesDialog() = default;

	void	show(const std::string& highlight_option = std::string(), const std::string& tab_name = std::string());
	bool recreate_GUI() const { return m_recreate_GUI; }
	bool seq_top_layer_only_changed() const { return m_seq_top_layer_only_changed; }
	bool settings_layout_changed() const { return m_settings_layout_changed; }
    /*wxDialog *getDialogObj() { return markDialog; }
    void      setDialogObj(wxDialog *indexObj) { markDialog = indexObj; }*/
	std::shared_ptr<ConfigOptionsGroup>	optgroup_language() { return m_optgroup_language; }
private:
	void create();
	void build();
	void msw_rescale();
	void on_dpi_changed(const wxRect& suggested_rect) override { msw_rescale(); }
	std::vector<ConfigOptionsGroup*> optgroups();
	wxMenuBar*  m_menubar{ nullptr };
    /*wxDialog *                       markDialog = nullptr;*/

private:
	wxBoxSizer* m_mainSizer = nullptr;
	wxBoxSizer* m_pageSizer = nullptr;
	ACStaticBox* m_page = nullptr;
	std::shared_ptr<ConfigOptionsGroup>	m_optgroup_language;
	std::shared_ptr<ConfigOptionsGroup>	m_optgroup_camera;
	std::map<std::string, std::string>	m_values;
	bool								m_recreate_GUI{false};
	bool								m_seq_top_layer_only_changed{ false };
	bool								m_settings_layout_changed {false};
};

struct ChangeLanguageInfo {
	std::string currentLanguage;
	const wxLanguageInfo* targetLanguage;
};

wxDECLARE_EVENT(EVT_ACSLICER_APP_CHANGE_LANGUAGE, wxCommandEvent);

} // GUI
} // Slic3r

#endif /* slic3r_ACPreferences_hpp_ */
