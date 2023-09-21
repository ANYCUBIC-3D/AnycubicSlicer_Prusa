#ifndef AC_CLOUD_MANGER_HPP
#define AC_CLOUD_MANGER_HPP


/*---------------------------------------------------------------------------------
								用于云SDK API交互调用
-----------------------------------------------------------------------------------*/

#include "ACTableDriver.hpp"
#include "slic3r/Utils/ACCloudWork.hpp"
#include "ACPrinterContainer.hpp"
#include "cloud_client.hxx"
#include "event_handler.hxx"
#include "contrl.hxx"//文件上传下载控制
#include "HttpClientBase/ITaskPool.hxx"

#include <string>
#include <wx/event.h>

namespace Slic3r {

namespace GUI {

wxDECLARE_EVENT(EVT_ACCLOUD_LOGIN,            wxCommandEvent);//登录
wxDECLARE_EVENT(EVT_ACCLOUD_LOGOUT,           wxCommandEvent);//登出

wxDECLARE_EVENT(EVT_ACCLOUD_CHECK_NICK_NAME,  wxCommandEvent);//检测昵称事件
wxDECLARE_EVENT(EVT_ACCLOUD_GET_CAPTCHA,      wxCommandEvent);//获取验证码
wxDECLARE_EVENT(EVT_ACCLOUD_RESET_PASSWORD,   wxCommandEvent);//重置密码
wxDECLARE_EVENT(EVT_ACCLOUD_REGISTER_USER,    wxCommandEvent);//用户注册
wxDECLARE_EVENT(EVT_ACCLOUD_FEEDBACK,         wxCommandEvent);//反馈内容
wxDECLARE_EVENT(EVT_ACCLOUD_COUNTRIES,        wxCommandEvent);//国家列表

wxDECLARE_EVENT(EVT_ACCLOUD_PRINTER_LIST,     wxCommandEvent);//打印机列表
wxDECLARE_EVENT(EVT_ACCLOUD_PRINTER_ADD,      wxCommandEvent);//打印机添加
wxDECLARE_EVENT(EVT_ACCLOUD_PRINTER_RENAME,   wxCommandEvent);//打印机重命名
wxDECLARE_EVENT(EVT_ACCLOUD_PRINTER_DELETE,   wxCommandEvent);//打印机删除

wxDECLARE_EVENT(EVT_ACCLOUD_SPACE_SIZE,       wxCommandEvent);//云空间容量
wxDECLARE_EVENT(EVT_ACCLOUD_FILE_LIST,        wxCommandEvent);//获取文件
wxDECLARE_EVENT(EVT_ACCLOUD_FILE_REMOVE,      wxCommandEvent);//文件删除
wxDECLARE_EVENT(EVT_ACCLOUD_FILE_RENAME,      wxCommandEvent);//文件重命名
wxDECLARE_EVENT(EVT_ACCLOUD_FILE_UPLOAD,      wxCommandEvent);//文件上传
wxDECLARE_EVENT(EVT_ACCLOUD_FILE_DOWNLOAD,    wxCommandEvent);//文件下载
wxDECLARE_EVENT(EVT_ACCLOUD_FILE_LOCK,        wxCommandEvent);//文件空间加锁
wxDECLARE_EVENT(EVT_ACCLOUD_FILE_UNLOCK,      wxCommandEvent);//文件空间解锁
wxDECLARE_EVENT(EVT_ACCLOUD_SLICE_STATUS,     wxCommandEvent);//切片状态
wxDECLARE_EVENT(EVT_ACCLOUD_SLICE_INFO,       wxCommandEvent);//切片信息
wxDECLARE_EVENT(EVT_ACCLOUD_MODEL_INFO,       wxCommandEvent);//模型信息

class ACCloudLoginDialog;

//可以通过获取Cloud->logMessage来打印执行消息
enum EResultCode
{
	E_FALSE,   //0 执行失败
	E_TRUE,    //1 执行成功
	E_TIMEOUT  //2 等待超时
};

#define Cloud ACCloudManger::GetInstance()

class ACCloudManger : public wxEvtHandler, public EmptyEventHandler
{
public:
	static ACCloudManger* GetInstance();
	~ACCloudManger();

	ACCloudManger(ACCloudManger&&) = delete;
	ACCloudManger(const ACCloudManger&) = delete;
	ACCloudManger& operator=(ACCloudManger&&) = delete;
	ACCloudManger& operator=(const ACCloudManger&) = delete;

	

