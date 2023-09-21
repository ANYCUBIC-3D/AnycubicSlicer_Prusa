#ifndef AC_PRINTER_CONTAINER_HPP
#define AC_PRINTER_CONTAINER_HPP

/****************************************************************

用于云打印机容器复用，可嵌入独立的dialog或者mainframe（目前用于）

****************************************************************/

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
#include "../ACDialogTopbar.hpp"

#include "libslic3r/Utils.hpp"
#include <boost/filesystem.hpp>

#include <wx/wx.h>
#include <wx/panel.h>
#include <wx/dialog.h>
#include <wx/sizer.h>
#include <wx/stattext.h>
#include <wx/checkbox.h>
#include <wx/textctrl.h>
#include <wx/statline.h>

#include <wx/button.h>
#include <wx/bmpbuttn.h>
#include <wx/scrolwin.h>

#include <vector>


namespace Slic3r {

namespace GUI {

namespace fs = boost::filesystem;

wxDECLARE_EVENT(EVT_ACLOUD_PRINTER_DELETE, wxCommandEvent);
wxDECLARE_EVENT(EVT_ACLOUD_PRINTER_RENAME, wxCommandEvent);
wxDECLARE_EVENT(EVT_OPERAT_PRINTER_RENAME, wxCommandEvent);


struct PrinterData;
struct ACPrinterMeta;


/*---------------------------------------------------------------------------------
								ACPrinterContainer
-----------------------------------------------------------------------------------*/
class ACPrinterContainer : public wxScrolledWindow
{
public:
    explicit ACPrinterContainer(wxWindow *parent /*, const std::vector<ACPrinterMeta*>& pm*/, std::vector<PrinterData> &info,bool checkIndex = true);
	ACPrinterContainer(ACPrinterContainer&&) = delete;
	ACPrinterContainer(const ACPrinterContainer&) = delete;
	ACPrinterContainer& operator=(ACPrinterContainer&&) = delete;
	ACPrinterContainer& operator=(const ACPrinterContainer&) = delete;
	~ACPrinterContainer();

	void Init();
	void InitPrinter(std::vector<PrinterData> &cloudInfo);
	void CheckClickSingleEvent(PrinterData &printerData);
	void SetPrinters(const std::vector<PrinterData>& printerInfo);
	void ReleasePrinterMeta();
    std::vector<PrinterData> GetCloudInfo();
    void ResetWindowEvent(std::vector<PrinterData> &printerInfo);
	void ReLayout();
	void Connect();

	void AddPrinterSingle(PrinterData& d);
	void AddPrinterMultiples(std::vector<PrinterData>& d);
	void DeletePrinter(int id);

	std::vector<ACPrinterMeta *> GetACPrinterMetaObj() { return pm; }

protected:
	void OnDelete(wxCommandEvent& event);
	void OnRename(wxCommandEvent& event);
	void OnSize(wxSizeEvent& event);
public:
	wxFlexGridSizer* flexSizer;//
	std::vector<ACPrinterMeta*> pm; //注意请维护pm跟info数组大小一致 ！！！
	std::vector<PrinterData> m_info;  //注意请维护pm跟info数组大小一致 ！！！
    std::vector<PrinterData> staticInfo;//
	bool m_checkIndex;
};


/*---------------------------------------------------------------------------------
								PrinterData
-----------------------------------------------------------------------------------*/
struct PrinterData
{
	std::string fileName;	// image filePath
	wxString name;			//printer name
	wxString type;			//printer type
	int status{ 0 };		//0: offline 1:free 2:busy
	int id;					//ID 布局需要用到的id
	int32_t id_printer;		//云打印机id 删除需要用到
	bool isChecked{ false };//checked state
	void        Clear() {
		fileName = "";
        name     = "";
        type     = "";
        status   = 0;
        id       = 0;
        id_printer = 0;
        isChecked = false;
	}
};

/*---------------------------------------------------------------------------------
								ACPrinterMeta
-----------------------------------------------------------------------------------*/
struct ACPrinterMeta : public ACStaticBox
{
	ACPrinterMeta(wxWindow* parent, const PrinterData& d, const wxString& label = "", const std::string& model = "", const std::string& variant = "");
	~ACPrinterMeta();

	enum ButtonState
	{
		Normal = 0,
		//Checked = 2,
		Hover = 8,
		Pressed = 16,
		Disabled = 1 << 16
	};

	void Init();
	void Connect();
	void SetIcon(const wxString& iconName);
	void SetImage(const std::string& fileFullPath);
	void SetHoverIcon(const wxString& iconName);
	void SetIconMaskChecked(const wxString& imgNameCheckedOn, const wxString& imgNameCheckedHover, int imgSize);
	
	void SetName(const wxString& t);
	void SetButtonState(ButtonState state);
	void SetPrinterData(const PrinterData& data);
	void SetChecked(bool isChecked);
protected:
	virtual void OnEnter(wxMouseEvent& event);
	virtual void OnLeave(wxMouseEvent& event);
	virtual void OnMouseDown(wxMouseEvent& event);
	virtual void OnMouseUp(wxMouseEvent& event);
	virtual void OnPaint(wxPaintEvent& event);

	virtual void OnDPIChanged(wxDPIChangedEvent& event);

	void OnButton(wxCommandEvent& event);
	void OnButtonRename(wxCommandEvent& event);
	void OnButtonDelete(wxCommandEvent& event);

public:

