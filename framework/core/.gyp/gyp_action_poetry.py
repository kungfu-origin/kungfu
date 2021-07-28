import gyp_action_lib as lib

lib.run(["yarn", "poetry", "lock", "-n", "-q", "--no-update"])
lib.run(["yarn", "poetry", "install", "-n", "-q", "--no-root"])
