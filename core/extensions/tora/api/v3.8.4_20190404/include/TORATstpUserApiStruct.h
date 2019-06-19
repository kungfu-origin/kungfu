/////////////////////////////////////////////////////////////////////////
///@company 上海泰琰信息科技有限公司
///@file TORATstpUserApiStruct.h
///@brief 定义了客户端接口使用的业务数据结构
///@history 
/////////////////////////////////////////////////////////////////////////

#if !defined(_TORA_TSTPCSTRUCT_H)
#define _TORA_TSTPCSTRUCT_H

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "TORATstpUserApiDataType.h"

/// fens用户信息
struct CTORATstpFensUserInfoField
{
		
	///登录账户
	TTORATstpLogInAccountType	LogInAccount;

	///登录账户类型
	TTORATstpLogInAccountTypeType	LogInAccountType;


};

/// 登录请求
struct CTORATstpReqUserLoginField
{
		
	///登录账户
	TTORATstpLogInAccountType	LogInAccount;

	///登录账户类型
	TTORATstpLogInAccountTypeType	LogInAccountType;

	///密码(密码认证时必填)
	TTORATstpPasswordType	Password;

	///用户端产品信息
	TTORATstpProductInfoType	UserProductInfo;

	///接口端产品信息
	TTORATstpProductInfoType	InterfaceProductInfo;

	///协议信息
	TTORATstpProtocolInfoType	ProtocolInfo;

	///Mac地址
	TTORATstpMacAddressType	MacAddress;

	///移动设备手机号
	TTORATstpMobileType	Mobile;

	///内网IP地址
	TTORATstpIPAddressType	InnerIPAddress;

	///接口语言
	TTORATstpLangType	Lang;

	///终端信息
	TTORATstpTerminalInfoType	TerminalInfo;

	///网关Mac地址
	TTORATstpMacAddressType	GWMacAddress;

	///网关内网IP地址
	TTORATstpIPAddressType	GWInnerIPAddress;

	///网关外网IP地址
	TTORATstpIPAddressType	GWOuterIPAddress;

	///一级机构代码（以资金账号方式登录时必填）
	TTORATstpDepartmentIDType	DepartmentID;

	///硬盘序列号
	TTORATstpHDSerialType	HDSerial;

	///认证方式(指纹认证时必填)
	TTORATstpAuthModeType	AuthMode;

	///设备标识(指纹认证时必填)
	TTORATstpDeviceIDType	DeviceID;

	///设备序列号(指纹认证时必填)
	TTORATstpDeviceSerialType	DeviceSerial;

	///外网IP地址
	TTORATstpIPAddressType	OuterIPAddress;


};

/// 登录应答
struct CTORATstpRspUserLoginField
{
		
	///登录时间
	TTORATstpTimeType	LoginTime;

	///登录账户
	TTORATstpLogInAccountType	LogInAccount;

	///登录账户类型
	TTORATstpLogInAccountTypeType	LogInAccountType;

	///交易系统名称
	TTORATstpSystemNameType	SystemName;

	///前置编号
	TTORATstpFrontIDType	FrontID;

	///会话编号
	TTORATstpSessionIDType	SessionID;

	///最大报单引用
	TTORATstpOrderRefType	MaxOrderRef;

	///私有流长度
	TTORATstpVolumeType	PrivateFlowCount;

	///公有流长度
	TTORATstpVolumeType	PublicFlowCount;

	///交易日
	TTORATstpDateType	TradingDay;

	///用户代码
	TTORATstpUserIDType	UserID;

	///用户名称
	TTORATstpUserNameType	UserName;

	///用户类型
	TTORATstpUserTypeType	UserType;

	///一级机构代码
	TTORATstpDepartmentIDType	DepartmentID;

	///终端IP地址
	TTORATstpIPAddressType	InnerIPAddress;

	///Mac地址
	TTORATstpMacAddressType	MacAddress;

	///硬盘序列号
	TTORATstpHDSerialType	HDSerial;

	///报单流控
	TTORATstpCommFluxType	OrderInsertCommFlux;

	///密码修改周期(天),置为0表永久有效
	TTORATstpCountType	PasswordUpdatePeriod;

	///密码有效剩余天数
	TTORATstpCountType	PasswordRemainDays;

	///是否需要改密
	TTORATstpBoolType	NeedUpdatePassword;

	///撤单流控
	TTORATstpCommFluxType	OrderActionCommFlux;

	///移动设备手机号
	TTORATstpMobileType	Mobile;

	///外网IP地址
	TTORATstpIPAddressType	OuterIPAddress;


};

/// 响应信息
struct CTORATstpRspInfoField
{
		
	///错误代码
	TTORATstpErrorIDType	ErrorID;

	///错误信息
	TTORATstpErrorMsgType	ErrorMsg;


};

/// 用户登出
struct CTORATstpUserLogoutField
{
		
	///用户代码
	TTORATstpUserIDType	UserID;


};

/// 强制交易员退出
struct CTORATstpForceUserLogoutField
{
		
	///用户代码
	TTORATstpUserIDType	UserID;


};

/// 重置用户密码
struct CTORATstpUserPasswordUpdateField
{
		
	///用户代码
	TTORATstpUserIDType	UserID;

	///旧密码
	TTORATstpPasswordType	OldPassword;

	///新密码
	TTORATstpPasswordType	NewPassword;


};

/// 请求录入设备序列
struct CTORATstpReqInputDeviceSerialField
{
		
	///用户代码
	TTORATstpUserIDType	UserID;

	///设备标识
	TTORATstpDeviceIDType	DeviceID;

	///设备序列号
	TTORATstpDeviceSerialType	DeviceSerial;


};

/// 请求录入设备序列应答
struct CTORATstpRspInputDeviceSerialField
{
		
	///用户代码
	TTORATstpUserIDType	UserID;


};

/// 启用解锁用户
struct CTORATstpActivateUserField
{
		
	///用户代码
	TTORATstpUserIDType	UserID;


};

/// 用户会话标识
struct CTORATstpVerifyUserPasswordField
{
		
	///用户代码
	TTORATstpUserIDType	UserID;

	///密码
	TTORATstpPasswordType	Password;


};

/// 输入报单
struct CTORATstpInputOrderField
{
		
	///投资者代码
	TTORATstpInvestorIDType	InvestorID;

	///证券代码
	TTORATstpSecurityIDType	SecurityID;

	///报单引用
	TTORATstpOrderRefType	OrderRef;

	///用户代码
	TTORATstpUserIDType	UserID;

	///报单价格条件
	TTORATstpOrderPriceTypeType	OrderPriceType;

	///买卖方向
	TTORATstpDirectionType	Direction;

	///组合开平标志
	TTORATstpCombOffsetFlagType	CombOffsetFlag;

	///组合投机套保标志
	TTORATstpCombHedgeFlagType	CombHedgeFlag;

	///价格
	TTORATstpPriceType	LimitPrice;

	///数量
	TTORATstpVolumeType	VolumeTotalOriginal;

	///有效期类型
	TTORATstpTimeConditionType	TimeCondition;

	///成交量类型
	TTORATstpVolumeConditionType	VolumeCondition;

	///最小成交量
	TTORATstpVolumeType	MinVolume;

	///强平原因
	TTORATstpForceCloseReasonType	ForceCloseReason;

	///请求编号
	TTORATstpRequestIDType	RequestID;

	///用户强评标志
	TTORATstpBoolType	UserForceClose;

	///互换单标志
	TTORATstpBoolType	IsSwapOrder;

	///交易所代码
	TTORATstpExchangeIDType	ExchangeID;

	///股东账户代码
	TTORATstpShareholderIDType	ShareholderID;

	///投资单元代码
	TTORATstpBusinessUnitIDType	BusinessUnitID;

	///资金账户代码
	TTORATstpAccountIDType	AccountID;

	///IP地址
	TTORATstpIPAddressType	IPAddress;

	///Mac地址
	TTORATstpMacAddressType	MacAddress;

	///港股通订单数量类型,默认整股买卖
	TTORATstpLotTypeType	LotType;

	///系统报单编号
	TTORATstpOrderSysIDType	OrderSysID;

	///终端信息
	TTORATstpTerminalInfoType	TerminalInfo;

	///长字符串附加信息
	TTORATstpBigsInfoType	BInfo;

	///短字符串附加信息
	TTORATstpShortsInfoType	SInfo;

	///整形附加信息
	TTORATstpIntInfoType	IInfo;

	///转入交易单元代码
	TTORATstpPbuIDType	TransfereePbuID;

	///委托方式
	TTORATstpOperwayType	Operway;

	///条件检查
	TTORATstpCondCheckType	CondCheck;

	///硬盘序列号
	TTORATstpHDSerialType	HDSerial;

	///移动设备手机号
	TTORATstpMobileType	Mobile;


};

/// 报单
struct CTORATstpOrderField
{
		
	///投资者代码
	TTORATstpInvestorIDType	InvestorID;

	///证券代码
	TTORATstpSecurityIDType	SecurityID;

	///报单引用
	TTORATstpOrderRefType	OrderRef;

	///用户代码
	TTORATstpUserIDType	UserID;

	///报单价格条件
	TTORATstpOrderPriceTypeType	OrderPriceType;

	///买卖方向
	TTORATstpDirectionType	Direction;

	///组合开平标志
	TTORATstpCombOffsetFlagType	CombOffsetFlag;

	///组合投机套保标志
	TTORATstpCombHedgeFlagType	CombHedgeFlag;

	///价格
	TTORATstpPriceType	LimitPrice;

	///数量
	TTORATstpVolumeType	VolumeTotalOriginal;

	///有效期类型
	TTORATstpTimeConditionType	TimeCondition;

	///成交量类型
	TTORATstpVolumeConditionType	VolumeCondition;

	///最小成交量
	TTORATstpVolumeType	MinVolume;

	///强平原因
	TTORATstpForceCloseReasonType	ForceCloseReason;

	///请求编号
	TTORATstpRequestIDType	RequestID;

	///本地报单编号
	TTORATstpOrderLocalIDType	OrderLocalID;

	///交易所代码
	TTORATstpExchangeIDType	ExchangeID;

	///市场代码
	TTORATstpMarketIDType	MarketID;

	///股东账户代码
	TTORATstpShareholderIDType	ShareholderID;

	///证券在交易所代码
	TTORATstpExchangeInstIDType	ExchangeInstID;

	///交易单元代码
	TTORATstpTraderIDType	TraderID;

	///报单提交状态
	TTORATstpOrderSubmitStatusType	OrderSubmitStatus;

	///交易日
	TTORATstpDateType	TradingDay;

	///系统报单编号
	TTORATstpOrderSysIDType	OrderSysID;

	///报单状态
	TTORATstpOrderStatusType	OrderStatus;

	///报单类型
	TTORATstpOrderTypeType	OrderType;

	///已成交数量
	TTORATstpVolumeType	VolumeTraded;

	///剩余未完成数量
	TTORATstpVolumeType	VolumeTotal;

	///报单日期
	TTORATstpDateType	InsertDate;

	///报单时间
	TTORATstpTimeType	InsertTime;

	///撤销时间
	TTORATstpTimeType	CancelTime;

	///最后修改交易单元代码
	TTORATstpTraderIDType	ActiveTraderID;

	///前置编号
	TTORATstpFrontIDType	FrontID;

	///会话编号
	TTORATstpSessionIDType	SessionID;

	///用户端产品信息
	TTORATstpProductInfoType	UserProductInfo;

	///状态信息
	TTORATstpErrorMsgType	StatusMsg;

	///用户强评标志
	TTORATstpBoolType	UserForceClose;

	///操作用户代码
	TTORATstpUserIDType	ActiveUserID;

	///投资单元代码
	TTORATstpBusinessUnitIDType	BusinessUnitID;

	///资金账户代码
	TTORATstpAccountIDType	AccountID;

	///IP地址
	TTORATstpIPAddressType	IPAddress;

	///Mac地址
	TTORATstpMacAddressType	MacAddress;

	///港股通订单数量类型,默认整股买卖
	TTORATstpLotTypeType	LotType;

	///长字符串附加信息
	TTORATstpBigsInfoType	BInfo;

	///短字符串附加信息
	TTORATstpShortsInfoType	SInfo;

	///整形附加信息
	TTORATstpIntInfoType	IInfo;

	///转入交易单元代码(仅在转托管操作时有效)
	TTORATstpPbuIDType	TransfereePbuID;

	///委托方式
	TTORATstpOperwayType	Operway;

	///一级机构代码
	TTORATstpDepartmentIDType	DepartmentID;

	///适当性控制业务类别
	TTORATstpProperCtrlBusinessTypeType	ProperCtrlBusinessType;

	///适当性控制通过标示
	TTORATstpProperCtrlPassFlagType	ProperCtrlPassFlag;

	///条件检查
	TTORATstpCondCheckType	CondCheck;

	///是否预埋
	TTORATstpBoolType	IsCacheOrder;

	///成交金额
	TTORATstpMoneyType	Turnover;

	///回报附加浮点型数据信息
	TTORATstpFloatInfoType	RtnFloatInfo;

	///回报附加整型数据
	TTORATstpIntInfoType	RtnIntInfo;

	///硬盘序列号
	TTORATstpHDSerialType	HDSerial;

	///移动设备手机号
	TTORATstpMobileType	Mobile;


};

/// 用户标识
struct CTORATstpUserRefField
{
		
	///用户代码
	TTORATstpUserIDType	UserID;


};

/// 输入撤单操作
struct CTORATstpInputOrderActionField
{
		
	///投资者代码
	TTORATstpInvestorIDType	InvestorID;

	///报单操作引用
	TTORATstpOrderRefType	OrderActionRef;

	///报单引用
	TTORATstpOrderRefType	OrderRef;

	///请求编号
	TTORATstpRequestIDType	RequestID;

	///前置编号
	TTORATstpFrontIDType	FrontID;

	///会话编号
	TTORATstpSessionIDType	SessionID;

	///交易所代码
	TTORATstpExchangeIDType	ExchangeID;

	///系统报单编号
	TTORATstpOrderSysIDType	OrderSysID;

	///操作标志
	TTORATstpActionFlagType	ActionFlag;

	///价格
	TTORATstpPriceType	LimitPrice;

	///报单数量变化
	TTORATstpVolumeType	VolumeChange;

	///用户代码
	TTORATstpUserIDType	UserID;

	///证券代码
	TTORATstpSecurityIDType	SecurityID;

	///IP地址
	TTORATstpIPAddressType	IPAddress;

	///Mac地址
	TTORATstpMacAddressType	MacAddress;

	///本地撤单编号
	TTORATstpOrderLocalIDType	CancelOrderLocalID;

	///终端信息
	TTORATstpTerminalInfoType	TerminalInfo;

	///长字符串附加信息
	TTORATstpBigsInfoType	BInfo;

	///短字符串附加信息
	TTORATstpShortsInfoType	SInfo;

	///整形附加信息
	TTORATstpIntInfoType	IInfo;

	///委托方式
	TTORATstpOperwayType	Operway;

	///硬盘序列号
	TTORATstpHDSerialType	HDSerial;

	///移动设备手机号
	TTORATstpMobileType	Mobile;


};

/// 报单操作
struct CTORATstpOrderActionField
{
		
	///投资者代码
	TTORATstpInvestorIDType	InvestorID;

	///报单操作引用
	TTORATstpOrderRefType	OrderActionRef;

	///报单引用
	TTORATstpOrderRefType	OrderRef;

	///请求编号
	TTORATstpRequestIDType	RequestID;

	///前置编号
	TTORATstpFrontIDType	FrontID;

	///会话编号
	TTORATstpSessionIDType	SessionID;

	///交易所代码
	TTORATstpExchangeIDType	ExchangeID;

	///系统报单编号
	TTORATstpOrderSysIDType	OrderSysID;

	///操作标志
	TTORATstpActionFlagType	ActionFlag;

	///价格
	TTORATstpPriceType	LimitPrice;

	///报单数量变化
	TTORATstpVolumeType	VolumeChange;

	///操作日期
	TTORATstpDateType	ActionDate;

	///操作时间
	TTORATstpTimeType	ActionTime;

	///交易单元代码
	TTORATstpTraderIDType	TraderID;

	///本地报单编号
	TTORATstpOrderLocalIDType	OrderLocalID;

	///操作本地编号
	TTORATstpOrderLocalIDType	ActionLocalID;

	///股东账户代码
	TTORATstpShareholderIDType	ShareholderID;

	///报单操作状态
	TTORATstpOrderActionStatusType	OrderActionStatus;

	///用户代码
	TTORATstpUserIDType	UserID;

	///状态信息
	TTORATstpErrorMsgType	StatusMsg;

	///证券代码
	TTORATstpSecurityIDType	SecurityID;

	///IP地址
	TTORATstpIPAddressType	IPAddress;

	///Mac地址
	TTORATstpMacAddressType	MacAddress;

	///长字符串附加信息
	TTORATstpBigsInfoType	BInfo;

	///短字符串附加信息
	TTORATstpShortsInfoType	SInfo;

	///整形附加信息
	TTORATstpIntInfoType	IInfo;

	///委托方式
	TTORATstpOperwayType	Operway;

	///硬盘序列号
	TTORATstpHDSerialType	HDSerial;

	///移动设备手机号
	TTORATstpMobileType	Mobile;


};

/// 成交
struct CTORATstpTradeField
{
		
	///投资者代码
	TTORATstpInvestorIDType	InvestorID;

	///证券代码
	TTORATstpSecurityIDType	SecurityID;

	///用户代码
	TTORATstpUserIDType	UserID;

	///交易所代码
	TTORATstpExchangeIDType	ExchangeID;

	///成交编号
	TTORATstpTradeIDType	TradeID;

	///买卖方向
	TTORATstpDirectionType	Direction;

	///系统报单编号
	TTORATstpOrderSysIDType	OrderSysID;

	///市场代码
	TTORATstpMarketIDType	MarketID;

	///股东账户代码
	TTORATstpShareholderIDType	ShareholderID;

	///证券在交易所代码
	TTORATstpExchangeInstIDType	ExchangeInstID;

	///开平标志
	TTORATstpOffsetFlagType	OffsetFlag;

	///投机套保标志
	TTORATstpHedgeFlagType	HedgeFlag;

	///成交价格
	TTORATstpPriceType	Price;

	///成交数量
	TTORATstpVolumeType	Volume;

	///成交日期
	TTORATstpDateType	TradeDate;

	///成交时间
	TTORATstpTimeType	TradeTime;

	///交易单元代码
	TTORATstpTraderIDType	TraderID;

	///本地报单编号
	TTORATstpOrderLocalIDType	OrderLocalID;

	///交易日
	TTORATstpDateType	TradingDay;

	///投资单元代码
	TTORATstpBusinessUnitIDType	BusinessUnitID;

	///资金账户代码
	TTORATstpAccountIDType	AccountID;

	///报单引用
	TTORATstpOrderRefType	OrderRef;

	///一级机构代码
	TTORATstpDepartmentIDType	DepartmentID;


};

/// 深度行情通知
struct CTORATstpMarketDataField
{
		
	///交易日
	TTORATstpDateType	TradingDay;

	///证券代码
	TTORATstpSecurityIDType	SecurityID;

	///交易所代码
	TTORATstpExchangeIDType	ExchangeID;

	///证券名称
	TTORATstpSecurityNameType	SecurityName;

	///昨收盘价
	TTORATstpPriceType	PreClosePrice;

	///今开盘价
	TTORATstpPriceType	OpenPrice;

	///成交量
	TTORATstpLongVolumeType	Volume;

	///成交金额
	TTORATstpMoneyType	Turnover;

	///成交笔数
	TTORATstpLongVolumeType	TradingCount;

	///最新价
	TTORATstpPriceType	LastPrice;

	///最高价
	TTORATstpPriceType	HighestPrice;

	///最低价
	TTORATstpPriceType	LowestPrice;

	///买1价
	TTORATstpPriceType	BidPrice1;

	///卖1价
	TTORATstpPriceType	AskPrice1;

	///涨停价
	TTORATstpPriceType	UpperLimitPrice;

	///跌停价
	TTORATstpPriceType	LowerLimitPrice;

	///市盈率1
	TTORATstpRatioType	PERatio1;

	///市盈率2
	TTORATstpRatioType	PERatio2;

	///价格升跌1
	TTORATstpPriceType	PriceUpDown1;

	///价格升跌2
	TTORATstpPriceType	PriceUpDown2;

	///持仓量
	TTORATstpLargeVolumeType	OpenInterest;

	///买1量
	TTORATstpLongVolumeType	BidVolume1;

	///卖1量
	TTORATstpLongVolumeType	AskVolume1;

	///买2价
	TTORATstpPriceType	BidPrice2;

	///买2量
	TTORATstpLongVolumeType	BidVolume2;

	///卖2价
	TTORATstpPriceType	AskPrice2;

	///卖2量
	TTORATstpLongVolumeType	AskVolume2;

	///买3价
	TTORATstpPriceType	BidPrice3;

	///买3量
	TTORATstpLongVolumeType	BidVolume3;

	///卖3价
	TTORATstpPriceType	AskPrice3;

	///卖3量
	TTORATstpLongVolumeType	AskVolume3;

	///买4价
	TTORATstpPriceType	BidPrice4;

	///买4量
	TTORATstpLongVolumeType	BidVolume4;

	///卖4价
	TTORATstpPriceType	AskPrice4;

	///卖4量
	TTORATstpLongVolumeType	AskVolume4;

	///买5价
	TTORATstpPriceType	BidPrice5;

	///买5量
	TTORATstpLongVolumeType	BidVolume5;

	///卖5价
	TTORATstpPriceType	AskPrice5;

	///卖5量
	TTORATstpLongVolumeType	AskVolume5;

	///更新时间
	TTORATstpTimeType	UpdateTime;

	///更新毫秒
	TTORATstpMillisecType	UpdateMillisec;

	///今收盘价
	TTORATstpPriceType	ClosePrice;

	///行情产品实时状态
	TTORATstpMDSecurityStatType	MDSecurityStat;

	///是否警示板块
	TTORATstpBoolType	HWFlag;


};

/// 盘后行情通知
struct CTORATstpPHMarketDataField
{
		
	///交易日
	TTORATstpDateType	TradingDay;

	///证券代码
	TTORATstpSecurityIDType	SecurityID;

	///交易所代码
	TTORATstpExchangeIDType	ExchangeID;

	///证券名称
	TTORATstpSecurityNameType	SecurityName;

	///成交量
	TTORATstpLongVolumeType	Volume;

	///成交金额
	TTORATstpMoneyType	Turnover;

	///今收盘价
	TTORATstpPriceType	ClosePrice;

	///涨停价
	TTORATstpPriceType	UpperLimitPrice;

	///跌停价
	TTORATstpPriceType	LowerLimitPrice;

	///买入申报数量
	TTORATstpLongVolumeType	BidVolume;

	///卖出申报数量
	TTORATstpLongVolumeType	AskVolume;

	///更新时间
	TTORATstpTimeType	UpdateTime;

	///更新毫秒
	TTORATstpMillisecType	UpdateMillisec;

