#ifndef slic3r_AppUpdate_hpp_
#define slic3r_AppUpdate_hpp_

#include <boost/filesystem.hpp>
#include <boost/optional.hpp>
#include "libslic3r/Utils.hpp"
#include "wx/event.h"

//class boost::filesystem::path;

namespace Slic3r {

#ifdef __APPLE__
// implmented at MacUtils.mm
std::string get_downloads_path_mac();
#endif //__APPLE__

struct DownloadAppData
{
	std::string				url;
	bool					start_after;
	boost::optional<Semver> version;
	size_t				    size;
	boost::filesystem::path target_path;
};

class AppUpdater
{
public:
	AppUpdater();
	~AppUpdater();
	AppUpdater(AppUpdater&&) = delete;
	AppUpdater(const AppUpdater&) = delete;
	AppUpdater& operator=(AppUpdater&&) = delete;
	AppUpdater& operator=(const AppUpdater&) = delete;

	// downloads app file
	void sync_download();
    void sync_download_ac();
	// downloads version file
	void sync_version(const std::string& version_check_url, bool from_user);
	void sync_version(const std::string &version_check_url, bool from_user,const std::string &iniFileName);
    void sync_version(const std::string &version_check_url, bool from_user, const std::string &envIniFileName,const std::string &iniFileName);
	void cancel();
	bool cancel_callback();

	std::string get_default_dest_folder();
    boost::filesystem::path get_default_dest_folder_path();

	static std::string get_filename_from_url(const std::string& url);
	static std::string get_file_extension_from_url(const std::string& url);

	// atomic bool
	bool				get_triggered_by_user() const;
	bool				get_download_ongoing() const;
	// mutex access
	void				set_app_data(DownloadAppData data);
	DownloadAppData		get_app_data();
    bool                run_downloaded_file(boost::filesystem::path &path);
    bool                checkNetWork(std::string &check_url, int limitSize = 1024 * 30);

private:
	struct priv;
	std::unique_ptr<priv> p;
};

wxDECLARE_EVENT(EVT_ACSLICER_VERSION_ONLINE, wxCommandEvent);
wxDECLARE_EVENT(EVT_ACSLICER_EXPERIMENTAL_VERSION_ONLINE, wxCommandEvent);
wxDECLARE_EVENT(EVT_ACSLICER_APP_DOWNLOAD_PROGRESS, wxCommandEvent);
wxDECLARE_EVENT(EVT_ACSLICER_APP_DOWNLOAD_FAILED, wxCommandEvent);
wxDECLARE_EVENT(EVT_ACSLICER_APP_OPEN_FAILED, wxCommandEvent);



wxDECLARE_EVENT(EVT_ACSLICER_APP_UPDATEDIALOG_SHOW, wxCommandEvent);
wxDECLARE_EVENT(EVT_ACSLICER_APP_DOWNLOAD_RUN, wxCommandEvent);
wxDECLARE_EVENT(EVT_ACSLICER_APP_UPDATEDIALOG_GIF, wxCommandEvent);
wxDECLARE_EVENT(EVT_ACSLICER_APP_UPDATEDIALOG_TEXT4_COLOR, wxCommandEvent);
wxDECLARE_EVENT(EVT_ACSLICER_APP_UPDATEDIALOG_TEXT4_VALUE, wxCommandEvent);
wxDECLARE_EVENT(EVT_ACSLICER_APP_DOWNLOAD_OPEN, wxCommandEvent);
wxDECLARE_EVENT(EVT_ACSLICER_APP_DOWNLOAD_CANCEL_CLOSE, wxCommandEvent);
wxDECLARE_EVENT(EVT_ACSLICER_APP_DOWNLOAD_CANCEL, wxCommandEvent);
wxDECLARE_EVENT(EVT_ACSLICER_APP_DOWNLOAD_FINISH, wxCommandEvent);
wxDECLARE_EVENT(EVT_ACSLICER_DOWNLOAD_ONLINE, wxCommandEvent);
wxDECLARE_EVENT(EVT_ACSLICER_INI_DOWNLOAD_PROGRESS, wxCommandEvent);
wxDECLARE_EVENT(EVT_ACSLICER_INI_DOWNLOAD_FAILED, wxCommandEvent);
wxDECLARE_EVENT(EVT_ACSLICER_INI_OPEN_FAILED, wxCommandEvent);

} //namespace Slic3r 
#endif
