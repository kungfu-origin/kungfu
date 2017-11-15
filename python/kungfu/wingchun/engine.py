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

import signal, json
from kungfu.longfist.longfist_utils import _byteify as _byteify

def register_exit_handler(engine, name, tp):
    def exit_handler(*args):
        print 'stop engine: {}_{}'.format(tp, name)
        engine.stop()
    signal.signal(signal.SIGTERM, exit_handler)

class Engine(object):

    account_file = '/opt/kungfu/master/etc/kungfu/kungfu.json'

    def __init__(self, name, tp):
        lib_name = 'lib{}{}'.format(name, tp)
        lib = None
        try:
            lib = __import__(lib_name)
        except:
            print 'Unexpected lib is imported', lib_name
            exit(1)

        globals()[lib_name] = lib
        self.core_engine = lib.Engine()
        self.api_name = name
        self.tp = tp
        fin = open(self.account_file)
        try:
            # we first parse account.json and get account info
            json_info = json.load(fin, object_hook=_byteify)[tp][name]
            # then simply dump to str and will init
            self.info = json.dumps(json_info)
        except Exception as e:
            print 'Cannot find related account info: ', tp, name, ' in ', self.account_file
            print e
            exit(1)

    def start(self):
        register_exit_handler(self.core_engine, self.api_name, self.tp)
        self.core_engine.init(self.info)
        self.core_engine.start()
        self.core_engine.wait_for_stop()