	///行情产品实时状态
	TTORATstpMDSecurityStatType	MDSecurityStat;

	///是否警示板块
	TTORATstpBoolType	HWFlag;


};

/// 市场状态
struct CTORATstpMarketStatusField
{
		
	///市场代码
	TTORATstpMarketIDType	MarketID;

	///市场状态
	TTORATstpMarketStatusType	MarketStatus;


};

/// 条件单录入域
struct CTORATstpInputCondOrderField
{
		
	///交易所代码
	TTORATstpExchangeIDType	ExchangeID;

	///投资者代码
	TTORATstpInvestorIDType	InvestorID;

	///投资单元代码
	TTORATstpBusinessUnitIDType	BusinessUnitID;

	///股东账户代码
	TTORATstpShareholderIDType	ShareholderID;

	///证券代码
	TTORATstpSecurityIDType	SecurityID;

	///买卖方向
	TTORATstpDirectionType	Direction;

	///条件单价格类型
	TTORATstpOrderPriceTypeType	OrderPriceType;

	///条件单数量类型
	TTORATstpOrderVolumeTypeType	OrderVolumeType;

	///有效期类型
	TTORATstpTimeConditionType	TimeCondition;

	///成交量类型
	TTORATstpVolumeConditionType	VolumeCondition;

	///报单价格
	TTORATstpPriceType	LimitPrice;

	///报单数量
	TTORATstpVolumeType	VolumeTotalOriginal;

	///组合开平标志
	TTORATstpCombOffsetFlagType	CombOffsetFlag;

	///组合投机套保标志
	TTORATstpCombHedgeFlagType	CombHedgeFlag;

	///条件报单引用
	TTORATstpOrderRefType	CondOrderRef;

	///资金账户代码
	TTORATstpAccountIDType	AccountID;

	///用户代码
	TTORATstpUserIDType	UserID;

	///请求编号
	TTORATstpRequestIDType	RequestID;

	///IP地址
	TTORATstpIPAddressType	IPAddress;

	///Mac地址
	TTORATstpMacAddressType	MacAddress;

	///报单编号
	TTORATstpCondOrderIDType	CondOrderID;

	///终端信息
	TTORATstpTerminalInfoType	TerminalInfo;

	///长字符串附加信息
	TTORATstpBigsInfoType	BInfo;

	///短字符串附加信息
	TTORATstpShortsInfoType	SInfo;

	///整形附加信息
	TTORATstpIntInfoType	IInfo;

	///委托方式
	TTORATstpOperwayType	Operway;

	///条件检查
	TTORATstpCondCheckType	CondCheck;

	///条件单触发条件
	TTORATstpContingentConditionType	ContingentCondition;

	///条件价
	TTORATstpPriceType	ConditionPrice;

	///价格浮动tick数
	TTORATstpVolumeType	PriceTicks;

	///数量浮动倍数
	TTORATstpVolumeMultipleType	VolumeMultiple;

	///相关前置编号
	TTORATstpFrontIDType	RelativeFrontID;

	///相关会话编号
	TTORATstpSessionIDType	RelativeSessionID;

	///相关条件参数
	TTORATstpRelativeCondParamType	RelativeParam;

	///附加条件单触发条件
	TTORATstpContingentConditionType	AppendContingentCondition;

	///附加条件价
	TTORATstpPriceType	AppendConditionPrice;

	///附加相关前置编号
	TTORATstpFrontIDType	AppendRelativeFrontID;

	///附加相关会话编号
	TTORATstpSessionIDType	AppendRelativeSessionID;

	///附加相关条件参数
	TTORATstpRelativeCondParamType	AppendRelativeParam;

	///硬盘序列号
	TTORATstpHDSerialType	HDSerial;

	///港股通订单数量类型,默认整股买卖
	TTORATstpLotTypeType	LotType;

	///移动设备手机号
	TTORATstpMobileType	Mobile;


};

/// 条件单域
struct CTORATstpConditionOrderField
{
		
	///交易所代码
	TTORATstpExchangeIDType	ExchangeID;

	///投资者代码
	TTORATstpInvestorIDType	InvestorID;

	///投资单元代码
	TTORATstpBusinessUnitIDType	BusinessUnitID;

	///股东账户代码
	TTORATstpShareholderIDType	ShareholderID;

	///证券代码
	TTORATstpSecurityIDType	SecurityID;

	///买卖方向
	TTORATstpDirectionType	Direction;

	///条件单价格类型
	TTORATstpOrderPriceTypeType	OrderPriceType;

	///条件单数量类型
	TTORATstpOrderVolumeTypeType	OrderVolumeType;

	///有效期类型
	TTORATstpTimeConditionType	TimeCondition;

	///成交量类型
	TTORATstpVolumeConditionType	VolumeCondition;

	///报单价格
	TTORATstpPriceType	LimitPrice;

	///报单数量
	TTORATstpVolumeType	VolumeTotalOriginal;

	///组合开平标志
	TTORATstpCombOffsetFlagType	CombOffsetFlag;

	///组合投机套保标志
	TTORATstpCombHedgeFlagType	CombHedgeFlag;

	///条件报单引用
	TTORATstpOrderRefType	CondOrderRef;

	///资金账户代码
	TTORATstpAccountIDType	AccountID;

	///用户代码
	TTORATstpUserIDType	UserID;

	///请求编号
	TTORATstpRequestIDType	RequestID;

	///IP地址
	TTORATstpIPAddressType	IPAddress;

	///Mac地址
	TTORATstpMacAddressType	MacAddress;

	///报单编号
	TTORATstpCondOrderIDType	CondOrderID;

	///终端信息
	TTORATstpTerminalInfoType	TerminalInfo;

	///长字符串附加信息
	TTORATstpBigsInfoType	BInfo;

	///短字符串附加信息
	TTORATstpShortsInfoType	SInfo;

	///整形附加信息
	TTORATstpIntInfoType	IInfo;

	///委托方式
	TTORATstpOperwayType	Operway;

	///条件检查
	TTORATstpCondCheckType	CondCheck;

	///条件单触发条件
	TTORATstpContingentConditionType	ContingentCondition;

	///条件价
	TTORATstpPriceType	ConditionPrice;

	///价格浮动tick数
	TTORATstpVolumeType	PriceTicks;

	///数量浮动倍数
	TTORATstpVolumeMultipleType	VolumeMultiple;

	///相关前置编号
	TTORATstpFrontIDType	RelativeFrontID;

	///相关会话编号
	TTORATstpSessionIDType	RelativeSessionID;

	///相关条件参数
	TTORATstpRelativeCondParamType	RelativeParam;

	///附加条件单触发条件
	TTORATstpContingentConditionType	AppendContingentCondition;

	///附加条件价
	TTORATstpPriceType	AppendConditionPrice;

	///附加相关前置编号
	TTORATstpFrontIDType	AppendRelativeFrontID;

	///附加相关会话编号
	TTORATstpSessionIDType	AppendRelativeSessionID;

	///附加相关条件参数
	TTORATstpRelativeCondParamType	AppendRelativeParam;

	///交易日
	TTORATstpDateType	TradingDay;

	///条件单状态
	TTORATstpCondOrderStatusType	CondOrderStatus;

	///报单日期
	TTORATstpDateType	InsertDate;

	///报单时间
	TTORATstpTimeType	InsertTime;

	///撤销时间
	TTORATstpTimeType	CancelTime;

	///撤销用户
	TTORATstpUserIDType	CancelUser;

	///前置编号
	TTORATstpFrontIDType	FrontID;

	///会话编号
	TTORATstpSessionIDType	SessionID;

	///用户端产品信息
	TTORATstpProductInfoType	UserProductInfo;

	///状态信息
	TTORATstpErrorMsgType	StatusMsg;

	///一级机构代码
	TTORATstpDepartmentIDType	DepartmentID;

	///适当性控制业务类别
	TTORATstpProperCtrlBusinessTypeType	ProperCtrlBusinessType;

	///适当性控制通过标识
	TTORATstpProperCtrlPassFlagType	ProperCtrlPassFlag;

	///触发日期
	TTORATstpDateType	ActiveDate;

	///触发时间
	TTORATstpTimeType	ActiveTime;

	///硬盘序列号
	TTORATstpHDSerialType	HDSerial;

	///港股通订单数量类型,默认整股买卖
	TTORATstpLotTypeType	LotType;

	///移动设备手机号
	TTORATstpMobileType	Mobile;


};

/// 条件单操作录入
struct CTORATstpInputCondOrderActionField
{
		
	///请求编号
	TTORATstpRequestIDType	RequestID;

	///交易所代码
	TTORATstpExchangeIDType	ExchangeID;

	///条件单操作引用
	TTORATstpOrderRefType	CondOrderActionRef;

	///条件单引用
	TTORATstpOrderRefType	CondOrderRef;

	///前置编号
	TTORATstpFrontIDType	FrontID;

	///会话编号
	TTORATstpSessionIDType	SessionID;

	///条件单编号
	TTORATstpCondOrderIDType	CondOrderID;

	///操作标志
	TTORATstpActionFlagType	ActionFlag;

	///投资者代码
	TTORATstpInvestorIDType	InvestorID;

	///证券代码
	TTORATstpSecurityIDType	SecurityID;

	///用户代码
	TTORATstpUserIDType	UserID;

	///IP地址
	TTORATstpIPAddressType	IPAddress;

	///Mac地址
	TTORATstpMacAddressType	MacAddress;

	///条件单撤单编号
	TTORATstpCondOrderIDType	CancelCondOrderID;

	///终端信息
	TTORATstpTerminalInfoType	TerminalInfo;

	///长字符串附加信息
	TTORATstpBigsInfoType	BInfo;

	///短字符串附加信息
	TTORATstpShortsInfoType	SInfo;

	///整形附加信息
	TTORATstpIntInfoType	IInfo;

	///委托方式
	TTORATstpOperwayType	Operway;

	///硬盘序列号
	TTORATstpHDSerialType	HDSerial;

	///移动设备手机号
	TTORATstpMobileType	Mobile;


};

/// 查询集中交易系统资金请求
struct CTORATstpReqInquiryJZFundField
{
		
	///资金账户代码
	TTORATstpAccountIDType	AccountID;

	///币种
	TTORATstpCurrencyIDType	CurrencyID;

	///一级机构代码
	TTORATstpDepartmentIDType	DepartmentID;


};

/// 查询集中交易系统资金响应
struct CTORATstpRspInquiryJZFundField
{
		
	///资金账户代码
	TTORATstpAccountIDType	AccountID;

	///币种
	TTORATstpCurrencyIDType	CurrencyID;

	///可用金额
	TTORATstpMoneyType	UsefulMoney;

	///可取额度
	TTORATstpMoneyType	FetchLimit;

	///一级机构代码
	TTORATstpDepartmentIDType	DepartmentID;


};

/// 资金转移
struct CTORATstpInputTransferFundField
{
		
	///资金账户代码
	TTORATstpAccountIDType	AccountID;

	///币种
	TTORATstpCurrencyIDType	CurrencyID;

	///申请流水号
	TTORATstpExternalSerialType	ApplySerial;

	///转移方向
	TTORATstpTransferDirectionType	TransferDirection;

	///转移金额
	TTORATstpMoneyType	Amount;

	///一级机构代码
	TTORATstpDepartmentIDType	DepartmentID;

	///银行代码(银证转账时必填)
	TTORATstpBankIDType	BankID;

	///资金密码(证券转银行时必填)
	TTORATstpPasswordType	AccountPassword;

	///银行密码(银行转证券时必填)
	TTORATstpPasswordType	BankPassword;

	///外部系统一级机构代码(外部系统转账时必填)
	TTORATstpDepartmentIDType	ExternalDepartmentID;

	///外部系统资金账户(外部系统转账时必填)
	TTORATstpAccountIDType	ExternalAccountID;

	///外部系统币种(外部系统转账时必填)
	TTORATstpCurrencyIDType	ExternalCurrencyID;

	///外部系统银行代码(外部系统转账时必填)
	TTORATstpBankIDType	ExternalBankID;

	///外部系统资金密码(外部系统转入时必填)
	TTORATstpPasswordType	ExternalAccountPassword;

	///外部系统银行密码(外部系统转出时必填)
	TTORATstpPasswordType	ExternalBankPassword;

	///外部系统交易密码
	TTORATstpPasswordType	ExternalTradePassword;


};

/// 仓位转移
struct CTORATstpInputTransferPositionField
{
		
	///投资者代码
	TTORATstpInvestorIDType	InvestorID;

	///投资单元代码
	TTORATstpBusinessUnitIDType	BusinessUnitID;

	///交易所代码
	TTORATstpExchangeIDType	ExchangeID;

	///股东账户代码
	TTORATstpShareholderIDType	ShareholderID;

	///证券代码
	TTORATstpSecurityIDType	SecurityID;

	///申请流水号
	TTORATstpExternalSerialType	ApplySerial;

	///转移方向
	TTORATstpTransferDirectionType	TransferDirection;

	///数量
	TTORATstpVolumeType	Volume;

	///转移持仓类型
	TTORATstpTransferPositionTypeType	TransferPositionType;

	///市场代码
	TTORATstpMarketIDType	MarketID;


};

/// 资金转移回报
struct CTORATstpTransferFundField
{
		
	///转移流水号
	TTORATstpSerialType	FundSerial;

	///申请流水号
	TTORATstpExternalSerialType	ApplySerial;

	///前置编号
	TTORATstpFrontIDType	FrontID;

	///会话编号
	TTORATstpSessionIDType	SessionID;

	///资金账户代码
	TTORATstpAccountIDType	AccountID;

	///币种
	TTORATstpCurrencyIDType	CurrencyID;

	///转移方向
	TTORATstpTransferDirectionType	TransferDirection;

	///转移金额
	TTORATstpMoneyType	Amount;

	///转移状态
	TTORATstpTransferStatusType	TransferStatus;

	///操作人员
	TTORATstpUserIDType	OperatorID;

	///操作日期
	TTORATstpDateType	OperateDate;

	///操作时间
	TTORATstpTimeType	OperateTime;

	///一级机构代码
	TTORATstpDepartmentIDType	DepartmentID;

	///签约银行账户
	TTORATstpBankAccountIDType	BankAccountID;

	///银行代码
	TTORATstpBankIDType	BankID;

	///IP地址
	TTORATstpIPAddressType	IPAddress;

	///Mac地址
	TTORATstpMacAddressType	MacAddress;

	///硬盘序列号
	TTORATstpHDSerialType	HDSerial;

	///移动端手机号
	TTORATstpMobileType	Mobile;


};

/// 仓位转移回报
struct CTORATstpTransferPositionField
{
		
	///仓位转移流水号
	TTORATstpSerialType	PositionSerial;

	///申请流水号
	TTORATstpExternalSerialType	ApplySerial;

	///前置编号
	TTORATstpFrontIDType	FrontID;

	///会话编号
	TTORATstpSessionIDType	SessionID;

	///投资者代码
	TTORATstpInvestorIDType	InvestorID;

	///投资单元代码
	TTORATstpBusinessUnitIDType	BusinessUnitID;

	///交易所代码
	TTORATstpExchangeIDType	ExchangeID;

	///股东账户代码
	TTORATstpShareholderIDType	ShareholderID;

	///市场代码
	TTORATstpMarketIDType	MarketID;

	///证券代码
	TTORATstpSecurityIDType	SecurityID;

	///交易日
	TTORATstpDateType	TradingDay;

	///转移方向
	TTORATstpTransferDirectionType	TransferDirection;

	///转移持仓类型
	TTORATstpTransferPositionTypeType	TransferPositionType;

	///昨日持仓数量
	TTORATstpVolumeType	HistoryVolume;

	///今日买卖持仓数量
	TTORATstpVolumeType	TodayBSVolume;

	///今日申赎持仓数量
	TTORATstpVolumeType	TodayPRVolume;

	///转移状态
	TTORATstpTransferStatusType	TransferStatus;

	///操作人员
	TTORATstpUserIDType	OperatorID;

	///操作日期
	TTORATstpDateType	OperateDate;

	///操作时间
	TTORATstpTimeType	OperateTime;

	///IP地址
	TTORATstpIPAddressType	IPAddress;

	///Mac地址
	TTORATstpMacAddressType	MacAddress;

	///硬盘序列号
	TTORATstpHDSerialType	HDSerial;

	///移动端手机号
	TTORATstpMobileType	Mobile;


};

/// 订阅行情
struct CTORATstpSpecificSecurityField
{
		
	///交易所代码
	TTORATstpExchangeIDType	ExchangeID;

	///证券代码
	TTORATstpSecurityIDType	SecurityID;


};

/// 担保品转移请求
struct CTORATstpInputTransferCollateralField
{
		
	///投资者代码
	TTORATstpInvestorIDType	InvestorID;

	///投资单元代码
	TTORATstpBusinessUnitIDType	BusinessUnitID;

	///交易所代码
	TTORATstpExchangeIDType	ExchangeID;

	///股东账户代码
	TTORATstpShareholderIDType	ShareholderID;

	///证券代码
	TTORATstpSecurityIDType	SecurityID;

	///申请流水号
	TTORATstpExternalSerialType	ApplySerial;

	///担保品划转方向
	TTORATstpCollateralDirectionType	CollateralDirection;

	///数量
	TTORATstpVolumeType	Volume;

	///市场代码
	TTORATstpMarketIDType	MarketID;


};

/// 查询银行账户余额请求
struct CTORATstpReqInquiryBankAccountFundField
{
		
	///一级机构代码
	TTORATstpDepartmentIDType	DepartmentID;

	///资金账户代码
	TTORATstpAccountIDType	AccountID;

	///币种
	TTORATstpCurrencyIDType	CurrencyID;

	///银行代码
	TTORATstpBankIDType	BankID;

	///银行密码
	TTORATstpPasswordType	BankPassword;


};

/// 查询银行账户余额响应
struct CTORATstpRspInquiryBankAccountFundField
{
		
	///一级机构代码
	TTORATstpDepartmentIDType	DepartmentID;

	///资金账户代码
	TTORATstpAccountIDType	AccountID;

	///币种
	TTORATstpCurrencyIDType	CurrencyID;

	///银行代码
	TTORATstpBankIDType	BankID;

	///签约银行账户
	TTORATstpBankAccountIDType	BankAccountID;

	///账户余额
	TTORATstpMoneyType	Balance;


};

/// Lev2行情
struct CTORATstpLev2MarketDataField
{
		
	///证券代码
	TTORATstpSecurityIDType	SecurityID;

	///行情交易所代码
	TTORATstpExchangeIDType	ExchangeID;

	///时间戳
	TTORATstpTimeStampType	DataTimeStamp;

	///昨收盘
	TTORATstpPriceType	PreClosePrice;

	///今开盘
	TTORATstpPriceType	OpenPrice;

	///成交笔数
	TTORATstpLongVolumeType	NumTrades;

	///成交总量
	TTORATstpLongVolumeType	TotalVolumeTrade;

	///成交总金额
	TTORATstpMoneyType	TotalValueTrade;

	///委托买入总量（只有上交所推送）
	TTORATstpLongVolumeType	TotalBidVolume;

	///加权平均委托买价格（只有上交所推送）
	TTORATstpPriceType	AvgBidPrice;

	///委托卖出总量（只有上交所推送）
	TTORATstpLongVolumeType	TotalAskVolume;

	///加权平均委托卖价格（只有上交所推送）
	TTORATstpPriceType	AvgAskPrice;

	///最高价
	TTORATstpPriceType	HighestPrice;

	///最低价
	TTORATstpPriceType	LowestPrice;

	///现价
	TTORATstpPriceType	LastPrice;

	///申买价一
	TTORATstpPriceType	BidPrice1;

	///申买量一
	TTORATstpLongVolumeType	BidVolume1;

	///申卖价一
	TTORATstpPriceType	AskPrice1;

	///申卖量一
	TTORATstpLongVolumeType	AskVolume1;

	///申卖价二
	TTORATstpPriceType	AskPrice2;

	///申卖量二
	TTORATstpLongVolumeType	AskVolume2;

	///申卖价三
	TTORATstpPriceType	AskPrice3;

	///申卖量三
	TTORATstpLongVolumeType	AskVolume3;

	///申买价二
	TTORATstpPriceType	BidPrice2;

	///申买量二
	TTORATstpLongVolumeType	BidVolume2;

	///申买价三
	TTORATstpPriceType	BidPrice3;

	///申买量三
	TTORATstpLongVolumeType	BidVolume3;

	///申卖价四
	TTORATstpPriceType	AskPrice4;

	///申卖量四
	TTORATstpLongVolumeType	AskVolume4;

	///申卖价五
	TTORATstpPriceType	AskPrice5;

	///申卖量五
	TTORATstpLongVolumeType	AskVolume5;

	///申买价四
	TTORATstpPriceType	BidPrice4;

	///申买量四
	TTORATstpLongVolumeType	BidVolume4;

	///申买价五
	TTORATstpPriceType	BidPrice5;

	///申买量五
	TTORATstpLongVolumeType	BidVolume5;

	///申卖价六
	TTORATstpPriceType	AskPrice6;

	///申卖量六
	TTORATstpLongVolumeType	AskVolume6;

	///申卖价七
	TTORATstpPriceType	AskPrice7;

	///申卖量七
	TTORATstpLongVolumeType	AskVolume7;

	///申买价六
	TTORATstpPriceType	BidPrice6;

	///申买量六
	TTORATstpLongVolumeType	BidVolume6;

	///申买价七
	TTORATstpPriceType	BidPrice7;

	///申买量七
	TTORATstpLongVolumeType	BidVolume7;

	///申卖价八
	TTORATstpPriceType	AskPrice8;

	///申卖量八
	TTORATstpLongVolumeType	AskVolume8;

	///申卖价九
	TTORATstpPriceType	AskPrice9;

	///申卖量九
	TTORATstpLongVolumeType	AskVolume9;

	///申买价八
	TTORATstpPriceType	BidPrice8;

	///申买量八
	TTORATstpLongVolumeType	BidVolume8;

	///申买价九
	TTORATstpPriceType	BidPrice9;

	///申买量九
	TTORATstpLongVolumeType	BidVolume9;

	///申买价十
	TTORATstpPriceType	BidPrice10;

	///申买量十
	TTORATstpLongVolumeType	BidVolume10;

	///申卖价十
	TTORATstpPriceType	AskPrice10;

	///申卖量十
	TTORATstpLongVolumeType	AskVolume10;

	///附加信息1
	TTORATstpIntInfoType	Info1;

	///附加信息2
	TTORATstpIntInfoType	Info2;

	///附加信息3
	TTORATstpIntInfoType	Info3;

	///涨停板价(只有深圳行情有效)
	TTORATstpPriceType	UpperLimitPrice;

	///跌停板价(只有深圳行情有效)
	TTORATstpPriceType	LowerLimitPrice;

	///今收盘价(只有上海行情有效)
	TTORATstpPriceType	ClosePrice;

	///行情产品实时状态
	TTORATstpMDSecurityStatType	MDSecurityStat;


};

/// Lev2指数行情
struct CTORATstpLev2IndexField
{
		