	void InitLater();//只进行一次初始化
	void PreInit();//实例生成，每次都需要初始化的一些操作
	void SetPtr(ACCloudLoginDialog* p) { ui = p; }

	void GetCloudInfo();
	std::vector<PrinterData> GetPrinterInfo() { return printerDataList; }


	//wxString 要转 const char* 建议用 .utf8_str().data()  UTF8编码

	//登录相关
	EResultCode Login(const char* username, const char* password, int accountType);//登录
	EResultCode Logout();//登出

	void login(const char* username, const char* password, int accountType);//登录
	void logout();//登出


	//注册相关
	EResultCode CheckNickname(const char* nick);//昵称检测
	EResultCode GetCaptcha(const char* username, int type);//获取验证码
	EResultCode ResetPassword(const char* username, const char* pwd, const char* captcha, int resetType);//重置密码
	EResultCode RegisterUser(const char* account, const char* password, const char* nickname, int accountType, const char* captcha);//用户注册
	EResultCode Feedback(const char* username, const char* content);//反馈内容
	EResultCode Countries(bool hot = false);//获取国家列表

	void checkNickname(const char* nick);//昵称检测
	void getCaptcha(const char* username, int type);//获取验证码
	void resetPassword(const char* username, const char* pwd, const char* captcha, int resetType);//重置密码
	void registerUser(const char* account, const char* password, const char* nickname, int accountType, const char* captcha);//用户注册
	void feedback(const char* username, const char* content);//反馈内容
	void countries(bool hot = false);//获取国家列表


	//打印机相关
	EResultCode PrinterList();//获取打印机列表
	EResultCode AddPrinter(const char* deviceCN);//添加打印机
	EResultCode RenamePrinter(uint32_t printerID, const char* printerName);//重命名打印机
	EResultCode DeletePrinter(uint32_t printerID);//删除打印机

	void printerList();//获取打印机列表
	void addPrinter(const char* deviceCN);//添加打印机
	void renamePrinter(uint32_t printerID, const char* printerName);//重命名打印机
	void deletePrinter(uint32_t printerID);//删除打印机

	//文件相关
	EResultCode SpaceInfo();//云空间容量信息
	EResultCode FileList(int32_t pageIndex, int32_t pageSize, int32_t file_type, int32_t orderby);//文件获取列表
	EResultCode FileRemove(const int* fileID, int count);//文件删除
	EResultCode FileRename(int fileID, const char* filename);//文件重命名
	EResultCode FileUpload(const char* objectName, const char* filename, uint32_t id, uint32_t timeout = 0);//文件上传
	EResultCode FileDownload(const char* url, const char* filename, const char* fmd5,
		uint32_t id, uint32_t timeout = 0, long buffersize = 16384L);//文件下载
	EResultCode SliceStatus(uint32_t id);//获取切片状态
	EResultCode SliceInfo(uint64_t fileID);//获取切片文件信息
	EResultCode ModelInfo(uint32_t id);//获取模型文件信息
	
	void spaceInfo();//云空间容量信息
	void fileList(int32_t pageIndex, int32_t pageSize, int32_t file_type, int32_t orderby);//文件获取列表
	void fileRemove(const int* fileID, int count);//文件删除
	void fileRename(int fileID, const char* filename);//文件重命名
	void fileUpload(const char* objectName, const char* filename, uint32_t id, uint32_t timeout = 0);//文件上传
	void fileDownload(const char* url, const char* filename, const char* fmd5,
		uint32_t id, uint32_t timeout = 0, long buffersize = 16384L);//文件下载
	void sliceStatus(uint32_t id);//获取切片状态
	void sliceInfo(uint64_t fileID);//获取切片文件信息
	void modelInfo(uint32_t id);//获取模型文件信息


private:
	bool GetCloudClient();
	bool EnableTestServer();


	std::vector<PrinterData> printerDataList;

protected:
	ACCloudManger();
	void RegisterHandler();
	//void Signal(bool isSuccess, int eventID, const wxString& str);
	void WaitResult(EResultCode& resultCode);
private:
	virtual void event(const CloudEvent* e) override;
	virtual void event(const PrinterResponse* printer, uint32_t count) override;
	virtual void event(const CountryResponse* ctry, uint32_t count) override;
	virtual void event(const FileInfoResponse* info, uint32_t count, uint32_t pageIndex, uint32_t pageSize, uint32_t countAll) override;
	virtual void event(const ModelInfoResponse* info) override;
	virtual void event(const GCodeInfoResponse* info) override;
	virtual void event(const ProgressResponse* progress) override;
private:
	//回调事件执行结果 有需要的可以做拦截处理，不需要的直接跳过
	void onLogin            (const CloudEvent* e);//登录
	void onLogout           (const CloudEvent* e);//登出
	
