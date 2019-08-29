import json
import pyyjj
from kungfu.wingchun import msg


def new_order_single(cmd_sock, commander_location, account):
    cmd_sock.send(json.dumps({
        'msg_type': msg.NewOrderSingle,
        'dest': commander_location.uid,
        'data': {
            'mode': 'live',
            'category': 'td',
            'group': 'xtp',
            'name': account,
            'orderinput': {}
        }
    }))
    cmd_sock.recv()
    print(cmd_sock.last_message())


def cancel_order(cmd_sock, commander_location, account, order_id):
    cmd_sock.send(json.dumps({
        'msg_type': msg.CancelOrder,
        'dest': commander_location.uid,
        'data': {
            'mode': 'live',
            'category': 'td',
            'group': 'xtp',
            'name': account,
            'order_id': order_id
        }
    }))
    cmd_sock.recv()
    print(cmd_sock.last_message())

def cancel_all_order_for_account(cmd_sock, commander_location, account):
    print("cancel all order for account {}".format(account))
    message = json.dumps({
        'msg_type': msg.CancelAllOrder,
        'dest': commander_location.uid,
        'data': {
            'mode': 'live',
            'category': 'td',
            'group': 'ctp',
            'name': account
        }
    })
    cmd_sock.send(message)
    print("message {} sent".format(message))
    cmd_sock.recv()
    print("message {} received".format(cmd_sock.last_message()))

def cancel_all_order_for_strategy(cmd_sock, commander_location, name):
    print("cancel all order for strategy {}".format(name))
    message = json.dumps({
        'msg_type': msg.CancelAllOrder,
        'dest': commander_location.uid,
        'data': {
            'mode': 'live',
            'category': 'strategy',
            'group': 'default',
            'name': name
        }
    })
    cmd_sock.send(message)
    print("message {} sent".format(message))
    cmd_sock.recv()
    print("message {} received".format(cmd_sock.last_message()))
