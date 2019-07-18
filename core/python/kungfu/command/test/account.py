import pyyjj
import json
import click
from kungfu.command.test import test, pass_ctx_from_parent, send_to_passive
import kungfu.nanomsg as nn
import time
import pywingchun
import copy
import os
import subprocess

#insert_order = json.loads(str(pywingchun.OrderInput()))
#rtn_order = json.loads(str(pywingchun.Order()))
#rtn_trade = json.loads(str(pywingchun.Trade()))

#insert_order = {'account_id': '', 'exchange_id': '', 'frozen_price': 0.0, 'instrument_id': '', 'instrument_type': 0, 'limit_price': 0.0, 'offset': 0, 'order_id': 0, 'parent_id': 0, 'price_type': 0, 'side': 0, 'time_condition': 0, 'volume': 0, 'volume_condition': 0}
#rtn_order = {'account_id': '', 'client_id': '', 'commission': 0.0, 'error_id': 0, 'error_msg': '', 'exchange_id': '', 'frozen_price': 0.0, 'insert_time': 0, 'instrument_id': '', 'instrument_type': 0, 'limit_price': 0.0, 'offset': 0, 'order_id': '0', 'parent_id': 0, 'price_type': 0, 'side': 0, 'status': 0, 'tax': 0.0, 'time_condition': 0, 'trading_day': '', 'update_time': 0, 'volume': 0, 'volume_condition': 0, 'volume_left': 0, 'volume_traded': 0}
#rtn_trade = {'account_id': '', 'client_id': '', 'commission': 0.0, 'exchange_id': '', 'instrument_id': '', 'instrument_type': 0, 'offset': 0, 'order_id': '0', 'price': 0.0, 'side': 0, 'tax': 0.0, 'trade_id': 0, 'trade_time': 0, 'volume': 0}

# Error = 0,
# Quote = 101,
# Entrust = 102,
# Transaction = 103,
# OrderInput = 201,
# OrderAction = 202,
# Order = 203,
# Trade = 204,
# Position = 205,
# AssetInfo = 206,
# AssetInfoSnapshot = 207,
# PositionDetail = 208,
# Subscribe = 302,
# GatewayState = 401,
# SwitchDay = 601,
# RspTradingDay = 602,
# PositionEnd = 800,
# PositionDetailEnd = 801,
# PassiveCtrl = 10099

insert_order = {'account_id': 'test', 'exchange_id': 'SSE', 'frozen_price': 13.6, 'instrument_id': '600000', 'instrument_type': 1, 'limit_price': 13.7, 'offset': 0, 'order_id': 12000001, 'parent_id': 0, 'price_type': 3, 'side': 0, 'time_condition': 0, 'volume': 10000, 'volume_condition': 0}

def get_insert_order(insert_order):
    return {'msg_type': 201,  'data':insert_order}

def get_rtn_order(insert_order, status):
    # Unknown, 0
    # Submitted, 1
    # Pending, 2
    # Cancelled, 3
    # Error, 4 
    # Filled, 5
    # PartialFilledNotActive, 6
    # PartialFilledActive 7
    rtn_order = {'account_id': '', 'client_id': '', 'commission': 0.0, 'error_id': 0, 'error_msg': '', 'exchange_id': '', 'frozen_price': 0.0, 'insert_time': 0, 'instrument_id': '', 'instrument_type': 0, 'limit_price': 0.0, 'offset': 0, 'order_id': '0', 'parent_id': 0, 'price_type': 0, 'side': 0, 'status': 0, 'tax': 0.0, 'time_condition': 0, 'trading_day': '', 'update_time': 0, 'volume': 0, 'volume_condition': 0, 'volume_left': 0, 'volume_traded': 0}
    for key in insert_order.keys():
        if key in rtn_order.keys():
            rtn_order[key] = insert_order[key]
    rtn_order['status'] = status
    return {'msg_type': 203,  'data':rtn_order}

