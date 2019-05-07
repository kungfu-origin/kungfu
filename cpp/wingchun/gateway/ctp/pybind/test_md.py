

import xtp
client_id = 1
ip = "120.27.164.138"
port = 6002
software_key = "b8aa7173bba3470e390d787219b2112e"
user_id = "15040900"
password = "ZJgFwv6V"
save_path_file = "/shared/kungfu/runtime"

md = xtp.MdService(client_id, save_path_file, ip, port, user_id, password)
print("starting...")
md.run()
