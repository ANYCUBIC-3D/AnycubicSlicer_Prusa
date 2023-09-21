#ifndef slic3r_GUI_Cloud_ACCloudSelectMachine_hpp_
#define slic3r_GUI_Cloud_ACCloudSelectMachine_hpp_

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

class ACCloudSelectMachine : public DPIDialog
{
public:
    ACCloudSelectMachine(wxWindow *parent);
    ACCloudSelectMachine(ACCloudSelectMachine &&)      = delete;
    ACCloudSelectMachine(const ACCloudSelectMachine &) = delete;
    ACCloudSelectMachine &operator=(ACCloudSelectMachine &&) = delete;
    ACCloudSelectMachine &operator=(const ACCloudSelectMachine &) = delete;
    ~ACCloudSelectMachine();

    void Init();
    
	void OnButtonCancelEvent(wxCommandEvent &event);
    void OnButtonStartPrintEvent(wxCommandEvent &event);

    void OnButtonRefreshEvent(wxCommandEvent &event);
    void OnButtonAddPrinterEvent(wxCommandEvent &event);

	void SetAvailablePrinterNum(int num);
    void SetUnailablePrinterNum(int num);

    void GetUpdatePrinterListEvent(wxCommandEvent &event);
    void GetOperatPrinterEvent(wxCommandEvent &event);

    void         CheckClickPrinterEvent(wxCommandEvent &evt);
    PrinterData GetPrinterInfo() { return m_SelectPrinter; }
    void         SetPrinterDataEvent(PrinterData info) { m_SelectPrinter = info; }

	wxPanel *CreateShowPanel(std::vector<PrinterData> &printerInfo);
    wxPanel *CreateEmptyPanel();

    void SetButtonStyle(ACButton *btn);
    void SetButtonStyle_Label(ACButton *btn);

    void SetTipBtnStyle(bool isSucce, bool isAddObj, int opStyle=0);

	void msw_rescale();
    void                     OnTimer(wxTimerEvent &event);
    std::vector<PrinterData> GetCloudInfo(bool index = true);
    ACButton *CreateAddPrinterEvent(wxWindow* win);

    wxSize GetWindowSize();

    void RefreshContentEvent(bool all=false);

protected:
    void on_dpi_changed(const wxRect &suggested_rect) override { msw_rescale(); }

private:

	wxWindow *          m_parent;
    ACStaticBox *       m_page;
    wxPanel *           m_show_panel;
    ACButton *          m_AvailablePrinter;
    ACButton *          m_UnailablePrinter;
    ACButton *          m_UnailablePrinter_btn;
    PrinterData        m_SelectPrinter;

    ACButton *m_showTipInfoBtn;

    wxStaticText *	m_AvailablePrinter_Num;
    wxStaticText *	m_UnailablePrinter_Num;
    ACButton *		m_buttonRefresh;
    ACButton *		m_buttonAddNew;
    ACButton *		m_buttonCancel;
    ACButton *		m_buttonStartPrint;
    ACButton *      m_buttonEmptyAddNew;

    wxBoxSizer *	m_up_sizer;
    wxBoxSizer *        m_pageSizer;
    wxBoxSizer *        m_mainSizer;
    wxBoxSizer *        unvaliable_sizer;
    wxBoxSizer *        m_down_sizer;
    wxBoxSizer *        center_label_Sizer;
    ACPrinterContainer *printer;

    std::vector<PrinterData> m_PrinterInfo;
    ACPrinterContainer *     m_Avprinter;
    ACPrinterContainer *     m_Unvprinter;

    wxTimer *m_timer = nullptr;
    int      m_timeCount = 0;
    int      m_opModel = -1;
    bool     m_isAddPanel  = false;
    bool     m_isOpResult = false;
};





} // GUI
} // Slic3r

#endif //!AC_CLOUD_MACHINE_HPP
