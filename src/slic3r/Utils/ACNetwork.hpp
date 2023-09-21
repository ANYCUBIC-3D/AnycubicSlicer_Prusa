#ifndef slic3r_ACNETWORK_hpp_
#define slic3r_ACNETWORK_hpp_

#include <functional>

struct AnycubicNetPCInfo
{
  std::string os         ; // : win10,
  std::string cpu        ; // : amd,
  std::string graphics   ; // : nvidia,
  std::string ram        ; // : 1g
};

namespace Slic3r {

typedef std::string (*func_getVersion)();
typedef void (*func_sendInfo)(const std::string& appVersion, const std::string& userID, const AnycubicNetPCInfo& pcInfo, std::function<void(bool)> onFinished, bool isRegionCN);

class ACNetwork
{
public:
	ACNetwork();
	~ACNetwork(){}

	void startSendAnonymous(std::function<void(bool)> onFinished);
private:
	bool loadLibrary();
	void callSendAnonymous(std::function<void(bool)> onFinished);
private:
	bool m_libLoaded = false;

	func_getVersion m_getVersionPtr = nullptr;
	func_sendInfo m_sendInfoPtr  = nullptr;
};

}

#endif // slic3r_ACNETWORK_hpp_