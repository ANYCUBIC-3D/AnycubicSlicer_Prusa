#include "ACCloudWork.hpp"
#include "global.hxx"
#include "libslic3r/Utils.hpp"

#if __APPLE__
    #import <IOKit/IOKitLib.h>
    #include <dlfcn.h>
#endif

#include "libslic3r/AppConfig.hpp"
#include "libslic3r/BlacklistedLibraryCheck.hpp"
#include "libslic3r/Platform.hpp"
#include "libslic3r/Utils.hpp"

#include "slic3r/GUI/format.hpp"
#include "slic3r/Utils/Http.hpp"
#include "slic3r/Utils/PresetUpdater.hpp"

#include "../GUI/GUI_App.hpp"
#include "../GUI/GUI_Utils.hpp"
#include "../GUI/I18N.hpp"
#include "../GUI/OpenGLManager.hpp"

#include "Http.hpp"

#include <boost/log/trivial.hpp>
#include <boost/algorithm/hex.hpp>
#include <boost/algorithm/string/split.hpp>
#include <boost/algorithm/string/trim_all.hpp>
#include <boost/log/trivial.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <boost/uuid/detail/md5.hpp>
#include <boost/filesystem/fstream.hpp>
#include <boost/filesystem/path.hpp>
#include <boost/filesystem.hpp>

#include <fstream>

#include "GL/glew.h"

#include <wx/display.h>
#include <wx/htmllbox.h>
#include <wx/stattext.h>
#include <wx/timer.h>
#include <wx/utils.h>

#include <atomic>
#include <thread>

#ifdef _WIN32
    #include <windows.h>
    #include <netlistmgr.h>

    #include <Iphlpapi.h>
    #pragma comment(lib, "iphlpapi.lib")
#elif __APPLE__
    #include <CoreFoundation/CoreFoundation.h>
#else // Linux/BSD
    #include <charconv>
#endif

