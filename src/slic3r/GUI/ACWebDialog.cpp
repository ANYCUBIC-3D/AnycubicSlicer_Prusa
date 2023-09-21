#include "ACWebDialog.hpp"
#include <boost/property_tree/json_parser.hpp>
#include <boost/property_tree/ptree.hpp>
#include <wx/timer.h>

namespace Slic3r { 
namespace GUI {

ACWebDialog::ACWebDialog(const wxString &url) : wxDialog(NULL, wxID_ANY, "", wxDefaultPosition, wxDefaultSize, wxNO_BORDER)
{
    wxBoxSizer *topsizer = new wxBoxSizer(wxVERTICAL);

#if wxUSE_WEBVIEW_EDGE
    // Check if a fixed version of edge is present in
    // $executable_path/edge_fixed and use it
    wxFileName edgeFixedDir(wxStandardPaths::Get().GetExecutablePath());
    edgeFixedDir.SetFullName("");
    edgeFixedDir.AppendDir("edge_fixed");
    if (edgeFixedDir.DirExists()) {
        wxWebViewEdge::MSWSetBrowserExecutableDir(edgeFixedDir.GetFullPath());
    }
#endif
    // Create the webview
    m_browser =
#if wxUSE_WEBVIEW_EDGE
        wxWebViewEdge::New();
#else
        wxWebView::New();
#endif

#ifdef __WXMAC__
    // With WKWebView handlers need to be registered before creation
    m_browser->RegisterHandler(wxSharedPtr<wxWebViewHandler>(new wxWebViewArchiveHandler("wxfs")));
    m_browser->RegisterHandler(wxSharedPtr<wxWebViewHandler>(new wxWebViewFSHandler("memory")));
#endif
    m_browser->Create(this, wxID_ANY, url, wxDefaultPosition, wxDefaultSize);

    topsizer->Add(m_browser, wxSizerFlags().Expand().Proportion(1));
#ifndef __WXMAC__
    // We register the wxfs:// protocol for testing purposes
    m_browser->RegisterHandler(wxSharedPtr<wxWebViewHandler>(new wxWebViewArchiveHandler("wxfs")));
    // And the memory: file system
    m_browser->RegisterHandler(wxSharedPtr<wxWebViewHandler>(new wxWebViewFSHandler("memory")));
#endif

    m_browser->AddScriptMessageHandler("anycubicHandler");
    m_browser->EnableContextMenu(false);
    m_browser->EnableAccessToDevTools(false);
    m_browser->EnableHistory(false);
    
    SetSizer(topsizer);

    // Set a more sensible size for web browsing
    SetSize(FromDIP(wxSize(360, 360)));
    
    // Connect the webview events
    Bind(wxEVT_WEBVIEW_NAVIGATING, &ACWebDialog::OnNavigationRequest, this, m_browser->GetId());
    Bind(wxEVT_WEBVIEW_NAVIGATED, &ACWebDialog::OnNavigationComplete, this, m_browser->GetId());
    Bind(wxEVT_WEBVIEW_LOADED, &ACWebDialog::OnDocumentLoaded, this, m_browser->GetId());
    Bind(wxEVT_WEBVIEW_NEWWINDOW, &ACWebDialog::OnNewWindow, this, m_browser->GetId());
    Bind(wxEVT_WEBVIEW_TITLE_CHANGED, &ACWebDialog::OnTitleChanged, this, m_browser->GetId());
    Bind(wxEVT_WEBVIEW_FULLSCREEN_CHANGED, &ACWebDialog::OnFullScreenChanged, this, m_browser->GetId());
    Bind(wxEVT_WEBVIEW_SCRIPT_MESSAGE_RECEIVED, &ACWebDialog::OnScriptMessage, this, m_browser->GetId());
    Bind(wxEVT_WEBVIEW_SCRIPT_RESULT, &ACWebDialog::OnScriptResult, this, m_browser->GetId());
    Bind(wxEVT_WEBVIEW_ERROR, &ACWebDialog::OnError, this, m_browser->GetId());

    //Bind(wxEVT_IDLE, &ACWebDialog::OnIdle, this);
}


ACWebDialog::~ACWebDialog() { 
    if (m_browser != nullptr) {
        m_browser->Close();
        m_browser = nullptr;
    }
}

void ACWebDialog::OnNavigationRequest(wxWebViewEvent &evt)
{
    
}

void ACWebDialog::OnNavigationComplete(wxWebViewEvent &evt)
{
    auto result = ("%s", "Navigation complete; url='" + evt.GetURL() + "'");

}

void ACWebDialog::OnNewWindow(wxWebViewEvent &evt)
{
    

}

void ACWebDialog::OnTitleChanged(wxWebViewEvent &evt)
{
    auto result = ("%s", "Title changed; title='" + evt.GetString() + "'");
}

void ACWebDialog::OnFullScreenChanged(wxWebViewEvent &evt)
{
    auto result = ("Full screen changed; status = %d", evt.GetInt());
}

void ACWebDialog::OnScriptMessage(wxWebViewEvent &evt)
{
    if (evt.GetMessageHandler() == "anycubicHandler") {
        std::string                 jsonStr = evt.GetString().ToStdString();
        boost::property_tree::ptree pt;
        std::istringstream          iss(jsonStr);
        try {
            boost::property_tree::read_json(iss, pt);

            std::string ticket  = pt.get<std::string>("ticket");
            int         ret     = pt.get<int>("ret");
            std::string randstr = pt.get<std::string>("randstr");
            // 0:OK  2: Close
            if (ret == 0) {
                this->EndModal(wxID_YES);
            } else {
                this->EndModal(wxID_NO);
            }
        } catch (const boost::property_tree::ptree_error &e) {
            this->EndModal(wxID_NO);
        }
    }
}

void ACWebDialog::OnDocumentLoaded(wxWebViewEvent &evt)
{
    
    //m_browser->SetZoom(wxWEBVIEW_ZOOM_SMALL);
    m_timer = new wxTimer(this, wxID_ANY);
    this->Bind(wxEVT_TIMER, &ACWebDialog::OnTimer, this, m_timer->GetId());
    m_timer->Start(500, wxTIMER_ONE_SHOT);
}
void ACWebDialog::OnTimer(wxTimerEvent &event)
{
    wxString scriptStr = R"(let tcaptchaObj = document.getElementById('tcaptcha_transform_dy');
                if (tcaptchaObj)
            {
                tcaptchaObj.style.transform = 'scale(1.0)';
            }
            )";
    if (m_browser->RunScript(scriptStr)) {
        if (m_timer) {
            m_timer->Stop();
            m_timer = nullptr;
        }
    }
    
}

void ACWebDialog::OnIdle(wxIdleEvent &WXUNUSED(evt))
{
    if (m_browser == nullptr)
        return;
    if (m_browser->IsBusy()) {
        wxSetCursor(wxCURSOR_ARROWWAIT);
    } else {
        if (m_isRunInfo) {
            m_isRunInfo        = false;
        wxString scriptStr = R"(let tcaptchaObj = document.getElementById('tcaptcha_transform_dy');
                if (tcaptchaObj)
            {
                tcaptchaObj.style.transform = scale(1.0);
            }
            )";
            m_browser->RunScriptAsync(scriptStr);
        }
        wxSetCursor(wxNullCursor);
    }
}

