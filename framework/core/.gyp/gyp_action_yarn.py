from gyp_action_lib import Shell

Shell.run(["yarn", *Shell.extract_argv()])