	virtual void render(wxDC& dc) override;


	

	bool isChecked{ false };
	bool isMouseInside{ false };
	bool isMousePressed{ false };
	//wxButton* button;
	wxBitmapButton* button;
	wxPopupTransientWindow* popup;
	ACButton* buttonRename;
	ACButton* buttonDelete;
	PrinterData data;

	ACStateColor fgColor;

	wxImage image;       //云打印机图片
	wxBitmap bitmapImage;//云打印机Bitmap图
	ScalableBitmap iconNormal;
	ScalableBitmap iconHover;
	ScalableBitmap iconNormalMaskChecked;
	ScalableBitmap iconHoverMaskChecked;

	wxSize sizeIcon = wxSize(64, 64);
	wxSize sizeIconCheck = wxSize(36, 36);
	//wxSize sizeText = wxSize(100, 20);

	std::string model;
	std::string variant;

	ButtonState buttonState = ButtonState::Normal;
};

/*---------------------------------------------------------------------------------
								ACPrinterRenameDialog
-----------------------------------------------------------------------------------*/
class ACPrinterRenameDialog : public DPIDialog
{
public:
	explicit ACPrinterRenameDialog(wxWindow* parent, ACPrinterMeta* p = nullptr);
	ACPrinterRenameDialog(ACPrinterRenameDialog&&) = delete;
	ACPrinterRenameDialog(const ACPrinterRenameDialog&) = delete;
	ACPrinterRenameDialog& operator=(ACPrinterRenameDialog&&) = delete;
	ACPrinterRenameDialog& operator=(const ACPrinterRenameDialog&) = delete;
	~ACPrinterRenameDialog();

	void Init();
	void Connect();

	//Slots
	void OnClose();
	void OnRename();
	void OnText(wxCommandEvent& event);
protected:
	void msw_rescale();
	void on_dpi_changed(const wxRect& suggested_rect) override { msw_rescale(); };
private:
	ACPrinterMeta* p{ nullptr };
	ACDialogTopbar* topbar;
	wxBoxSizer* hBox;
	wxBoxSizer* vBox;
	wxStaticText* text;			//text
	wxStaticText* textWarning;	//warning
public:
	ACButton* buttonCancel; //Cancel
	ACButton* buttonRename; //Rename
	wxTextCtrl* name;		//line edit
};


/*---------------------------------------------------------------------------------
								ACPrinterDeleteDialog
-----------------------------------------------------------------------------------*/
class ACPrinterDeleteDialog : public DPIDialog
{
public:
	explicit ACPrinterDeleteDialog(wxWindow* parent, const wxString& name);
	ACPrinterDeleteDialog(ACPrinterDeleteDialog&&) = delete;
	ACPrinterDeleteDialog(const ACPrinterDeleteDialog&) = delete;
	ACPrinterDeleteDialog& operator=(ACPrinterDeleteDialog&&) = delete;
	ACPrinterDeleteDialog& operator=(const ACPrinterDeleteDialog&) = delete;
	~ACPrinterDeleteDialog();

	void Init();
	void Connect();

	//Slots
	void OnClose();
	void OnDelete();
protected:
	void msw_rescale();
	void on_dpi_changed(const wxRect& suggested_rect) override { msw_rescale(); };
private:
	ACPrinterMeta* p{ nullptr };
	ACDialogTopbar* topbar;
	wxBoxSizer* vBox;
	wxBoxSizer* hBox;
	wxStaticText* text;			//text
public:
	wxString name;
	ACButton* buttonCancel; //Cancel
	ACButton* buttonDelete; //Delete
};

/*---------------------------------------------------------------------------------
								ACPrinterRenameDialog
-----------------------------------------------------------------------------------*/
class ACPrinterAddDialog : public DPIDialog
{
public:
	explicit ACPrinterAddDialog(wxWindow* parent);
	ACPrinterAddDialog(ACPrinterAddDialog&&) = delete;
	ACPrinterAddDialog(const ACPrinterAddDialog&) = delete;
	ACPrinterAddDialog& operator=(ACPrinterAddDialog&&) = delete;
	ACPrinterAddDialog& operator=(const ACPrinterAddDialog&) = delete;
	~ACPrinterAddDialog();

	void Init();
	void Connect();

	//Slots
	void OnClose();
	void OnAdd(wxCommandEvent& event);
	void OnText(wxCommandEvent& event);
protected:
	void msw_rescale();
	void on_dpi_changed(const wxRect& suggested_rect) override { msw_rescale(); };
private:
	ACDialogTopbar* topbar;
	wxStaticText* text1;		//text1
	wxStaticText* text2;		//text2
	wxStaticText* textWarning;	//warning
public:
	ACButton* buttonCancel; //Cancel
	ACButton* buttonAdd;	//Add
	wxTextCtrl* name;		//line edit
	wxTextCtrl* deviceCN;	//device code CN
private:
	wxBoxSizer* sizer;
	wxBoxSizer* hBox1;
	wxBoxSizer* hBox2;
	wxBoxSizer* hBox3;
};

wxDECLARE_EVENT(EVT_ACLOUD_PRINTER_CHECK_CLICK, wxCommandEvent);

} // GUI
} // Slic3r

#endif //!AC_PRINTER_CONTAINER_HPP
