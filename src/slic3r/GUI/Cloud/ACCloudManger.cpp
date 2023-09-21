#include "ACCloudManger.hpp"
#include "ACCloudLogin.hpp"

#include "../I18N.hpp"
#include "../GUI_App.hpp"
#include "../MainFrame.hpp"

#include <wx/string.h>

#include <thread>
#include <chrono>

namespace Slic3r {

namespace GUI {


wxDEFINE_EVENT(EVT_ACCLOUD_LOGIN,            wxCommandEvent);//登录
wxDEFINE_EVENT(EVT_ACCLOUD_LOGOUT,           wxCommandEvent);//登出

wxDEFINE_EVENT(EVT_ACCLOUD_CHECK_NICK_NAME,  wxCommandEvent);//检测昵称事件
wxDEFINE_EVENT(EVT_ACCLOUD_GET_CAPTCHA,      wxCommandEvent);//获取验证码
wxDEFINE_EVENT(EVT_ACCLOUD_RESET_PASSWORD,   wxCommandEvent);//重置密码
wxDEFINE_EVENT(EVT_ACCLOUD_REGISTER_USER,    wxCommandEvent);//用户注册
wxDEFINE_EVENT(EVT_ACCLOUD_FEEDBACK,         wxCommandEvent);//反馈内容
wxDEFINE_EVENT(EVT_ACCLOUD_COUNTRIES,        wxCommandEvent);//国家列表

wxDEFINE_EVENT(EVT_ACCLOUD_PRINTER_LIST,     wxCommandEvent);//打印机列表
wxDEFINE_EVENT(EVT_ACCLOUD_PRINTER_ADD,      wxCommandEvent);//打印机添加
wxDEFINE_EVENT(EVT_ACCLOUD_PRINTER_RENAME,   wxCommandEvent);//打印机重命名
wxDEFINE_EVENT(EVT_ACCLOUD_PRINTER_DELETE,   wxCommandEvent);//打印机删除

wxDEFINE_EVENT(EVT_ACCLOUD_SPACE_SIZE,       wxCommandEvent);//云空间容量
wxDEFINE_EVENT(EVT_ACCLOUD_FILE_LIST,        wxCommandEvent);//获取文件
wxDEFINE_EVENT(EVT_ACCLOUD_FILE_REMOVE,      wxCommandEvent);//文件删除
wxDEFINE_EVENT(EVT_ACCLOUD_FILE_RENAME,      wxCommandEvent);//文件重命名
wxDEFINE_EVENT(EVT_ACCLOUD_FILE_UPLOAD,      wxCommandEvent);//文件上传
wxDEFINE_EVENT(EVT_ACCLOUD_FILE_DOWNLOAD,    wxCommandEvent);//文件下载
wxDEFINE_EVENT(EVT_ACCLOUD_FILE_LOCK,        wxCommandEvent);//文件空间加锁
wxDEFINE_EVENT(EVT_ACCLOUD_FILE_UNLOCK,      wxCommandEvent);//文件空间解锁
wxDEFINE_EVENT(EVT_ACCLOUD_SLICE_STATUS,     wxCommandEvent);//切片状态
wxDEFINE_EVENT(EVT_ACCLOUD_SLICE_INFO,       wxCommandEvent);//切片信息
wxDEFINE_EVENT(EVT_ACCLOUD_MODEL_INFO,       wxCommandEvent);//模型信息


#define RESPONSE_FROM_CLOUD(a) auto SUCCEED = e->fFlag == SUCCEED_FLAG; \
	logMessage = wxString::FromUTF8(e->fMsg);                           \
	SUCCEED ? a = E_TRUE : a = E_FALSE;

#define CLOUD_EVENT_SEND(ex) wxCommandEvent* evt = new wxCommandEvent(ex); \
evt->SetInt(SUCCEED ? 100 : 0);\
evt->SetString(wxString::FromUTF8(e->fMsg));\
wxPostEvent(GUI::wxGetApp().mainframe, *evt);//异步\

ACCloudManger* ACCloudManger::GetInstance()
{
	static ACCloudManger obj;
	return &obj;
}
ACCloudManger::ACCloudManger()
{

}
ACCloudManger::~ACCloudManger()
{
	ui = nullptr;

}
void ACCloudManger::InitLater()
{
	RegisterHandler();
	EnableTestServer(); //开启测试服务
	if (CloudInstance->m_isTestServerPtr)
		bool isTestServer = CloudInstance->m_isTestServerPtr();
	GetCloudClient();     //
	isInit = true;
}
void ACCloudManger::PreInit()
{
	if (cloudClient)
	{
		//设置客户端语言
		cloudClient->config()->setLanguage(GUI::wxGetApp().is_language_chinese() ? "CN" : "US");
		cloudClient->config()->setDevice();//默认设置PC

	}
}
//#include <boost/preprocessor/stringize.hpp>
void ACCloudManger::RegisterHandler()
{
#define REGISTER_MESSAGE_HANDLER(EVENT, X, H)                                  \
  m_td.Register(EVENT, [this](const CloudEvent *e) {                              \
    auto SUCCEED = e->fFlag == SUCCEED_FLAG;                                   \
    H;                                                                         \
  })
#define REGISTER_MESSAGE(EVENT, X) REGISTER_MESSAGE_HANDLER(EVENT, X, (void)0)

#define REGISTER_HANDLER(EVENT, handler)                                       \
  m_td.Register(EVENT,                                                         \
                std::bind(&ACCloudManger::handler, this, std::placeholders::_1))

	REGISTER_HANDLER(EventLogin,            onLogin); //登录
	REGISTER_HANDLER(EventLogout,           onLogout);//登出
	REGISTER_HANDLER(EventResetPWD,         onReset); //重置密码
	REGISTER_HANDLER(EventPrinterList,      onPrinterList);//打印机列表
	REGISTER_HANDLER(EventAddPrinter,       onAddPrinter); //添加打印机
	REGISTER_HANDLER(EventSpaceInfo,        onSpaceInfo);  //获取空间信息
	REGISTER_HANDLER(EventDelPrinter,       onDeletePrinter);//删除成功
	REGISTER_HANDLER(EventRenamePrinter,    onRenamePrinter );//重命名成功

	REGISTER_HANDLER(EventCheckNick,        onCheckNick);//昵称可用
	REGISTER_HANDLER(EventCAPTCHA,          onCaptcha);//获取成功
	REGISTER_HANDLER(EventRegister,         onRegister);//注册成功
	REGISTER_HANDLER(EventFeedback,         onFeedback);//反馈成功
	REGISTER_HANDLER(EventCountries,        onCountries);//国家列表获取成功
	REGISTER_HANDLER(EventFilelist,         onFileList);//文件列表成功
	REGISTER_HANDLER(EventSliceStatus,      onSliceStatus);//获取切片状态成功
	REGISTER_HANDLER(EventSliceInfo,        onSliceInfo);//获取切片文件信息成功
	REGISTER_HANDLER(EventModelInfo,        onModelInfo);//获取模型文件信息成功
	REGISTER_HANDLER(EventFileLock,         onFileLock);//文件锁定成功

	REGISTER_HANDLER(EventFileremove,       onFileRemove);//文件删除成功
	REGISTER_HANDLER(EventFilerename,       onFileRename);//重命名成功
	REGISTER_HANDLER(EventFileDownload,     onFileDownload);//下载成功
	REGISTER_HANDLER(EventFileConfirmation, onFileConfirmation);//文件上传成功
	REGISTER_HANDLER(EventFileUnlock,       onFileUnlock);//文件解锁成功

	//REGISTER_MESSAGE(EventCheckNick,   _L("nickname available"));//昵称可用
	//REGISTER_MESSAGE(EventCAPTCHA,     _L("get successed"));//获取成功
	//REGISTER_MESSAGE(EventRegister,    _L("register success"));//注册成功
	//REGISTER_MESSAGE(EventFeedback,    _L("feedback success"));//反馈成功
	//REGISTER_MESSAGE(EventCountries,   _L("country list fetched successfully"));//国家列表获取成功
	//REGISTER_MESSAGE(EventFilelist,    _L("file list successful"));//文件列表成功
	//REGISTER_MESSAGE(EventSliceStatus, _L("get slice status successfully"));//获取切片状态成功
	//REGISTER_MESSAGE(EventSliceInfo,   _L("obtaining slice file information successfully"));//获取切片文件信息成功
	//REGISTER_MESSAGE(EventModelInfo,   _L("successfully obtained model file information"));//获取模型文件信息成功
	//REGISTER_MESSAGE(EventFileLock,    _L("file locked successfully"));//文件锁定成功

	//REGISTER_MESSAGE(EventFileremove,       _L("file deleted successfully"));//文件删除成功
	//REGISTER_MESSAGE(EventFilerename,       _L("renamed successfully"));//重命名成功
	//REGISTER_MESSAGE(EventDelPrinter,       _L("successfully deleted"));//删除成功
	//REGISTER_MESSAGE(EventRenamePrinter,    _L("重命名成功"));//重命名成功
	//REGISTER_MESSAGE(EventFileDownload,     _L("下载成功"));//下载成功
	//REGISTER_MESSAGE(EventFileConfirmation, _L("文件上传成功"));//文件上传成功
	//REGISTER_MESSAGE(EventFileUnlock,       _L("文件解锁成功"));//文件解锁成功
#undef REGISTER_HANDLER
#undef REGISTER_MESSAGE
#undef REGISTER_MESSAGE_HANDLER
}
bool ACCloudManger::GetCloudClient()
{
	if (CloudInstance->m_newPoolPtr)
	{
		int maxCore = CloudInstance->getHardConcurrency();
		taskPool = CloudInstance->m_newPoolPtr(maxCore);//最大核心
		CloudInstance->setBackTaskPoolPtr(taskPool);//回设指针
		if (!taskPool)
			wxLogMessage(" taskPool = nullptr");
	}
	if (taskPool && CloudInstance->m_newCloudClientPtr)
	{
		cloudClient = CloudInstance->m_newCloudClientPtr(this, taskPool);
		CloudInstance->setBackCloudClientPtr(cloudClient);//回设指针
	}
	if (cloudClient)
		return true;
	else
		return false;
}
bool ACCloudManger::EnableTestServer()
{
	if (CloudInstance->m_enableTestServerPtr)
	{
		CloudInstance->m_enableTestServerPtr(true);
		return true;
	}
	return false;
}

void ACCloudManger::onLogin(const CloudEvent* e)
{
	RESPONSE_FROM_CLOUD(eLogin)
	if (SUCCEED)
		GUI::wxGetApp().set_cloud_login(true);//将登录状态回设给主UI状态
	assert(SUCCEED == false || cloudClient->loginState() == ONLINE);
	
	CLOUD_EVENT_SEND(EVT_ACCLOUD_LOGIN);
}
void ACCloudManger::onLogout(const CloudEvent* e)
{
	RESPONSE_FROM_CLOUD(eLogout)
	if (SUCCEED)
		GUI::wxGetApp().set_cloud_login(false);//将登录状态回设给主UI状态

	CLOUD_EVENT_SEND(EVT_ACCLOUD_LOGOUT)
}
void ACCloudManger::onReset(const CloudEvent* e)
{
	RESPONSE_FROM_CLOUD(eResetPassword)

	CLOUD_EVENT_SEND(EVT_ACCLOUD_RESET_PASSWORD)
}
void ACCloudManger::onPrinterList(const CloudEvent* e)
{
	auto SUCCEED = e->fFlag == SUCCEED_FLAG;
	logMessage = wxString::FromUTF8(e->fMsg);
	if (!SUCCEED)//失败的情况 执行成功需要在打印机获取列表里赋值
		ePrinterList = E_FALSE;

	if (!SUCCEED)//失败发送，成功在获取完d打印机列表表里面发送事件
	{
        CLOUD_EVENT_SEND(EVT_ACCLOUD_PRINTER_LIST);
        wxCommandEvent *m_evt = new wxCommandEvent(EVT_ACCLOUD_PRINTER_LIST);
        m_evt->SetInt(SUCCEED ? 100 : 0);
        wxPostEvent(GUI::wxGetApp().mainframe->GetACCloudSelectMachineObj(), *m_evt);
	}
}
void ACCloudManger::onAddPrinter(const CloudEvent* e)
{
	RESPONSE_FROM_CLOUD(eAddPrinter)

	wxCommandEvent* evt = new wxCommandEvent(EVT_ACCLOUD_PRINTER_ADD);
	evt->SetInt(SUCCEED ? 100 : 0);//通知成功与否
	evt->SetString(wxString::FromUTF8(e->fMsg));
	wxPostEvent(GUI::wxGetApp().mainframe, *evt);//异步
}
void ACCloudManger::onRenamePrinter(const CloudEvent* e)
{
	RESPONSE_FROM_CLOUD(eRenamePrinter)

	wxCommandEvent* evt = new wxCommandEvent(EVT_ACCLOUD_PRINTER_RENAME);
	evt->SetInt(SUCCEED ? 100 : 0);//通知成功与否
	evt->SetString(wxString::FromUTF8(e->fMsg));
	wxPostEvent(GUI::wxGetApp().mainframe, *evt);//异步
}
void ACCloudManger::onDeletePrinter(const CloudEvent* e)
{
	RESPONSE_FROM_CLOUD(eDeletePrinter)

	wxCommandEvent* evt = new wxCommandEvent(EVT_ACCLOUD_PRINTER_DELETE);
	evt->SetInt(SUCCEED ? 100 : 0);//通知成功与否
	evt->SetString(wxString::FromUTF8(e->fMsg));
	wxPostEvent(GUI::wxGetApp().mainframe, *evt);//异步
}


void  ACCloudManger::event(const CloudEvent* e)
{
	assert(e != nullptr);
	m_td.Execute(e->fEvent, e);
}

void ACCloudManger::onCheckNick(const CloudEvent* e)//昵称可用
{
	RESPONSE_FROM_CLOUD(eCheckNickname)

	CLOUD_EVENT_SEND(EVT_ACCLOUD_CHECK_NICK_NAME)
}
void ACCloudManger::onCaptcha(const CloudEvent* e)//获取验证成功
{
	RESPONSE_FROM_CLOUD(eGetCaptcha)

	CLOUD_EVENT_SEND(EVT_ACCLOUD_GET_CAPTCHA)
}
void ACCloudManger::onRegister(const CloudEvent* e)//注册成功
{
	RESPONSE_FROM_CLOUD(eRegisterUser)

	CLOUD_EVENT_SEND(EVT_ACCLOUD_REGISTER_USER)
}
void ACCloudManger::onFeedback(const CloudEvent* e)//反馈成功
{
	RESPONSE_FROM_CLOUD(eFeedback)

	CLOUD_EVENT_SEND(EVT_ACCLOUD_FEEDBACK)
}
void ACCloudManger::onCountries(const CloudEvent* e)//国家列表获取成功
{
	auto SUCCEED = e->fFlag == SUCCEED_FLAG;
	logMessage = wxString::FromUTF8(e->fMsg);
	if (!SUCCEED)//失败的情况 执行成功需要在国家获取列表里赋值
		eCountries = E_FALSE;

	if (!SUCCEED)//失败发送，成功在获取完国家列表里面发送事件
	{
		CLOUD_EVENT_SEND(EVT_ACCLOUD_COUNTRIES)
	}
}


void ACCloudManger::onSpaceInfo(const CloudEvent* e)
{
	RESPONSE_FROM_CLOUD(eSpaceInfo)

	CLOUD_EVENT_SEND(EVT_ACCLOUD_SPACE_SIZE)
}
void ACCloudManger::onFileList(const CloudEvent* e)//文件列表成功
{
	auto SUCCEED = e->fFlag == SUCCEED_FLAG;
	logMessage = wxString::FromUTF8(e->fMsg);
	if (!SUCCEED)//失败的情况 执行成功需要在文件获取列表里赋值
		eFileList = E_FALSE;

	if (!SUCCEED)//失败
	{
		CLOUD_EVENT_SEND(EVT_ACCLOUD_FILE_LIST)
	}
}
void ACCloudManger::onFileLock(const CloudEvent* e)//文件锁定成功
{
	RESPONSE_FROM_CLOUD(eFileLock)

	CLOUD_EVENT_SEND(EVT_ACCLOUD_FILE_LOCK)
}
void ACCloudManger::onFileUnlock(const CloudEvent* e)//文件解锁成功
{
	RESPONSE_FROM_CLOUD(eFileUnlock)

	CLOUD_EVENT_SEND(EVT_ACCLOUD_FILE_UNLOCK)
}
void ACCloudManger::onFileRemove(const CloudEvent* e)//文件删除成功
{
	RESPONSE_FROM_CLOUD(eFileRemove)

	CLOUD_EVENT_SEND(EVT_ACCLOUD_FILE_REMOVE)
}
void ACCloudManger::onFileRename(const CloudEvent* e)//重命名成功
{
	RESPONSE_FROM_CLOUD(eFileRename)

	CLOUD_EVENT_SEND(EVT_ACCLOUD_FILE_RENAME)
}
void ACCloudManger::onFileDownload(const CloudEvent* e)//下载成功
{
	RESPONSE_FROM_CLOUD(eFileDownload)

	CLOUD_EVENT_SEND(EVT_ACCLOUD_FILE_DOWNLOAD)
}
void ACCloudManger::onFileConfirmation(const CloudEvent* e)//文件上传成功
{
	RESPONSE_FROM_CLOUD(eFileUpload)

	CLOUD_EVENT_SEND(EVT_ACCLOUD_FILE_UPLOAD)
}
void ACCloudManger::onSliceStatus(const CloudEvent* e)//获取切片状态成功
{
	RESPONSE_FROM_CLOUD(eSliceStatus)

	CLOUD_EVENT_SEND(EVT_ACCLOUD_SLICE_STATUS)
}
void ACCloudManger::onSliceInfo(const CloudEvent* e)//获取切片文件信息成功
{
	auto SUCCEED = e->fFlag == SUCCEED_FLAG;
	logMessage = wxString::FromUTF8(e->fMsg);
	if (!SUCCEED)//失败的情况 执行成功需要GCode信息获取里赋值
		eSliceInfo = E_FALSE;

	if (!SUCCEED)
	{
		CLOUD_EVENT_SEND(EVT_ACCLOUD_SLICE_INFO)
	}
}
void ACCloudManger::onModelInfo(const CloudEvent* e)//获取模型文件信息成功
{
	auto SUCCEED = e->fFlag == SUCCEED_FLAG;
	logMessage = wxString::FromUTF8(e->fMsg);
	if (!SUCCEED)//失败的情况 执行成功需要在模型信息里赋值
		eModelInfo = E_FALSE;

	if (!SUCCEED)
	{
		CLOUD_EVENT_SEND(EVT_ACCLOUD_MODEL_INFO)
	}
}


void  ACCloudManger::event(const PrinterResponse* printer, uint32_t count)
{
	printerDataList.clear();
	for (int i = 0; i < count; i++)
	{
		if (printer)
		{
			PrinterData data;
			data.id = i;
			data.id_printer = printer->id;
			data.name = wxString(printer->name, wxConvUTF8);
			data.status = printer->device_status;
			data.type = printer->model;
			data.isChecked = false;

			if (printer->img)
			{
				auto onDownloaded = [this](bool succeed)
				{
					std::cout << succeed;
				};

				auto onProgress = [](int percent, bool& cancel)
				{
					std::cout << percent << std::endl;
				};

				auto onCancel = []()
				{
					return false;
				};
				data.fileName = CloudInstance->getPrinterImageWorkFullPath(printer->img);
				if (!CloudInstance->isImageExist(data.fileName))//如果不存在，则进行下载
					CloudInstance->downloadImage(onDownloaded, onProgress, onCancel, printer->img);
			}
			printerDataList.push_back(data);
		}
		printer++;
	}
	
	wxCommandEvent* evt = new wxCommandEvent(EVT_ACCLOUD_PRINTER_LIST);
	evt->SetInt(100);//设置获取成功状态
    wxPostEvent(GUI::wxGetApp().mainframe->GetACCloudSelectMachineObj(), *evt); //异步
	ePrinterList = E_TRUE;
}
void  ACCloudManger::event(const CountryResponse* ctry, uint32_t count)
{
	wxCommandEvent* evt = new wxCommandEvent(EVT_ACCLOUD_FILE_LIST);
	evt->SetInt(100);//设置获取成功状态
	wxPostEvent(GUI::wxGetApp().mainframe, *evt);//异步
	eCountries = E_TRUE;
	//CLOUD_EVENT_SEND(EVT_ACCLOUD_COUNTRIES)
}
void  ACCloudManger::event(const FileInfoResponse* info, uint32_t count, uint32_t pageIndex, uint32_t pageSize, uint32_t countAll)
{
	wxCommandEvent* evt = new wxCommandEvent(EVT_ACCLOUD_PRINTER_LIST);
	evt->SetInt(100);//设置获取成功状态
	wxPostEvent(GUI::wxGetApp().mainframe, *evt);//异步
	eFileList = E_TRUE;
}
void  ACCloudManger::event(const ModelInfoResponse* info)
{
	wxCommandEvent* evt = new wxCommandEvent(EVT_ACCLOUD_MODEL_INFO);
	evt->SetInt(100);//设置获取成功状态
	wxPostEvent(GUI::wxGetApp().mainframe, *evt);//异步
	eModelInfo = E_TRUE;
}
void  ACCloudManger::event(const GCodeInfoResponse* info)
{
	wxCommandEvent* evt = new wxCommandEvent(EVT_ACCLOUD_SLICE_INFO);
	evt->SetInt(100);//设置获取成功状态
	wxPostEvent(GUI::wxGetApp().mainframe, *evt);//异步
	eSliceInfo = E_TRUE;
}
void  ACCloudManger::event(const ProgressResponse* progress)
{
	//进度信息反馈
}

void ACCloudManger::GetCloudInfo()
{
    std::vector<PrinterData> m_info;
    PrinterData              data;
    data.fileName = std::string("4MAXPRO20_thumbnail");
    data.name     = "My Printer 1 123456789 123456789123456789123";
    data.status   = 0;
    data.type     = "Kobra 2 neo";
    data.id       = 0;
    m_info.push_back(data);

    data.fileName = std::string("Kobra2 Pro_thumbnail");
    data.name     = "My Printer 2 2341afwetg";
    data.status   = 1;
    data.type     = "Kobra 2 pro";
    data.id       = 1;
    m_info.push_back(data);

    data.fileName = std::string("Kobra2_thumbnail");
    data.name     = "My Printer 3";
    data.status   = 2;
    data.type     = "Kobra 2";
    data.id       = 2;
    m_info.push_back(data);

    data.fileName = std::string("I3MEGAS_thumbnail");
    data.name     = "My Printer 4";
    data.status   = 0;
    data.type     = "Kobra 2 neo";
    data.id       = 3;
    m_info.push_back(data);

    data.fileName = std::string("MEGA0_thumbnail");
    data.name     = "My Printer 5";
    data.status   = 1;
    data.type     = "Kobra 2 pro";
    data.id       = 4;
    m_info.push_back(data);

    data.fileName = std::string("Kobra2_thumbnail");
    data.name     = "My Printer 6";
    data.status   = 2;
    data.type     = "Kobra 2";
    data.id       = 5;
    m_info.push_back(data);

    data.fileName = std::string("Kobra2 Neo_thumbnail");
    data.name     = "My Printer 7";
    data.status   = 0;
    data.type     = "Kobra 2 neo";
    data.id       = 6;
    m_info.push_back(data);

    data.fileName = std::string("Kobra2 Pro_thumbnail");
    data.name     = "My Printer 8";
    data.status   = 1;
    data.type     = "Kobra 2 pro";
    data.id       = 7;
    m_info.push_back(data);

    data.fileName = std::string("Kobra2_thumbnail");
    data.name     = "My Printer 9";
    data.status   = 2;
    data.type     = "Kobra 2";
    data.id       = 8;
    m_info.push_back(data);

    data.fileName = std::string("Kobra2_thumbnail");
    data.name     = "My Printer 10";
    data.status   = 2;
    data.type     = "Kobra 2";
    data.id       = 9;
    m_info.push_back(data);

    data.fileName = std::string("4MAXPRO20_thumbnail");
    data.name     = "My Printer 11 123456789 123456789123456789123";
    data.status   = 0;
    data.type     = "Kobra 2 neo";
    data.id       = 10;
    m_info.push_back(data);

    data.fileName = std::string("Kobra2 Pro_thumbnail");
    data.name     = "My Printer 12 2341afwetg";
    data.status   = 1;
    data.type     = "Kobra 2 pro";
    data.id       = 11;
    m_info.push_back(data);

    data.fileName = std::string("Kobra2_thumbnail");
    data.name     = "My Printer 13";
    data.status   = 2;
    data.type     = "Kobra 2";
    data.id       = 12;
    m_info.push_back(data);

    data.fileName = std::string("I3MEGAS_thumbnail");
    data.name     = "My Printer 14";
    data.status   = 0;
    data.type     = "Kobra 2 neo";
    data.id       = 13;
    m_info.push_back(data);

    data.fileName = std::string("MEGA0_thumbnail");
    data.name     = "My Printer 15";
    data.status   = 1;
    data.type     = "Kobra 2 pro";
    data.id       = 14;
    m_info.push_back(data);

    data.fileName = std::string("Kobra2_thumbnail");
    data.name     = "My Printer 16";
    data.status   = 2;
    data.type     = "Kobra 2";
    data.id       = 15;
    m_info.push_back(data);

    data.fileName = std::string("Kobra2 Neo_thumbnail");
    data.name     = "My Printer 17";
    data.status   = 0;
    data.type     = "Kobra 2 neo";
    data.id       = 16;
    m_info.push_back(data);

    data.fileName = std::string("Kobra2 Pro_thumbnail");
    data.name     = "My Printer 18";
    data.status   = 1;
    data.type     = "Kobra 2 pro";
    data.id       = 17;
    m_info.push_back(data);

    data.fileName = std::string("Kobra2_thumbnail");
    data.name     = "My Printer 19";
    data.status   = 2;
    data.type     = "Kobra 2";
    data.id       = 18;
    m_info.push_back(data);

    data.fileName = std::string("Kobra2_thumbnail");
    data.name     = "My Printer 20";
    data.status   = 2;
    data.type     = "Kobra 2";
    data.id       = 19;
    m_info.push_back(data);

    data.fileName = std::string("4MAXPRO20_thumbnail");
    data.name     = "My Printer 21 123456789 123456789123456789123";
    data.status   = 0;
    data.type     = "Kobra 2 neo";
    data.id       = 20;
    m_info.push_back(data);

    data.fileName = std::string("Kobra2 Pro_thumbnail");
    data.name     = "My Printer 22 2341afwetg";
    data.status   = 1;
    data.type     = "Kobra 2 pro";
    data.id       = 21;
    m_info.push_back(data);

    data.fileName = std::string("Kobra2_thumbnail");
    data.name     = "My Printer 23";
    data.status   = 2;
    data.type     = "Kobra 2";
    data.id       = 22;
    m_info.push_back(data);

    data.fileName = std::string("I3MEGAS_thumbnail");
    data.name     = "My Printer 24";
    data.status   = 0;
    data.type     = "Kobra 2 neo";
    data.id       = 23;
    m_info.push_back(data);

    data.fileName = std::string("MEGA0_thumbnail");
    data.name     = "My Printer 25";
    data.status   = 1;
    data.type     = "Kobra 2 pro";
    data.id       = 24;
    m_info.push_back(data);

    data.fileName = std::string("Kobra2_thumbnail");
    data.name     = "My Printer 26";
    data.status   = 2;
    data.type     = "Kobra 2";
    data.id       = 25;
    m_info.push_back(data);

    data.fileName = std::string("Kobra2 Neo_thumbnail");
    data.name     = "My Printer 27";
    data.status   = 0;
    data.type     = "Kobra 2 neo";
    data.id       = 26;
    m_info.push_back(data);

    data.fileName = std::string("Kobra2 Pro_thumbnail");
    data.name     = "My Printer 28";
    data.status   = 1;
    data.type     = "Kobra 2 pro";
    data.id       = 27;
    m_info.push_back(data);

    data.fileName = std::string("Kobra2_thumbnail");
    data.name     = "My Printer 29";
    data.status   = 2;
    data.id       = 28;
    data.type     = "Kobra 2";
    m_info.push_back(data);

    data.fileName = std::string("Kobra2_thumbnail");
    data.name     = "My Printer 30";
    data.status   = 2;
    data.type     = "Kobra 2";
    data.id       = 29;
    m_info.push_back(data);

    printerDataList = m_info;

}
void ACCloudManger::WaitResult(EResultCode& resultCode)
{
	resultCode = E_TIMEOUT;
	const std::chrono::time_point<std::chrono::steady_clock> start = std::chrono::steady_clock::now();
	while (E_TIMEOUT == resultCode)
	{
		std::this_thread::sleep_for(std::chrono::milliseconds(50));//休眠50毫秒
		const auto end = std::chrono::steady_clock::now();
		std::chrono::milliseconds ms = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
		long long lv = ms.count();
		if (lv > 3000)
			return;
	}
}

EResultCode ACCloudManger::Login(const char* username, const char* password, int accountType)
{
	/**
	* @brief 登录操作
	*
	* @param username 用户名
	* @param password 用户密码
	* @param accountType 帐号类型 2:国外,其他:国内
	*/
	cloudClient->login(username, password, accountType);
	WaitResult(eLogin);
	return eLogin;
}
EResultCode ACCloudManger::Logout()
{
	/**
	* @brief 退出登录状态
	*
	*/
	cloudClient->logout();
	WaitResult(eLogout);
	return eLogout;
}
void ACCloudManger::login(const char* username, const char* password, int accountType)
{
	/**
	* @brief 登录操作
	*
	* @param username 用户名
	* @param password 用户密码
	* @param accountType 帐号类型 2:国外,其他:国内
	*/
	cloudClient->login(username, password, accountType);
}
void ACCloudManger::logout()
{
	/**
	* @brief 退出登录状态
	*
	*/
	cloudClient->logout();
}

//注册相关
EResultCode ACCloudManger::CheckNickname(const char* nick)//昵称检测
{
	/**
	* @brief 检测昵称占用状态
	*
	* @param nick
	*/
	cloudClient->checkNickname(nick);
	WaitResult(eCheckNickname);
	return eCheckNickname;
}
EResultCode ACCloudManger::GetCaptcha(const char* username, int type)//获取验证码
{
	/**
	* @brief
	*
	* @param username
	* @param type 注册时：email=1, phone=2， 重置密码时：email=3, phone=4
	*/
	cloudClient->getCAPTCHA(username, type);
	WaitResult(eGetCaptcha);
	return eGetCaptcha;
}
EResultCode ACCloudManger::ResetPassword(const char* username, const char* pwd, const char* captcha, int resetType)
{
	/**
	* @brief 重置密码
	*
	* @param username 用户名
	* @param pwd      新密码
	* @param captcha  验证码
	* @param resetType email == 3, phone == 4
	*/
	cloudClient->resetPassword(username, pwd, captcha, resetType);
	WaitResult(eResetPassword);
	return eResetPassword;
}
EResultCode ACCloudManger::RegisterUser(const char* account, const char* password,
	const char* nickname, int accountType, const char* captcha)//用户注册
{
	/**
	* @brief  用主注册
	*
	* @param account 帐号名
	* @param password 密码
	* @param nickname 昵称
	* @param accountType //2 国外,1 国内
	* @param captcha 验证码
	*/
	cloudClient->registerUser(account, password, nickname, accountType, captcha);
	WaitResult(eRegisterUser);
	return eRegisterUser;
}
EResultCode ACCloudManger::Feedback(const char* username, const char* content)//反馈内容
{
	/**
	* @brief 发送反馈信息
	*
	* @param username 用户名可为空，但不能为nullptr
	* @param content 反馈内容
	* @note 不要求登录可用
	*/
	cloudClient->feedback(username, content);
	WaitResult(eFeedback);
	return eFeedback;
}
EResultCode ACCloudManger::Countries(bool hot)//获取国家列表
{
	/**
	* @brief 获取国家列表
	*
	* @param hot 是否为热点国家
	*/
	cloudClient->countries(hot);
	WaitResult(eCountries);
	return eCountries;
}
void ACCloudManger::checkNickname(const char* nick)//昵称检测
{
	/**
	* @brief 检测昵称占用状态
	*
	* @param nick
	*/
	cloudClient->checkNickname(nick);
}
void ACCloudManger::getCaptcha(const char* username, int type)//获取验证码
{
	/**
	* @brief
	*
	* @param username
	* @param type 注册时：email=1, phone=2， 重置密码时：email=3, phone=4
	*/
	cloudClient->getCAPTCHA(username, type);
}
void ACCloudManger::resetPassword(const char* username, const char* pwd, const char* captcha, int resetType)
{
	/**
	* @brief 重置密码
	*
	* @param username 用户名
	* @param pwd      新密码
	* @param captcha  验证码
	* @param resetType email == 3, phone == 4
	*/
	cloudClient->resetPassword(username, pwd, captcha, resetType);
}
void ACCloudManger::registerUser(const char* account, const char* password,
	const char* nickname, int accountType, const char* captcha)//用户注册
{
	/**
	* @brief  用主注册
	*
	* @param account 帐号名
	* @param password 密码
	* @param nickname 昵称
	* @param accountType //2 国外,1 国内
	* @param captcha 验证码
	*/
	cloudClient->registerUser(account, password, nickname, accountType, captcha);
}
void ACCloudManger::feedback(const char* username, const char* content)//反馈内容
{
	/**
	* @brief 发送反馈信息
	*
	* @param username 用户名可为空，但不能为nullptr
	* @param content 反馈内容
	* @note 不要求登录可用
	*/
	cloudClient->feedback(username, content);
}
void ACCloudManger::countries(bool hot)//获取国家列表
{
	/**
	* @brief 获取国家列表
	*
	* @param hot 是否为热点国家
	*/
	cloudClient->countries(hot);
}


//打印机相关
EResultCode ACCloudManger::PrinterList()
{
	/**
	* @brief 获取已添加打印机列表
	*
	*/
	cloudClient->printerList();//发送打印机列表
	WaitResult(ePrinterList);
	return ePrinterList;
}
EResultCode ACCloudManger::AddPrinter(const char* deviceCN)
{
	/**
	* @brief 添加在线打印机
	*
	* @param deviceID 设备ID
	*/
	cloudClient->addPrinter(deviceCN);
	WaitResult(eAddPrinter);
	return eAddPrinter;
}
EResultCode ACCloudManger::RenamePrinter(uint32_t printerID, const char* printerName)
{
	/**
	* @brief 重命名打印机
	*
	* @param printerID 打印机ID
	* @param printerName 新名称
	*/
	cloudClient->renamePrinter(printerID, printerName);
	WaitResult(eRenamePrinter);
	return eRenamePrinter;
}
EResultCode ACCloudManger::DeletePrinter(uint32_t printerID)
{
	/**
	* @brief 删除打印机
	*
	* @param printerID 打印机ID
	*/
	cloudClient->delPrinter(printerID);
	WaitResult(eDeletePrinter);
	return eDeletePrinter;
}
void ACCloudManger::printerList()//获取打印机列表
{
	/**
	* @brief 获取已添加打印机列表
	*
	*/
	cloudClient->printerList();
}
void ACCloudManger::addPrinter(const char* deviceCN)//添加打印机
{
	/**
	* @brief 添加在线打印机
	*
	* @param deviceID 设备ID
	*/
	cloudClient->addPrinter(deviceCN);
}
void ACCloudManger::renamePrinter(uint32_t printerID, const char* printerName)//重命名打印机
{
	/**
	* @brief 重命名打印机
	*
	* @param printerID 打印机ID
	* @param printerName 新名称
	*/
	cloudClient->renamePrinter(printerID, printerName);
}
void ACCloudManger::deletePrinter(uint32_t printerID)//删除打印机
{
	/**
	* @brief 删除打印机
	*
	* @param printerID 打印机ID
	*/
	cloudClient->delPrinter(printerID);
}

//文件相关
EResultCode ACCloudManger::SpaceInfo()//获取文件空间信息
{
	cloudClient->spaceInfo();
	WaitResult(eSpaceInfo);
	return eSpaceInfo;
}
EResultCode ACCloudManger::FileList(int32_t pageIndex, int32_t pageSize, int32_t file_type, int32_t orderby)//文件获取列表
{
	/**
	* @brief 在线文件列表
	*
	* @param pageIndex 页码--从1开始
	* @param pageSize  每页显示的条数
	* @param file_type 1:模型文件,2:切片文件,3:其他,0：全部
	* @param orderby 1:创建时间倒序,2:创建时间正序,3:文件大小倒序,4:文件大小正序
	*/
	cloudClient->fileLists(pageIndex, pageSize, file_type, orderby);
	WaitResult(eFileList);
	return eFileList;
}
EResultCode ACCloudManger::FileRemove(const int* fileID, int count)//文件删除
{
	/**
	* @brief 移除在线文件
	*
	* @param fileID 文件ID数组
	* @param count id数量
	*/
	cloudClient->removeFile(fileID, count);
	WaitResult(eFileRemove);
	return eFileRemove;
}
EResultCode ACCloudManger::FileRename(int fileID, const char* filename)//文件重命名
{
	/**
	* @brief 重命名文件
	*
	* @param fileID 文件ID
	* @param filename 文件名，不能带扩展名！！！
	* @note 文件修改不允许修改文件扩展名
	*/
	cloudClient->renameFile(fileID, filename);
	WaitResult(eFileRename);
	return eFileRename;
}
EResultCode ACCloudManger::FileUpload(const char* objectName, const char* filename, uint32_t id, uint32_t timeout)//文件上传
{
	ctrlUpload = cloudClient->uploadFile(objectName, filename, id, timeout);
	WaitResult(eFileUpload);//这地方需要重新设计，上传是一个耗时过程，暂且这样写
	return eFileUpload;
}
EResultCode ACCloudManger::FileDownload(const char* url, const char* filename, const char* fmd5,
	uint32_t id, uint32_t timeout, long buffersize)//文件下载
{
	ctrlDownload = cloudClient->downloadFile(url, filename, fmd5, id, timeout, buffersize);
	WaitResult(eFileDownload);//这地方需要重新设计，下载是一个耗时过程，暂且这样写
	return eFileDownload;
}
EResultCode ACCloudManger::SliceStatus(uint32_t id)//获取切片状态
{
	/**
	* @brief 获取切片文件状态
	*
	* @param id 上传成功后返回的id；或者文件列表的id
	* @note Event::GetValue("status") : uint32_t 1=解析成功，2=解析中，3=解析失败
	* @note Event::GetValue("gcode_id") :  uint32_t 切片文件的id
	* @note Event::GetValue("id") : uint32_t 文件ID
	*/
	cloudClient->sliceStatus(id);
	WaitResult(eSliceStatus);
	return eSliceStatus;
}
EResultCode ACCloudManger::SliceInfo(uint64_t fileID)//获取切片文件信息
{
	/**
	* @brief 获取切片文件信息
	*
	* @param fileID
	*/
	cloudClient->sliceInfo(fileID);
	WaitResult(eSliceInfo);
	return eSliceInfo;

}
EResultCode ACCloudManger::ModelInfo(uint32_t id)//获取模型文件信息
{
	/**
	* @brief 获取模型文件信息
	*
	* @param id 文件id
	*/
	cloudClient->modelInfo(id);
	WaitResult(eModelInfo);
	return eModelInfo;
}

void ACCloudManger::spaceInfo()//获取文件空间信息
{
	cloudClient->spaceInfo();
}
void ACCloudManger::fileList(int32_t pageIndex, int32_t pageSize, int32_t file_type, int32_t orderby)//文件获取列表
{
	/**
	* @brief 在线文件列表
	*
	* @param pageIndex 页码--从1开始
	* @param pageSize  每页显示的条数
	* @param file_type 1:模型文件,2:切片文件,3:其他,0：全部
	* @param orderby 1:创建时间倒序,2:创建时间正序,3:文件大小倒序,4:文件大小正序
	*/
	cloudClient->fileLists(pageIndex, pageSize, file_type, orderby);
}
void ACCloudManger::fileRemove(const int* fileID, int count)//文件删除
{
	/**
	* @brief 移除在线文件
	*
	* @param fileID 文件ID数组
	* @param count id数量
	*/
	cloudClient->removeFile(fileID, count);
}
void ACCloudManger::fileRename(int fileID, const char* filename)//文件重命名
{
	/**
	* @brief 重命名文件
	*
	* @param fileID 文件ID
	* @param filename 文件名，不能带扩展名！！！
	* @note 文件修改不允许修改文件扩展名
	*/
	cloudClient->renameFile(fileID, filename);
}
void ACCloudManger::fileUpload(const char* objectName, const char* filename, uint32_t id, uint32_t timeout)//文件上传
{
	ctrlUpload = cloudClient->uploadFile(objectName, filename, id, timeout);
}
void ACCloudManger::fileDownload(const char* url, const char* filename, const char* fmd5,
	uint32_t id, uint32_t timeout, long buffersize)//文件下载
{
	ctrlDownload = cloudClient->downloadFile(url, filename, fmd5, id, timeout, buffersize);
}
void ACCloudManger::sliceStatus(uint32_t id)//获取切片状态
{
	/**
	* @brief 获取切片文件状态
	*
	* @param id 上传成功后返回的id；或者文件列表的id
	* @note Event::GetValue("status") : uint32_t 1=解析成功，2=解析中，3=解析失败
	* @note Event::GetValue("gcode_id") :  uint32_t 切片文件的id
	* @note Event::GetValue("id") : uint32_t 文件ID
	*/
	cloudClient->sliceStatus(id);
}
void ACCloudManger::sliceInfo(uint64_t fileID)//获取切片文件信息
{
	/**
	* @brief 获取切片文件信息
	*
	* @param fileID
	*/
	cloudClient->sliceInfo(fileID);
}
void ACCloudManger::modelInfo(uint32_t id)//获取模型文件信息
{
	/**
	* @brief 获取模型文件信息
	*
	* @param id 文件id
	*/
	cloudClient->modelInfo(id);
}




#undef RESPONSE_FROM_CLOUD
} // GUI
} // Slic3r


