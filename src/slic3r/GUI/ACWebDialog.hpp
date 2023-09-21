#ifndef slic3r_ACWebDialog_hpp_
#define slic3r_ACWebDialog_hpp_

#include "GUI.hpp"
#include "GUI_App.hpp"
#include "GUI_Utils.hpp"

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

#if !wxUSE_WEBVIEW_WEBKIT && !wxUSE_WEBVIEW_WEBKIT2 && !wxUSE_WEBVIEW_IE && !wxUSE_WEBVIEW_EDGE
#error "A wxWebView backend is required by this sample"
#endif

#include "wx/artprov.h"
#include "wx/cmdline.h"
#include "wx/notifmsg.h"
#include "wx/settings.h"
#include "wx/webview.h"
#if wxUSE_WEBVIEW_IE
#include "wx/msw/webview_ie.h"
#endif
#if wxUSE_WEBVIEW_EDGE
#include "wx/msw/webview_edge.h"
#endif
#include "wx/webviewarchivehandler.h"
#include "wx/webviewfshandler.h"
#include "wx/numdlg.h"
#include "wx/infobar.h"
#include "wx/filesys.h"
#include "wx/fs_arc.h"
#include "wx/fs_mem.h"
#include "wx/stdpaths.h"


#if wxUSE_STC
#include "wx/stc/stc.h"
#else
#include "wx/textctrl.h"
#endif

namespace Slic3r {

namespace GUI {

class ACWebDialog : public wxDialog
{
public:
    ACWebDialog(const wxString &url);
    virtual ~ACWebDialog();

    void OnIdle(wxIdleEvent &evt);
    void OnNavigationRequest(wxWebViewEvent &evt);
    void OnNavigationComplete(wxWebViewEvent &evt);
    void OnDocumentLoaded(wxWebViewEvent &evt);
    void OnNewWindow(wxWebViewEvent &evt);
    void OnTitleChanged(wxWebViewEvent &evt);
    void OnFullScreenChanged(wxWebViewEvent &evt);
    void OnScriptMessage(wxWebViewEvent &evt);
    void OnScriptResult(wxWebViewEvent &evt);
    void       OnError(wxWebViewEvent &evt);
    void       OnTimer(wxTimerEvent &event);
    wxWebView *GetWebViewObj() { return m_browser; }
    

private:
    wxWebView * m_browser;
    bool       m_isRunInfo = false;
    wxTimer *   m_timer;
    
};

}} // namespace Slic3r

#endif /* slic3r_ACWebDialog_hpp_ */
