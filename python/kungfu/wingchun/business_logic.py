'''
Copyright [2017] [taurus.ai]

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
'''

from kungfu.longfist.longfist_utils import _byteify as _byteify
import kungfu.longfist.longfist_constants as lf
import kungfu.longfist.longfist_structs as lf_structs
import kungfu.longfist.longfist_utils as lf_utils

import ctypes
import libwingchunbl

bl = {}
rid_info = {}

class BusinessLogic(libwingchunbl.WcStrategy):

    def __init__(self, name, source):
        self.source = source
        libwingchunbl.WcStrategy.__init__(self, name, -1)

    # [WcStrategy]: must-implemented function
    def get_source(self):
        return self.source

    # [WcStrategy]: must-implemented function
    def get_tds(self):
        return [self.get_source()]

    # [WcStrategy]: must-implemented function
    def get_mds(self):
        return []

    def init_misc(self):
        pass

    def on_data(self, frame, msg_type, request_id, source, rcv_time):
        if msg_type == lf.MsgTypes.RTN_ORDER:
            # got a return order
            data = lf_utils.dcast_frame(frame)
            self.on_rtn_order(data, request_id, source, rcv_time)
        elif msg_type == lf.MsgTypes.RTN_TRADE:
            # got a return trade
            data = lf_utils.dcast_frame(frame)
            self.on_rtn_trade(data, request_id, source, rcv_time)
        elif msg_type == lf.MsgTypes.ORDER:
            if frame.error_id() != 0:
                # if this order is blocked by some reason...
                data = lf_utils.dcast_frame(frame)
                self.on_order_err(data, frame.error_id(), frame.error_msg(), request_id, source, rcv_time)

    def on_rtn_order(self, data, request_id, source, rcv_time):
        global rid_info
        ticker = data.InstrumentID
        msg = '[ORDER] (s){} (tm){} (rid){} (lid){} (ticker){} (v_tot){} (v_traded){} (v_remain){} (d){} (ofs){} (status){}'.format(
            source, rcv_time, request_id, data.OrderRef, ticker, data.VolumeTotalOriginal, data.VolumeTraded, data.VolumeTotal, data.Direction, data.OffsetFlag, data.OrderStatus)
        if request_id in rid_info.keys():
            rid_info[request_id].append(msg)

    def on_rtn_trade(self, data, request_id, source, rcv_time):
        global rid_info
        ticker = data.InstrumentID
        msg = '[TRADE] (s){} (tm){} (rid){} (lid){} (ticker){} (p){} (v){} (d){} (ofs){}'.format(
            source, rcv_time, request_id, data.OrderRef, ticker, data.Price, data.Volume, data.Direction, data.OffsetFlag)
        if request_id in rid_info.keys():
            rid_info[request_id].append(msg)

    def on_order_err(self, data, error_id, error_msg, request_id, source, rcv_time):
        global rid_info
        ticker = data.InstrumentID
        msg = '[REJECTED] (rid){} (ticker){} (source){} (errId){} (errMsg){}'.format(request_id, ticker, source, error_id, error_msg)
        if request_id in rid_info.keys():
            rid_info[request_id].append(msg)

    def insert_order(self, info):
        source = info['Source']
        operator_id = info['OperatorId']

        order = lf_structs.LFInputOrderField()
        order.InstrumentID = info['InstrumentID']
        order.Volume = info['Volume']
        order.TimeCondition = info['TimeCondition']
        order.VolumeCondition = info['VolumeCondition']
        order.LimitPrice = info['LimitPrice']
        order.Direction = info['Direction']
        order.OffsetFlag = info['OffsetFlag']
        order.OrderPriceType = info['OrderPriceType']
        order.ForceCloseReason = info['ForceCloseReason']
        order.ContingentCondition = info['ContingentCondition']
        order.HedgeFlag = info['HedgeFlag']
        order.MinVolume = info['MinVolume']
        order.MiscInfo = operator_id

        rid = self.get_rid()
        self.req_insert_order(ctypes.addressof(order), rid, source)
        return rid

def insert_order(js):
    global bl, rid_info
    info = _byteify(js)
    source = info['Source']
    pm_id = info['PmId']
    product_id = info['ProductId']
    name = '{}_{}_{}'.format(product_id, 'Manual', pm_id) # product, strategy, pm
    key = (source, name)
    if bl.get(key, None) is None:
        _bl = BusinessLogic(name, source)
        _bl.init()
        _bl.run()
        bl[key] = _bl

    rid = bl[key].insert_order(info)
    rid_info[rid] = []
    return rid

def get_order_info(rid):
    global rid_info
    return rid_info.get(rid, None)

def refresh():
    global bl
    for _bl in bl.values():
        _bl.stop()
    bl = {}