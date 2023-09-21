#ifndef slic3r_GUI_Cloud_ACCloudAddMachine_hpp_
#define slic3r_GUI_Cloud_ACCloudAddMachine_hpp_

#include <wx/wx.h>
#include "ACPrinterContainer.hpp"
#include "../GUI.hpp"
#include "../GUI_App.hpp"
#include "../GUI_Utils.hpp"
#include "../BitmapCache.hpp"
#include "libslic3r/Utils.hpp"
#include "../ACLabel.hpp"


namespace Slic3r {

namespace GUI {

class ACCloudAddMachine : public DPIDialog
{
public:
    ACCloudAddMachine(wxWindow *parent);
    ACCloudAddMachine(ACCloudAddMachine &&)            = delete;
    ACCloudAddMachine(const ACCloudAddMachine &) = delete;
    ACCloudAddMachine &   operator=(ACCloudAddMachine &&) = delete;
    ACCloudAddMachine &operator=(const ACCloudAddMachine &) = delete;
    ~ACCloudAddMachine();

    void Init();
    
	void OnButtonCancelEvent(wxCommandEvent &event);
    void OnButtonAddEvent(wxCommandEvent &event);

    wxPanel *CreateInputEvent();

    void SetButtonStyle(ACButton *btn);
    void SetButtonStyle_Label(ACButton *btn);
	void msw_rescale();

    wxSize GetWindowSize();
    wxString GetCNInfo() { return m_CNInfo; }

protected:
    void on_dpi_changed(const wxRect &suggested_rect) override { msw_rescale(); }

private:

	wxWindow *          m_parent;

    std::vector<ACTextInput*> m_inputList;
    ACButton *               m_buttonAdd;
    ACButton *               m_buttonCancel;
    ACButton *               m_showErrInfo;
    wxPanel *                m_inputPanel;

    wxBoxSizer *m_up_sizer;
    wxBoxSizer *m_down_sizer;
    wxBoxSizer *m_center_sizer;
    wxBoxSizer *m_pageSizer;
    wxBoxSizer *m_mainSizer;

    wxString m_CNInfo;

};





} // GUI
} // Slic3r

#endif //!AC_CLOUD_MACHINE_HPP
