import json
from kungfu.wingchun import msg

def asset_request(cmd_sock, commander_location):
    cmd_sock.send(json.dumps({
        'msg_type': msg.QryAsset,
        'dest': commander_location.uid,
        'data': {"ledger_category": 0, "account_id": "089270", "source_id": "ctp", "client_id": ""}
    }))
    cmd_sock.recv()
    print("reply {} for request: {}".format(cmd_sock.last_message(), msg.QryAsset))
