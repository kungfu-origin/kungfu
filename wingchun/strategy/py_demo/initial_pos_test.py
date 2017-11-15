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

'''
test position for new wingchun strategy system.
you may run this program by:
wingchun strategy -n my_test -p basic_usage.py
'''

def initialize(context):
    context.add_td(source=SOURCE.CTP)
    context.pos_set = True

def on_pos(context, pos_handler, request_id, source, rcv_time):
    if request_id == -1:
        if pos_handler is None:
            print '-- got no pos in initial, so req pos --'
            context.req_pos(source=SOURCE.CTP)
            context.pos_set = False
            return
        else:
            print '-- got pos in initial --'
            context.print_pos(pos_handler)
            context.stop()
    else:
        print '-- got pos requested --'
        context.print_pos(pos_handler)
        if not context.pos_set:
            context.data_wrapper.set_pos(pos_handler, source)
        context.stop()