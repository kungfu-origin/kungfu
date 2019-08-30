
import json
import pyyjj
from kungfu.wingchun import msg

def remove_strategy_request(cmd_sock, commander_location):
    print("remove_strategy_request")
    message = json.dumps({
        'msg_type': msg.RemoveStrategy,
        'dest': commander_location.uid,
        'data': {"mode": "live", "category": "strategy", "group": "default", "name": "test"}})
    cmd_sock.send(message)
    print("message {} sent".format(message))
    cmd_sock.recv()
    print("message {} received".format(cmd_sock.last_message()))