void ACWebDialog::OnScriptResult(wxWebViewEvent &evt)
{
    if (evt.GetString().length() == 0 || evt.GetString() == "undefined")
        return;
    if (!evt.IsError())
        auto result = ("Async script result received; value = %s", evt.GetString());
}


void ACWebDialog::OnError(wxWebViewEvent &evt)
{
#define WX_ERROR_CASE(type) \
    case type: category = #type; break;

    wxString category;
    switch (evt.GetInt()) {
        WX_ERROR_CASE(wxWEBVIEW_NAV_ERR_CONNECTION);
        WX_ERROR_CASE(wxWEBVIEW_NAV_ERR_CERTIFICATE);
        WX_ERROR_CASE(wxWEBVIEW_NAV_ERR_AUTH);
        WX_ERROR_CASE(wxWEBVIEW_NAV_ERR_SECURITY);
        WX_ERROR_CASE(wxWEBVIEW_NAV_ERR_NOT_FOUND);
        WX_ERROR_CASE(wxWEBVIEW_NAV_ERR_REQUEST);
        WX_ERROR_CASE(wxWEBVIEW_NAV_ERR_USER_CANCELLED);
        WX_ERROR_CASE(wxWEBVIEW_NAV_ERR_OTHER);
    }
    
}
}
}