	///交易所代码
	TTORATstpExchangeIDType	ExchangeID;

	///证券代码
	TTORATstpSecurityIDType	SecurityID;

	///时间戳
	TTORATstpTimeStampType	DataTimeStamp;

	///前收盘指数
	TTORATstpPriceType	PreCloseIndex;

	///今开盘指数
	TTORATstpPriceType	OpenIndex;

	///最高指数
	TTORATstpPriceType	HighIndex;

	///最低指数
	TTORATstpPriceType	LowIndex;

	///最新指数
	TTORATstpPriceType	LastIndex;

	///参与计算相应指数的成交金额（元
	TTORATstpMoneyType	Turnover;

	///参与计算相应指数的交易数量(手)
	TTORATstpLongVolumeType	TotalVolumeTraded;

	///附加信息1
	TTORATstpIntInfoType	Info1;

	///附加信息2
	TTORATstpIntInfoType	Info2;

	///附加信息3
	TTORATstpIntInfoType	Info3;


};

/// Lev2逐笔成交
struct CTORATstpLev2TransactionField
{
		
	///交易所代码
	TTORATstpExchangeIDType	ExchangeID;

	///证券代码
	TTORATstpSecurityIDType	SecurityID;

	///时间戳
	TTORATstpTimeStampType	TradeTime;

	///成交价格
	TTORATstpPriceType	TradePrice;

	///成交数量
	TTORATstpLongVolumeType	TradeVolume;

	///成交类别
	TTORATstpExecTypeType	ExecType;

	///主序号
	TTORATstpSequenceNoType	MainSeq;

	///子序号
	TTORATstpLongSequenceType	SubSeq;

	///买方委托序号
	TTORATstpLongSequenceType	BuyNo;

	///卖方委托序号
	TTORATstpLongSequenceType	SellNo;

	///附加信息1
	TTORATstpIntInfoType	Info1;

	///附加信息2
	TTORATstpIntInfoType	Info2;

	///附加信息3
	TTORATstpIntInfoType	Info3;


};

/// Lev2逐笔委托
struct CTORATstpLev2OrderDetailField
{
		
	///交易所代码
	TTORATstpExchangeIDType	ExchangeID;

	///证券代码
	TTORATstpSecurityIDType	SecurityID;

	///时间戳
	TTORATstpTimeStampType	OrderTime;

	///委托价格
	TTORATstpPriceType	Price;

	///委托数量
	TTORATstpLongVolumeType	Volume;

	///委托方向
	TTORATstpLSideType	Side;

	///订单类别
	TTORATstpLOrderTypeType	OrderType;

	///主序号
	TTORATstpSequenceNoType	MainSeq;

	///子序号
	TTORATstpSequenceNoType	SubSeq;

	///附加信息1
	TTORATstpIntInfoType	Info1;

	///附加信息2
	TTORATstpIntInfoType	Info2;

	///附加信息3
	TTORATstpIntInfoType	Info3;


};

/// Lev2盘后定价行情
struct CTORATstpLev2PHMarketDataField
{
		
	///证券代码
	TTORATstpSecurityIDType	SecurityID;

	///行情交易所代码
	TTORATstpExchangeIDType	ExchangeID;

	///时间戳
	TTORATstpTimeStampType	DataTimeStamp;

	///今收盘价(只有上海行情有效)
	TTORATstpPriceType	ClosePrice;

	///行情产品实时状态
	TTORATstpMDSecurityStatType	MDSecurityStat;

	///成交笔数
	TTORATstpLongVolumeType	NumTrades;

	///成交总量
	TTORATstpLongVolumeType	TotalVolumeTrade;

	///成交总金额
	TTORATstpMoneyType	TotalValueTrade;

	///委托买入总量
	TTORATstpLongVolumeType	TotalBidVolume;

	///委托卖出总量
	TTORATstpLongVolumeType	TotalAskVolume;

	///买入撤单笔数
	TTORATstpLongVolumeType	WithdrawBuyNumber;

	///买入撤单数量
	TTORATstpLongVolumeType	WithdrawBuyAmount;

	///卖出撤单笔数
	TTORATstpLongVolumeType	WithdrawSellNumber;

	///卖出撤单数量
	TTORATstpLongVolumeType	WithdrawSellAmount;

	///申买量1
	TTORATstpLongVolumeType	BidOrderQty;

	///实际的买委托笔数1
	TTORATstpLongVolumeType	BidNumOrders;

	///申卖量1
	TTORATstpLongVolumeType	AskOrderQty;

	///实际的卖委托笔数1
	TTORATstpLongVolumeType	AskNumOrders;

	///附加信息1
	TTORATstpIntInfoType	Info1;

	///附加信息2
	TTORATstpIntInfoType	Info2;

	///附加信息3
	TTORATstpIntInfoType	Info3;


};

/// Lev2盘后定价逐笔成交
struct CTORATstpLev2PHTransactionField
{
		
	///交易所代码
	TTORATstpExchangeIDType	ExchangeID;

	///证券代码
	TTORATstpSecurityIDType	SecurityID;

	///时间戳
	TTORATstpTimeStampType	TradeTime;

	///成交价格
	TTORATstpPriceType	TradePrice;

	///成交数量
	TTORATstpLongVolumeType	TradeVolume;

	///成交金额(元)
	TTORATstpMoneyType	TradeMoney;

	///成交类别
	TTORATstpExecTypeType	ExecType;

	///主序号
	TTORATstpSequenceNoType	MainSeq;

	///子序号
	TTORATstpLongSequenceType	SubSeq;

	///买方委托序号
	TTORATstpLongSequenceType	BuyNo;

	///卖方委托序号
	TTORATstpLongSequenceType	SellNo;

	///附加信息1
	TTORATstpIntInfoType	Info1;

	///附加信息2
	TTORATstpIntInfoType	Info2;

	///附加信息3
	TTORATstpIntInfoType	Info3;


};

/// 回传交易数据
struct CTORATstpUploadTradeDataField
{
		
	///交易所代码
	TTORATstpExchangeIDType	ExchangeID;

	///交易日
	TTORATstpDateType	TradingDay;

	///是否强制回传
	TTORATstpBoolType	bForce;


};

/// 输入指定交易登记&撤销
struct CTORATstpInputDesignationRegistrationField
{
		
	///投资者代码
	TTORATstpInvestorIDType	InvestorID;

	///用户代码
	TTORATstpUserIDType	UserID;

	///指定交易操作类型
	TTORATstpDesignationTypeType	DesignationType;

	///股东账户代码
	TTORATstpShareholderIDType	ShareholderID;

	///投资单元代码
	TTORATstpBusinessUnitIDType	BusinessUnitID;

	///资金账户代码
	TTORATstpAccountIDType	AccountID;

	///系统报单编号
	TTORATstpOrderSysIDType	OrderSysID;

	///IP地址
	TTORATstpIPAddressType	IPAddress;

	///Mac地址
	TTORATstpMacAddressType	MacAddress;

	///终端信息
	TTORATstpTerminalInfoType	TerminalInfo;

	///硬盘序列号
	TTORATstpHDSerialType	HDSerial;

	///移动设备手机号
	TTORATstpMobileType	Mobile;


};

/// 输入深证转托管
struct CTORATstpInputCustodyTransferField
{
		
	///投资者代码
	TTORATstpInvestorIDType	InvestorID;

	///用户代码
	TTORATstpUserIDType	UserID;

	///转托管类型
	TTORATstpCustodyTransferTypeType	CustodyTransferType;

	///股东账户代码
	TTORATstpShareholderIDType	ShareholderID;

	///投资单元代码
	TTORATstpBusinessUnitIDType	BusinessUnitID;

	///资金账户代码
	TTORATstpAccountIDType	AccountID;

	///系统报单编号
	TTORATstpOrderSysIDType	OrderSysID;

	///转入交易单元代码
	TTORATstpPbuIDType	TransfereePbuID;

	///证券代码
	TTORATstpSecurityIDType	SecurityID;

	///原始转托管本地报单编号
	TTORATstpOrderLocalIDType	OrignalOrderLocalID;

	///本地报单编号
	TTORATstpOrderLocalIDType	OrderLocalID;

	///数量
	TTORATstpVolumeType	VolumeTotalOriginal;

	///IP地址
	TTORATstpIPAddressType	IPAddress;

	///Mac地址
	TTORATstpMacAddressType	MacAddress;

	///终端信息
	TTORATstpTerminalInfoType	TerminalInfo;

	///硬盘序列号
	TTORATstpHDSerialType	HDSerial;

	///移动设备手机号
	TTORATstpMobileType	Mobile;


};

/// 交易成交集中度
struct CTORATstpInquiryTradeConcentrationField
{
		
	///交易所代码
	TTORATstpExchangeIDType	ExchangeID;

	///市场代码
	TTORATstpMarketIDType	MarketID;

	///证券代码
	TTORATstpSecurityIDType	SecurityID;

	///投资者代码
	TTORATstpInvestorIDType	InvestorID;

	///投资单元代码
	TTORATstpBusinessUnitIDType	BusinessUnitID;

	///资金账户代码
	TTORATstpAccountIDType	AccountID;

	///股东账户代码
	TTORATstpShareholderIDType	ShareholderID;

	///投资者成交量/成交总量
	TTORATstpRatioType	ConcentrationRatio1;

	///投资者成交金额/成交总金额
	TTORATstpRatioType	ConcentrationRatio2;


};

/// 特别行情通知
struct CTORATstpSpecialMarketDataField
{
		
	///交易日
	TTORATstpDateType	TradingDay;

	///证券代码
	TTORATstpSecurityIDType	SecurityID;

	///交易所代码
	TTORATstpExchangeIDType	ExchangeID;

	///证券名称
	TTORATstpSecurityNameType	SecurityName;

	///移动平均价
	TTORATstpPriceType	MovingAvgPrice;

	///计算移动平均价的采样数量
	TTORATstpVolumeType	MovingAvgPriceSamplingNum;

	///最后修改时间
	TTORATstpTimeType	UpdateTime;

	///最后修改毫秒
	TTORATstpMillisecType	UpdateMillisec;


};

/// 价格异常波动行情
struct CTORATstpEffectPriceMarketDataField
{
		
	///交易所代码
	TTORATstpExchangeIDType	ExchangeID;

	///证券代码
	TTORATstpSecurityIDType	SecurityID;

	///最新价
	TTORATstpPriceType	LastPrice;

	///当前成交数量
	TTORATstpLongVolumeType	TradeVol;

	///当前成交金额
	TTORATstpMoneyType	TradeTurnover;

	///最后修改时间
	TTORATstpTimeType	UpdateTime;

	///最后修改毫秒
	TTORATstpMillisecType	UpdateMillisec;


};

/// 数量异常波动行情
struct CTORATstpEffectVolumeMarketDataField
{
		
	///交易所代码
	TTORATstpExchangeIDType	ExchangeID;

	///证券代码
	TTORATstpSecurityIDType	SecurityID;

	///最新价
	TTORATstpPriceType	LastPrice;

	///当前成交数量
	TTORATstpLongVolumeType	TradeVol;

	///当前成交金额
	TTORATstpMoneyType	TradeTurnover;

	///最后修改时间
	TTORATstpTimeType	UpdateTime;

	///最后修改毫秒
	TTORATstpMillisecType	UpdateMillisec;


};

/// 资金流向数据行情
struct CTORATstpFundsFlowMarketDataField
{
		
	///交易所代码
	TTORATstpExchangeIDType	ExchangeID;

	///证券代码
	TTORATstpSecurityIDType	SecurityID;

	///最后修改时间
	TTORATstpTimeType	UpdateTime;

	///最后修改毫秒
	TTORATstpMillisecType	UpdateMillisec;

	///散户买入金额
	TTORATstpMoneyType	RetailBuyTurnover;

	///散户买入数量
	TTORATstpLongVolumeType	RetailBuyVolume;

	///散户买入笔数
	TTORATstpLongVolumeType	RetailBuyAmount;

	///散户卖出金额
	TTORATstpMoneyType	RetailSellTurnover;

	///散户卖出数量
	TTORATstpLongVolumeType	RetailSellVolume;

	///散户卖出笔数
	TTORATstpLongVolumeType	RetailSellAmount;

	///中户买入金额
	TTORATstpMoneyType	MiddleBuyTurnover;

	///中户买入数量
	TTORATstpLongVolumeType	MiddleBuyVolume;

	///中户买入笔数
	TTORATstpLongVolumeType	MiddleBuyAmount;

	///中户卖出金额
	TTORATstpMoneyType	MiddleSellTurnover;

	///中户卖出数量
	TTORATstpLongVolumeType	MiddleSellVolume;

	///中户卖出笔数
	TTORATstpLongVolumeType	MiddleSellAmount;

	///大户买入金额
	TTORATstpMoneyType	LargeBuyTurnover;

	///大户买入数量
	TTORATstpLongVolumeType	LargeBuyVolume;

	///大户买入笔数
	TTORATstpLongVolumeType	LargeBuyAmount;

	///大户卖出金额
	TTORATstpMoneyType	LargeSellTurnover;

	///大户卖出数量
	TTORATstpLongVolumeType	LargeSellVolume;

	///大户卖出笔数
	TTORATstpLongVolumeType	LargeSellAmount;

	///机构买入金额
	TTORATstpMoneyType	InstitutionBuyTurnover;

	///机构买入数量
	TTORATstpLongVolumeType	InstitutionBuyVolume;

	///机构买入笔数
	TTORATstpLongVolumeType	InstitutionBuyAmount;

	///机构卖出金额
	TTORATstpMoneyType	InstitutionSellTurnover;

	///机构卖出数量
	TTORATstpLongVolumeType	InstitutionSellVolume;

	///机构卖出笔数
	TTORATstpLongVolumeType	InstitutionSellAmount;


};

/// 查询复权信息
struct CTORATstpQryRightsAdjustmentInfoField
{
		
	///起始日期
	TTORATstpDateType	BegDate;

	///结束日期
	TTORATstpDateType	EndDate;

	///交易所代码
	TTORATstpExchangeIDType	ExchangeID;

	///证券代码
	TTORATstpSecurityIDType	SecurityID;

	///每页记录数
	TTORATstpVolumeType	PageCount;

	///页定位符
	TTORATstpPageLocateType	PageLocate;


};

/// 查询历史响应信息
struct CTORATstpQryHistoryRspInfoField
{
		
	///错误代码
	TTORATstpErrorIDType	ErrorID;

	///错误信息
	TTORATstpErrorMsgType	ErrorMsg;

	///分页是否结束
	TTORATstpBoolType	bPageEnd;

	///总查询结果是否结束
	TTORATstpBoolType	bResultEnd;


};

/// 复权数据
struct CTORATstpRightsAdjustmentDataField
{
		
	///交易日期
	TTORATstpDateType	TradingDay;

	///复权昨收盘价(元)
	TTORATstpPriceType	ADJPreClose;

	///复权开盘价(元)
	TTORATstpPriceType	ADJOpen;

	///复权最高价(元)
	TTORATstpPriceType	ADJHigh;

	///复权最低价(元)
	TTORATstpPriceType	ADJLow;

	///复权收盘价(元)
	TTORATstpPriceType	ADJClose;

	///复权因子
	TTORATstpPriceType	ADJFactor;

	///交易所代码
	TTORATstpExchangeIDType	ExchangeID;

	///证券代码
	TTORATstpSecurityIDType	SecurityID;

	///页定位符
	TTORATstpPageLocateType	PageLocate;


};

/// 查询历史资金流向数据
struct CTORATstpQryHistoryFundsFlowInfoField
{
		
	///起始日期
	TTORATstpDateType	BegDate;

	///结束日期
	TTORATstpDateType	EndDate;

	///交易所代码
	TTORATstpExchangeIDType	ExchangeID;

	///证券代码
	TTORATstpSecurityIDType	SecurityID;

	///每页记录数
	TTORATstpVolumeType	PageCount;

	///页定位符
	TTORATstpPageLocateType	PageLocate;


};

/// 复权历史资金流向数据
struct CTORATstpHistoryFundsFlowDataField
{
		
	///交易日期
	TTORATstpDateType	TradingDay;

	///机构买入金额(万元)
	TTORATstpMoneyType	BuyValueExlargeOrder;

	///机构卖出金额(万元)
	TTORATstpMoneyType	SellValueExlargeOrder;

	///大户买入金额(万元)
	TTORATstpMoneyType	BuyValueLargeOrder;

	///大户卖出金额(万元)
	TTORATstpMoneyType	SellValueLargeOrder;

	///中户买入金额(万元)
	TTORATstpMoneyType	BuyValueMedOrder;

	///中户卖出金额(万元)
	TTORATstpMoneyType	SellValueMedOrder;

	///散户买入金额(万元)
	TTORATstpMoneyType	BuyValueSmallOrder;

	///散户卖出金额(万元)
	TTORATstpMoneyType	SellValueSmallOrder;

	///机构买入总量(手)
	TTORATstpQuantityType	BuyVolumeExlargeOrder;

	///机构卖出总量(手)
	TTORATstpQuantityType	SellVolumeExlargeOrder;

	///大户买入总量(手)
	TTORATstpQuantityType	BuyVolumeLargeOrder;

	///大户卖出总量(手)
	TTORATstpQuantityType	SellVolumeLargeOrder;

	///中户买入总量(手)
	TTORATstpQuantityType	BuyVolumeMedOrder;

	///中户卖出总量(手)
	TTORATstpQuantityType	SellVolumeMedOrder;

	///散户买入总量(手)
	TTORATstpQuantityType	BuyVolumeSmallOrder;

	///散户卖出总量(手)
	TTORATstpQuantityType	SellVolumeSmallOrder;

	///交易所代码
	TTORATstpExchangeIDType	ExchangeID;

	///证券代码
	TTORATstpSecurityIDType	SecurityID;

	///页定位符
	TTORATstpPageLocateType	PageLocate;


};

/// 查询财务指标信息
struct CTORATstpQryFinancialIndicatorInfoField
{
		
	///交易所代码
	TTORATstpExchangeIDType	ExchangeID;

	///证券代码
	TTORATstpSecurityIDType	SecurityID;

	///每页记录数
	TTORATstpVolumeType	PageCount;

	///页定位符
	TTORATstpPageLocateType	PageLocate;


};

/// 财务指标数据
struct CTORATstpFinancialIndicatorDataField
{
		
	///公告日期
	TTORATstpDateType	AnnouncementDate;

	///基本每股收益
	TTORATstpPriceType	EPSBasic;

	///每股净资产
	TTORATstpPriceType	BPS;

	///每股资本公积
	TTORATstpPriceType	SurplusCapitalPS;

	///每股未分配利润
	TTORATstpPriceType	UndistributedPS;

	///每股经营活动产生的现金流量净额
	TTORATstpPriceType	OCFPS;

	///每股营业收入
	TTORATstpPriceType	ORPS;

	///交易所代码
	TTORATstpExchangeIDType	ExchangeID;

	///证券代码
	TTORATstpSecurityIDType	SecurityID;

	///页定位符
	TTORATstpPageLocateType	PageLocate;


};

/// 查询分红信息
struct CTORATstpQryDividendInfoField
{
		
	///交易所代码
	TTORATstpExchangeIDType	ExchangeID;

	///证券代码
	TTORATstpSecurityIDType	SecurityID;

	///每页记录数
	TTORATstpVolumeType	PageCount;

	///页定位符
	TTORATstpPageLocateType	PageLocate;


};

/// 分红数据
struct CTORATstpDividendDataField
{
		
	///最新公告日期
	TTORATstpDateType	AnnouncementDate;

	///方案进度
	TTORATstpProgressType	Progress;

	///除权除息日
	TTORATstpDateType	ExDate;

	///每股送转
	TTORATstpShareType	STKDvdPerSh;

	///每股派息(税前)(元)
	TTORATstpCashShareType	CashDvdPerShPreTax;

	///每股派息(税后)(元)
	TTORATstpCashShareType	CashDvdPerShAfterTax;

	///股权登记日
	TTORATstpDateType	EqyRecordDate;

	///派息日
	TTORATstpDateType	DvdPayoutDate;

	///红股上市日
	TTORATstpDateType	ListingDateOfDvdShr;

	///预案公告日
	TTORATstpDateType	PrelanDate;

	///股东大会公告日
	TTORATstpDateType	SMTGDate;

	///分红实施公告日
	TTORATstpDateType	DvdAnnDate;

	///基准日期
	TTORATstpDateType	BaseDate;

	///基准股本(万股)
	TTORATstpQuantityType	BaseShare;

	///货币代码
	TTORATstpCodeType	CrncyCode;

	///方案是否变更
	TTORATstpIsChangedType	IsChanged;

	///分红年度
	TTORATstpDateType	ReportPeriod;

	///方案变更说明
	TTORATstpChangeContentType	Change;

	///每股送股比例
	TTORATstpShareType	BonusRate;

	///每股转增比例
	TTORATstpShareType	ConversedRate;

	///备注
	TTORATstpMemoType	Memo;

	///预案预披露公告日
	TTORATstpDateType	PreAnnDate;

	///分红对象
	TTORATstpObjectType	DivObject;

	///交易所代码
	TTORATstpExchangeIDType	ExchangeID;

	///证券代码
	TTORATstpSecurityIDType	SecurityID;

	///页定位符
	TTORATstpPageLocateType	PageLocate;


};

/// 查询送股配股信息
struct CTORATstpQryRightIssueInfoField
{
		
	///交易所代码
	TTORATstpExchangeIDType	ExchangeID;

	///证券代码
	TTORATstpSecurityIDType	SecurityID;

	///每页记录数
	TTORATstpVolumeType	PageCount;

	///页定位符
	TTORATstpPageLocateType	PageLocate;


};

/// 送股配股数据
struct CTORATstpRightIssueDataField
{
		
	///最新公告日期
	TTORATstpDateType	AnnouncementDate;

	///方案进度
	TTORATstpProgressType	Progress;

	///配股价格(元)
	TTORATstpDvdPriceType	Price;

	///配股比例
	TTORATstpRatioShareType	Ratio;

	///配股计划数量(万股)
	TTORATstpQuantityType	Amount;

	///配股实际数量(万股)
	TTORATstpQuantityType	AmountAct;

	///募集资金(元)
	TTORATstpDvdPriceType	NetCollection;

	///股权登记日
	TTORATstpDateType	RegDateShare;

	///除权日
	TTORATstpDateType	ExDividendDate;

	///配股上市日
	TTORATstpDateType	ListedDate;

	///缴款起始日
	TTORATstpDateType	PayStartDate;

	///缴款终止日
	TTORATstpDateType	PayEndDate;

	///预案公告日
	TTORATstpDateType	PrePlanDate;

	///股东大会公告日
	TTORATstpDateType	SMTGAnnceDate;

	///发审委通过公告日
	TTORATstpDateType	PassDate;

	///证监会核准公告日
	TTORATstpDateType	ApprovedDate;

	///配股实施公告日
	TTORATstpDateType	AnnceDate;

	///配股结果公告日
	TTORATstpDateType	ResultDate;

	///上市公告日
	TTORATstpDateType	ListAnnDate;

