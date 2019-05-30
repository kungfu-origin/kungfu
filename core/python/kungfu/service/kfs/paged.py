import os
import psutil
import pyyjj

import kungfu.service.kfs as kfs

@kfs.on('journal/register')
def journal_register(ctx, request):
    pid = request['pid']
    mode = request['mode']
    category = request['category']
    group = request['group']
    name = request['name']
    is_writing = request['is_writing']
    if pid not in ctx._apprentices:
        return {
            'success': False,
            'error_msg': 'apprentice not checked in'
        }
    mm_id = ctx._page_service.get_mm_block(mode, category, group, name, is_writing)
    if mm_id < 0:
        return {
            'success': False,
            'error_msg': 'unable to allocate memory msg block'
        }
    else:
        ctx._apprentices[pid]['mm_ids'].append(mm_id)
        return {
            'file_size': ctx._page_service._memory_msg_file_size,
            'memory_msg_file': ctx._page_service._memory_msg_file,
            'memory_msg_id': mm_id,
            'success': True
        }

@kfs.on('journal/deregister')
def journal_deregister(ctx, request):
    pid = request['pid']
    for mm_id in ctx._apprentices[pid]['mm_ids']:
        ctx._page_service.release_mm_block(mm_id)
    return {
        'success': True
    }
