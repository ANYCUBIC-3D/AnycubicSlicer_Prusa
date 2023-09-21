#ifndef AC_CLOUD_WORK_HPP
#define AC_CLOUD_WORK_HPP
/*---------------------------------------------------------------------------------
								云相关SDK，动态库加载方式
								拷贝自ACNetWork，适当做修改
-----------------------------------------------------------------------------------*/

#include "ACNetwork.hpp"
#include <functional>

struct ICloudClient;
struct EventHandler;
class ITaskPool;

namespace Slic3r {

#define CloudInstance ACCloudWork::GetInstance()

typedef std::function<void(int percent, bool& cancel)> InstallProgressFn;
typedef std::function<bool()> WasCancelledFn;

typedef void (*log_print)(int level, const char* msg);



typedef int           (*func_setupApi)             (log_print, const char*);
typedef void          (*func_shutdownApi)          (void);
typedef ITaskPool*    (*func_newPool)              (int);
typedef ICloudClient* (*func_newCloudClient)       (EventHandler*, ITaskPool*);
typedef bool          (*func_isTestServer)         (void);
typedef void          (*func_enalbeTestServer)     (bool);
typedef void          (*func_setDomain)            (const char*, const char*);
typedef void          (*func_setPrefix)            (const char*);
typedef const char*   (*func_QRCodeUrl)            (bool, bool);
typedef int           (*func_currentCountry)       (void);
typedef int           (*func_setCurrentCountry)    (int);
typedef void          (*func_freeMemory)           (void*);
typedef const char*   (*func_imageScale)           (const char*, int, int);

class ACCloudWork
{
public:
	static ACCloudWork* GetInstance();
	~ACCloudWork();

	ACCloudWork(ACCloudWork&&) = delete;
	ACCloudWork(const ACCloudWork&) = delete;
	ACCloudWork& operator=(ACCloudWork&&) = delete;
	ACCloudWork& operator=(const ACCloudWork&) = delete;

	void checkCloudSDKAndDownload(std::function<void(bool)> onFinished);

	void* getFunction(const char* name);

	bool getAllFuncAddress();//获取API地址

	int getHardConcurrency();

	//回设指针，析构时候释放资源
	void setBackCloudClientPtr(ICloudClient* ptr) { m_cloudClient = ptr; }
	void setBackTaskPoolPtr(ITaskPool* ptr) { m_taskPool = ptr; }

	bool isImageExist(const std::string& target_file_path);
	void downloadImage(std::function<void(bool)> onDownloaded, InstallProgressFn pro_fn, WasCancelledFn cancel_fn, const char* url);
protected:
	ACCloudWork();
private:
	bool loadLibrary();
	void downloadLibrary  (std::function<void(bool)> onDownloaded, InstallProgressFn pro_fn, WasCancelledFn cancel_fn);


	std::string getLibDir();
	std::string getLibName();
	std::string getLibTempName();
	std::string getLibPath();
	std::string getLibTempPath();

	bool libExist();
public:
	std::string getPrinterImageTempDir();
	std::string getPrinterImageWorkDir();
	std::string getPrinterImageName(const std::string& url);
	std::string getPrinterImageWorkFullPath(const std::string& url);//包含文件名的全路径
private:
	bool            m_libLoaded         { false };
	func_getVersion m_getVersionPtr     { nullptr };
	func_sendInfo   m_sendInfoPtr       { nullptr };
	ICloudClient*   m_cloudClient       { nullptr };
	ITaskPool*      m_taskPool          { nullptr };
public:
	func_setupApi              m_setupApiPtr           { nullptr };
	func_shutdownApi           m_shutdownApiPtr        { nullptr };
	func_newPool               m_newPoolPtr            { nullptr };
	func_newCloudClient        m_newCloudClientPtr     { nullptr };
	func_isTestServer          m_isTestServerPtr       { nullptr };
	func_enalbeTestServer      m_enableTestServerPtr   { nullptr };
	func_setDomain             m_setDomainPtr          { nullptr };
	func_setPrefix             m_setPrefixPtr          { nullptr };
	func_QRCodeUrl             m_QRCodeUrlPtr          { nullptr };
	func_currentCountry        m_currentCountryPtr     { nullptr };
	func_setCurrentCountry     m_setCurrentCountryPtr  { nullptr };
	func_freeMemory            m_freeMemoryPtr         { nullptr };
	func_imageScale            m_imageScalePtr         { nullptr };
};

}

#endif //!AC_CLOUD_WORK_HPP