	///基准年度
	TTORATstpDateType	Guarantor;

	///基准股本(万股)
	TTORATstpQuantityType	Guartype;

	///配售代码
	TTORATstpCodeType	Code;

	///配股年度
	TTORATstpDateType	Year;

	///配股说明
	TTORATstpShareContentType	Content;

	///配股简称
	TTORATstpDvdNameType	Name;

	///配股比例分母
	TTORATstpRatioShareType	RatioDenominator;

	///配股比例分子
	TTORATstpRatioShareType	RatioMolecular;

	///认购方式
	TTORATstpSubscriptionMethodType	SubscriptionMethod;

	///预计募集资金(元)
	TTORATstpDvdPriceType	ExpectedFundRaising;

	///交易所代码
	TTORATstpExchangeIDType	ExchangeID;

	///证券代码
	TTORATstpSecurityIDType	SecurityID;

	///页定位符
	TTORATstpPageLocateType	PageLocate;


};

/// 查询公司资料信息
struct CTORATstpQryCompanyDescriptionInfoField
{
		
	///交易所代码
	TTORATstpExchangeIDType	ExchangeID;

	///证券代码
	TTORATstpSecurityIDType	SecurityID;

	///每页记录数
	TTORATstpVolumeType	PageCount;

	///页定位符
	TTORATstpPageLocateType	PageLocate;


};

/// 公司资料数据
struct CTORATstpCompanyDescriptionDataField
{
		
	///公司中文名称
	TTORATstpCompNameType	CompName;

	///上市日期
	TTORATstpDateType	ListDate;

	///经营范围
	TTORATstpContentType	BusinessScope;

	///发行价格(元)
	TTORATstpIPOPriceType	IPOPrice;

	///公开及老股发行数量合计(万股)
	TTORATstpQuantityType	IPOAmount;

	///网上发行数量(万股)
	TTORATstpShareType	AMTByPlacing;

	///网下发行数量(万股)
	TTORATstpShareType	AMTToJur;

	///交易所代码
	TTORATstpExchangeIDType	ExchangeID;

	///证券代码
	TTORATstpSecurityIDType	SecurityID;

	///页定位符
	TTORATstpPageLocateType	PageLocate;


};

/// 查询股本结构信息
struct CTORATstpQryEquityStructureInfoField
{
		
	///交易所代码
	TTORATstpExchangeIDType	ExchangeID;

	///证券代码
	TTORATstpSecurityIDType	SecurityID;

	///每页记录数
	TTORATstpVolumeType	PageCount;

	///页定位符
	TTORATstpPageLocateType	PageLocate;


};

/// 股本结构数据
struct CTORATstpEquityStructureDataField
{
		
	///交易日期
	TTORATstpDateType	TradingDay;

	///当日总股本
	TTORATstpShareType	TotalShareToday;

	///当日流通股本
	TTORATstpShareType	FloatShareToday;

	///股东总户数
	TTORATstpHolderNumType	HolderTotalNum;

	///最新公告日期
	TTORATstpDateType	AnnouncementDate;

	///交易所代码
	TTORATstpExchangeIDType	ExchangeID;

	///证券代码
	TTORATstpSecurityIDType	SecurityID;

	///页定位符
	TTORATstpPageLocateType	PageLocate;


};

/// 查询主营业务信息
struct CTORATstpQrySalesSegmentInfoField
{
		
	///交易所代码
	TTORATstpExchangeIDType	ExchangeID;

	///证券代码
	TTORATstpSecurityIDType	SecurityID;

	///每页记录数
	TTORATstpVolumeType	PageCount;

	///页定位符
	TTORATstpPageLocateType	PageLocate;


};

/// 主营业务数据
struct CTORATstpSalesSegmentDataField
{
		
	///报告期
	TTORATstpDateType	ReportPeriod;

	///货币代码
	TTORATstpCodeType	CrncyCode;

	///项目类别
	TTORATstpItemCodeType	ItemCode;

	///主营业务项目
	TTORATstpItemType	Item;

	///主营业务收入(元)
	TTORATstpPriceType	Sales;

	///主营业务利润(元)
	TTORATstpPriceType	Profit;

	///主营业务成本(元)
	TTORATstpPriceType	Cost;

	///是否公布值
	TTORATstpIsChangedType	IsPublishedValue;

	///交易所代码
	TTORATstpExchangeIDType	ExchangeID;

	///证券代码
	TTORATstpSecurityIDType	SecurityID;

	///页定位符
	TTORATstpPageLocateType	PageLocate;


};

/// 查询十大股东信息
struct CTORATstpQryTopTenHoldersInfoField
{
		
	///交易所代码
	TTORATstpExchangeIDType	ExchangeID;

	///证券代码
	TTORATstpSecurityIDType	SecurityID;

	///每页记录数
	TTORATstpVolumeType	PageCount;

	///页定位符
	TTORATstpPageLocateType	PageLocate;


};

/// 十大股东数据
struct CTORATstpTopTenHoldersDataField
{
		
	///最新公告日期
	TTORATstpDateType	AnnouncementDate;

	///股东名称
	TTORATstpHolderNameType	HolderName;

	///持股数量
	TTORATstpQuantityType	HolderQuantity;

	///交易所代码
	TTORATstpExchangeIDType	ExchangeID;

	///证券代码
	TTORATstpSecurityIDType	SecurityID;

	///页定位符
	TTORATstpPageLocateType	PageLocate;


};

/// 查询十大流通股东信息
struct CTORATstpQryTopTenFloatHoldersInfoField
{
		
	///交易所代码
	TTORATstpExchangeIDType	ExchangeID;

	///证券代码
	TTORATstpSecurityIDType	SecurityID;

	///每页记录数
	TTORATstpVolumeType	PageCount;

	///页定位符
	TTORATstpPageLocateType	PageLocate;


};

/// 十大流通股东数据
struct CTORATstpTopTenFloatHoldersDataField
{
		
	///最新公告日期
	TTORATstpDateType	AnnouncementDate;

	///股东名称
	TTORATstpFloatHolderNameType	HolderName;

	///持股数量
	TTORATstpQuantityType	HolderQuantity;

	///交易所代码
	TTORATstpExchangeIDType	ExchangeID;

	///证券代码
	TTORATstpSecurityIDType	SecurityID;

	///页定位符
	TTORATstpPageLocateType	PageLocate;


};

/// 查询个股所属行业板块信息
struct CTORATstpQryIndustryInfoField
{
		
	///交易所代码
	TTORATstpExchangeIDType	ExchangeID;

	///证券代码
	TTORATstpSecurityIDType	SecurityID;

	///每页记录数
	TTORATstpVolumeType	PageCount;

	///页定位符
	TTORATstpPageLocateType	PageLocate;


};

/// 个股所属行业板块数据
struct CTORATstpIndustryDataField
{
		
	///纳入日期
	TTORATstpDateType	EntryDate;

	///剔除日期
	TTORATstpDateType	RemoveDate;

	///行业代码
	TTORATstpIndustriesCodeType	IndustriesCode;

	///行业名称
	TTORATstpIndustriesNameType	IndustriesName;

	///级数
	TTORATstpLevelNumType	LevelNum;

	///是否有效
	TTORATstpUsedType	Used;

	///板块别名
	TTORATstpIndustriesAliasType	IndustriesAlias;

	///展示序号
	TTORATstpSequenceType	Sequence;

	///备注
	TTORATstpIndustriesMemoType	Memo;

	///板块中文定义
	TTORATstpChineseDfinitionType	ChineseDfinition;

	///板块英文名称
	TTORATstpIndustriesNameEngType	IndustriesNameEng;

	///行业对应指数代码
	TTORATstpWindCodeType	IndexCode;

	///行业对应指数名称
	TTORATstpIndustriesNameType	Name;

	///交易所代码
	TTORATstpExchangeIDType	ExchangeID;

	///证券代码
	TTORATstpSecurityIDType	SecurityID;

	///页定位符
	TTORATstpPageLocateType	PageLocate;


};

/// 查询个股所属概念板块信息
struct CTORATstpQryConceptionInfoField
{
		
	///交易所代码
	TTORATstpExchangeIDType	ExchangeID;

	///证券代码
	TTORATstpSecurityIDType	SecurityID;

	///每页记录数
	TTORATstpVolumeType	PageCount;

	///页定位符
	TTORATstpPageLocateType	PageLocate;


};

/// 个股所属概念板块数据
struct CTORATstpConceptionDataField
{
		
	///Wind概念板块代码
	TTORATstpConceptionCodeType	ConceptionCode;

	///Wind概念板块名称
	TTORATstpConceptionNameType	ConceptionName;

	///纳入日期
	TTORATstpDateType	EntryDate;

	///剔除日期
	TTORATstpDateType	RemoveDate;

	///最新标志
	TTORATstpCurSignType	CurSign;

	///行业代码
	TTORATstpIndustriesCodeType	IndustriesCode;

	///行业名称
	TTORATstpIndustriesNameType	IndustriesName;

	///级数
	TTORATstpLevelNumType	LevelNum;

	///是否有效
	TTORATstpUsedType	Used;

	///板块别名
	TTORATstpIndustriesAliasType	IndustriesAlias;

	///展示序号
	TTORATstpSequenceType	Sequence;

	///备注
	TTORATstpIndustriesMemoType	Memo;

	///板块中文定义
	TTORATstpChineseDfinitionType	ChineseDfinition;

	///板块英文名称
	TTORATstpIndustriesNameEngType	IndustriesNameEng;

	///行业对应指数代码
	TTORATstpWindCodeType	IndexCode;

	///行业对应指数名称
	TTORATstpIndustriesNameType	Name;

	///交易所代码
	TTORATstpExchangeIDType	ExchangeID;

	///证券代码
	TTORATstpSecurityIDType	SecurityID;

	///页定位符
	TTORATstpPageLocateType	PageLocate;


};

/// 查询个股所属地域板块信息
struct CTORATstpQryRegionInfoField
{
		
	///交易所代码
	TTORATstpExchangeIDType	ExchangeID;

	///证券代码
	TTORATstpSecurityIDType	SecurityID;

	///每页记录数
	TTORATstpVolumeType	PageCount;

	///页定位符
	TTORATstpPageLocateType	PageLocate;


};

/// 个股所属地域板块数据
struct CTORATstpRegionDataField
{
		
	///行业代码
	TTORATstpIndustriesCodeType	IndustriesCode;

	///行业名称
	TTORATstpIndustriesNameType	IndustriesName;

	///级数
	TTORATstpLevelNumType	LevelNum;

	///是否有效
	TTORATstpUsedType	Used;

	///板块别名
	TTORATstpIndustriesAliasType	IndustriesAlias;

	///展示序号
	TTORATstpSequenceType	Sequence;

	///备注
	TTORATstpIndustriesMemoType	Memo;

	///板块中文定义
	TTORATstpChineseDfinitionType	ChineseDfinition;

	///板块英文名称
	TTORATstpIndustriesNameEngType	IndustriesNameEng;

	///交易所代码
	TTORATstpExchangeIDType	ExchangeID;

	///证券代码
	TTORATstpSecurityIDType	SecurityID;

	///页定位符
	TTORATstpPageLocateType	PageLocate;


};

/// 查询指数描述信息
struct CTORATstpQryIndexDescriptionInfoField
{
		
	///指数代码
	TTORATstpIndexIDType	IndexID;

	///每页记录数
	TTORATstpVolumeType	PageCount;

	///页定位符
	TTORATstpPageLocateType	PageLocate;


};

/// 指数描述信息数据
struct CTORATstpIndexDescriptionDataField
{
		
	///指数简称
	TTORATstpIndustriesNameType	Name;

	///板块代码
	TTORATstpIndustryCodeType	IndustryCode;

	///板块名称
	TTORATstpIndustriesNameType	IndustryName;

	///板块代码2
	TTORATstpIndustryCodeType	IndustryCode2;

	///板块英文名称
	TTORATstpIndustriesNameEngType	IndustryNameEng;

	///指数代码
	TTORATstpIndexIDType	IndexID;

	///页定位符
	TTORATstpPageLocateType	PageLocate;


};

/// 查询行业板块成分股信息
struct CTORATstpQryIndustryConstituentsInfoField
{
		
	///指数代码
	TTORATstpIndexIDType	IndexID;

	///每页记录数
	TTORATstpVolumeType	PageCount;

	///页定位符
	TTORATstpPageLocateType	PageLocate;


};

/// 行业板块成分股数据
struct CTORATstpIndustryConstituentsDataField
{
		
	///Wind行业代码
	TTORATstpIndustriesNameType	WindIndCode;

	///纳入日期
	TTORATstpDateType	EntryDate;

	///剔除日期
	TTORATstpDateType	RemoveDate;

	///最新标志
	TTORATstpCurSignType	CurSign;

	///交易所代码
	TTORATstpExchangeIDType	ExchangeID;

	///证券代码
	TTORATstpSecurityIDType	SecurityID;

	///页定位符
	TTORATstpPageLocateType	PageLocate;


};

/// 查询概念板块成分股信息
struct CTORATstpQryConceptionConstituentsInfoField
{
		
	///指数代码
	TTORATstpIndexIDType	IndexID;

	///每页记录数
	TTORATstpVolumeType	PageCount;

	///页定位符
	TTORATstpPageLocateType	PageLocate;


};

/// 概念板块成分股数据
struct CTORATstpConceptionConstituentsDataField
{
		
	///Wind概念代码
	TTORATstpIndustriesNameType	WindSecCode;

	///纳入日期
	TTORATstpDateType	EntryDate;

	///剔除日期
	TTORATstpDateType	RemoveDate;

	///最新标志
	TTORATstpCurSignType	CurSign;

	///交易所代码
	TTORATstpExchangeIDType	ExchangeID;

	///证券代码
	TTORATstpSecurityIDType	SecurityID;

	///页定位符
	TTORATstpPageLocateType	PageLocate;


};

/// 查询板块指数实时行情信息
struct CTORATstpQryIndustryIndexInfoField
{
		
	///前一交易日日期
	TTORATstpDateType	PreTradingDay;

	///指数代码
	TTORATstpIndexIDType	IndexID;

	///每页记录数
	TTORATstpVolumeType	PageCount;

	///页定位符
	TTORATstpPageLocateType	PageLocate;


};

/// 板块指数实时行情数据
struct CTORATstpIndustryIndexDataField
{
		
	///Wind板块指数行情点位
	TTORATstpPriceType	IndexPoint;

	///指数代码
	TTORATstpIndexIDType	IndexID;

	///页定位符
	TTORATstpPageLocateType	PageLocate;


};

/// 查询概念指数实时行情信息
struct CTORATstpQryConceptionIndexInfoField
{
		
	///前一交易日日期
	TTORATstpDateType	PreTradingDay;

	///指数代码
	TTORATstpIndexIDType	IndexID;

	///每页记录数
	TTORATstpVolumeType	PageCount;

	///页定位符
	TTORATstpPageLocateType	PageLocate;


};

/// 概念指数实时行情数据
struct CTORATstpConceptionIndexDataField
{
		
	///Wind板块指数行情点位
	TTORATstpPriceType	IndexPoint;

	///指数代码
	TTORATstpIndexIDType	IndexID;

	///页定位符
	TTORATstpPageLocateType	PageLocate;


};

/// 查询自由流通股本信息
struct CTORATstpQryFreeFloatSharesInfoField
{
		
	///起始日期
	TTORATstpDateType	BegDate;

	///结束日期
	TTORATstpDateType	EndDate;

	///交易所代码
	TTORATstpExchangeIDType	ExchangeID;

	///证券代码
	TTORATstpSecurityIDType	SecurityID;

	///每页记录数
	TTORATstpVolumeType	PageCount;

	///页定位符
	TTORATstpPageLocateType	PageLocate;


};

/// 自由流通股本数据
struct CTORATstpFreeFloatSharesDataField
{
		
	///自由流通股本(万股)
	TTORATstpShareType	FreeShares;

	///变动日期(除权日)
	TTORATstpDateType	ChangeDateEX;

	///变动日期(上市日)
	TTORATstpDateType	ChangeDateList;

	///最新公告日期
	TTORATstpDateType	AnnouncementDate;

	///交易所代码
	TTORATstpExchangeIDType	ExchangeID;

	///证券代码
	TTORATstpSecurityIDType	SecurityID;

	///页定位符
	TTORATstpPageLocateType	PageLocate;


};

/// 订阅异常明细项
struct CTORATstpEffectDetailItemField
{
		
	///交易所代码
	TTORATstpExchangeIDType	ExchangeID;

	///证券代码
	TTORATstpSecurityIDType	SecurityID;

	///波动阀值
	TTORATstpRatioType	EffectRatio;


};

/// 价格异常波动委托明细
struct CTORATstpEffectOrderDetailField
{
		
	///交易所代码
	TTORATstpExchangeIDType	ExchangeID;

	///证券代码
	TTORATstpSecurityIDType	SecurityID;

	///委托价格
	TTORATstpPriceType	Price;

	///委托数量
	TTORATstpLongVolumeType	Volume;

	///委托类别
	TTORATstpLOrderTypeType	OrderType;

	///委托方向
	TTORATstpLSideType	Side;

	///波动幅度
	TTORATstpRatioType	EffectRatio;

	///委托主序号
	TTORATstpSequenceNoType	OrderSeq1;

	///委托子序号
	TTORATstpSequenceNoType	OrderSeq2;

	///更新时间(秒)
	TTORATstpTimeType	UpdateTime;

	///更新时间(毫秒)
	TTORATstpMillisecType	UpdateMillisec;


};

/// 价格异常波动成交明细
struct CTORATstpEffectTradeDetailField
{
		
	///交易所代码
	TTORATstpExchangeIDType	ExchangeID;

	///证券代码
	TTORATstpSecurityIDType	SecurityID;

	///成交价格
	TTORATstpPriceType	TradePrice;

	///成交数量
	TTORATstpLongVolumeType	TradeVolume;

	///成交类别
	TTORATstpExecTypeType	ExecType;

	///波动幅度
	TTORATstpRatioType	EffectRatio;

	///成交主序号
	TTORATstpSequenceNoType	TradeSeq1;

	///成交子序号
	TTORATstpSequenceNoType	TradeSeq2;

	///买方委托序号
	TTORATstpSequenceNoType	BuySideSeq;

	///卖方委托序号
	TTORATstpSequenceNoType	SelSideSeq;

	///更新时间(秒)
	TTORATstpTimeType	UpdateTime;

	///更新时间(毫秒)
	TTORATstpMillisecType	UpdateMillisec;


};

/// 查询盘口委托
struct CTORATstpInquiryQueueingOrdersField
{
		
	///交易所代码
	TTORATstpExchangeIDType	ExchangeID;

	///证券代码
	TTORATstpSecurityIDType	SecurityID;

	///盘口价格
	TTORATstpPriceType	QueuePrice;


};

/// 盘口委托
struct CTORATstpQueueingOrderField
{
		
	///交易所代码
	TTORATstpExchangeIDType	ExchangeID;

	///证券代码
	TTORATstpSecurityIDType	SecurityID;

	///委托价格
	TTORATstpPriceType	Price;

	///委托数量
	TTORATstpLongVolumeType	Volume;

	///委托类别
	TTORATstpLOrderTypeType	OrderType;

	///委托方向
	TTORATstpLSideType	Side;

	///委托主序号
	TTORATstpSequenceNoType	OrderSeq1;

	///委托子序号
	TTORATstpSequenceNoType	OrderSeq2;


};

/// 查询持仓量价分布信息
struct CTORATstpQryPriceDistributionInfoField
{
		
	///起始日期
	TTORATstpDateType	BegDate;

	///结束日期
	TTORATstpDateType	EndDate;

	///持仓分位点百分比
	TTORATstpPercentValType	PercentNum;

	///价格数量分布柱状图的柱子数
	TTORATstpVolumeType	DistributionType;

	///交易所代码
	TTORATstpExchangeIDType	ExchangeID;

	///证券代码
	TTORATstpSecurityIDType	SecurityID;

	///每页记录数
	TTORATstpVolumeType	PageCount;

	///页定位符
	TTORATstpPageLocateType	PageLocate;


};

/// 持仓量价分布数据
struct CTORATstpPriceDistributionDataField
{
		
	///数据日期
	TTORATstpDateType	DataDate;

	///持仓分位点百分比
	TTORATstpPercentValType	PercentNum;

	///在持仓分位点的价格
	TTORATstpRatioType	PercentValue;

	///价格数量分布柱状图的柱子数
	TTORATstpVolumeType	DistributionType;

	///价格数量分布的数据
	TTORATstpDistriValueType	DistributionValue;

	///交易所代码
	TTORATstpExchangeIDType	ExchangeID;

	///证券代码
	TTORATstpSecurityIDType	SecurityID;

	///页定位符
	TTORATstpPageLocateType	PageLocate;


};

/// 查询历史极值价格信息
struct CTORATstpQryPriceExtremumInfoField
{
		
	///交易日期
	TTORATstpDateType	TradingDay;

	///起始时间
	TTORATstpTimeType	BegTime;

	///结束时间
	TTORATstpTimeType	EndTime;

	///交易所代码
	TTORATstpExchangeIDType	ExchangeID;

	///证券代码
	TTORATstpSecurityIDType	SecurityID;

	///每页记录数
	TTORATstpVolumeType	PageCount;

	///页定位符
	TTORATstpPageLocateType	PageLocate;


};

/// 历史极值价格数据
struct CTORATstpPriceExtremumDataField
{
		
	///交易日期
	TTORATstpDateType	HighestTradingDay;

	///更新时间
	TTORATstpTimeType	HighestUpdateTime;

	///更新毫秒
	TTORATstpMillisecType	HighestUpdateMillisec;

	///最高价(元)
	TTORATstpPriceType	HighestPrice;

	///交易日期
	TTORATstpDateType	LowestTradingDay;

	///更新时间
	TTORATstpTimeType	LowestUpdateTime;

	///更新毫秒
	TTORATstpMillisecType	LowestUpdateMillisec;

	///最高价(元)
	TTORATstpPriceType	LowestPrice;

	///交易所代码
	TTORATstpExchangeIDType	ExchangeID;

	///证券代码
	TTORATstpSecurityIDType	SecurityID;

	///页定位符
	TTORATstpPageLocateType	PageLocate;


};

/// 查询文件委托请求
struct CTORATstpInquiryFileOrderField
{
		
	///交易所代码
	TTORATstpExchangeIDType	ExchangeID;

	///股东账户代码
	TTORATstpShareholderIDType	ShareholderID;

	///投资者代码
	TTORATstpInvestorIDType	InvestorID;

	///证券代码
	TTORATstpSecurityIDType	SecurityID;

	///委托起始流水号
	TTORATstpSequenceNoType	OrderSerialBeg;

	///委托结束流水号
	TTORATstpSequenceNoType	OrderSerialEnd;

	///委托提交状态
	TTORATstpCommitStatusType	CommitStatus;


};

/// 文件委托
struct CTORATstpFileOrderField
{
		
	///请求编号
	TTORATstpRequestIDType	RequestID;

