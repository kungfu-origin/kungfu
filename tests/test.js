
// add xtp md config
var mdLocation = kfDebugger.makeMdLocation('xtp')
var mdConfig = {"account_id":"15040900","password":"ZJgFwv6V","md_ip":"120.27.164.138","protocol":"","md_port":6002,"client_id":59}
kfDebugger.setConfig(mdLocation, mdConfig)

//add xtp td config
var tdLocation = kfDebugger.makeTdLocation('xtp', '15040910')
var tdConfig = {
    "account_name":"15040910",
    "account_id":"15040910",
    "password":"E6CtjM60",
    "software_key":"b8aa7173bba3470e390d787219b2112e",
    "td_ip":"61.152.102.111",
    "td_port":8601,
    "client_id":52}
kfDebugger.setConfig(tdLocation, tdConfig)

//add strategy config
var strategyLocation = kfDebugger.makeStrategyLocation('xyz')
var strategyConfig ={"strategy_id":"xyz","strategy_path":"/Users/luqing/Documents/code/kungfu/examples/strategy/py/xtp_demo.py"}
kfDebugger.setConfig(strategyLocation,  strategyConfig)

//start md
kfDebugger.startProcess(mdLocation)

//start td
kfDebugger.startProcess(tdLocation)

//setting target account & strategy location
kfDebugger.setTargetAccountLocation(tdLocation)

//
kfDebugger.buy('600000', 10.5, 200)

kfDebugger.sell('600000', 10.5, 200)
