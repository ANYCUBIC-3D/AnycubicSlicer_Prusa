#ifndef AC_CLOUD_MACHINE_HPP
#define AC_CLOUD_MACHINE_HPP

#include "ACPrinterContainer.hpp"


namespace Slic3r {

namespace GUI {


/*---------------------------------------------------------------------------------
								ACCloudMachine
-----------------------------------------------------------------------------------*/
class ACCloudMachine : public wxPanel
{
public:
	ACCloudMachine(wxWindow* parent, MainFrame* main_frame);
	ACCloudMachine(ACCloudMachine&&) = delete;
	ACCloudMachine(const ACCloudMachine&) = delete;
	ACCloudMachine& operator=(ACCloudMachine&&) = delete;
	ACCloudMachine& operator=(const ACCloudMachine&) = delete;
	~ACCloudMachine();

	void Init();
	void Connect();

	void OnButtonRefresh(wxCommandEvent& event);
	void OnButtonAddNew(wxCommandEvent& event);

	void UpdatePrinterList();//刷新打印机列表
	//void OnSize(wxSizeEvent& event);
private:
	MainFrame* mf;
	ACStaticBox* box;
	wxPanel* panel;
	ACPrinterContainer* printer;

	wxStaticText* title;
	ACButton* buttonRefresh;
	ACButton* buttonAddNew;
	wxStaticLine* line;

	wxBoxSizer* sizer;
	wxBoxSizer* vBox;//Panel private
};

} // GUI
} // Slic3r

#endif //!AC_CLOUD_MACHINE_HPP