	///交易所代码
	TTORATstpExchangeIDType	ExchangeID;

	///股东账户代码
	TTORATstpShareholderIDType	ShareholderID;

	///投资者代码
	TTORATstpInvestorIDType	InvestorID;

	///证券代码
	TTORATstpSecurityIDType	SecurityID;

	///报单引用
	TTORATstpOrderRefType	OrderRef;

	///文件报单类别
	TTORATstpFileOrderTypeType	FileOrderType;

	///买卖方向
	TTORATstpDirectionType	Direction;

	///限价单价格
	TTORATstpPriceType	LimitPrice;

	///报单数量
	TTORATstpVolumeType	VolumeTotalOriginal;

	///委托方式
	TTORATstpOperwayType	Operway;

	///报单操作引用
	TTORATstpOrderRefType	OrderActionRef;

	///报单编号
	TTORATstpOrderSysIDType	OrderSysID;

	///委托检查
	TTORATstpCondCheckType	CondCheck;

	///委托流水号
	TTORATstpSequenceNoType	OrderSerial;

	///文件委托提交状态
	TTORATstpCommitStatusType	CommitStatus;

	///文件委托状态信息
	TTORATstpStatusMsgType	StatusMsg;

	///委托时间戳
	TTORATstpBigTimeStampType	TimeStamp;


};

/// 查询应答域
struct CTORATstpQryRspInfoField
{
		
	///结束标识
	TTORATstpEndFlagType	EndFlag;

	///错误代码
	TTORATstpErrorIDType	ErrorID;

	///错误信息
	TTORATstpErrorMsgType	ErrorMsg;


};

/// 文件委托复核请求
struct CTORATstpReviewFileOrderField
{
		
	///交易所代码
	TTORATstpExchangeIDType	ExchangeID;

	///股东账户代码
	TTORATstpShareholderIDType	ShareholderID;

	///投资者代码
	TTORATstpInvestorIDType	InvestorID;

	///证券代码
	TTORATstpSecurityIDType	SecurityID;

	///委托起始流水号
	TTORATstpSequenceNoType	OrderSerialBeg;

	///委托结束流水号
	TTORATstpSequenceNoType	OrderSerialEnd;


};

/// 文件提交信息
struct CTORATstpCommitInfoField
{
		
	///委托流水号
	TTORATstpSequenceNoType	OrderSerial;

	///文件委托提交状态
	TTORATstpCommitStatusType	CommitStatus;

	///文件委托状态信息
	TTORATstpStatusMsgType	StatusMsg;


};

/// 请求插入交易通知
struct CTORATstpReqInsTradingNoticeField
{
		
	///通知流水号
	TTORATstpSerialType	NoticeSerial;

	///通知日期
	TTORATstpDateType	InsertDate;

	///通知时间
	TTORATstpTimeType	InsertTime;

	///投资者代码
	TTORATstpInvestorIDType	InvestorID;

	///投资单元代码
	TTORATstpBusinessUnitIDType	BusinessUnitID;

	///通知消息内容
	TTORATstpContentType	Content;

	///操作员
	TTORATstpUserIDType	OperatorID;


};

/// 交易通知
struct CTORATstpTradingNoticeField
{
		
	///通知流水号
	TTORATstpSerialType	NoticeSerial;

	///通知日期
	TTORATstpDateType	InsertDate;

	///通知时间
	TTORATstpTimeType	InsertTime;

	///投资者代码
	TTORATstpInvestorIDType	InvestorID;

	///投资单元代码
	TTORATstpBusinessUnitIDType	BusinessUnitID;

	///通知消息内容
	TTORATstpContentType	Content;

	///操作员
	TTORATstpUserIDType	OperatorID;


};

/// 装载文件委托
struct CTORATstpLoadFileOrderField
{
		
	///投资者代码
	TTORATstpInvestorIDType	InvestorID;

	///是否需要复核
	TTORATstpBoolType	bReview;


};

/// 文件委托信息
struct CTORATstpFileOrderInfoField
{
		
	///请求编号
	TTORATstpRequestIDType	RequestID;

	///交易所代码
	TTORATstpExchangeIDType	ExchangeID;

	///股东账户代码
	TTORATstpShareholderIDType	ShareholderID;

	///投资者代码
	TTORATstpInvestorIDType	InvestorID;

	///证券代码
	TTORATstpSecurityIDType	SecurityID;

	///报单引用
	TTORATstpOrderRefType	OrderRef;

	///文件报单类别
	TTORATstpFileOrderTypeType	FileOrderType;

	///买卖方向
	TTORATstpDirectionType	Direction;

	///限价单价格
	TTORATstpPriceType	LimitPrice;

	///报单数量
	TTORATstpVolumeType	VolumeTotalOriginal;

	///委托方式
	TTORATstpOperwayType	Operway;

	///报单操作引用
	TTORATstpOrderRefType	OrderActionRef;

	///报单编号
	TTORATstpOrderSysIDType	OrderSysID;

	///委托检查
	TTORATstpCondCheckType	CondCheck;

	///委托流水号
	TTORATstpSequenceNoType	OrderSerial;

	///文件委托提交状态
	TTORATstpCommitStatusType	CommitStatus;

	///文件委托状态信息
	TTORATstpStatusMsgType	StatusMsg;


};

/// 查询最大报单量请求
struct CTORATstpReqInquiryMaxOrderVolumeField
{
		
	///交易所代码
	TTORATstpExchangeIDType	ExchangeID;

	///证券代码
	TTORATstpSecurityIDType	SecurityID;

	///投资者代码
	TTORATstpInvestorIDType	InvestorID;

	///投资单元代码
	TTORATstpBusinessUnitIDType	BusinessUnitID;

	///资金账户代码
	TTORATstpAccountIDType	AccountID;

	///股东账户代码
	TTORATstpShareholderIDType	ShareholderID;

	///买卖方向
	TTORATstpDirectionType	Direction;

	///报单价格条件
	TTORATstpOrderPriceTypeType	OrderPriceType;

	///组合开平标志
	TTORATstpCombOffsetFlagType	CombOffsetFlag;

	///组合投机套保标志
	TTORATstpCombHedgeFlagType	CombHedgeFlag;

	///有效期类型
	TTORATstpTimeConditionType	TimeCondition;

	///成交量类型
	TTORATstpVolumeConditionType	VolumeCondition;

	///价格
	TTORATstpPriceType	LimitPrice;

	///转入交易单元代码(仅在转托管操作时有效)
	TTORATstpPbuIDType	TransfereePbuID;

	///最大委托手数
	TTORATstpVolumeType	MaxVolume;

	///港股通订单数量类型,默认整股买卖
	TTORATstpLotTypeType	LotType;


};

/// 查询最大报单量应答
struct CTORATstpRspInquiryMaxOrderVolumeField
{
		
	///交易所代码
	TTORATstpExchangeIDType	ExchangeID;

	///证券代码
	TTORATstpSecurityIDType	SecurityID;

	///投资者代码
	TTORATstpInvestorIDType	InvestorID;

	///投资单元代码
	TTORATstpBusinessUnitIDType	BusinessUnitID;

	///资金账户代码
	TTORATstpAccountIDType	AccountID;

	///股东账户代码
	TTORATstpShareholderIDType	ShareholderID;

	///买卖方向
	TTORATstpDirectionType	Direction;

	///报单价格条件
	TTORATstpOrderPriceTypeType	OrderPriceType;

	///组合开平标志
	TTORATstpCombOffsetFlagType	CombOffsetFlag;

	///组合投机套保标志
	TTORATstpCombHedgeFlagType	CombHedgeFlag;

	///有效期类型
	TTORATstpTimeConditionType	TimeCondition;

	///成交量类型
	TTORATstpVolumeConditionType	VolumeCondition;

	///价格
	TTORATstpPriceType	LimitPrice;

	///转入交易单元代码(仅在转托管操作时有效)
	TTORATstpPbuIDType	TransfereePbuID;

	///最大委托手数
	TTORATstpVolumeType	MaxVolume;

	///港股通订单数量类型,默认整股买卖
	TTORATstpLotTypeType	LotType;


};

/// 外围系统仓位调拨流水明细
struct CTORATstpPeripheryPositionTransferDetailField
{
		
	///仓位调拨流水号
	TTORATstpIntSerialType	PositionSerial;

	///仓位调拨请求流水号
	TTORATstpIntSerialType	ApplySerial;

	///前置编号
	TTORATstpFrontIDType	FrontID;

	///会话编号
	TTORATstpSessionIDType	SessionID;

	///仓位调拨方向
	TTORATstpTransferDirectionType	TransferDirection;

	///交易所代码
	TTORATstpExchangeIDType	ExchangeID;

	///市场代码
	TTORATstpMarketIDType	MarketID;

	///投资者代码
	TTORATstpInvestorIDType	InvestorID;

	///投资单元代码
	TTORATstpBusinessUnitIDType	BusinessUnitID;

	///股东账户代码
	TTORATstpShareholderIDType	ShareholderID;

	///证券代码
	TTORATstpSecurityIDType	SecurityID;

	///今日买卖仓位转入转出数量
	TTORATstpVolumeType	TodayBSPos;

	///今日申赎仓位转入数量
	TTORATstpVolumeType	TodayPRPos;

	///昨日仓位转入数量
	TTORATstpVolumeType	HistoryPos;

	///交易日
	TTORATstpDateType	TradingDay;

	///仓位调拨原因
	TTORATstpTransferReasonType	TransferReason;

	///转移状态
	TTORATstpTransferStatusType	TransferStatus;

	///操作日期
	TTORATstpDateType	OperateDate;

	///操作时间
	TTORATstpTimeType	OperateTime;

	///冲正日期
	TTORATstpDateType	RepealDate;

	///冲正时间
	TTORATstpTimeType	RepealTime;

	///冲正原因
	TTORATstpTransferReasonType	RepealReason;

	///状态信息
	TTORATstpErrorMsgType	StatusMsg;


};

/// 查询历史委托
struct CTORATstpQryHistoryOrderField
{
		
	///交易所代码
	TTORATstpExchangeIDType	ExchangeID;

	///投资者代码
	TTORATstpInvestorIDType	InvestorID;

	///起始日期
	TTORATstpDateType	BegDate;

	///结束日期
	TTORATstpDateType	EndDate;

	///证券代码
	TTORATstpSecurityIDType	SecurityID;

	///每页记录数
	TTORATstpVolumeType	PageCount;

	///页定位符
	TTORATstpPageLocateType	PageLocate;


};

/// 历史委托信息
struct CTORATstpHistoryOrderField
{
		
	///投资者代码
	TTORATstpInvestorIDType	InvestorID;

	///证券代码
	TTORATstpSecurityIDType	SecurityID;

	///报单引用
	TTORATstpOrderRefType	OrderRef;

	///用户代码
	TTORATstpUserIDType	UserID;

	///报单价格条件
	TTORATstpOrderPriceTypeType	OrderPriceType;

	///买卖方向
	TTORATstpDirectionType	Direction;

	///组合开平标志
	TTORATstpCombOffsetFlagType	CombOffsetFlag;

	///组合投机套保标志
	TTORATstpCombHedgeFlagType	CombHedgeFlag;

	///价格
	TTORATstpPriceType	LimitPrice;

	///数量
	TTORATstpVolumeType	VolumeTotalOriginal;

	///有效期类型
	TTORATstpTimeConditionType	TimeCondition;

	///成交量类型
	TTORATstpVolumeConditionType	VolumeCondition;

	///最小成交量
	TTORATstpVolumeType	MinVolume;

	///强平原因
	TTORATstpForceCloseReasonType	ForceCloseReason;

	///请求编号
	TTORATstpRequestIDType	RequestID;

	///本地报单编号
	TTORATstpOrderLocalIDType	OrderLocalID;

	///交易所代码
	TTORATstpExchangeIDType	ExchangeID;

	///市场代码
	TTORATstpMarketIDType	MarketID;

	///股东账户代码
	TTORATstpShareholderIDType	ShareholderID;

	///证券在交易所代码
	TTORATstpExchangeInstIDType	ExchangeInstID;

	///交易单元代码
	TTORATstpTraderIDType	TraderID;

	///报单提交状态
	TTORATstpOrderSubmitStatusType	OrderSubmitStatus;

	///交易日
	TTORATstpDateType	TradingDay;

	///系统报单编号
	TTORATstpOrderSysIDType	OrderSysID;

	///报单状态
	TTORATstpOrderStatusType	OrderStatus;

	///报单类型
	TTORATstpOrderTypeType	OrderType;

	///已成交数量
	TTORATstpVolumeType	VolumeTraded;

	///剩余未完成数量
	TTORATstpVolumeType	VolumeTotal;

	///报单日期
	TTORATstpDateType	InsertDate;

	///报单时间
	TTORATstpTimeType	InsertTime;

	///撤销时间
	TTORATstpTimeType	CancelTime;

	///最后修改交易单元代码
	TTORATstpTraderIDType	ActiveTraderID;

	///前置编号
	TTORATstpFrontIDType	FrontID;

	///会话编号
	TTORATstpSessionIDType	SessionID;

	///用户端产品信息
	TTORATstpProductInfoType	UserProductInfo;

	///状态信息
	TTORATstpErrorMsgType	StatusMsg;

	///用户强评标志
	TTORATstpBoolType	UserForceClose;

	///操作用户代码
	TTORATstpUserIDType	ActiveUserID;

	///投资单元代码
	TTORATstpBusinessUnitIDType	BusinessUnitID;

	///资金账户代码
	TTORATstpAccountIDType	AccountID;

	///IP地址
	TTORATstpIPAddressType	IPAddress;

	///Mac地址
	TTORATstpMacAddressType	MacAddress;

	///港股通订单数量类型,默认整股买卖
	TTORATstpLotTypeType	LotType;

	///长字符串附加信息
	TTORATstpBigsInfoType	BInfo;

	///短字符串附加信息
	TTORATstpShortsInfoType	SInfo;

	///整形附加信息
	TTORATstpIntInfoType	IInfo;

	///转入交易单元代码(仅在转托管操作时有效)
	TTORATstpPbuIDType	TransfereePbuID;

	///委托方式
	TTORATstpOperwayType	Operway;

	///一级机构代码
	TTORATstpDepartmentIDType	DepartmentID;

	///适当性控制业务类别
	TTORATstpProperCtrlBusinessTypeType	ProperCtrlBusinessType;

	///适当性控制通过标示
	TTORATstpProperCtrlPassFlagType	ProperCtrlPassFlag;

	///条件检查
	TTORATstpCondCheckType	CondCheck;

	///是否预埋
	TTORATstpBoolType	IsCacheOrder;

	///成交金额
	TTORATstpMoneyType	Turnover;

	///回报附加浮点型数据信息
	TTORATstpFloatInfoType	RtnFloatInfo;

	///回报附加整型数据
	TTORATstpIntInfoType	RtnIntInfo;

	///硬盘序列号
	TTORATstpHDSerialType	HDSerial;

	///移动设备手机号
	TTORATstpMobileType	Mobile;

	///页定位符
	TTORATstpPageLocateType	PageLocate;


};

/// 查询历史成交
struct CTORATstpQryHistoryTradeField
{
		
	///交易所代码
	TTORATstpExchangeIDType	ExchangeID;

	///投资者代码
	TTORATstpInvestorIDType	InvestorID;

	///起始日期
	TTORATstpDateType	BegDate;

	///结束日期
	TTORATstpDateType	EndDate;

	///证券代码
	TTORATstpSecurityIDType	SecurityID;

	///每页记录数
	TTORATstpVolumeType	PageCount;

	///页定位符
	TTORATstpPageLocateType	PageLocate;


};

/// 历史成交信息
struct CTORATstpHistoryTradeField
{
		
	///投资者代码
	TTORATstpInvestorIDType	InvestorID;

	///证券代码
	TTORATstpSecurityIDType	SecurityID;

	///用户代码
	TTORATstpUserIDType	UserID;

	///交易所代码
	TTORATstpExchangeIDType	ExchangeID;

	///成交编号
	TTORATstpTradeIDType	TradeID;

	///买卖方向
	TTORATstpDirectionType	Direction;

	///系统报单编号
	TTORATstpOrderSysIDType	OrderSysID;

	///市场代码
	TTORATstpMarketIDType	MarketID;

	///股东账户代码
	TTORATstpShareholderIDType	ShareholderID;

	///证券在交易所代码
	TTORATstpExchangeInstIDType	ExchangeInstID;

	///开平标志
	TTORATstpOffsetFlagType	OffsetFlag;

	///投机套保标志
	TTORATstpHedgeFlagType	HedgeFlag;

	///成交价格
	TTORATstpPriceType	Price;

	///成交数量
	TTORATstpVolumeType	Volume;

	///成交日期
	TTORATstpDateType	TradeDate;

	///成交时间
	TTORATstpTimeType	TradeTime;

	///交易单元代码
	TTORATstpTraderIDType	TraderID;

	///本地报单编号
	TTORATstpOrderLocalIDType	OrderLocalID;

	///交易日
	TTORATstpDateType	TradingDay;

	///投资单元代码
	TTORATstpBusinessUnitIDType	BusinessUnitID;

	///资金账户代码
	TTORATstpAccountIDType	AccountID;

	///报单引用
	TTORATstpOrderRefType	OrderRef;

	///一级机构代码
	TTORATstpDepartmentIDType	DepartmentID;

	///页定位符
	TTORATstpPageLocateType	PageLocate;


};

/// 录入标记事件
struct CTORATstpInputRemarkEventField
{
		
	///投资者代码
	TTORATstpInvestorIDType	InvestorID;

	///交易所代码
	TTORATstpExchangeIDType	ExchangeID;

	///证券代码
	TTORATstpSecurityIDType	SecurityID;

	///标记内容
	TTORATstpRemarkType	Remark;


};

/// 录入标记事件结果返回
struct CTORATstpRspInputRemarkEventField
{
		
	///录入标记事件序号
	TTORATstpSequenceNoType	SequenceNo;

	///发生日期
	TTORATstpDateType	EventDate;

	///发生时间
	TTORATstpTimeType	EventTime;

	///交易所代码
	TTORATstpExchangeIDType	ExchangeID;

	///证券代码
	TTORATstpSecurityIDType	SecurityID;

	///标记内容
	TTORATstpRemarkType	Remark;

	///投资者代码
	TTORATstpInvestorIDType	InvestorID;


};

/// 更新标记事件
struct CTORATstpUpdateRemarkEventField
{
		
	///录入标记事件序号
	TTORATstpSequenceNoType	SequenceNo;

	///交易所代码
	TTORATstpExchangeIDType	ExchangeID;

	///证券代码
	TTORATstpSecurityIDType	SecurityID;

	///标记内容
	TTORATstpRemarkType	Remark;

	///投资者代码
	TTORATstpInvestorIDType	InvestorID;


};

/// 更新标记事件结果返回
struct CTORATstpRspUpdateRemarkEventField
{
		
	///录入标记事件序号
	TTORATstpSequenceNoType	SequenceNo;

	///交易所代码
	TTORATstpExchangeIDType	ExchangeID;

	///证券代码
	TTORATstpSecurityIDType	SecurityID;

	///标记内容
	TTORATstpRemarkType	Remark;

	///投资者代码
	TTORATstpInvestorIDType	InvestorID;


};

/// 查询历史标记事件
struct CTORATstpQryRemarkEventField
{
		
	///投资者代码
	TTORATstpInvestorIDType	InvestorID;

	///发生起始日期
	TTORATstpDateType	EventDateStart;

	///发生结束日期
	TTORATstpDateType	EventDateEnd;

	///发生起始时间
	TTORATstpTimeType	EventTimeStart;

	///发生结束时间
	TTORATstpTimeType	EventTimeEnd;

	///证券代码
	TTORATstpSecurityIDType	SecurityID;

	///交易所代码
	TTORATstpExchangeIDType	ExchangeID;

	///录入标记事件序号
	TTORATstpSequenceNoType	SequenceNo;

	///每页记录数
	TTORATstpVolumeType	PageCount;

	///页定位符
	TTORATstpPageLocateType	PageLocate;


};

/// 标记事件
struct CTORATstpRemarkEventField
{
		
	///录入标记事件序号
	TTORATstpSequenceNoType	SequenceNo;

	///发生日期
	TTORATstpDateType	EventDate;

	///发生时间
	TTORATstpTimeType	EventTime;

	///交易所代码
	TTORATstpExchangeIDType	ExchangeID;

	///证券代码
	TTORATstpSecurityIDType	SecurityID;

	///标记内容
	TTORATstpRemarkType	Remark;

	///投资者代码
	TTORATstpInvestorIDType	InvestorID;

	///页定位符
	TTORATstpPageLocateType	PageLocate;


};

/// 查询交易所
struct CTORATstpQryExchangeField
{
		
	///交易所代码
	TTORATstpExchangeIDType	ExchangeID;


};

/// 交易所
struct CTORATstpExchangeField
{
		
	///交易所代码
	TTORATstpExchangeIDType	ExchangeID;

	///交易所名称
	TTORATstpNameType	ExchangeName;

	///交易日
	TTORATstpDateType	TradingDay;


};

/// 查询PBU
struct CTORATstpQryPBUField
{
		
	///交易单元代码
	TTORATstpPbuIDType	PbuID;

	///交易所代码
	TTORATstpExchangeIDType	ExchangeID;

	///市场代码
	TTORATstpMarketIDType	MarketID;


};

/// PBU
struct CTORATstpPBUField
{
		
	///交易单元代码
	TTORATstpPbuIDType	PbuID;

	///交易单元名称
	TTORATstpNameType	PbuName;

	///交易所代码
	TTORATstpExchangeIDType	ExchangeID;

	///市场代码
	TTORATstpMarketIDType	MarketID;


};

/// 查询实时行情
struct CTORATstpQryMarketDataField
{
		
	///证券代码
	TTORATstpSecurityIDType	SecurityID;

	///交易所代码
	TTORATstpExchangeIDType	ExchangeID;


};

/// 查询证券信息
struct CTORATstpQrySecurityField
{
		
	///证券代码
	TTORATstpSecurityIDType	SecurityID;

	///交易所代码
	TTORATstpExchangeIDType	ExchangeID;

	///证券在交易所代码
	TTORATstpExchangeInstIDType	ExchangeInstID;

	///产品代码
	TTORATstpProductIDType	ProductID;


};

/// 证券信息
struct CTORATstpSecurityField
{
		
	///合约代码
	TTORATstpSecurityIDType	SecurityID;

	///交易所代码
	TTORATstpExchangeIDType	ExchangeID;

	///合约名称
	TTORATstpSecurityNameType	SecurityName;

	///合约在交易所的代码
	TTORATstpExchangeInstIDType	ExchangeInstID;

	///市场代码
	TTORATstpMarketIDType	MarketID;

	///产品代码
	TTORATstpProductIDType	ProductID;

	///证券类别代码
	TTORATstpSecurityTypeType	SecurityType;

	///申报单位
	TTORATstpOrderUnitType	OrderUnit;

	///买入交易单位
	TTORATstpTradingUnitType	BuyTradingUnit;