	void onPrinterList      (const CloudEvent* e);//获取打印机
	void onAddPrinter       (const CloudEvent* e);//添加打印机
	void onRenamePrinter    (const CloudEvent* e);//重命名打印机
	void onDeletePrinter    (const CloudEvent* e);//删除打印机
	
	//注册相关
	void onCheckNick        (const CloudEvent* e);//昵称可用
	void onCaptcha          (const CloudEvent* e);//获取验证成功
	void onReset            (const CloudEvent* e);//重置密码
	void onRegister         (const CloudEvent* e);//注册成功
	void onFeedback         (const CloudEvent* e);//反馈成功
	void onCountries        (const CloudEvent* e);//国家列表获取成功

	//文件相关事件
	void onSpaceInfo        (const CloudEvent* e);//空间信息，云空间容量
	void onFileList         (const CloudEvent* e);//文件列表成功
	void onFileLock         (const CloudEvent* e);//文件锁定成功
	void onFileRemove       (const CloudEvent* e);//文件删除成功
	void onFileRename       (const CloudEvent* e);//重命名成功
	void onFileDownload     (const CloudEvent* e);//下载成功
	void onFileConfirmation (const CloudEvent* e);//文件上传成功
	void onFileUnlock       (const CloudEvent* e);//文件解锁成功
	void onSliceStatus      (const CloudEvent* e);//获取切片状态成功
	void onSliceInfo        (const CloudEvent* e);//获取切片文件信息成功
	void onModelInfo        (const CloudEvent* e);//获取模型文件信息成功
public:
	bool isInit{ false };
	bool isOriginCall{ true };
	ICloudClient* cloudClient { nullptr };
	ITaskPool*    taskPool    { nullptr };
	IContrl* ctrlUpload       { nullptr };//控制上传文件
	IContrl* ctrlDownload     { nullptr };//控制下载文件
	wxString logMessage;//用来存储执行某些结果的返回信息
private:
	ACCloudLoginDialog* ui{ nullptr };//主要用于更新输出日志信息之类的
	TableDriver<EventType, std::function<void(const CloudEvent*)>> m_td;

	//避免两个指令调用连续，分开定义状态处理
	//登录相关的
	EResultCode eLogin         { E_TIMEOUT };//执行登录结果
	EResultCode eLogout        { E_TIMEOUT };//执行登出结果

	//注册相关
	EResultCode eCheckNickname { E_TIMEOUT };//执行昵称检测结果
	EResultCode eGetCaptcha    { E_TIMEOUT };//执行验证码获取结果
	EResultCode eResetPassword { E_TIMEOUT };//执行重置密码结果
	EResultCode eRegisterUser  { E_TIMEOUT };//执行用户注册结果
	EResultCode eFeedback      { E_TIMEOUT };//执行反馈结果
	EResultCode eCountries     { E_TIMEOUT };//执行获取国家列表结果

	//打印机相关
	EResultCode ePrinterList   { E_TIMEOUT };//执行获取打印机列表结果
	EResultCode eAddPrinter    { E_TIMEOUT };//执行添加打印机结果
	EResultCode eRenamePrinter { E_TIMEOUT };//执行重命名打印机结果
	EResultCode eDeletePrinter { E_TIMEOUT };//执行删除打印机结果

	//文件相关
	EResultCode eSpaceInfo     { E_TIMEOUT };//执行云空间容量信息结果
	EResultCode eFileList      { E_TIMEOUT };//执行文件列表结果
	EResultCode eFileLock      { E_TIMEOUT };//执行文件空间锁定结果
	EResultCode eFileUnlock    { E_TIMEOUT };//执行文件空间解锁结果
	EResultCode eFileRemove    { E_TIMEOUT };//执行文件删除结果
	EResultCode eFileRename    { E_TIMEOUT };//执行文件重命名结果
	EResultCode eFileUpload    { E_TIMEOUT };//执行文件上传结果
	EResultCode eFileDownload  { E_TIMEOUT };//执行文件下载结果
	EResultCode eSliceStatus   { E_TIMEOUT };//执行获取切片状态结果
	EResultCode eSliceInfo     { E_TIMEOUT };//执行获取切片文件信息结果
	EResultCode eModelInfo     { E_TIMEOUT };//执行获取模型文件结果
};


} // GUI
} // Slic3r

#endif //!AC_CLOUD_MANGER_HPP
