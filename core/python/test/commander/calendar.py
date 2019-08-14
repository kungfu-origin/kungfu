import json
import pyyjj
from kungfu.yijinjing import msg


def calendar_request(cmd_sock, commander_location):
    cmd_sock.send(json.dumps({
        'msg_type': msg.UICalendarRequest,
        'dest': commander_location.uid,
        'data': {}
    }))
    cmd_sock.recv()
    print(cmd_sock.last_message())