	///卖出交易单位
	TTORATstpTradingUnitType	SellTradingUnit;

	///市价单买最大下单量
	TTORATstpVolumeType	MaxMarketOrderBuyVolume;

	///市价单买最小下单量
	TTORATstpVolumeType	MinMarketOrderBuyVolume;

	///限价单买最大下单量
	TTORATstpVolumeType	MaxLimitOrderBuyVolume;

	///限价单买最小下单量
	TTORATstpVolumeType	MinLimitOrderBuyVolume;

	///市价单卖最大下单量
	TTORATstpVolumeType	MaxMarketOrderSellVolume;

	///市价单卖最小下单量
	TTORATstpVolumeType	MinMarketOrderSellVolume;

	///限价单卖最大下单量
	TTORATstpVolumeType	MaxLimitOrderSellVolume;

	///限价单卖最小下单量
	TTORATstpVolumeType	MinLimitOrderSellVolume;

	///数量乘数
	TTORATstpVolumeMultipleType	VolumeMultiple;

	///最小变动价位
	TTORATstpPriceTickType	PriceTick;

	///上市日
	TTORATstpDateType	OpenDate;

	///持仓类型
	TTORATstpPositionTypeType	PositionType;

	///面值
	TTORATstpParValueType	ParValue;

	///证券状态
	TTORATstpSecurityStatusType	SecurityStatus;

	///债券应计利息
	TTORATstpInterestType	BondInterest;

	///折算率
	TTORATstpRatioType	ConversionRate;

	///是否担保品
	TTORATstpBoolType	IsCollateral;

	///昨收盘价
	TTORATstpPriceType	PreClosePrice;


};

/// 查询ETF清单信息
struct CTORATstpQryETFFileField
{
		
	///交易所代码
	TTORATstpExchangeIDType	ExchangeID;

	///ETF二级市场交易代码
	TTORATstpSecurityIDType	ETFSecurityID;

	///ETF一级市场申赎代码
	TTORATstpSecurityIDType	ETFCreRedSecurityID;


};

/// ETF清单信息
struct CTORATstpETFFileField
{
		
	///交易日
	TTORATstpDateType	TradingDay;

	///交易所代码
	TTORATstpExchangeIDType	ExchangeID;

	///ETF交易代码
	TTORATstpSecurityIDType	ETFSecurityID;

	///ETF申赎代码
	TTORATstpSecurityIDType	ETFCreRedSecurityID;

	///最小申购赎回单位份数
	TTORATstpVolumeType	CreationRedemptionUnit;

	///最大现金替代比例
	TTORATstpRatioType	Maxcashratio;

	///是否允许申购
	TTORATstpBoolType	CreationStatus;

	///是否允许赎回
	TTORATstpBoolType	RedemptionStatus;

	///预估现金差额
	TTORATstpMoneyType	EstimateCashComponent;

	///前一交易日现金差额
	TTORATstpMoneyType	CashComponent;

	///前一交易日基金单位净值
	TTORATstpMoneyType	NAV;

	///前一交易日申赎基准单位净值
	TTORATstpMoneyType	NAVperCU;

	///当日申购赎回基准单位的红利金额
	TTORATstpMoneyType	DividendPerCU;


};

/// 查询ETF成份证券信息
struct CTORATstpQryETFBasketField
{
		
	///交易所代码
	TTORATstpExchangeIDType	ExchangeID;

	///ETF二级市场交易代码
	TTORATstpSecurityIDType	ETFSecurityID;

	///ETF成份证券代码
	TTORATstpSecurityIDType	SecurityID;


};

/// ETF成份证券信息
struct CTORATstpETFBasketField
{
		
	///交易日
	TTORATstpDateType	TradingDay;

	///交易所代码
	TTORATstpExchangeIDType	ExchangeID;

	///ETF交易代码
	TTORATstpSecurityIDType	ETFSecurityID;

	///ETF成份证券代码
	TTORATstpSecurityIDType	SecurityID;

	///成分证券名称
	TTORATstpSecurityNameType	SecurityName;

	///成分证券数量
	TTORATstpVolumeType	Volume;

	///现金替代标志
	TTORATstpETFCurrenceReplaceStatusType	ETFCurrenceReplaceStatus;

	///溢价比例
	TTORATstpRatioType	Premium;

	///申购替代金额
	TTORATstpMoneyType	CreationReplaceAmount;

	///赎回替代金额
	TTORATstpMoneyType	RedemptionReplaceAmount;

	///挂牌市场
	TTORATstpMarketIDType	MarketID;


};

/// 查询经纪公司部门信息
struct CTORATstpQryDepartmentInfoField
{
		
	///一级机构代码
	TTORATstpDepartmentIDType	DepartmentID;


};

/// 经纪公司部门信息
struct CTORATstpDepartmentInfoField
{
		
	///经纪公司部门代码
	TTORATstpDepartmentIDType	DepartmentID;

	///经纪公司部门名称
	TTORATstpNameType	DepartmentName;


};

/// 查询新股信息
struct CTORATstpQryIPOInfoField
{
		
	///交易所代码
	TTORATstpExchangeIDType	ExchangeID;

	///申购代码
	TTORATstpSecurityIDType	SecurityID;


};

/// 新股信息
struct CTORATstpIPOInfoField
{
		
	///交易所代码
	TTORATstpExchangeIDType	ExchangeID;

	///申购代码
	TTORATstpSecurityIDType	SecurityID;

	///市场代码
	TTORATstpMarketIDType	MarketID;

	///产品代码
	TTORATstpProductIDType	ProductID;

	///证券类别代码
	TTORATstpSecurityTypeType	SecurityType;

	///网上申购上限
	TTORATstpVolumeType	OnlineLimit;

	///发行价格
	TTORATstpPriceType	Price;

	///币种代码
	TTORATstpCurrencyIDType	CurrencyID;

	///申购证券名称
	TTORATstpSecurityNameType	SecurityName;

	///新股证券代码
	TTORATstpSecurityIDType	UnderlyingSecurityID;

	///新股证券名称
	TTORATstpSecurityNameType	UnderlyingSecurityName;

	///网上申购最小数量
	TTORATstpVolumeType	OnlineMinVol;

	///网上申购单位数量
	TTORATstpVolumeType	OnlineVolUnit;


};

/// 查询BrokerUserFunction
struct CTORATstpQryBrokerUserFunctionField
{
		
	///用户代码
	TTORATstpUserIDType	UserID;


};

/// BrokerUserFunction
struct CTORATstpBrokerUserFunctionField
{
		
	///用户代码
	TTORATstpUserIDType	UserID;

	///功能代码
	TTORATstpFunctionIDType	FunctionID;


};

/// 查询经纪公司用户与投资者关系
struct CTORATstpQryBUProxyField
{
		
	///投资者代码
	TTORATstpInvestorIDType	InvestorID;

	///用户代码
	TTORATstpUserIDType	UserID;

	///投资单元代码
	TTORATstpBusinessUnitIDType	BusinessUnitID;


};

/// 经纪公司用户与投资者关系
struct CTORATstpBUProxyField
{
		
	///投资者代码
	TTORATstpInvestorIDType	InvestorID;

	///用户代码
	TTORATstpUserIDType	UserID;

	///投资单元代码
	TTORATstpBusinessUnitIDType	BusinessUnitID;

	///一级管理机构代码
	TTORATstpDepartmentIDType	ManageDepartmentID;

	///二级机构编码
	TTORATstpBranchIDType	InnerBranchID;


};

/// 查询User
struct CTORATstpQryUserField
{
		
	///用户代码
	TTORATstpUserIDType	UserID;

	///用户类型
	TTORATstpUserTypeType	UserType;


};

/// User
struct CTORATstpUserField
{
		
	///用户代码
	TTORATstpUserIDType	UserID;

	///用户名称
	TTORATstpUserNameType	UserName;

	///用户类型
	TTORATstpUserTypeType	UserType;

	///是否活跃
	TTORATstpBoolType	IsActive;

	///登录限制
	TTORATstpLoginLimitType	LoginLimit;


};

/// 查询投资者
struct CTORATstpQryInvestorField
{
		
	///投资者代码
	TTORATstpInvestorIDType	InvestorID;


};

/// 投资者
struct CTORATstpInvestorField
{
		
	///投资者代码
	TTORATstpInvestorIDType	InvestorID;

	///投资者名称
	TTORATstpInvestorNameType	InvestorName;

	///证件类型
	TTORATstpIdCardTypeType	IdCardType;

	///证件号码
	TTORATstpIdCardNoType	IdCardNo;

	///联系电话
	TTORATstpTelephoneType	Telephone;

	///通讯地址
	TTORATstpAddressType	Address;

	///开户日期
	TTORATstpDateType	OpenDate;

	///手机
	TTORATstpMobileType	Mobile;

	///委托方式
	TTORATstpOperwaysType	Operways;

	///客户风险等级
	TTORATstpCRiskLevelType	CRiskLevel;

	///专业投资者类别
	TTORATstpProfInvestorTypeType	ProfInvestorType;

	///一级机构代码
	TTORATstpDepartmentIDType	DepartmentID;

	///二级机构编码
	TTORATstpBranchIDType	InnerBranchID;

	///一级管理机构代码
	TTORATstpDepartmentIDType	ManageDepartmentID;

	///是否活跃
	TTORATstpBoolType	IsActive;

	///登录限制
	TTORATstpLoginLimitType	LoginLimit;


};

/// 查询交易编码
struct CTORATstpQryShareholderAccountField
{
		
	///投资者代码
	TTORATstpInvestorIDType	InvestorID;

	///交易所代码
	TTORATstpExchangeIDType	ExchangeID;

	///市场代码
	TTORATstpMarketIDType	MarketID;

	///股东账户账户代码
	TTORATstpShareholderIDType	ShareholderID;

	///股东账户类型
	TTORATstpClientIDTypeType	TradingCodeClass;


};

/// 交易编码
struct CTORATstpShareholderAccountField
{
		
	///投资者代码
	TTORATstpInvestorIDType	InvestorID;

	///交易所代码
	TTORATstpExchangeIDType	ExchangeID;

	///客户代码
	TTORATstpShareholderIDType	ShareholderID;

	///交易编码类型
	TTORATstpClientIDTypeType	ClientIDType;

	///市场代码
	TTORATstpMarketIDType	MarketID;


};

/// 查询投资单元
struct CTORATstpQryBusinessUnitField
{
		
	///投资者代码
	TTORATstpInvestorIDType	InvestorID;


};

/// 投资单元
struct CTORATstpBusinessUnitField
{
		
	///投资者代码
	TTORATstpInvestorIDType	InvestorID;

	///投资单元代码
	TTORATstpBusinessUnitIDType	BusinessUnitID;

	///投资单元名称
	TTORATstpBusinessUnitNameType	BusinessUnitName;


};

/// 查询投资单元与交易账户关系
struct CTORATstpQryBusinessUnitAndTradingAcctField
{
		
	///投资者代码
	TTORATstpInvestorIDType	InvestorID;

	///投资单元代码
	TTORATstpBusinessUnitIDType	BusinessUnitID;

	///产品代码
	TTORATstpProductIDType	ProductID;

	///资金账户代码
	TTORATstpAccountIDType	AccountID;

	///币种
	TTORATstpCurrencyIDType	CurrencyID;


};

/// 投资单元与交易账户关系
struct CTORATstpBusinessUnitAndTradingAcctField
{
		
	///经纪公司代码
	TTORATstpInvestorIDType	InvestorID;

	///投资单元代码
	TTORATstpBusinessUnitIDType	BusinessUnitID;

	///交易所代码
	TTORATstpExchangeIDType	ExchangeID;

	///市场代码
	TTORATstpMarketIDType	MarketID;

	///客户代码
	TTORATstpShareholderIDType	ShareholderID;

	///产品代码
	TTORATstpProductIDType	ProductID;

	///资金账户代码
	TTORATstpAccountIDType	AccountID;

	///资金账户代码
	TTORATstpCurrencyIDType	CurrencyID;

	///用户代码
	TTORATstpUserIDType	UserID;


};

/// 查询报单
struct CTORATstpQryOrderField
{
		
	///投资者代码
	TTORATstpInvestorIDType	InvestorID;

	///证券代码
	TTORATstpSecurityIDType	SecurityID;

	///交易所代码
	TTORATstpExchangeIDType	ExchangeID;

	///市场代码
	TTORATstpMarketIDType	MarketID;

	///股东账户代码
	TTORATstpShareholderIDType	ShareholderID;

	///系统报单编号
	TTORATstpOrderSysIDType	OrderSysID;

	///Insert Time
	TTORATstpTimeType	InsertTimeStart;

	///Insert Time
	TTORATstpTimeType	InsertTimeEnd;

	///投资单元代码
	TTORATstpBusinessUnitIDType	BusinessUnitID;

	///长字符串附加信息
	TTORATstpBigsInfoType	BInfo;

	///短字符串附加信息
	TTORATstpShortsInfoType	SInfo;

	///整形附加信息
	TTORATstpIntInfoType	IInfo;

	///是否可撤
	TTORATstpBoolType	IsCancel;


};

/// 查询撤单
struct CTORATstpQryOrderActionField
{
		
	///投资者代码
	TTORATstpInvestorIDType	InvestorID;

	///交易所代码
	TTORATstpExchangeIDType	ExchangeID;

	///市场代码
	TTORATstpMarketIDType	MarketID;

	///股东账户代码
	TTORATstpShareholderIDType	ShareholderID;

	///长字符串附加信息
	TTORATstpBigsInfoType	BInfo;

	///短字符串附加信息
	TTORATstpShortsInfoType	SInfo;

	///整形附加信息
	TTORATstpIntInfoType	IInfo;


};

/// 查询成交
struct CTORATstpQryTradeField
{
		
	///投资者代码
	TTORATstpInvestorIDType	InvestorID;

	///证券代码
	TTORATstpSecurityIDType	SecurityID;

	///交易所代码
	TTORATstpExchangeIDType	ExchangeID;

	///市场代码
	TTORATstpMarketIDType	MarketID;

	///股东账户代码
	TTORATstpShareholderIDType	ShareholderID;

	///成交编号
	TTORATstpTradeIDType	TradeID;

	///Insert Time
	TTORATstpTimeType	TradeTimeStart;

	///Insert Time
	TTORATstpTimeType	TradeTimeEnd;

	///投资单元代码
	TTORATstpBusinessUnitIDType	BusinessUnitID;


};

/// 查询资金账户
struct CTORATstpQryTradingAccountField
{
		
	///投资者代码
	TTORATstpInvestorIDType	InvestorID;

	///币种
	TTORATstpCurrencyIDType	CurrencyID;

	///资金账户代码
	TTORATstpAccountIDType	AccountID;

	///资金账户类型
	TTORATstpAccountTypeType	AccountType;

	///一级机构代码
	TTORATstpDepartmentIDType	DepartmentID;


};

/// 资金账户
struct CTORATstpTradingAccountField
{
		
	///资金账户代码
	TTORATstpAccountIDType	AccountID;

	///可用资金
	TTORATstpMoneyType	Available;

	///可取资金
	TTORATstpMoneyType	WithdrawQuota;

	///币种代码
	TTORATstpCurrencyIDType	CurrencyID;

	///入金金额
	TTORATstpMoneyType	Deposit;

	///出金金额
	TTORATstpMoneyType	Withdraw;

	///可用未交收金额(港股通专用字段)
	TTORATstpMoneyType	UnDeliveredMoney;

	///冻结的资金(港股通该字段不包括未交收部分冻结资金)
	TTORATstpMoneyType	FrozenCash;

	///冻结的手续费(港股通该字段不包括未交收部分冻结手续费)
	TTORATstpMoneyType	FrozenCommission;

	///上日未交收金额(港股通专用字段)
	TTORATstpMoneyType	PreUnDeliveredMoney;

	///手续费(港股通该字段不包括未交收部分手续费)
	TTORATstpMoneyType	Commission;

	///资金账户类型
	TTORATstpAccountTypeType	AccountType;

	///资金账户所属投资者代码
	TTORATstpInvestorIDType	AccountOwner;

	///部门代码
	TTORATstpDepartmentIDType	DepartmentID;

	///银行代码
	TTORATstpBankIDType	BankID;

	///银行账户
	TTORATstpBankAccountIDType	BankAccountID;

	///冻结未交收金额(港股通专用)
	TTORATstpMoneyType	UnDeliveredFrozenCash;

	///冻结未交收手续费(港股通专用)
	TTORATstpMoneyType	UnDeliveredFrozenCommission;

	///占用未交收手续费(港股通专用)
	TTORATstpMoneyType	UnDeliveredCommission;


};

/// 查询投资者持仓
struct CTORATstpQryPositionField
{
		
	///投资者代码
	TTORATstpInvestorIDType	InvestorID;

	///证券代码
	TTORATstpSecurityIDType	SecurityID;

	///交易所代码
	TTORATstpExchangeIDType	ExchangeID;

	///市场代码
	TTORATstpMarketIDType	MarketID;

	///股东账户代码
	TTORATstpShareholderIDType	ShareholderID;

	///投资单元代码
	TTORATstpBusinessUnitIDType	BusinessUnitID;


};

/// 投资者持仓
struct CTORATstpPositionField
{
		
	///合约代码
	TTORATstpSecurityIDType	SecurityID;

	///投资者代码
	TTORATstpInvestorIDType	InvestorID;

	///投资单元代码
	TTORATstpBusinessUnitIDType	BusinessUnitID;

	///交易所代码
	TTORATstpExchangeIDType	ExchangeID;

	///市场代码
	TTORATstpMarketIDType	MarketID;

	///客户代码
	TTORATstpShareholderIDType	ShareholderID;

	///交易日
	TTORATstpDateType	TradingDay;

	///昨仓
	TTORATstpVolumeType	HistoryPos;

	///昨仓冻结
	TTORATstpVolumeType	HistoryPosFrozen;

	///今买卖仓
	TTORATstpVolumeType	TodayBSPos;

	///今买卖仓冻结
	TTORATstpVolumeType	TodayBSFrozen;

	///今日申赎持仓
	TTORATstpVolumeType	TodayPRPos;

	///今日申赎持仓冻结
	TTORATstpVolumeType	TodayPRFrozen;

	///持仓成本
	TTORATstpMoneyType	TotalPosCost;

	///今日拆分合并持仓
	TTORATstpVolumeType	TodaySMPos;

	///今日拆分合并持仓冻结
	TTORATstpVolumeType	TodaySMPosFrozen;

	///融资仓位
	TTORATstpVolumeType	MarginBuyPos;

	///融券仓位
	TTORATstpVolumeType	ShortSellPos;

	///上次余额(盘中不变)
	TTORATstpVolumeType	PrePosition;

	///股份可用
	TTORATstpVolumeType	AvailablePosition;

	///股份余额
	TTORATstpVolumeType	CurrentPosition;

	///最新价
	TTORATstpPriceType	LastPrice;


};

/// 查询基础交易费用
struct CTORATstpQryTradingFeeField
{
		
	///交易所代码
	TTORATstpExchangeIDType	ExchangeID;


};

/// 基础交易费用
struct CTORATstpTradingFeeField
{
		
	///交易所代码
	TTORATstpExchangeIDType	ExchangeID;

	///产品代码
	TTORATstpProductIDType	ProductID;

	///证券类别代码
	TTORATstpSecurityTypeType	SecurityType;

	///合约代码
	TTORATstpSecurityIDType	SecurityID;

	///业务类别
	TTORATstpBizClassType	BizClass;

	///印花税(港股印花税)按金额收取比例
	TTORATstpRatioType	StampTaxRatioByAmt;

	///印花税(港股印花税)按面值收取比例
	TTORATstpRatioType	StampTaxRatioByPar;

	///印花税(港股印花税,单位港币)按笔收取金额
	TTORATstpMoneyType	StampTaxFeePerOrder;

	///印花税(港股印花税,单位港币)最低收取金额
	TTORATstpMoneyType	StampTaxFeeMin;

	///印花税(港股印花税,单位港币)最高收取金额
	TTORATstpMoneyType	StampTaxFeeMax;

	///过户费(港股证券组合费)按金额收取比例
	TTORATstpRatioType	TransferRatioByAmt;

	///过户费(港股证券组合费)按面值收取比例
	TTORATstpRatioType	TransferRatioByPar;

	///过户费(港股证券组合费,单位港币)按笔收取金额
	TTORATstpMoneyType	TransferFeePerOrder;

	///过户费(港股证券组合费,单位港币)最低收取金额
	TTORATstpMoneyType	TransferFeeMin;

	///过户费(港股证券组合费,单位港币)最高收取金额
	TTORATstpMoneyType	TransferFeeMax;

	///经手费(港股交易费)按金额收取比例
	TTORATstpRatioType	HandlingRatioByAmt;

	///经手费(港股交易费)按面值收取比例
	TTORATstpRatioType	HandlingRatioByPar;

	///经手费(港股交易费,单位港币)按笔收取金额
	TTORATstpMoneyType	HandlingFeePerOrder;

	///经手费(港股交易费,单位港币)最低收取金额
	TTORATstpMoneyType	HandlingFeeMin;

	///经手费(港股交易费,单位港币)最高收取金额
	TTORATstpMoneyType	HandlingFeeMax;

	///证管费(港股交易征费)按金额收取比例
	TTORATstpRatioType	RegulateRatioByAmt;

	///证管费(港股交易征费)按面值收取比例
	TTORATstpRatioType	RegulateRatioByPar;

	///证管费(港股交易征费,单位港币)按笔收取金额
	TTORATstpMoneyType	RegulateFeePerOrder;

	///证管费(港股交易征费,单位港币)最低收取金额
	TTORATstpMoneyType	RegulateFeeMin;

	///证管费(港股交易征费,单位港币)最高收取金额
	TTORATstpMoneyType	RegulateFeeMax;

	///过户费(港股证券组合费,单位港币)按数量收取金额
	TTORATstpMoneyType	TransferFeeByVolume;

	///经手费(港股交易费,单位港币)按数量收取金额
	TTORATstpMoneyType	HandlingFeeByVolume;

	///结算费(港股股份交收费)按金额收取比例
	TTORATstpRatioType	SettlementRatioByAmt;

	///结算费(港股股份交收费)按面值收取比例
	TTORATstpRatioType	SettlementRatioByPar;

	///结算费(港股股份交收费,单位港币)按笔收取金额
	TTORATstpMoneyType	SettlementFeePerOrder;

	///结算费(港股股份交收费,单位港币)按数量收取金额
	TTORATstpMoneyType	SettlementFeeByVolume;

	///结算费(港股股份交收费,单位港币)最低收取金额
	TTORATstpMoneyType	SettlementFeeMin;

	///结算费(港股股份交收费,单位港币)最高收取金额
	TTORATstpMoneyType	SettlementFeeMax;

	///印花税(港股印花税,单位港币)按数量收取金额
	TTORATstpMoneyType	StampTaxFeeByVolume;

	///证管费(港股交易征费,单位港币)按数量收取金额
	TTORATstpMoneyType	RegulateFeeByVolume;


};

/// 查询佣金费率
struct CTORATstpQryInvestorTradingFeeField
{
		
