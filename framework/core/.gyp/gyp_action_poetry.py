from gyp_action_lib import Shell

Shell.run(["yarn", "-s", "poetry", "lock", "-q"])
Shell.run(["yarn", "-s", "poetry", "install", "-q"])
Shell.touch("poetry.lock")
