#include "ACGcode.hpp"

#include "Plater.hpp"
//#include <common/httpClient/url_parse.h>
//#include <common/common_config.h>
//#include <cloud_sdk_cpp/contrl.hxx>
//#include <cloud_sdk_cpp/global.hxx>
//#include <cloud_sdk_cpp/cloud_client.hxx>
//
//#define query "id=42&name=John Doe Jingleheimer-Schmidt"
//#define pory 443
//#define scheme "https"
//#define user "user"
//#define password "pass"
//#define host "example.com"
//#define path "/path/to/my-file.txt"
//#define fragment "page anchor"
//#define TEST_URL \
//    "https://user:pass@example.com:443/path/to/" \
//    "my%2dfile.txt?id=42&name=John%20Doe+Jingleheimer%2DSchmidt#page%20anchor"
//
//
//
//TEST(common, urlparse)
//{
//    ParseUrl parse;
//    ASSERT_TRUE(parse.parse(TEST_URL));
//    ASSERT_EQ(parse.GetPort(), pory);
//    ASSERT_STREQ(parse.GetScheme().c_str(), scheme);
//    ASSERT_STREQ(parse.GetUser().c_str(), user);
//    ASSERT_STREQ(parse.GetPassword().c_str(), password);
//    ASSERT_STREQ(parse.GetHost().c_str(), host);
//    ASSERT_STREQ(parse.GetPath().c_str(), path);
//    ASSERT_STREQ(parse.GetFragment().c_str(), fragment);
//    ASSERT_STREQ(parse.GetQuery().c_str(), query);
//#define new_query_param "v=1023"
//    ASSERT_TRUE(parse.AppendQuery(new_query_param));
//    ASSERT_STREQ(parse.GetQuery().c_str(), query "&" new_query_param);
//}
//
//TEST(common, urlcreate)
//{
//    ParseUrl parse;
//
//    ASSERT_TRUE(parse.SetQuery(query));
//    ASSERT_TRUE(parse.SetPort(pory));
//    ASSERT_TRUE(parse.SetScheme(scheme));
//    ASSERT_TRUE(parse.SetUser(user));
//    ASSERT_TRUE(parse.SetPassword(password));
//    ASSERT_TRUE(parse.SetHost(host));
//    ASSERT_TRUE(parse.SetPath(path));
//    ASSERT_TRUE(parse.SetFragment(fragment));
//
//    ASSERT_STREQ(parse.GetQuery().c_str(), query);
//    ASSERT_EQ(parse.GetPort(), pory);
//    ASSERT_STREQ(parse.GetScheme().c_str(), scheme);
//    ASSERT_STREQ(parse.GetUser().c_str(), user);
//    ASSERT_STREQ(parse.GetPassword().c_str(), password);
//    ASSERT_STREQ(parse.GetHost().c_str(), host);
//    ASSERT_STREQ(parse.GetPath().c_str(), path);
//    ASSERT_STREQ(parse.GetFragment().c_str(), fragment);
//    ASSERT_FALSE(parse.GetURL().empty());
//    const char *p = TEST_URL;
//    auto        u = parse.GetURL();
//    ASSERT_STREQ(p, u.c_str());
//}
//
//#define DOMAIN_CN "cloud-platform.anycubic.com" // 国内域名
//#define DOMAIN_EN "cloud-universe.anycubic.com" // 国外域名
//
//static std::string gCnDomain = DOMAIN_CN;
//static std::string gEnDomain = DOMAIN_EN;

namespace Slic3r { 
namespace GUI {

ACGcode::ACGcode() { 
    plater = GUI::wxGetApp().plater();
}

void ACGcode::ac_load_gcode(const wxString &filename)
{
    
}

void ACGcode::ac_export_gcode() {}

void ACGcode::ac_upload_gcode() {}

void ACGcode::ac_remote_print() {}

ACGcode_Params::ACGcode_Params() : mode(ACGcodeMode::ACGcode_None) {}

}
}