	///投资者代码
	TTORATstpInvestorIDType	InvestorID;

	///交易所代码
	TTORATstpExchangeIDType	ExchangeID;

	///一级机构代码
	TTORATstpDepartmentIDType	DepartmentID;


};

/// 佣金费率
struct CTORATstpInvestorTradingFeeField
{
		
	///投资者代码
	TTORATstpInvestorIDType	InvestorID;

	///交易所代码
	TTORATstpExchangeIDType	ExchangeID;

	///产品代码
	TTORATstpProductIDType	ProductID;

	///证券类别代码
	TTORATstpSecurityTypeType	SecurityType;

	///合约代码
	TTORATstpSecurityIDType	SecurityID;

	///业务类别
	TTORATstpBizClassType	BizClass;

	///佣金类型
	TTORATstpBrokerageTypeType	BrokerageType;

	///佣金按金额收取比例
	TTORATstpRatioType	RatioByAmt;

	///佣金按面值收取比例
	TTORATstpRatioType	RatioByPar;

	///佣金按笔收取金额
	TTORATstpMoneyType	FeePerOrder;

	///佣金最低收取金额
	TTORATstpMoneyType	FeeMin;

	///佣金最高收取金额
	TTORATstpMoneyType	FeeMax;

	///佣金按数量收取金额
	TTORATstpMoneyType	FeeByVolume;

	///经纪公司部门代码
	TTORATstpDepartmentIDType	DepartmentID;


};

/// 查询新股申购额度
struct CTORATstpQryIPOQuotaField
{
		
	///投资者代码
	TTORATstpInvestorIDType	InvestorID;

	///交易所代码
	TTORATstpExchangeIDType	ExchangeID;

	///市场代码
	TTORATstpMarketIDType	MarketID;

	///股东账户代码
	TTORATstpShareholderIDType	ShareholderID;


};

/// 新股申购额度
struct CTORATstpIPOQuotaField
{
		
	///投资者代码
	TTORATstpInvestorIDType	InvestorID;

	///交易所代码
	TTORATstpExchangeIDType	ExchangeID;

	///市场代码
	TTORATstpMarketIDType	MarketID;

	///客户代码
	TTORATstpShareholderIDType	ShareholderID;

	///可申购额度
	TTORATstpLongVolumeType	MaxVolume;


};

/// 查询市场
struct CTORATstpQryMarketField
{
		
	///交易所代码
	TTORATstpExchangeIDType	ExchangeID;

	///市场代码
	TTORATstpMarketIDType	MarketID;


};

/// 市场
struct CTORATstpMarketField
{
		
	///市场代码
	TTORATstpMarketIDType	MarketID;

	///市场名称
	TTORATstpNameType	MarketName;

	///交易所代码
	TTORATstpExchangeIDType	ExchangeID;

	///市场状态
	TTORATstpMarketStatusType	MarketStatus;


};

/// 查询报单明细资金
struct CTORATstpQryOrderFundDetailField
{
		
	///投资者代码
	TTORATstpInvestorIDType	InvestorID;

	///证券代码
	TTORATstpSecurityIDType	SecurityID;

	///交易所代码
	TTORATstpExchangeIDType	ExchangeID;

	///系统报单编号
	TTORATstpOrderSysIDType	OrderSysID;

	///Insert Time
	TTORATstpTimeType	InsertTimeStart;

	///Insert Time
	TTORATstpTimeType	InsertTimeEnd;

	///投资单元代码
	TTORATstpBusinessUnitIDType	BusinessUnitID;


};

/// 报单明细资金
struct CTORATstpOrderFundDetailField
{
		
	///投资者代码
	TTORATstpInvestorIDType	InvestorID;

	///合约代码
	TTORATstpSecurityIDType	SecurityID;

	///交易所代码
	TTORATstpExchangeIDType	ExchangeID;

	///交易日
	TTORATstpDateType	TradingDay;

	///系统报单编号
	TTORATstpOrderSysIDType	OrderSysID;

	///投资单元代码
	TTORATstpBusinessUnitIDType	BusinessUnitID;

	///资金账户代码
	TTORATstpAccountIDType	AccountID;

	///初始冻结金额
	TTORATstpMoneyType	TotalFrozen;

	///总费用
	TTORATstpMoneyType	TotalFee;

	///印花税(港股为印花税)
	TTORATstpMoneyType	StampTaxFee;

	///经手费(港股为交易费)
	TTORATstpMoneyType	HandlingFee;

	///过户费(港股为证券组合费)
	TTORATstpMoneyType	TransferFee;

	///证管费(港股为交易征费)
	TTORATstpMoneyType	RegulateFee;

	///佣金
	TTORATstpMoneyType	BrokerageFee;

	///结算费(港股为股份交收费)
	TTORATstpMoneyType	SettlementFee;

	///初始冻结费用合计
	TTORATstpMoneyType	TotalFeeFrozen;

	///报单初始冻结金额
	TTORATstpMoneyType	OrderAmount;


};

/// 查询资金转移流水
struct CTORATstpQryFundTransferDetailField
{
		
	///资金账户代码
	TTORATstpAccountIDType	AccountID;

	///币种
	TTORATstpCurrencyIDType	CurrencyID;

	///转移方向
	TTORATstpTransferDirectionType	TransferDirection;

	///一级机构代码
	TTORATstpDepartmentIDType	DepartmentID;


};

/// 资金转移流水
struct CTORATstpFundTransferDetailField
{
		
	///转账流水号
	TTORATstpSerialType	FundSerial;

	///申请流水号
	TTORATstpExternalSerialType	ApplySerial;

	///前置编号
	TTORATstpFrontIDType	FrontID;

	///会话编号
	TTORATstpSessionIDType	SessionID;

	///资金账户代码
	TTORATstpAccountIDType	AccountID;

	///币种
	TTORATstpCurrencyIDType	CurrencyID;

	///转移方向
	TTORATstpTransferDirectionType	TransferDirection;

	///出入金金额
	TTORATstpMoneyType	Amount;

	///转移状态
	TTORATstpTransferStatusType	TransferStatus;

	///操作来源
	TTORATstpOperateSourceType	OperateSource;

	///操作人员
	TTORATstpUserIDType	OperatorID;

	///操作日期
	TTORATstpDateType	OperateDate;

	///操作时间
	TTORATstpTimeType	OperateTime;

	///状态信息
	TTORATstpErrorMsgType	StatusMsg;

	///经纪公司部门代码
	TTORATstpDepartmentIDType	DepartmentID;

	///银行代码
	TTORATstpBankIDType	BankID;

	///签约银行账户
	TTORATstpBankAccountIDType	BankAccountID;

	///IP地址
	TTORATstpIPAddressType	IPAddress;

	///Mac地址
	TTORATstpMacAddressType	MacAddress;

	///硬盘序列号
	TTORATstpHDSerialType	HDSerial;

	///移动设备手机号
	TTORATstpMobileType	Mobile;


};

/// 查询持仓转移流水
struct CTORATstpQryPositionTransferDetailField
{
		
	///股东账户代码
	TTORATstpShareholderIDType	ShareholderID;

	///证券代码
	TTORATstpSecurityIDType	SecurityID;

	///转移方向
	TTORATstpTransferDirectionType	TransferDirection;


};

/// 持仓转移流水
struct CTORATstpPositionTransferDetailField
{
		
	///流水号
	TTORATstpSerialType	PositionSerial;

	///申请流水号
	TTORATstpExternalSerialType	ApplySerial;

	///前置编号
	TTORATstpFrontIDType	FrontID;

	///会话编号
	TTORATstpSessionIDType	SessionID;

	///投资者代码
	TTORATstpInvestorIDType	InvestorID;

	///交易所代码
	TTORATstpExchangeIDType	ExchangeID;

	///交易账户代码
	TTORATstpShareholderIDType	ShareholderID;

	///市场代码
	TTORATstpMarketIDType	MarketID;

	///证券代码
	TTORATstpSecurityIDType	SecurityID;

	///交易日期
	TTORATstpDateType	TradingDay;

	///转移方向
	TTORATstpTransferDirectionType	TransferDirection;

	///转移持仓类型
	TTORATstpTransferPositionTypeType	TransferPositionType;

	///转移状态
	TTORATstpTransferStatusType	TransferStatus;

	///昨日仓位数量
	TTORATstpVolumeType	HistoryVolume;

	///今日买卖仓位数量
	TTORATstpVolumeType	TodayBSVolume;

	///今日申赎仓位数量
	TTORATstpVolumeType	TodayPRVolume;

	///操作人员
	TTORATstpUserIDType	OperatorID;

	///操作日期
	TTORATstpDateType	OperateDate;

	///操作时间
	TTORATstpTimeType	OperateTime;

	///投资单元代码
	TTORATstpBusinessUnitIDType	BusinessUnitID;

	///状态信息
	TTORATstpErrorMsgType	StatusMsg;

	///IP地址
	TTORATstpIPAddressType	IPAddress;

	///Mac地址
	TTORATstpMacAddressType	MacAddress;

	///硬盘序列号
	TTORATstpHDSerialType	HDSerial;

	///移动设备手机号
	TTORATstpMobileType	Mobile;


};

/// 查询投资者质押持仓
struct CTORATstpQryPledgePositionField
{
		
	///投资者代码
	TTORATstpInvestorIDType	InvestorID;

	///证券代码
	TTORATstpSecurityIDType	SecurityID;

	///交易所代码
	TTORATstpExchangeIDType	ExchangeID;

	///市场代码
	TTORATstpMarketIDType	MarketID;

	///股东账户代码
	TTORATstpShareholderIDType	ShareholderID;

	///投资单元代码
	TTORATstpBusinessUnitIDType	BusinessUnitID;


};

/// 投资者质押持仓
struct CTORATstpPledgePositionField
{
		
	///合约代码
	TTORATstpSecurityIDType	SecurityID;

	///投资者代码
	TTORATstpInvestorIDType	InvestorID;

	///投资单元代码
	TTORATstpBusinessUnitIDType	BusinessUnitID;

	///交易所代码
	TTORATstpExchangeIDType	ExchangeID;

	///市场代码
	TTORATstpMarketIDType	MarketID;

	///客户代码
	TTORATstpShareholderIDType	ShareholderID;

	///交易日
	TTORATstpDateType	TradingDay;

	///昨日质押持仓
	TTORATstpVolumeType	HisPledgePos;

	///昨日质押持仓冻结
	TTORATstpVolumeType	HisPledgePosFrozen;

	///今日入库的质押持仓
	TTORATstpVolumeType	TodayPledgePos;

	///今日入库的质押持仓冻结
	TTORATstpVolumeType	TodayPledgePosFrozen;

	///昨日质押入库的现券总量
	TTORATstpVolumeType	PreTotalPledgePos;

	///昨日质押入库的现券可用数量
	TTORATstpVolumeType	preAvailablePledgePos;


};

/// 查询证券质押信息
struct CTORATstpQryPledgeInfoField
{
		
	///交易所代码
	TTORATstpExchangeIDType	ExchangeID;

	///证券代码
	TTORATstpSecurityIDType	SecurityID;


};

/// 证券质押信息
struct CTORATstpPledgeInfoField
{
		
	///交易所代码
	TTORATstpExchangeIDType	ExchangeID;

	///市场代码
	TTORATstpMarketIDType	MarketID;

	///证券代码
	TTORATstpSecurityIDType	SecurityID;

	///质押申报代码
	TTORATstpSecurityIDType	PledgeOrderID;

	///标准券代码
	TTORATstpSecurityIDType	StandardBondID;

	///是否可入库
	TTORATstpBoolType	AllowPledgeIn;

	///是否可出库
	TTORATstpBoolType	AllowPledgeOut;

	///标准券折算率/折算值
	TTORATstpRatioType	ConversionRate;

	///每次可以入库的最小交易单位
	TTORATstpTradingUnitType	PledgeInTradingUnit;

	///每次可以出库的最小交易单位
	TTORATstpTradingUnitType	PledgeOutTradingUnit;

	///证券可以入库的最大数量
	TTORATstpVolumeType	PledgeInVolMax;

	///证券可以入库的最小数量
	TTORATstpVolumeType	PledgeInVolMin;

	///证券可以出库的最大数量
	TTORATstpVolumeType	PledgeOutVolMax;

	///证券可以出库的最小数量
	TTORATstpVolumeType	PledgeOutVolMin;

	///当日质押入库的质押券是否能出库
	TTORATstpBoolType	IsTodayToPlegeOut;

	///是否可撤单
	TTORATstpBoolType	IsCancelOrder;


};

/// 查询可转债信息
struct CTORATstpQryConversionBondInfoField
{
		
	///交易所代码
	TTORATstpExchangeIDType	ExchangeID;

	///证券代码
	TTORATstpSecurityIDType	SecurityID;


};

/// 可转债信息
struct CTORATstpConversionBondInfoField
{
		
	///交易所代码
	TTORATstpExchangeIDType	ExchangeID;

	///市场代码
	TTORATstpMarketIDType	MarketID;

	///证券代码
	TTORATstpSecurityIDType	SecurityID;

	///转股申报代码
	TTORATstpSecurityIDType	ConvertOrderID;

	///转股价格
	TTORATstpPriceType	ConvertPrice;

	///每次可以转股最小交易单位
	TTORATstpTradingUnitType	ConvertVolUnit;

	///证券可以转股的最大数量
	TTORATstpVolumeType	ConvertVolMax;

	///证券可以转股的最小数量
	TTORATstpVolumeType	ConvertVolMin;

	///转股开始日期
	TTORATstpDateType	BeginDate;

	///转股截至日期
	TTORATstpDateType	EndDate;

	///是否可撤单
	TTORATstpBoolType	IsSupportCancel;


};

/// 查询债券回售信息
struct CTORATstpQryBondPutbackInfoField
{
		
	///交易所代码
	TTORATstpExchangeIDType	ExchangeID;

	///证券代码
	TTORATstpSecurityIDType	SecurityID;


};

/// 债券回售信息
struct CTORATstpBondPutbackInfoField
{
		
	///交易所代码
	TTORATstpExchangeIDType	ExchangeID;

	///市场代码
	TTORATstpMarketIDType	MarketID;

	///证券代码
	TTORATstpSecurityIDType	SecurityID;

	///回售申报代码
	TTORATstpSecurityIDType	PutbackOrderID;

	///回售价格
	TTORATstpPriceType	PutbackPrice;

	///每次可以回售最小交易单位
	TTORATstpTradingUnitType	PutbackVolUnit;

	///债券可以回售的最大数量
	TTORATstpVolumeType	PutbackVolMax;

	///债券可以回售的最小数量
	TTORATstpVolumeType	PutbackVolMin;

	///回售开始日期
	TTORATstpDateType	BeginDate;

	///回售截至日期
	TTORATstpDateType	EndDate;

	///是否可撤单
	TTORATstpBoolType	IsSupportCancel;


};

/// 查询投资者标准券额度
struct CTORATstpQryStandardBondPositionField
{
		
	///投资者代码
	TTORATstpInvestorIDType	InvestorID;

	///证券代码
	TTORATstpSecurityIDType	SecurityID;

	///交易所代码
	TTORATstpExchangeIDType	ExchangeID;

	///市场代码
	TTORATstpMarketIDType	MarketID;

	///股东账户代码
	TTORATstpShareholderIDType	ShareholderID;

	///投资单元代码
	TTORATstpBusinessUnitIDType	BusinessUnitID;


};

/// 投资者标准券额度
struct CTORATstpStandardBondPositionField
{
		
	///合约代码
	TTORATstpSecurityIDType	SecurityID;

	///投资者代码
	TTORATstpInvestorIDType	InvestorID;

	///投资单元代码
	TTORATstpBusinessUnitIDType	BusinessUnitID;

	///交易所代码
	TTORATstpExchangeIDType	ExchangeID;

	///市场代码
	TTORATstpMarketIDType	MarketID;

	///客户代码
	TTORATstpShareholderIDType	ShareholderID;

	///交易日
	TTORATstpDateType	TradingDay;

	///标准券可用额度
	TTORATstpPositionVolumeType	AvailablePosition;

	///标准券可用额度冻结
	TTORATstpPositionVolumeType	AvailablePosFrozen;

	///标准券额度总量
	TTORATstpPositionVolumeType	TotalPosition;


};

/// 查询指定交易登记&撤销报单
struct CTORATstpQryDesignationRegistrationField
{
		
	///投资者代码
	TTORATstpInvestorIDType	InvestorID;

	///股东账户代码
	TTORATstpShareholderIDType	ShareholderID;

	///系统报单编号
	TTORATstpOrderSysIDType	OrderSysID;

	///Insert Time
	TTORATstpTimeType	InsertTimeStart;

	///Insert Time
	TTORATstpTimeType	InsertTimeEnd;

	///投资单元代码
	TTORATstpBusinessUnitIDType	BusinessUnitID;


};

/// 指定交易登记&撤销报单
struct CTORATstpDesignationRegistrationField
{
		
	///投资者代码
	TTORATstpInvestorIDType	InvestorID;

	///用户代码
	TTORATstpUserIDType	UserID;

	///买卖方向
	TTORATstpDesignationTypeType	DesignationType;

	///本地报单编号
	TTORATstpOrderLocalIDType	OrderLocalID;

	///客户代码
	TTORATstpShareholderIDType	ShareholderID;

	///交易单元代码
	TTORATstpPbuIDType	PbuID;

	///报单提交状态
	TTORATstpOrderSubmitStatusType	OrderSubmitStatus;

	///交易日
	TTORATstpDateType	TradingDay;

	///报单编号
	TTORATstpOrderSysIDType	OrderSysID;

	///报单状态
	TTORATstpOrderStatusType	OrderStatus;

	///报单日期
	TTORATstpDateType	InsertDate;

	///委托时间
	TTORATstpTimeType	InsertTime;

	///状态信息
	TTORATstpErrorMsgType	StatusMsg;

	///投资单元代码
	TTORATstpBusinessUnitIDType	BusinessUnitID;

	///资金账户代码
	TTORATstpAccountIDType	AccountID;

	///币种
	TTORATstpCurrencyIDType	CurrencyID;

	///经纪公司部门代码
	TTORATstpDepartmentIDType	DepartmentID;

	///IP地址
	TTORATstpIPAddressType	IPAddress;

	///Mac地址
	TTORATstpMacAddressType	MacAddress;

	///硬盘序列号
	TTORATstpHDSerialType	HDSerial;

	///移动设备手机号
	TTORATstpMobileType	Mobile;


};

/// 深证转托管报单表
struct CTORATstpQryCustodyTransferField
{
		
	///投资者代码
	TTORATstpInvestorIDType	InvestorID;

	///股东账户代码
	TTORATstpShareholderIDType	ShareholderID;

	///系统报单编号
	TTORATstpOrderSysIDType	OrderSysID;

	///Insert Time
	TTORATstpTimeType	InsertTimeStart;

	///Insert Time
	TTORATstpTimeType	InsertTimeEnd;

	///投资单元代码
	TTORATstpBusinessUnitIDType	BusinessUnitID;

	///证券代码
	TTORATstpSecurityIDType	SecurityID;


};

/// 转托管报单表
struct CTORATstpCustodyTransferField
{
		
	///投资者代码
	TTORATstpInvestorIDType	InvestorID;

	///用户代码
	TTORATstpUserIDType	UserID;

	///转托管类型
	TTORATstpCustodyTransferTypeType	CustodyTransferType;

	///本地报单编号
	TTORATstpOrderLocalIDType	OrderLocalID;

	///客户代码
	TTORATstpShareholderIDType	ShareholderID;

	///交易单元代码
	TTORATstpPbuIDType	PbuID;

	///报单提交状态
	TTORATstpOrderSubmitStatusType	OrderSubmitStatus;

	///交易日
	TTORATstpDateType	TradingDay;

	///报单编号
	TTORATstpOrderSysIDType	OrderSysID;

	///报单状态
	TTORATstpOrderStatusType	OrderStatus;

	///报单日期
	TTORATstpDateType	InsertDate;

	///委托时间
	TTORATstpTimeType	InsertTime;

	///状态信息
	TTORATstpErrorMsgType	StatusMsg;

	///投资单元代码
	TTORATstpBusinessUnitIDType	BusinessUnitID;

	///资金账户代码
	TTORATstpAccountIDType	AccountID;

	///币种
	TTORATstpCurrencyIDType	CurrencyID;

	///经纪公司部门代码
	TTORATstpDepartmentIDType	DepartmentID;

	///转入交易单元代码
	TTORATstpPbuIDType	TransfereePbuID;

	///合约代码
	TTORATstpSecurityIDType	SecurityID;

	///本地撤单编号,被主动撤单的转托管的原始编号
	TTORATstpOrderLocalIDType	OrignalOrderLocalID;

	///报单数量
	TTORATstpVolumeType	VolumeTotalOriginal;

	///撤销时间
	TTORATstpTimeType	CancelTime;

	///撤销交易单元代码
	TTORATstpPbuIDType	ActiveTraderID;

	///撤销操作员
	TTORATstpUserIDType	ActiveUserID;

	///IP地址
	TTORATstpIPAddressType	IPAddress;

	///Mac地址
	TTORATstpMacAddressType	MacAddress;

	///硬盘序列号
	TTORATstpHDSerialType	HDSerial;

	///移动设备手机号
	TTORATstpMobileType	Mobile;


};

/// 查询实时特别行情
struct CTORATstpQrySpecialMarketDataField
{
		
	///证券代码
	TTORATstpSecurityIDType	SecurityID;

	///交易所代码
	TTORATstpExchangeIDType	ExchangeID;


};

/// 查询未到期债券质押回购委托
struct CTORATstpQryPrematurityRepoOrderField
{
		
	///投资者代码
	TTORATstpInvestorIDType	InvestorID;

	///证券代码
	TTORATstpSecurityIDType	SecurityID;

	///交易所代码
	TTORATstpExchangeIDType	ExchangeID;

	///市场代码
	TTORATstpMarketIDType	MarketID;

	///股东账户代码
	TTORATstpShareholderIDType	ShareholderID;

	///投资单元代码
	TTORATstpBusinessUnitIDType	BusinessUnitID;

	///本地报单编号
	TTORATstpOrderLocalIDType	OrderLocalID;

	///证券品种代码
	TTORATstpProductIDType	ProductID;

	///证券类别代码
	TTORATstpSecurityTypeType	SecurityType;

	///买卖方向
	TTORATstpDirectionType	Direction;

	///成交编号
	TTORATstpTradeIDType	TradeID;


};

/// 未到期债券质押回购委托
struct CTORATstpPrematurityRepoOrderField
{
		
	///交易所代码
	TTORATstpExchangeIDType	ExchangeID;

	///市场代码
	TTORATstpMarketIDType	MarketID;

	///投资者代码
	TTORATstpInvestorIDType	InvestorID;

	///交易账户代码
	TTORATstpShareholderIDType	ShareholderID;

	///投资单元代码
	TTORATstpBusinessUnitIDType	BusinessUnitID;

	///成交日期
	TTORATstpDateType	TradeDay;

