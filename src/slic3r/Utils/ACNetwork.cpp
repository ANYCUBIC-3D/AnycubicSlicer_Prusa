#include "ACNetwork.hpp"
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
#define NETWORK_LIBRARY_NAME "anycubic_netd"
#else
#define NETWORK_LIBRARY_NAME "anycubic_net"
#endif

#if defined(_MSC_VER) || defined(_WIN32)
static HMODULE netwoking_module = NULL;
#else
static void* netwoking_module = NULL;
#endif


std::string getLibDir()
{
    std::string data_dir_str = Slic3r::data_dir();
    boost::filesystem::path data_dir_path(data_dir_str);
    auto plugin_folder = data_dir_path / "plugins";

    if (boost::filesystem::exists(plugin_folder) == false) {
        boost::filesystem::create_directory(plugin_folder);
    }

	return plugin_folder.string();
}

std::string getLibName()
{
    std::string libName;

#if defined(_MSC_VER) || defined(_WIN32)
    libName = std::string(NETWORK_LIBRARY_NAME) + ".dll";
#else
    #if defined(__WXMAC__)
    libName = std::string("lib") + std::string(NETWORK_LIBRARY_NAME) + ".dylib";
    #else
    libName = std::string("lib") + std::string(NETWORK_LIBRARY_NAME) + ".so";
    #endif
#endif

	return libName;
}

std::string getLibTempName()
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

std::string getLibPath()
{
    std::string libPath;
    std::string libDir = getLibDir();

    libPath = libDir + "/" + getLibName();

	return libPath;
}

std::string getLibTempPath()
{
    std::string libTempPath;
    std::string libTempDir = boost::filesystem::temp_directory_path().string();

    libTempPath = libTempDir + "/" + getLibTempName();

	return libTempPath;
}


bool libExist()
{
	return boost::filesystem::exists(getLibPath());
}

typedef std::function<void(int percent, bool& cancel)> InstallProgressFn;
typedef std::function<bool()>                       WasCancelledFn;

