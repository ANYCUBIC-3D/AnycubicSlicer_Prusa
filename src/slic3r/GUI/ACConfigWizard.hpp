#ifndef slic3r_ACConfigWizard_hpp_
#define slic3r_ACConfigWizard_hpp_

#include <memory>

#include <wx/dialog.h>
#include <wx/sizer.h>
#include <wx/textctrl.h>

#include "GUI_Utils.hpp"

namespace Slic3r {

class PresetBundle;
class PresetUpdater;

namespace GUI {

namespace DownloaderUtils {
    class Worker : public wxBoxSizer
    {
        wxWindow*   m_parent{ nullptr };
        wxTextCtrl* m_input_path{ nullptr };
        bool        downloader_checked{ false };
#ifdef __linux__
        bool        perform_registration_linux{ false };
#endif // __linux__

        void deregister();

    public:
        Worker(wxWindow* parent);
        ~Worker() {}

        void allow(bool allow_) { downloader_checked = allow_; }
        bool is_checked() const { return downloader_checked; }
        wxString path_name() const { return m_input_path ? m_input_path->GetValue() : wxString(); }

        void set_path_name(wxString name);
        void set_path_name(const std::string& name);

        bool on_finish();
        bool perform_register(const std::string& path_override = {});
#ifdef __linux__
        bool get_perform_registration_linux() { return perform_registration_linux; }
#endif // __linux__
    };
}

class ACConfigWizard : public DPIDialog
{
public:
    // Why is the Wizard run
    enum RunReason {
        RR_DATA_EMPTY,                  // No or empty datadir
        RR_DATA_LEGACY,                 // Pre-updating datadir
        RR_DATA_INCOMPAT,               // Incompatible datadir - Slic3r downgrade situation
        RR_USER,                        // User requested the Wizard from the menus
    };

    // What page should wizard start on
    enum StartPage {
        SP_WELCOME,
        SP_PRINTERS,
        SP_FILAMENTS,
        SP_MATERIALS,
    };

    enum ButtonsID {
        BT_PREV    = 0x01,
        BT_NEXT    = 0x02,
        BT_FINISH  = 0x04,
        BT_CANCEL  = 0x08,
        BT_ALL     = BT_PREV | BT_NEXT | BT_FINISH | BT_CANCEL,
    };

    ACConfigWizard(wxWindow *parent);
    ACConfigWizard(ACConfigWizard &&) = delete;
    ACConfigWizard(const ACConfigWizard &) = delete;
    ACConfigWizard &operator=(ACConfigWizard &&) = delete;
    ACConfigWizard &operator=(const ACConfigWizard &) = delete;
    ~ACConfigWizard();

    // Run the Wizard. Return whether it was completed.
    bool run(RunReason reason, StartPage start_page = SP_WELCOME);

    void setButtonVisiable(int btID, bool visiable); 

    static const wxString& name(const bool from_menu = false);
protected:
    void on_dpi_changed(const wxRect &suggested_rect) override ;
    void on_sys_color_changed() override;

private:
    struct priv;
    std::unique_ptr<priv> p;

    friend struct ConfigWizardPage;
};



}
}

#endif
