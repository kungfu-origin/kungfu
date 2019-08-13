import json
import pyyjj
from kungfu.yijinjing import msg


def send_order(cmd_sock, commander_location):
    cmd_sock.send(json.dumps({
        'msg_type': msg.UIActionCancelAllOrder,
        'dest': commander_location.uid,
        'data': {
            'orderinput': {}
        }
    }))
    cmd_sock.recv()
    return cmd_sock.last_message()