void downloadLibrary(std::function<void(bool)> onDownloaded, InstallProgressFn pro_fn, WasCancelledFn cancel_fn)
{
    std::string download_url     = GUI::wxGetApp().ac_net_lib_url();
    std::string tmp_path         = getLibTempPath();
    std::string target_file_path = getLibPath();

    Slic3r::Http http = Slic3r::Http::get(download_url);

    http
        .on_progress(
        [ &pro_fn, &cancel_fn](Slic3r::Http::Progress progress, bool& cancel) {
            int percent = 0;
            if (progress.dltotal != 0)
                percent = progress.dlnow * 90 / progress.dltotal;

            bool was_cancel = false;
            if (pro_fn) {
                pro_fn(percent, was_cancel);
                BOOST_LOG_TRIVIAL(info) << "[download_plugin 2] progress: " << percent;
            }

            cancel = was_cancel;
            if (cancel_fn)
                if (cancel_fn())
                    cancel = true;
        })
        .on_complete([&pro_fn, &onDownloaded, tmp_path, target_file_path](std::string body, unsigned status) {
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
        .on_error([&pro_fn, &onDownloaded](std::string body, std::string error, unsigned int status) {
            bool cancel = false;
            if (pro_fn) 
                pro_fn(0, cancel);

            onDownloaded(false);

            BOOST_LOG_TRIVIAL(error) << "[download_plugin 2] on_error: " << error<<", body = " << body;
        });

    http.perform_sync();

//	return ;
}


void* getNetworkFunction(const char* name)
{
    void* function = nullptr;

    if (!netwoking_module)
        return function;

#if defined(_MSC_VER) || defined(_WIN32)
    function = GetProcAddress(netwoking_module, name);
#else
    function = dlsym(netwoking_module, name);
#endif

    if (!function) {
        BOOST_LOG_TRIVIAL(warning) << __FUNCTION__ << boost::format(", can not find function %1%")%name;
    }
    return function;
}

ACNetwork::ACNetwork()
{
}

bool ACNetwork::loadLibrary()
{
	std::string libPath = getLibPath();

#if defined(_MSC_VER) || defined(_WIN32)
    wchar_t lib_wstr[2048];
    memset(lib_wstr, 0, sizeof(lib_wstr));
    ::MultiByteToWideChar(CP_UTF8, NULL, libPath.c_str(), strlen(libPath.c_str())+1, lib_wstr, sizeof(lib_wstr) / sizeof(lib_wstr[0]));
    netwoking_module = LoadLibrary(lib_wstr);
#else
    printf("loading network module at %s\n", libPath.c_str());
    netwoking_module = dlopen( libPath.c_str(), RTLD_LAZY);
    if (!netwoking_module) {
        char* dll_error = dlerror();
        printf("error, dlerror is %s\n", dll_error);
        BOOST_LOG_TRIVIAL(info) << __FUNCTION__ << boost::format(", error, dlerror is %1%")%dll_error;
    }
    printf("after dlopen, network_module is %p\n", netwoking_module);
#endif

    if (!netwoking_module) {
        BOOST_LOG_TRIVIAL(info) << __FUNCTION__ << boost::format(", can not Load Library for %1%")%libPath;
        return false;
    }
    BOOST_LOG_TRIVIAL(info) << __FUNCTION__ << boost::format(", successfully loaded library %1%, module %2%")%libPath %netwoking_module;

    ACNetwork::m_getVersionPtr  = reinterpret_cast<func_getVersion>(getNetworkFunction("getVersion"));
    ACNetwork::m_sendInfoPtr = reinterpret_cast<func_sendInfo  >(getNetworkFunction("sendInfo")); 

    return true;
}

std::string getUserID()
{
    std::vector<unsigned char> unique;

#ifdef _WIN32
    // On Windows, get the MAC address of a network adaptor (preferably Ethernet
    // or IEEE 802.11 wireless

    DWORD dwBufLen = sizeof(IP_ADAPTER_INFO);
    PIP_ADAPTER_INFO AdapterInfo = (PIP_ADAPTER_INFO)malloc(dwBufLen);

    if (GetAdaptersInfo(AdapterInfo, &dwBufLen) == ERROR_BUFFER_OVERFLOW) {
        free(AdapterInfo);
        AdapterInfo = (IP_ADAPTER_INFO*)malloc(dwBufLen);
    }
    if (GetAdaptersInfo(AdapterInfo, &dwBufLen) == NO_ERROR) {
        const IP_ADAPTER_INFO* pAdapterInfo = AdapterInfo;
        std::vector<std::vector<unsigned char>> macs;
        bool ethernet_seen = false;
        while (pAdapterInfo) {
            macs.emplace_back();
            for (unsigned char i = 0; i < pAdapterInfo->AddressLength; ++i)
                macs.back().emplace_back(pAdapterInfo->Address[i]);
            // Prefer Ethernet and IEEE 802.11 wireless
            if (! ethernet_seen) {
                if ((pAdapterInfo->Type == MIB_IF_TYPE_ETHERNET && (ethernet_seen = true))
                 ||  pAdapterInfo->Type == IF_TYPE_IEEE80211)
                    std::swap(macs.front(), macs.back());
            }
            pAdapterInfo = pAdapterInfo->Next;
        }
        if (! macs.empty())
            unique = macs.front();
    }
    free(AdapterInfo);
#elif __APPLE__
    constexpr int buf_size = 100;
    char buf[buf_size] = "";
    memset(&buf, 0, sizeof(buf));
    io_registry_entry_t ioRegistryRoot = IORegistryEntryFromPath(kIOMasterPortDefault, "IOService:/");
    if (ioRegistryRoot != MACH_PORT_NULL) {
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
    if (fp != NULL) {
        // Maybe the only way to silence -Wunused-result on gcc...
        // cline is simply not modified on failure, who cares.
        [[maybe_unused]]auto dummy = fgets(cline, max_len, fp);
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

std::string getOSInfo()
{
    std::string strInfo;
    strInfo = platform_to_string(platform()) 
        + "-" + platform_flavor_to_string(platform_flavor()) 
        + "-" + std::string(wxPlatformInfo::Get().GetOperatingSystemDescription().ToUTF8().data());

    return strInfo;
}


#ifdef _WIN32
static std::map<std::string, std::string> get_cpu_info_from_registry()
{
    std::map<std::string, std::string> out;

    int idx = -1;
    constexpr DWORD bufsize_ = 500;
    DWORD bufsize = bufsize_-1; // Ensure a terminating zero.
    char buf[bufsize_] = "";
    memset(buf, 0, bufsize_);
    const std::string reg_dir = "HARDWARE\\DESCRIPTION\\System\\CentralProcessor\\";
    std::string reg_path = reg_dir;

    // Look into that reg dir and possibly into subdirs called 0, 1, 2, etc.
    // If the latter, count them.

    while (true) {
        if (RegGetValueA(HKEY_LOCAL_MACHINE, reg_path.c_str(), "ProcessorNameString",
            RRF_RT_REG_SZ, NULL, &buf, &bufsize) == ERROR_SUCCESS) {
            out["Model"] = buf;
            out["Cores"] = std::to_string(std::max(1, idx + 1));
            if (RegGetValueA(HKEY_LOCAL_MACHINE, reg_path.c_str(),
                "VendorIdentifier", RRF_RT_REG_SZ, NULL, &buf, &bufsize) == ERROR_SUCCESS)
                out["Vendor"] = buf;
        }
        else {
            if (idx >= 0)
                break;
        }
        ++idx;
        reg_path = reg_dir + std::to_string(idx) + "\\";
        bufsize = bufsize_-1;
    }
    return out;
}
#else // Apple, Linux, BSD
static std::map<std::string, std::string> parse_lscpu_etc(const std::string& name, char delimiter)
{
    std::map<std::string, std::string> out;
    constexpr size_t max_len = 1000;
    char cline[max_len] = "";
    FILE* fp = popen(name.data(), "r");
    if (fp != NULL) {
        while (fgets(cline, max_len, fp) != NULL) {
            std::string line(cline);
            line.erase(std::remove_if(line.begin(), line.end(),
                [](char c) { return c == '\"' || c == '\r' || c == '\n'; }),
                line.end());
            size_t pos = line.find(delimiter);
            if (pos < line.size() - 1) {
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

std::string getCpuInfo()
{
    std::string strInfo;
#ifdef _WIN32
    std::map<std::string, std::string> cpu_info = get_cpu_info_from_registry();
    strInfo = "Cores:"    + cpu_info["Cores"] 
        + "-" + "Model:"  + cpu_info["Model" ] 
        + "-" + "Vendor:" + cpu_info["Vendor"];
#elif __APPLE__
    std::map<std::string, std::string> cpu_info = parse_lscpu_etc("sysctl -a", ':');
    strInfo = "Cores:"    + cpu_info["hw.ncpu"] 
        + "-" + "Model:"  + cpu_info["machdep.cpu.brand_string"] 
        + "-" + "Vendor:" + cpu_info["machdep.cpu.vendor"];
#else // linux/BSD
    std::map<std::string, std::string> cpu_info = parse_lscpu_etc("cat /proc/cpuinfo", ':');
    if (auto ncpu_it = cpu_info.find("processor"); ncpu_it != cpu_info.end()) {
        std::string& ncpu = ncpu_it->second;
        if (int num=0; std::from_chars(ncpu.data(), ncpu.data() + ncpu.size(), num).ec != std::errc::invalid_argument)
            ncpu = std::to_string(num + 1);
    }
    strInfo = "Cores:"    + cpu_info["processor"] 
        + "-" + "Model:"  + cpu_info["model name"] 
        + "-" + "Vendor:" + cpu_info["vendor_id"];
#endif

    return strInfo;
}

std::string getGraphicsInfo()
{
    std::string strInfo;
    strInfo = "Renderer:" + Slic3r::GUI::OpenGLManager::get_gl_info().get_renderer();
;
    return strInfo;
}

std::string getRamInfo()
{
    std::string strInfo;
    size_t num = std::round(Slic3r::total_physical_memory()/107374100.);
    strInfo = "RAM_GiB:" + std::to_string(num / 10) + "." + std::to_string(num % 10);
    return strInfo;
}

AnycubicNetPCInfo getPCInfo()
{
    AnycubicNetPCInfo pcInfo;

    pcInfo.os         = getOSInfo(); // : win10,
    pcInfo.cpu        = getCpuInfo(); // : amd,
    pcInfo.graphics   = getGraphicsInfo(); // : nvidia,
    pcInfo.ram        = getRamInfo(); // : 1g

    return pcInfo;
}

void ACNetwork::callSendAnonymous(std::function<void(bool)> onFinished)
{
    if (ACNetwork::m_sendInfoPtr) {
        std::string appVersion     = SLIC3R_VERSION;
        std::string userID         = getUserID();
        AnycubicNetPCInfo pcInfo = getPCInfo();

        ACNetwork::m_sendInfoPtr(appVersion, userID, pcInfo, onFinished, GUI::wxGetApp().is_region_CN());
    }
}

void ACNetwork::startSendAnonymous(std::function<void(bool)> onFinished)
{
	if (libExist()) {
		if (m_libLoaded == false) {
			m_libLoaded = loadLibrary();
		}
		if (m_libLoaded) {
			callSendAnonymous(onFinished);
		} else {
			onFinished(false);
		}
	} else {
		auto onDownloaded = [this, onFinished](bool succeed){
			if (succeed) {
				startSendAnonymous(onFinished);
			} else {
				onFinished(false);
			}
		};

        auto onProgress = [](int percent, bool& cancel){
            std::cout << percent << std::endl;
        };

        auto onCancel = [](){
            return false;
        };

		downloadLibrary(onDownloaded, onProgress, onCancel);
	}
}


}