def get_rtn_trade(insert_order,volume):
    rtn_trade = {'account_id': '', 'client_id': '', 'commission': 0.0, 'exchange_id': '', 'instrument_id': '', 'instrument_type': 0, 'offset': 0, 'order_id': '0', 'price': 0.0, 'side': 0, 'tax': 0.0, 'trade_id': 0, 'trade_time': 0, 'volume': 0}
    for key in insert_order.keys():
        if key in rtn_trade.keys():
            rtn_trade[key] = insert_order[key]
    rtn_trade['volume'] = volume
    return {'msg_type': 204,  'data':rtn_trade}


login_test_dict = {1:((3,4),(3,4),"行情登录成功"),
                   2:((3,5),(3,5),"行情登录失败"),
                   3:((3,4,2),(3,4,2),"行情断连"),
                   4:((3,4,2),(3,4,2),"交易登录成功"),
                   5:((3,4,2),(3,4,2),"交易柜台可连接账户密码错误"),
                   6:((3,4,2),(3,4,2),"交易柜台可连接，可查询资金不可查询持仓"),
                   7:((3,4,2),(3,4,2),"交易柜台可连接，一个账户可登录")}

def login_test(test_case, watcher_pub_sock, ctx, td_passive_rep_sock, td_location):
    expected_result = []
    time_start=time.time()
    test_time = 30
    for status in login_test_dict[test_case][0]:
        send_to_passive(ctx, td_passive_rep_sock, td_location, {'msg_type': 401,'state': status})
    for i in range(len(login_test_dict[test_case][1])):
        watcher_pub_sock.recv(0)
        expected_result.append(json.loads(watcher_pub_sock.last_messsage()))
        assert(expected_result[-1]['data']['state'] == login_test_dict[test_case][1][i])
        if i == len(login_test_dict[test_case][1]):
            print(login_test_dict[test_case][1][2],"测试通过")


trade_test_dict = {1:((get_insert_order(insert_order), get_rtn_order(insert_order, 1)),(get_insert_order(insert_order),get_rtn_order(insert_order, 1)),"报单成功"),
                   2:((get_insert_order(insert_order), get_rtn_order(insert_order, 2)),(get_insert_order(insert_order), get_rtn_order(insert_order, 2)),"订单排队"),
                   3:((get_insert_order(insert_order), get_rtn_trade(insert_order,5000)),(get_insert_order(insert_order), get_rtn_trade(insert_order,5000)),"成交回报"),
                   4:((get_insert_order(insert_order), get_rtn_order(insert_order, 7)),(get_insert_order(insert_order), get_rtn_order(insert_order, 7)),"部分成交在排队"),
                   5:((get_insert_order(insert_order), get_rtn_order(insert_order, 5)),(get_insert_order(insert_order), get_rtn_order(insert_order, 5)),"全部成交"),
                   6:((get_insert_order(insert_order), get_rtn_order(insert_order, 6)),(get_insert_order(insert_order), get_rtn_order(insert_order, 6)),"部分成交撤单"),
                   6:((get_insert_order(insert_order), get_rtn_order(insert_order, 3)),(get_insert_order(insert_order), get_rtn_order(insert_order, 3)),"撤单")}

def trade_test(test_case, watcher_pub_sock, ctx, td_passive_rep_sock, td_location):
    expected_result = []
    time_start=time.time()
    test_time = 30
    os.system("yarn run dev -l trace strategy -n test -p /Users/qliu/taurus/kungfu/core/python/kungfu/command/test/test_strategy.py &")
    time.sleep(5)
    for msg in trade_test_dict[test_case][0][1:]:
        send_to_passive(ctx, td_passive_rep_sock, td_location, msg)
        time.sleep(1)
    watcher_pub_sock.recv(0)
    expected_result.append(json.loads(watcher_pub_sock.last_messsage()))
    assert(expected_result[-1]['data']['data'] == trade_test_dict[test_case][1][0]['data'])
    watcher_pub_sock.recv(0)
    expected_result.append(json.loads(watcher_pub_sock.last_messsage()))
    assert(expected_result[-1]['data']['data'] == trade_test_dict[test_case][1][1]['data'])
    # for i in range(len(trade_test_dict[test_case][1])):
    #     watcher_pub_sock.recv(0)
    #     expected_result.append(json.loads(watcher_pub_sock.last_messsage()))
    #     assert(expected_result[-1]['data']['data'] == trade_test_dict[test_case][1][i]['data'])
    #     if i == len(trade_test_dict[test_case][1]):
    #         print(trade_test_dict[test_case][1][2],"测试通过")