	///到期日期
	TTORATstpDateType	ExpireDay;

	///报单编号
	TTORATstpOrderLocalIDType	OrderLocalID;

	///证券代码
	TTORATstpSecurityIDType	SecurityID;

	///证券名称
	TTORATstpSecurityNameType	SecurityName;

	///证券品种代码
	TTORATstpProductIDType	ProductID;

	///证券类别代码
	TTORATstpSecurityTypeType	SecurityType;

	///报单类别
	TTORATstpDirectionType	Direction;

	///成交数量
	TTORATstpVolumeType	VolumeTraded;

	///成交价格
	TTORATstpPriceType	Price;

	///成交金额
	TTORATstpMoneyType	Turnover;

	///成交编号
	TTORATstpTradeIDType	TradeID;

	///购回应收金额
	TTORATstpMoneyType	RepoTotalMoney;

	///利息金额
	TTORATstpMoneyType	InterestAmount;


};

/// 查询股东参数
struct CTORATstpQryShareholderParamField
{
		
	///市场代码
	TTORATstpMarketIDType	MarketID;

	///股东账户代码
	TTORATstpShareholderIDType	ShareholderID;

	///股东账户类型
	TTORATstpClientIDTypeType	TradingCodeClass;

	///证券产品代码
	TTORATstpProductIDType	ProductID;

	///证券类别代码
	TTORATstpSecurityTypeType	SecurityType;

	///证券代码
	TTORATstpSecurityIDType	SecurityID;

	///参数类型
	TTORATstpParamTypeType	ParamType;

	///交易所代码
	TTORATstpExchangeIDType	ExchangeID;


};

/// 股东参数
struct CTORATstpShareholderParamField
{
		
	///交易所代码
	TTORATstpExchangeIDType	ExchangeID;

	///市场代码
	TTORATstpMarketIDType	MarketID;

	///交易账户代码
	TTORATstpShareholderIDType	ShareholderID;

	///账户类型
	TTORATstpClientIDTypeType	TradingCodeClass;

	///证券品种
	TTORATstpProductIDType	ProductID;

	///证券类别
	TTORATstpSecurityTypeType	SecurityType;

	///证券代码
	TTORATstpSecurityIDType	SecurityID;

	///股东参数类型
	TTORATstpParamTypeType	ParamType;

	///股东参数值
	TTORATstpParameterCharValType	ParamValue;


};

/// 查询外围系统仓位调拨流水
struct CTORATstpQryPeripheryPositionTransferDetailField
{
		
	///投资者代码
	TTORATstpInvestorIDType	InvestorID;

	///股东账户代码
	TTORATstpShareholderIDType	ShareholderID;

	///证券代码
	TTORATstpSecurityIDType	SecurityID;

	///仓位调拨方向
	TTORATstpTransferDirectionType	TransferDirection;

	///投资单元代码
	TTORATstpBusinessUnitIDType	BusinessUnitID;


};

/// 查询投资者条件单限制参数
struct CTORATstpQryInvestorCondOrderLimitParamField
{
		
	///投资者代码
	TTORATstpInvestorIDType	InvestorID;


};

/// 投资者条件单限制参数
struct CTORATstpInvestorCondOrderLimitParamField
{
		
	///投资者代码
	TTORATstpInvestorIDType	InvestorID;

	///最大条件单数
	TTORATstpVolumeType	MaxCondOrderLimitCnt;

	///当前条件单数
	TTORATstpVolumeType	CurrCondOrderCnt;


};

/// 查询条件单
struct CTORATstpQryCondOrderField
{
		
	///投资者代码
	TTORATstpInvestorIDType	InvestorID;

	///证券代码
	TTORATstpSecurityIDType	SecurityID;

	///交易所代码
	TTORATstpExchangeIDType	ExchangeID;

	///股东账户账户代码
	TTORATstpShareholderIDType	ShareholderID;

	///报单编号
	TTORATstpCondOrderIDType	CondOrderID;

	///Insert Time
	TTORATstpTimeType	InsertTimeStart;

	///Insert Time
	TTORATstpTimeType	InsertTimeEnd;

	///投资单元代码
	TTORATstpBusinessUnitIDType	BusinessUnitID;

	///长字符串附加信息
	TTORATstpBigsInfoType	BInfo;

	///短字符串附加信息
	TTORATstpShortsInfoType	SInfo;

	///整形附加信息
	TTORATstpIntInfoType	IInfo;


};

/// 条件单
struct CTORATstpCondOrderField
{
		
	///交易所代码
	TTORATstpExchangeIDType	ExchangeID;

	///投资者代码
	TTORATstpInvestorIDType	InvestorID;

	///投资单元代码
	TTORATstpBusinessUnitIDType	BusinessUnitID;

	///股东账户代码
	TTORATstpShareholderIDType	ShareholderID;

	///合约代码
	TTORATstpSecurityIDType	SecurityID;

	///买卖方向
	TTORATstpDirectionType	Direction;

	///条件单价格类型
	TTORATstpOrderPriceTypeType	OrderPriceType;

	///条件单数量类型
	TTORATstpOrderVolumeTypeType	OrderVolumeType;

	///有效期类型
	TTORATstpTimeConditionType	TimeCondition;

	///成交量类型
	TTORATstpVolumeConditionType	VolumeCondition;

	///报单价格
	TTORATstpPriceType	LimitPrice;

	///报单数量
	TTORATstpVolumeType	VolumeTotalOriginal;

	///组合开平标志
	TTORATstpCombOffsetFlagType	CombOffsetFlag;

	///组合投机套保标志
	TTORATstpCombHedgeFlagType	CombHedgeFlag;

	///条件报单引用
	TTORATstpOrderRefType	CondOrderRef;

	///资金账户代码
	TTORATstpAccountIDType	AccountID;

	///用户代码
	TTORATstpUserIDType	UserID;

	///请求编号
	TTORATstpRequestIDType	RequestID;

	///IP地址
	TTORATstpIPAddressType	IPAddress;

	///Mac地址
	TTORATstpMacAddressType	MacAddress;

	///条件报单编号
	TTORATstpCondOrderIDType	CondOrderID;

	///终端信息
	TTORATstpTerminalInfoType	TerminalInfo;

	///长字符串附加信息
	TTORATstpBigsInfoType	BInfo;

	///短字符串附加信息
	TTORATstpShortsInfoType	SInfo;

	///整形附加信息
	TTORATstpIntInfoType	IInfo;

	///委托方式
	TTORATstpOperwayType	Operway;

	///条件检查
	TTORATstpCondCheckType	CondCheck;

	///触发条件
	TTORATstpContingentConditionType	ContingentCondition;

	///条件价
	TTORATstpPriceType	ConditionPrice;

	///价格浮动tick数
	TTORATstpVolumeType	PriceTicks;

	///数量浮动倍数
	TTORATstpVolumeMultipleType	VolumeMultiple;

	///相关前置编号
	TTORATstpFrontIDType	RelativeFrontID;

	///相关会话编号
	TTORATstpSessionIDType	RelativeSessionID;

	///相关条件参数
	TTORATstpRelativeCondParamType	RelativeParam;

	///附加触发条件
	TTORATstpContingentConditionType	AppendContingentCondition;

	///附加条件价
	TTORATstpPriceType	AppendConditionPrice;

	///附加相关前置编号
	TTORATstpFrontIDType	AppendRelativeFrontID;

	///附加相关会话编号
	TTORATstpSessionIDType	AppendRelativeSessionID;

	///附加相关条件参数
	TTORATstpRelativeCondParamType	AppendRelativeParam;

	///交易日
	TTORATstpDateType	TradingDay;

	///条件单状态
	TTORATstpCondOrderStatusType	CondOrderStatus;

	///报单日期
	TTORATstpDateType	InsertDate;

	///委托时间
	TTORATstpTimeType	InsertTime;

	///撤销时间
	TTORATstpTimeType	CancelTime;

	///撤销用户
	TTORATstpUserIDType	CancelUser;

	///前置编号
	TTORATstpFrontIDType	FrontID;

	///会话编号
	TTORATstpSessionIDType	SessionID;

	///用户端产品信息
	TTORATstpProductInfoType	UserProductInfo;

	///状态信息
	TTORATstpErrorMsgType	StatusMsg;

	///经纪公司部门代码
	TTORATstpDepartmentIDType	DepartmentID;

	///适当性控制业务类别
	TTORATstpProperCtrlBusinessTypeType	ProperCtrlBusinessType;

	///适当性控制通过标示
	TTORATstpProperCtrlPassFlagType	ProperCtrlPassFlag;

	///触发日期
	TTORATstpDateType	ActiveDate;

	///触发时间
	TTORATstpTimeType	ActiveTime;

	///硬盘序列号
	TTORATstpHDSerialType	HDSerial;

	///港股通订单数量类型
	TTORATstpLotTypeType	LotType;


};

/// 查询条件单撤单
struct CTORATstpQryCondOrderActionField
{
		
	///投资者代码
	TTORATstpInvestorIDType	InvestorID;

	///交易所代码
	TTORATstpExchangeIDType	ExchangeID;

	///股东账户代码
	TTORATstpShareholderIDType	ShareholderID;

	///长字符串附加信息
	TTORATstpBigsInfoType	BInfo;

	///短字符串附加信息
	TTORATstpShortsInfoType	SInfo;

	///整形附加信息
	TTORATstpIntInfoType	IInfo;


};

/// 条件单撤单
struct CTORATstpCondOrderActionField
{
		
	///交易所代码
	TTORATstpExchangeIDType	ExchangeID;

	///请求编号
	TTORATstpRequestIDType	RequestID;

	///条件单操作引用
	TTORATstpOrderRefType	CondOrderActionRef;

	///条件单引用
	TTORATstpOrderRefType	CondOrderRef;

	///前置编号
	TTORATstpFrontIDType	FrontID;

	///会话编号
	TTORATstpSessionIDType	SessionID;

	///条件单编号
	TTORATstpCondOrderIDType	CondOrderID;

	///操作标志
	TTORATstpActionFlagType	ActionFlag;

	///投资者代码
	TTORATstpInvestorIDType	InvestorID;

	///合约代码
	TTORATstpSecurityIDType	SecurityID;

	///用户代码
	TTORATstpUserIDType	UserID;

	///条件单撤单编号
	TTORATstpCondOrderIDType	CancelCondOrderID;

	///IP地址
	TTORATstpIPAddressType	IPAddress;

	///MAC地址
	TTORATstpMacAddressType	MacAddress;

	///终端信息
	TTORATstpTerminalInfoType	TerminalInfo;

	///长字符串附加信息
	TTORATstpBigsInfoType	BInfo;

	///短字符串附加信息
	TTORATstpShortsInfoType	SInfo;

	///整形附加信息
	TTORATstpIntInfoType	IInfo;

	///委托方式
	TTORATstpOperwayType	Operway;

	///投资单元代码
	TTORATstpBusinessUnitIDType	BusinessUnitID;

	///股东账户代码
	TTORATstpShareholderIDType	ShareholderID;

	///操作日期
	TTORATstpDateType	ActionDate;

	///操作时间
	TTORATstpTimeType	ActionTime;


};

/// 查询BrokerUserRole
struct CTORATstpQryBrokerUserRoleField
{
		
	///角色编号
	TTORATstpRoleIDType	RoleID;


};

/// BrokerUserRole
struct CTORATstpBrokerUserRoleField
{
		
	///角色编号
	TTORATstpRoleIDType	RoleID;

	///角色描述
	TTORATstpRoleDescriptionType	RoleDescription;

	///功能权限集合
	TTORATstpFunctionsType	Functions;


};

/// 查询BrokerUserRoleAssignment
struct CTORATstpQryBrokerUserRoleAssignmentField
{
		
	///用户代码
	TTORATstpUserIDType	UserID;


};

/// BrokerUserRoleAssignment
struct CTORATstpBrokerUserRoleAssignmentField
{
		
	///用户代码
	TTORATstpUserIDType	UserID;

	///角色编号
	TTORATstpRoleIDType	RoleID;

	///角色描述
	TTORATstpRoleDescriptionType	RoleDescription;


};

/// 查询交易通知
struct CTORATstpQryTradingNoticeField
{
		
	///投资者代码
	TTORATstpInvestorIDType	InvestorID;

	///Insert Date
	TTORATstpDateType	InsertDateStart;

	///Insert Date
	TTORATstpDateType	InsertDateEnd;

	///Insert Time
	TTORATstpTimeType	InsertTimeStart;

	///Insert Time
	TTORATstpTimeType	InsertTimeEnd;


};

/// 查询新股申购配号结果
struct CTORATstpQryIPONumberResultField
{
		
	///申购代码
	TTORATstpSecurityIDType	SecurityID;

	///交易所代码
	TTORATstpExchangeIDType	ExchangeID;

	///股东代码
	TTORATstpShareholderIDType	ShareholderID;


};

/// 新股申购配号结果
struct CTORATstpIPONumberResultField
{
		
	///交易所代码
	TTORATstpExchangeIDType	ExchangeID;

	///证券代码
	TTORATstpSecurityIDType	SecurityID;

	///日期
	TTORATstpDateType	Day;

	///申购证券名称
	TTORATstpSecurityNameType	SecurityName;

	///股东代码
	TTORATstpShareholderIDType	ShareholderID;

	///证券类别代码
	TTORATstpSecurityTypeType	SecurityType;

	///起始配号
	TTORATstpIPONumberIDType	BeginNumberID;

	///配号数量
	TTORATstpVolumeType	Volume;


};

/// 查询新股申购中签结果
struct CTORATstpQryIPOMatchNumberResultField
{
		
	///申购代码
	TTORATstpSecurityIDType	SecurityID;

	///交易所代码
	TTORATstpExchangeIDType	ExchangeID;

	///股东代码
	TTORATstpShareholderIDType	ShareholderID;

	///中签配号
	TTORATstpIPONumberIDType	MatchNumberID;


};

/// 新股申购中签结果
struct CTORATstpIPOMatchNumberResultField
{
		
	///交易所代码
	TTORATstpExchangeIDType	ExchangeID;

	///证券代码
	TTORATstpSecurityIDType	SecurityID;

	///日期
	TTORATstpDateType	Day;

	///申购证券名称
	TTORATstpSecurityNameType	SecurityName;

	///股东代码
	TTORATstpShareholderIDType	ShareholderID;

	///证券类别代码
	TTORATstpSecurityTypeType	SecurityType;

	///中签配号
	TTORATstpIPONumberIDType	MatchNumberID;

	///此中签号拥有的证券数量
	TTORATstpVolumeType	Volume;

	///申购价格
	TTORATstpPriceType	Price;

	///申购金额
	TTORATstpMoneyType	Amout;


};

/// 查询经纪公司部门信息
struct CTORATstpQryInnerBranchInfoField
{
		
	///二级机构代码
	TTORATstpBranchIDType	InnerBranchID;


};

/// 经纪公司部门信息
struct CTORATstpInnerBranchInfoField
{
		
	///二级机构代码
	TTORATstpBranchIDType	InnerBranchID;

	///二级机构名称
	TTORATstpNameType	InnerBranchName;


};

/// 查询机构组织关系基础信息
struct CTORATstpQryDepartmentBranchInfoField
{
		
	///一级机构代码
	TTORATstpDepartmentIDType	DepartmentID;

	///二级机构代码
	TTORATstpBranchIDType	InnerBranchID;


};

/// 机构组织关系基础信息
struct CTORATstpDepartmentBranchInfoField
{
		
	///一级机构代码
	TTORATstpDepartmentIDType	DepartmentID;

	///一级机构名称
	TTORATstpNameType	DepartmentName;

	///二级机构名称
	TTORATstpBranchIDType	InnerBranchID;

	///二级机构名称
	TTORATstpNameType	InnerBranchName;


};

/// 查询机构组织关系基础信息
struct CTORATstpQryDepartmentManageInfoField
{
		
	///一级管理机构代码
	TTORATstpDepartmentIDType	ManageDepartmentID;

	///二级机构代码
	TTORATstpBranchIDType	InnerBranchID;


};

/// 机构组织关系基础信息
struct CTORATstpDepartmentManageInfoField
{
		
	///一级管理机构代码
	TTORATstpDepartmentIDType	ManageDepartmentID;

	///一级管理机构名称
	TTORATstpNameType	DepartmentName;

	///二级机构名称
	TTORATstpBranchIDType	InnerBranchID;

	///二级机构名称
	TTORATstpNameType	InnerBranchName;


};

/// 查询BrokerUser
struct CTORATstpQryBrokerUserField
{
		
	///用户代码
	TTORATstpUserIDType	UserID;

	///一级机构代码
	TTORATstpDepartmentIDType	DepartmentID;


};

/// BrokerUser
struct CTORATstpBrokerUserField
{
		
	///用户代码
	TTORATstpUserIDType	UserID;

	///用户名称
	TTORATstpUserNameType	UserName;

	///用户类型
	TTORATstpUserTypeType	UserType;

	///状态
	TTORATstpActiveStatusType	Status;

	///登录限制
	TTORATstpLoginLimitType	LoginLimit;

	///密码连续输入错误限制
	TTORATstpLoginLimitType	PasswordFailLimit;

	///一级机构编码
	TTORATstpDepartmentIDType	DepartmentID;

	///二级机构编码
	TTORATstpBranchIDType	InnerBranchID;

	///密码修改周期
	TTORATstpCountType	PasswordUpdatePeriod;

	///密码有效剩余天数
	TTORATstpCountType	PasswordRemainDays;

	///一级管理机构代码
	TTORATstpDepartmentIDType	ManageDepartmentID;


};

/// 查询深港通国际市场互联状态信息
struct CTORATstpQrySZSEImcParamsField
{
		
	///市场代码
	TTORATstpMarketIDType	MarketID;


};

/// 深港通国际市场互联状态信息
struct CTORATstpSZSEImcParamsField
{
		
	///市场代码
	TTORATstpMarketIDType	MarketID;

	///是否开放
	TTORATstpBoolType	OpenFlag;

	///初始额度
	TTORATstpMoneyType	ThresholdAmount;

	///日中剩余额度
	TTORATstpMoneyType	PosAmt;

	///额度是否可用
	TTORATstpBoolType	AmountStatus;


};

/// 查询深港通国际市场互联汇率信息
struct CTORATstpQrySZSEImcExchangeRateField
{
		
	///源货币币种
	TTORATstpCurrencyIDType	FromCurrency;

	///目标货币币种
	TTORATstpCurrencyIDType	ToCurrency;


};

/// 深港通国际市场互联汇率信息
struct CTORATstpSZSEImcExchangeRateField
{
		
	///源货币币种
	TTORATstpCurrencyIDType	FromCurrency;

	///目标货币币种
	TTORATstpCurrencyIDType	ToCurrency;

	///参考汇率买入价
	TTORATstpPriceType	BidRate;

	///参考汇率卖出价
	TTORATstpPriceType	OfferRate;

	///参考汇率中间价
	TTORATstpPriceType	MidPointRate;


};

/// 查询深港通最小价差信息
struct CTORATstpQrySZSEHKPriceTickInfoField
{
		
	///价差品种
	TTORATstpPriceTickIDType	PriceTickID;


};

/// 深港通最小价差信息
struct CTORATstpSZSEHKPriceTickInfoField
{
		
	///价差品种
	TTORATstpPriceTickIDType	PriceTickID;

	///价差组号
	TTORATstpPriceTickGroupIDType	PriceTickGroupID;

	///价差类别
	TTORATstpPriceTickTypeType	PriceTickType;

	///价差组起始价格
	TTORATstpPriceType	BeginPrice;

	///价差组结束价格
	TTORATstpPriceType	EndPrice;

	///价差值
	TTORATstpPriceType	PriceTick;


};

/// 查询交易协议
struct CTORATstpQryShareholderSpecPrivilegeField
{
		
	///交易所代码
	TTORATstpExchangeIDType	ExchangeID;

	///股东账户代码
	TTORATstpShareholderIDType	ShareholderID;


};

/// 交易协议
struct CTORATstpShareholderSpecPrivilegeField
{
		
	///交易所代码
	TTORATstpExchangeIDType	ExchangeID;

	///股东账户代码
	TTORATstpShareholderIDType	ShareholderID;

	///市场代码
	TTORATstpMarketIDType	MarketID;

	///交易协议类别
	TTORATstpSpecPrivilegeTypeType	SpecPrivilegeType;

	///买卖方向
	TTORATstpDirectionType	Direction;

	///是否禁止
	TTORATstpBoolType	bForbidden;

	///投资者代码
	TTORATstpInvestorIDType	InvestorID;


};

/// 查询投资者限仓信息
struct CTORATstpQryInvestorPositionLimitField
{
		
	///交易所代码
	TTORATstpExchangeIDType	ExchangeID;

	///投资者代码
	TTORATstpInvestorIDType	InvestorID;

	///证券代码
	TTORATstpSecurityIDType	SecurityID;


};

/// 投资者限仓信息
struct CTORATstpInvestorPositionLimitField
{
		
	///交易所代码
	TTORATstpExchangeIDType	ExchangeID;

	///投资者代码
	TTORATstpInvestorIDType	InvestorID;

	///证券代码
	TTORATstpSecurityIDType	SecurityID;

	///当日买入额度
	TTORATstpVolumeType	BuyLimit;

	///已使用当日买入额度
	TTORATstpVolumeType	BuyFrozen;

	///当日卖出额度
	TTORATstpVolumeType	SellLimit;

	///已使用当日卖出额度
	TTORATstpVolumeType	SellFrozen;

	///当日申购额度
	TTORATstpVolumeType	PurchaseLimit;

	///已使用当日申购额度
	TTORATstpVolumeType	PurchaseFrozen;

	///当日赎回额度
	TTORATstpVolumeType	RedeemLimit;

	///已使用当日赎回额度
	TTORATstpVolumeType	RedeemFrozen;

	///当日质押入库额度
	TTORATstpVolumeType	PledgeInLimit;

	///已使用当日质押入库额度
	TTORATstpVolumeType	PledgeInFrozen;

	///当日质押出库额度
	TTORATstpVolumeType	PledgeOutLimit;

	///已使用当日质押出库额度
	TTORATstpVolumeType	PledgeOutFrozen;

	///当日债转股额度
	TTORATstpVolumeType	ConvertLimit;

	///已使用当日债转股额度
	TTORATstpVolumeType	ConvertFrozen;

	///当日债券回售额度
	TTORATstpVolumeType	PutbackLimit;

	///已使用当日债券回售额度
	TTORATstpVolumeType	PutbackFrozen;

	///当日配股配债额度
	TTORATstpVolumeType	RationalLimit;

	///已使用当日配股配债额度
	TTORATstpVolumeType	RationalFrozen;

	///总持仓额度
	TTORATstpVolumeType	TotalPositionLimit;

	///已使用总持仓额度
	TTORATstpVolumeType	TotalPositionFrozen;


};

/// 查询盘后行情
struct CTORATstpQryPHMarketDataField
{
		
	///证券代码
	TTORATstpSecurityIDType	SecurityID;

	///交易所代码
	TTORATstpExchangeIDType	ExchangeID;


};

#endif