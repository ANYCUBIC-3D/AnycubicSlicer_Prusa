#ifndef AC_CLOUD_LOGIN_HPP
#define AC_CLOUD_LOGIN_HPP

#include "ACCloudLoginPrivate.hpp"
#include "ACDrawBackgroundPrivate.hpp"
#include "ACTextInputPrivate.hpp"
#include "../ACCheckBox.hpp"


namespace Slic3r {

namespace GUI {

wxDECLARE_EVENT(EVT_ACCLOUD_LOGIN_SUCCESS, wxCommandEvent);

class ACCloudLoginDialog : public DPIDialog
{
public:
	explicit ACCloudLoginDialog(wxWindow* parent);
	~ACCloudLoginDialog();

	enum LayoutMode
	{
		L_LOGIN_CN,   //LOG IN  China
		L_LOGIN_INT,  //LOG IN International
		L_SIGNUP_CN,
		L_SIGNUP_INT,
		L_RESET_CN,
		L_RESET_INT
	};

	void Init();
	void Create();
	void Connect();

	void InitLayoutPanel();
	void ReLayout(LayoutMode mode);
	void ChangeText(LayoutMode mode);
	void ResetMobile();

	//Test Function

	//void OnComboBoxServer(wxCommandEvent& event);
	void OnButtonMobile(wxCommandEvent& event);
	void OnButtonEmail(wxCommandEvent& event);
	void OnButtonForgetPassword(wxCommandEvent& event);
	void OnButtonSignUpNow(wxCommandEvent& event);
	void OnButtonGuide(wxCommandEvent& event);
	void OnButtonGetCode(wxCommandEvent& event);
	void OnButtonClicked(wxCommandEvent& event);//登录
	void OnTimer(wxTimerEvent& event);
	void OnCheckBox();
	void OnCheckBoxAgree();


	//void CallLogPut(bool isSuccess, int id, const wxString& str);
protected:
	bool CrcAccountPhone();//校验电话号码
	bool CrcAccountEmail();//校验邮箱
	bool CrcVerification();//校验验证码
	bool CrcName();        //校验昵称
	bool CrcPassword();    //校验密码
	bool CrcConfirm();     //校验密码确认
private:
	void msw_rescale();
	void on_dpi_changed(const wxRect& suggested_rect) override { msw_rescale(); }

private:
	int seconds{ 60 };
	ACDrawBackgroundPrivate* panel0;//DrawBackground 绘制背景图，单独作为一个容器
	wxPanel* panel;

	/*-------------------Login text && Close button----------------------*/
	wxStaticText* textLogin{ nullptr };//登录提示，比如登录  忘记密码  重置密码等文本
	ACButton* buttonClose;//关闭按钮，为了适配背景，自定义了该按

	/*-------------------Guide text and Guide button----------------------*/
	wxStaticText* textGuide{ nullptr };
	ACButton* buttonGuide;

	/*-------------------Server----------------------*/
	/*wxStaticText* textServer{nullptr};//取消国内跟海外区别
	//ACComboBoxIconLeft* comboServer{ nullptr };
	//wxBitmapComboBox* comboServer{ nullptr };
	//wxComboBox* comboServer{ nullptr };
	//ACComboBox* comboServer{ nullptr };
	ACComboBoxDrawText* comboServer{ nullptr };
	*/

	/*-------------------Dot line----------------------*/
	//ACStaticDashLine* line; //横线

	/*-------------------Register button----------------------*/
	ACButtonUnderline* buttonMobile;//手机号码
	ACButtonUnderline* buttonEmail; //邮箱

	/*-------------------Name----------------------*/
	wxStaticText* textName{ nullptr };//昵称
	wxTextCtrl* name;                 //输入昵称控件


	/*-------------------AccountType----------------------*/
	//wxStaticText* textAccountType;
	//ACComboBoxIcon* comboAccountType{ nullptr };

	/*-------------------Account----------------------*/
	wxStaticText* textAccount; //账号
	ACTextCtrlAccount* account;//输入账号
	//ACTextCtrlIcon* account;

	/*-------------------Verification code----------------------*/
	wxStaticText* textVerication{ nullptr };//验证
	wxTextCtrl* verication;	//输入验证码
	ACButton* buttonGetCode;//获取验证码

	/*-------------------Password----------------------*/
	wxStaticText* textPassword;  //密码
	ACTextCtrlPassword* password;//密码（输入控件）

	/*-------------------Confirm----------------------*/
	wxStaticText* textConfirm;  //确认密码
	ACTextCtrlPassword* confirm;//确认密码（输入控件）

	/*-------------------[√] [Remember me] [Forget Password?]----------------------*/
	//wxCheckBox* checkBox;
	ACCheckBox* checkBox;          //记住密码 -> 保持登录
	wxButton* buttonForgetPassword;//忘记密码

	/*--------------------[√] Terms---------------------*/
	ACCheckBox* checkBoxAgree;//同意条款
	wxStaticText* textAgree0;// I agree to the
	wxStaticText* textAgree1;// and
	wxButton* buttonTerms;   //Terms  条款1
	wxButton* buttonPrivacy;//Privacy 条款2
	


	/*-------------------[Login] [Reset Password] [Sign Up]  ----------------------*/
	ACButton* button;//登录 注册 重置 按钮

	/*-------------------[No Account?] [Sign Up Now!]----------------------*/
	wxStaticText* textNoAccount;//没有账号？
	wxButton* buttonSignUpNow;  //登录按钮

	/*-------------------Log----------------------*/
	wxStaticText* textLog;//日志信息 输出 在最底下

	/*-------------------Timer----------------------*/
	wxTimer timer;//用指针形式没工作，这里采用对象形式， 定时器，用来记时60S的重新发送验证码

	wxBoxSizer* sizer;

	wxBoxSizer* hBoxTop;       //Login text && Close
	wxBoxSizer* hBoxGuide;     //Guide text and Guide button
	wxBoxSizer* hBoxServer;    //Server
	wxBoxSizer* hBoxRegister;  //Register button
	wxBoxSizer* hBoxAgree;     //Agree terms
	wxBoxSizer* hBoxVerication;//Verication
	wxBoxSizer* vBox; //panel main sizer
	wxBoxSizer* hBoxPassword1;// [√] [Remember me] [Forget Password?]
	wxBoxSizer* hBoxTips;

	LayoutMode layoutMode{ L_LOGIN_CN };

public:
	bool isRegionCN{ true };//定义是否为国内版或者国外版，依据此变量来展示不同的ui
};



} // GUI
} // Slic3r

#endif //AC_CLOUD_LOGIN_HPP