@test.command()
@click.option('-t', '--type', type=int, help='test type')
@click.option('-c', '--case', type=int, help='test case')
@click.pass_context
def account_test(ctx, type, case):
    # master_proc = subprocess.Popen(("yarn dev -l trace master &").split(), stdout=subprocess.PIPE, stderr=subprocess.PIPE, preexec_fn=os.setsid)
    # time.sleep(2)
    # os.system("yarn dev -l trace watcher &"); time.sleep(2)
    # os.system("yarn dev -l trace md -s passive &"); time.sleep(2)
    # os.system("yarn dev -l trace td -s passive -a test &"); time.sleep(5)
    pass_ctx_from_parent(ctx)
    test_io_location = pyyjj.location(pyyjj.mode.LIVE, pyyjj.category.SYSTEM, "test", "test", ctx.locator)
    io_device = pyyjj.io_device_client(test_io_location, False)
    click.echo('account test')
    td_location = pyyjj.location(pyyjj.mode.LIVE, pyyjj.category.TD, 'passive', 'test', ctx.locator)
    td_passive_rep_sock = ctx.io_device.connect_socket(td_location, pyyjj.protocol.REQUEST)
    # send_to_passive(ctx, td_passive_rep_sock, td_location, {'msg_type': 401,'state': 3})

    watcher_location = pyyjj.location(pyyjj.mode.LIVE, pyyjj.category.SYSTEM, "watcher", "watcher", ctx.locator)
    watcher_pub_sock = io_device.connect_socket(watcher_location, pyyjj.protocol.SUBSCRIBE, 30 * 1000)
    watcher_pub_sock.setsockopt(nn.SUB, nn.SUB_SUBSCRIBE, '')
    if type == 1:
        login_test(case, watcher_pub_sock, ctx, td_passive_rep_sock, td_location)
    if type == 2:
        trade_test(case, watcher_pub_sock, ctx, td_passive_rep_sock, td_location)
    # os.killpg(os.getpgid(master_proc.pid), signal.SIGTERM)
    # master_proc.wait()


# @test.command()
# @click.pass_context
# def account(ctx):
#     master_proc = subprocess.Popen(("yarn dev -l trace master &").split(), stdout=subprocess.PIPE, stderr=subprocess.PIPE, preexec_fn=os.setsid)
#     time.sleep(2)
#     #os.system("yarn dev -l trace master &"); time.sleep(2)
#     os.system("yarn dev -l trace watcher &"); time.sleep(2)
#     os.system("yarn dev -l trace md -s passive &"); time.sleep(2)
#     os.system("yarn dev -l trace td -s passive -a test &"); time.sleep(5)
#     pass_ctx_from_parent(ctx)
#     click.echo('account')
#     td_location = pyyjj.location(pyyjj.mode.LIVE, pyyjj.category.TD, 'passive', 'test', ctx.locator)
#     td_passive_rep_sock = ctx.io_device.connect_socket(td_location, pyyjj.protocol.REQUEST)
#     send_to_passive(ctx, td_passive_rep_sock, td_location, {'msg_type': 401,'state': 3})
#     time.sleep(1)
#     os.killpg(os.getpgid(master_proc.pid), signal.SIGTERM)
#     master_proc.wait()
@test.command()
@click.pass_context
def account(ctx):
    pass_ctx_from_parent(ctx)
    click.echo('account')
    td_location = pyyjj.location(pyyjj.mode.LIVE, pyyjj.category.TD, 'passive', 'test', ctx.locator)
    td_passive_rep_sock = ctx.io_device.connect_socket(td_location, pyyjj.protocol.REQUEST)
    send_to_passive(ctx, td_passive_rep_sock, td_location, {'msg_type': 401,'state': 3})
    time.sleep(1)
    
