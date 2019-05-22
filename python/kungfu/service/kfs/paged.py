import os
import psutil
import nnpy, pyyjj

import kungfu.service.kfs as kfs

@kfs.on('journal/register')
def register_journal(ctx, request):
    name = request['name']
    result = {
        'success': False,
        'error_msg': '',
        'memory_msg_idx': -1
    }
    if not name in ctx._client_info:
        result['error_msg'] = 'client ' + name + ' does not exist'
    else:
        client = ctx._client_info[name]
        idx = ctx._page_service.register_journal(name)
        if idx >= 1000:
            result['error_msg'] = 'idx exceeds limit'
        else:
            result['success'] = True
            result['memory_msg_idx'] = idx
            client['journals'].append(idx)
    return result

@kfs.on('client/register/writer')
def register_client_read(ctx, request):
    return register_client(ctx, request, False)

@kfs.on('client/register/reader')
def register_client_read(ctx, request):
    return register_client(ctx, request, True)

@kfs.on('client/exit')
def exit_client(ctx, request):
    name = request['name']
    hash_code = request['hash_code']
    result = {
        'success': False,
        'error_msg': ''
    }
    release_client(ctx, name, hash_code, True, result)
    return result

def register_client(ctx, request, is_writer):
    name = request['name']
    pid = request['pid']
    result = {
        'success': False,
        'error_msg': '',
        'memory_msg_file': ctx._page_service.memory_msg_file(),
        'file_size': ctx._page_service.memory_msg_file_size(),
        'hash_code': -1
    }
    if name in ctx._client_info:
        result['error_msg'] = 'client ' + name + ' already exists'
    else:
        hash_code = ctx._page_service.register_client(name, pid, is_writer)
        if not pid in ctx._client_processes:
            ctx._client_processes[pid] = {}
            ctx._client_processes[pid]['process'] = psutil.Process(pid)
            ctx._client_processes[pid]['client_info'] = []
        ctx._client_processes[pid]['client_info'].append(name)
        ctx._client_info[name] = {
            'journals': [],
            'reg_nano': pyyjj.nano_time(),
            'is_writing': is_writer,
            'is_strategy': False,
            'rid_start': -1,
            'rid_end': -1,
            'pid': pid,
            'hash_code': hash_code
        }
        result['success'] = True
        result['hash_code'] = hash_code
    return result

def release_client(ctx, name, hash_code, validation, result):
    if not name in ctx._client_info:
        result['error_msg'] = 'client ' + name + ' does not exist'
        return
    client = ctx._client_info[name]
    if validation and hash_code != client['hash_code']:
        result['error_msg'] = 'hash code validation failed ' + str(hash_code) + '!=' + str(client['hash_code'])
    else:
        for idx in client['journals']:
            ctx._page_service.release_page(idx)
        del ctx._client_info[name]
        result['success'] = True