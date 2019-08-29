
import json
import pyyjj
from kungfu.wingchun import msg

def broker_state_request(cmd_sock, commander_location):
    print("request broker state")
    message = json.dumps({
        'msg_type': msg.BrokerStateRefresh,
        'dest': commander_location.uid,
        'data': {}})
    cmd_sock.send(message)
    print("message {} sent".format(message))
    cmd_sock.recv()
    print("message {} received".format(cmd_sock.last_message()))