######### start
### para
_TradingLot = 100    # 开仓数量
_LongShort = "short" # 做多/做空
_Start = 14:30       # 交易开始时间
_End = 14:45         # 交易结束时间
_MaxBackward = 12%   # 交易最大年化贴水
_TimeGap = 5         # 每5秒交易一次

### def
flag = 0
iniLot = 1     # 每次下单1张
timeRemain = (now() - _End) / (_Start - _End)
tradeRemain = 1 - flag / _TradingLot
	
### getData
zz500Ind = get(中证500指数点位)
zz500F0 = get(中证500股指期货当月合约价格)
zz500F1 = get(中证500股指期货下月合约价格)
zz500F2 = get(中证500股指期货当季合约价格)
zz500F3 = get(中证500股指期货下季合约价格)

zz500F0_T = get(中证500股指期货当月合约到期日) - today() # 中证500股指期货当月合约剩余天数
zz500F1_T = get(中证500股指期货下月合约到期日) - today() # 中证500股指期货下月合约剩余天数
zz500F2_T = get(中证500股指期货当季合约到期日) - today() # 中证500股指期货当季合约剩余天数
zz500F3_T = get(中证500股指期货下季合约到期日) - today() # 中证500股指期货下季合约剩余天数

zz500F0_L = get(中证500股指期货当月合约过去一周平均成交量(_Start, _End)) # 中证500股指期货当月合约指定交易时间过去一周平均成交量
zz500F1_L = get(中证500股指期货下月合约过去一周平均成交量(_Start, _End)) # 中证500股指期货下月合约指定交易时间过去一周平均成交量
zz500F2_L = get(中证500股指期货当季合约过去一周平均成交量(_Start, _End)) # 中证500股指期货当季合约指定交易时间过去一周平均成交量
zz500F3_L = get(中证500股指期货下季合约过去一周平均成交量(_Start, _End)) # 中证500股指期货下季合约指定交易时间过去一周平均成交量

### process
zz500F0_Backward = if(zz500FT0 = 0, 0, abs((zz500F0 - zz500Ind) / zz500F0 / zz500F0_T * 365)) # 中证500股指期货当月合约年化贴水
zz500F1_Backward = abs((zz500F1 - zz500Ind) / zz500F1 / zz500F1_T) * 365 # 中证500股指期货下月合约年化贴水
zz500F2_Backward = abs((zz500F2 - zz500Ind) / zz500F2 / zz500F2_T) * 365 # 中证500股指期货当季合约年化贴水
zz500F3_Backward = abs((zz500F3 - zz500Ind) / zz500F3 / zz500F3_T) * 365 # 中证500股指期货下季合约年化贴水

while(now() >= _Start & now() < _End){
    sleep(_TimeGap) # 每5秒交易一次
    
    minBackward = min(zz500F0_Backward, zz500F1_Backward, zz500F2_Backward, zz500F3_Backward) # 计算最小年化贴水
    minBackward_name = whichmin(zz500F0_Backward, zz500F1_Backward, zz500F2_Backward, zz500F3_Backward) # 0当月, 1下月, 2当季, 3下季

    if(minBackward > _MaxBackward){
        tradeCon = whichmax_L(zz500F1_T, zz500F2_T, zz500F3_T) # 找出流动性最大的合约
        trade(tradeCon, lot = iniLot, direction = _LongShort) # 交易一次
        flag = flag + lot
        
    }
    else{
        tradeCon = whichmax_L(which(贴水 < _MaxBackward)) # 在贴水小于_MaxBackward的合约中，找出流动性最大的合约，排除当月合约
        trade(tradeCon, lot = iniLot, direction = _LongShort) # 交易一次
        flag = flag + lot
    }

    if(tradeRemain - timeRemain > 0.2){
        warning(tradeRemain & "remain")
        if(iniLot < 3) iniLot = iniLot + 1
    }

    if(tradeRemain - timeRemain > 0.5){
        warning(tradeRemain & "remain")
        if(iniLot < 5) iniLot = iniLot + 2
    }

    if(flag >= _TradingLot){stop trading}	


}



######### end