namespace Slic3r {

using namespace std;
namespace fs = boost::filesystem;

#if defined(_DEBUG)
#define CLOUDWORK_LIBRARY_NAME "cloud_sdk_cpp"
#else
#define CLOUDWORK_LIBRARY_NAME "cloud_sdk_cpp"
#endif

#if defined(_MSC_VER) || defined(_WIN32)
static HMODULE cloudworking_module = NULL;
#else
static void* cloudworking_module = NULL;
#endif


ACCloudWork* ACCloudWork::GetInstance()
{
	static ACCloudWork obj;//线程安全，也避免加锁
	return &obj;
}
ACCloudWork::~ACCloudWork()
{
	if (m_taskPool && m_freeMemoryPtr)   //析构
	{
		m_freeMemoryPtr(m_taskPool);
		m_taskPool = nullptr;
	}
	if (m_cloudClient && m_freeMemoryPtr)//析构
	{
		m_freeMemoryPtr(m_cloudClient);
		m_cloudClient = nullptr;
	}

#if defined(_MSC_VER) || defined(_WIN32)
	FreeLibrary(cloudworking_module);//释放DLL
#endif

	m_newPoolPtr = nullptr;
	m_newCloudClientPtr = nullptr;
	m_getVersionPtr = nullptr;
}
std::string ACCloudWork::getLibDir()
{
	std::string data_dir_str = Slic3r::data_dir();
	boost::filesystem::path data_dir_path(data_dir_str);
	auto plugin_folder = data_dir_path / "plugins";

	if (boost::filesystem::exists(plugin_folder) == false)
	{
		boost::filesystem::create_directory(plugin_folder);
	}

	return plugin_folder.string();
}

std::string ACCloudWork::getPrinterImageTempDir()
{
	std::string data_dir_str = boost::filesystem::temp_directory_path().string();
	boost::filesystem::path data_dir_path(data_dir_str);
	auto printer_folder = data_dir_path / "AnycubicSlicer";

	if (boost::filesystem::exists(printer_folder) == false)
		boost::filesystem::create_directory(printer_folder);

	return printer_folder.string();
}

std::string ACCloudWork::getPrinterImageWorkDir()
{
	std::string data_dir_str = Slic3r::data_dir();
	boost::filesystem::path data_dir_path(data_dir_str);
	auto printer_folder = data_dir_path / "cloud";

	if (boost::filesystem::exists(printer_folder) == false)
		boost::filesystem::create_directory(printer_folder);

	return printer_folder.string();
}

std::string ACCloudWork::getLibName()
{
	std::string libName;

#if defined(_MSC_VER) || defined(_WIN32)
	libName = std::string(CLOUDWORK_LIBRARY_NAME) + ".dll";
#else
#if defined(__WXMAC__)
	libName = std::string("lib") + std::string(CLOUDWORK_LIBRARY_NAME) + ".dylib";
#else
	libName = std::string("lib") + std::string(CLOUDWORK_LIBRARY_NAME) + ".so";
#endif
#endif

	return libName;
}

std::string ACCloudWork::getLibTempName()
{
	std::string libName = boost::filesystem::unique_path().string();

#if defined(_MSC_VER) || defined(_WIN32)
	libName += ".dll";
#else
#if defined(__WXMAC__)
	libName += ".dylib";
#else
	libName += ".so";
#endif
#endif

	return libName;
}

std::string ACCloudWork::getLibPath()
{
	std::string libPath;
	std::string libDir = getLibDir();

#if defined(__WXMSW__)
	libPath = libDir + "\\" + getLibName();
#else
	libPath = libDir + "/" + getLibName();
#endif
	

	return libPath;
}

std::string ACCloudWork::getLibTempPath()
{
	std::string libTempPath;

	/*
	Windows: 通常是 C:\Users\<Username>\AppData\Local\Temp
	Linux: 通常是 /tmp
	macOS: 通常是 /tmp
	*/
	std::string libTempDir = boost::filesystem::temp_directory_path().string();

	libTempPath = libTempDir + "/" + getLibTempName();

	return libTempPath;
}

std::string ACCloudWork::getPrinterImageName(const std::string& url_)
{
	std::string rv;
	std::string url = url_;

	// 找到最后一个斜杠的位置
	size_t lastSlashPos = url.rfind('/');

	if (lastSlashPos != std::string::npos && lastSlashPos < url.length() - 1)
	{
		// 提取子字符串
		rv = url.substr(lastSlashPos + 1);
#if defined(__WXMSW__)
		rv = "\\" + rv;
#else
		rv = "/" + rv;
#endif
	}
	else
		rv = "";

	return rv;
}

std::string ACCloudWork::getPrinterImageWorkFullPath(const std::string& url)
{
	std::string name = getPrinterImageName(url);
	std::string target_file_path = getPrinterImageWorkDir() + name;//工作目录位置
	return target_file_path;
}

bool ACCloudWork::libExist()
{
	return boost::filesystem::exists(getLibPath());
}
bool ACCloudWork::isImageExist(const std::string& target_file_path)
{
	return boost::filesystem::exists(target_file_path);
}

int ACCloudWork::getHardConcurrency()
{
	return std::thread::hardware_concurrency();
}


void ACCloudWork::downloadLibrary(std::function<void(bool)> onDownloaded, InstallProgressFn pro_fn, WasCancelledFn cancel_fn)
{
	std::string download_url = GUI::wxGetApp().ac_cloud_lib_url();
	std::string tmp_path = getLibTempPath();
	std::string target_file_path = getLibPath();

	Slic3r::Http http = Slic3r::Http::get(download_url);

	http.on_progress(
			[&pro_fn, &cancel_fn](Slic3r::Http::Progress progress, bool& cancel)
			{
				int percent = 0;
				if (progress.dltotal != 0)
					percent = progress.dlnow * 90 / progress.dltotal;

				bool was_cancel = false;
				if (pro_fn)
				{
					pro_fn(percent, was_cancel);
					BOOST_LOG_TRIVIAL(info) << "[download_plugin 2] progress: " << percent;
				}

				cancel = was_cancel;
				if (cancel_fn)
					if (cancel_fn())
						cancel = true;
			})
		.on_complete([this, &pro_fn, &onDownloaded, tmp_path, target_file_path](std::string body, unsigned status)
			{
				BOOST_LOG_TRIVIAL(info) << "[download_plugin 2] completed";
				bool cancel = false;
				int percent = 0;
				int bodySize = body.size();
				fs::fstream file(tmp_path, std::ios::out | std::ios::binary | std::ios::trunc);
				file.write(body.c_str(), bodySize);
				file.close();
				fs::rename(tmp_path, target_file_path);
				if (pro_fn)
					pro_fn(100, cancel);

				onDownloaded(libExist());
			})
		.on_error([&pro_fn, &onDownloaded](std::string body, std::string error, unsigned int status)
			{
				bool cancel = false;
				if (pro_fn)
					pro_fn(0, cancel);

				onDownloaded(false);

				BOOST_LOG_TRIVIAL(error) << "[download_plugin 2] on_error: " << error << ", body = " << body;
			});

			http.perform_sync();

			//return;
}

void ACCloudWork::downloadImage(std::function<void(bool)> onDownloaded, InstallProgressFn pro_fn, WasCancelledFn cancel_fn, const char* url)
{
	std::string download_url(url);
	std::string name             = getPrinterImageName(download_url);
	std::string file_full_path   = getPrinterImageTempDir() + name;//临时下载目录
	std::string target_file_path = getPrinterImageWorkDir() + name;//工作目录位置

	Slic3r::Http http = Slic3r::Http::get(download_url);

	http.on_progress(
		[&pro_fn, &cancel_fn](Slic3r::Http::Progress progress, bool& cancel)
		{
			int percent = 0;
			if (progress.dltotal != 0)
				percent = progress.dlnow * 90 / progress.dltotal;

			bool was_cancel = false;
			if (pro_fn)
			{
				pro_fn(percent, was_cancel);
				BOOST_LOG_TRIVIAL(info) << "[download_plugin 2] progress: " << percent;
			}

			cancel = was_cancel;
			if (cancel_fn)
				if (cancel_fn())
					cancel = true;
		})
		.on_complete([this, &pro_fn, &onDownloaded, file_full_path, target_file_path](std::string body, unsigned status)
			{
				BOOST_LOG_TRIVIAL(info) << "[download_plugin 2] completed";
				bool cancel = false;
				int percent = 0;
				int bodySize = body.size();
				fs::fstream file(file_full_path, std::ios::out | std::ios::binary | std::ios::trunc);
				file.write(body.c_str(), bodySize);
				file.close();
				fs::rename(file_full_path, target_file_path);
				if (pro_fn)
					pro_fn(100, cancel);

				onDownloaded(libExist());
			})
			.on_error([&pro_fn, &onDownloaded](std::string body, std::string error, unsigned int status)
				{
					bool cancel = false;
					if (pro_fn)
						pro_fn(0, cancel);

					onDownloaded(false);

					BOOST_LOG_TRIVIAL(error) << "[download_plugin 2] on_error: " << error << ", body = " << body;
				});

			http.perform_sync();
}

void* ACCloudWork::getFunction(const char* name)
{
	void* function = nullptr;

	if (!cloudworking_module)
		return function;

#if defined(_MSC_VER) || defined(_WIN32)
	function = GetProcAddress(cloudworking_module, name);
#else
	function = dlsym(cloudworking_module, name);
#endif

	if (!function)
	{
		BOOST_LOG_TRIVIAL(warning) << __FUNCTION__ << boost::format(", can not find function %1%") % name;
	}
	return function;
}

bool ACCloudWork::getAllFuncAddress()
{
	m_setupApiPtr           = reinterpret_cast<func_setupApi>           (getFunction("setupApi"));
	m_shutdownApiPtr        = reinterpret_cast<func_shutdownApi>        (getFunction("shutdownApi"));
	m_newPoolPtr            = reinterpret_cast<func_newPool>            (getFunction("newPool"));
	m_newCloudClientPtr     = reinterpret_cast<func_newCloudClient>     (getFunction("newCloudClient"));
	m_isTestServerPtr       = reinterpret_cast<func_isTestServer>       (getFunction("isTestServer"));
	m_enableTestServerPtr   = reinterpret_cast<func_enalbeTestServer>   (getFunction("enalbeTestServer"));
	m_setDomainPtr          = reinterpret_cast<func_setDomain>          (getFunction("setDomain"));

	m_setPrefixPtr         = reinterpret_cast<func_setPrefix>           (getFunction("setPrefix"));
	m_QRCodeUrlPtr         = reinterpret_cast<func_QRCodeUrl>           (getFunction("QRCodeUrl"));
	m_currentCountryPtr    = reinterpret_cast<func_currentCountry>      (getFunction("currentCountry"));
	m_setCurrentCountryPtr = reinterpret_cast<func_setCurrentCountry>   (getFunction("setCurrentCountry"));
	m_freeMemoryPtr        = reinterpret_cast<func_freeMemory>          (getFunction("freeMemory"));
	m_imageScalePtr        = reinterpret_cast<func_imageScale>          (getFunction("imageScale"));
	return true;
}

ACCloudWork::ACCloudWork()
{

}

bool ACCloudWork::loadLibrary()
{
	std::string libPath = getLibPath();

#if defined(_MSC_VER) || defined(_WIN32)
    wchar_t lib_wstr[2048];
    memset(lib_wstr, 0, sizeof(lib_wstr));
    ::MultiByteToWideChar(CP_UTF8, NULL, libPath.c_str(), strlen(libPath.c_str())+1, lib_wstr, sizeof(lib_wstr) / sizeof(lib_wstr[0]));
	//SetErrorMode(0);//错误会弹出警告
	//cloudworking_module = LoadLibrary(lib_wstr);
	cloudworking_module = ::LoadLibraryExA(libPath.c_str(), nullptr, /*LOAD_LIBRARY_SEARCH_APPLICATION_DIR*/LOAD_LIBRARY_SEARCH_DLL_LOAD_DIR);
#else
    printf("loading network module at %s\n", libPath.c_str());
    cloudworking_module = dlopen( libPath.c_str(), RTLD_LAZY);
    if (!cloudworking_module) {
        char* dll_error = dlerror();
        printf("error, dlerror is %s\n", dll_error);
        BOOST_LOG_TRIVIAL(info) << __FUNCTION__ << boost::format(", error, dlerror is %1%")%dll_error;
    }
    printf("after dlopen, network_module is %p\n", cloudworking_module);
#endif

    if (!cloudworking_module) {
        BOOST_LOG_TRIVIAL(info) << __FUNCTION__ << boost::format(", can not Load Library for %1%")%libPath;
        return false;
    }
    BOOST_LOG_TRIVIAL(info) << __FUNCTION__ << boost::format(", successfully loaded library %1%, module %2%")%libPath %cloudworking_module;


	//映射所有API地址
	getAllFuncAddress();

	//原有的，后续待剔除
	ACCloudWork::m_getVersionPtr = reinterpret_cast<func_getVersion>(getFunction("getVersion"));
	ACCloudWork::m_sendInfoPtr   = reinterpret_cast<func_sendInfo>  (getFunction("sendInfo"));

    return true;
}


std::string GetUserID()
{
	std::vector<unsigned char> unique;

#ifdef _WIN32
	// On Windows, get the MAC address of a network adaptor (preferably Ethernet
	// or IEEE 802.11 wireless

	DWORD dwBufLen = sizeof(IP_ADAPTER_INFO);
	PIP_ADAPTER_INFO AdapterInfo = (PIP_ADAPTER_INFO)malloc(dwBufLen);

	if (GetAdaptersInfo(AdapterInfo, &dwBufLen) == ERROR_BUFFER_OVERFLOW)
	{
		free(AdapterInfo);
		AdapterInfo = (IP_ADAPTER_INFO*)malloc(dwBufLen);
	}
	if (GetAdaptersInfo(AdapterInfo, &dwBufLen) == NO_ERROR)
	{
		const IP_ADAPTER_INFO* pAdapterInfo = AdapterInfo;
		std::vector<std::vector<unsigned char>> macs;
		bool ethernet_seen = false;
		while (pAdapterInfo)
		{
			macs.emplace_back();
			for (unsigned char i = 0; i < pAdapterInfo->AddressLength; ++i)
				macs.back().emplace_back(pAdapterInfo->Address[i]);
			// Prefer Ethernet and IEEE 802.11 wireless
			if (!ethernet_seen)
			{
				if ((pAdapterInfo->Type == MIB_IF_TYPE_ETHERNET && (ethernet_seen = true))
					|| pAdapterInfo->Type == IF_TYPE_IEEE80211)
					std::swap(macs.front(), macs.back());
			}
			pAdapterInfo = pAdapterInfo->Next;
		}
		if (!macs.empty())
			unique = macs.front();
	}
	free(AdapterInfo);
#elif __APPLE__
	constexpr int buf_size = 100;
	char buf[buf_size] = "";
	memset(&buf, 0, sizeof(buf));
	io_registry_entry_t ioRegistryRoot = IORegistryEntryFromPath(kIOMasterPortDefault, "IOService:/");
	if (ioRegistryRoot != MACH_PORT_NULL)
	{
		CFStringRef uuidCf = (CFStringRef)IORegistryEntryCreateCFProperty(ioRegistryRoot, CFSTR(kIOPlatformUUIDKey), kCFAllocatorDefault, 0);
		IOObjectRelease(ioRegistryRoot);
		CFStringGetCString(uuidCf, buf, buf_size, kCFStringEncodingMacRoman);
		CFRelease(uuidCf);
	}
	// Now convert the string to std::vector<unsigned char>.
	for (char* c = buf; *c != 0; ++c)
		unique.emplace_back((unsigned char)(*c));
#else // Linux/BSD
	constexpr size_t max_len = 100;
	char cline[max_len] = "";
	FILE* fp = popen("cat /etc/machine-id", "r");
	if (fp != NULL)
	{
		// Maybe the only way to silence -Wunused-result on gcc...
		// cline is simply not modified on failure, who cares.
		[[maybe_unused]] auto dummy = fgets(cline, max_len, fp);
		pclose(fp);
	}
	// Now convert the string to std::vector<unsigned char>.
	for (char* c = cline; *c != 0; ++c)
		unique.emplace_back((unsigned char)(*c));
#endif

	// In case that we did not manage to get the unique info, just return an empty
	// string, so it is easily detectable and not masked by the hashing.
	if (unique.empty())
		return "";

	// We should have a unique vector<unsigned char>. Append a long prime to be
	// absolutely safe against unhashing.
	uint64_t prime = 1282147483647;
	size_t beg = unique.size();
	unique.resize(beg + 8);
	memcpy(&unique[beg], &prime, 8);

	// Compute an MD5 hash and convert to std::string.
	using boost::uuids::detail::md5;
	md5 hash;
	md5::digest_type digest;
	hash.process_bytes(unique.data(), unique.size());
	hash.get_digest(digest);
	const unsigned char* charDigest = reinterpret_cast<const unsigned char*>(&digest);

	std::string userID;
	boost::algorithm::hex(charDigest, charDigest + sizeof(md5::digest_type), std::back_inserter(userID));
	return userID;
}

std::string GetOSInfo()
{
	std::string strInfo;
	strInfo = platform_to_string(platform())
		+ "-" + platform_flavor_to_string(platform_flavor())
		+ "-" + std::string(wxPlatformInfo::Get().GetOperatingSystemDescription().ToUTF8().data());

	return strInfo;
}


#ifdef _WIN32
static std::map<std::string, std::string> Get_cpu_info_from_registry()
{
	std::map<std::string, std::string> out;

	int idx = -1;
	constexpr DWORD bufsize_ = 500;
	DWORD bufsize = bufsize_ - 1; // Ensure a terminating zero.
	char buf[bufsize_] = "";
	memset(buf, 0, bufsize_);
	const std::string reg_dir = "HARDWARE\\DESCRIPTION\\System\\CentralProcessor\\";
	std::string reg_path = reg_dir;

	// Look into that reg dir and possibly into subdirs called 0, 1, 2, etc.
	// If the latter, count them.

	while (true)
	{
		if (RegGetValueA(HKEY_LOCAL_MACHINE, reg_path.c_str(), "ProcessorNameString",
			RRF_RT_REG_SZ, NULL, &buf, &bufsize) == ERROR_SUCCESS)
		{
			out["Model"] = buf;
			out["Cores"] = std::to_string(std::max(1, idx + 1));
			if (RegGetValueA(HKEY_LOCAL_MACHINE, reg_path.c_str(),
				"VendorIdentifier", RRF_RT_REG_SZ, NULL, &buf, &bufsize) == ERROR_SUCCESS)
				out["Vendor"] = buf;
		}
		else
		{
			if (idx >= 0)
				break;
		}
		++idx;
		reg_path = reg_dir + std::to_string(idx) + "\\";
		bufsize = bufsize_ - 1;
	}
	return out;
}
#else // Apple, Linux, BSD
static std::map<std::string, std::string> Parse_lscpu_etc(const std::string& name, char delimiter)
{
	std::map<std::string, std::string> out;
	constexpr size_t max_len = 1000;
	char cline[max_len] = "";
	FILE* fp = popen(name.data(), "r");
	if (fp != NULL)
	{
		while (fgets(cline, max_len, fp) != NULL)
		{
			std::string line(cline);
			line.erase(std::remove_if(line.begin(), line.end(),
				[](char c) { return c == '\"' || c == '\r' || c == '\n'; }),
				line.end());
			size_t pos = line.find(delimiter);
			if (pos < line.size() - 1)
			{
				std::string key = line.substr(0, pos);
				std::string value = line.substr(pos + 1);
				boost::trim_all(key); // remove leading and trailing spaces
				boost::trim_all(value);
				out[key] = value;
			}
		}
		pclose(fp);
	}
	return out;
}
#endif

std::string GetCpuInfo()
{
	std::string strInfo;
#ifdef _WIN32
	std::map<std::string, std::string> cpu_info = Get_cpu_info_from_registry();
	strInfo = "Cores:" + cpu_info["Cores"]
		+ "-" + "Model:" + cpu_info["Model"]
		+ "-" + "Vendor:" + cpu_info["Vendor"];
#elif __APPLE__
	std::map<std::string, std::string> cpu_info = Parse_lscpu_etc("sysctl -a", ':');
	strInfo = "Cores:" + cpu_info["hw.ncpu"]
		+ "-" + "Model:" + cpu_info["machdep.cpu.brand_string"]
		+ "-" + "Vendor:" + cpu_info["machdep.cpu.vendor"];
#else // linux/BSD
	std::map<std::string, std::string> cpu_info = Parse_lscpu_etc("cat /proc/cpuinfo", ':');
	if (auto ncpu_it = cpu_info.find("processor"); ncpu_it != cpu_info.end())
	{
		std::string& ncpu = ncpu_it->second;
		if (int num = 0; std::from_chars(ncpu.data(), ncpu.data() + ncpu.size(), num).ec != std::errc::invalid_argument)
			ncpu = std::to_string(num + 1);
	}
	strInfo = "Cores:" + cpu_info["processor"]
		+ "-" + "Model:" + cpu_info["model name"]
		+ "-" + "Vendor:" + cpu_info["vendor_id"];
#endif

	return strInfo;
}

std::string GetGraphicsInfo()
{
	std::string strInfo;
	strInfo = "Renderer:" + Slic3r::GUI::OpenGLManager::get_gl_info().get_renderer();
	return strInfo;
}

std::string GetRamInfo()
{
	std::string strInfo;
	size_t num = std::round(Slic3r::total_physical_memory() / 107374100.);
	strInfo = "RAM_GiB:" + std::to_string(num / 10) + "." + std::to_string(num % 10);
	return strInfo;
}

AnycubicNetPCInfo GetPCInfo()
{
	AnycubicNetPCInfo pcInfo;

	pcInfo.os = GetOSInfo(); // : win10,
	pcInfo.cpu = GetCpuInfo(); // : amd,
	pcInfo.graphics = GetGraphicsInfo(); // : nvidia,
	pcInfo.ram = GetRamInfo(); // : 1g

	return pcInfo;
}


void ACCloudWork::checkCloudSDKAndDownload(std::function<void(bool)> onFinished)
{
	if (libExist())
	{
		if (!m_libLoaded)
		{
			m_libLoaded = loadLibrary();
			onFinished(false);
		}
	}
	else
	{
		auto onDownloaded = [this, onFinished](bool succeed)
		{
			if (succeed)
				checkCloudSDKAndDownload(onFinished);
			else
				onFinished(false);
		};

		auto onProgress = [](int percent, bool& cancel)
		{
			std::cout << percent << std::endl;
		};

		auto onCancel = []()
		{
			return false;
		};

		downloadLibrary(onDownloaded, onProgress, onCancel);
	}
}

}
