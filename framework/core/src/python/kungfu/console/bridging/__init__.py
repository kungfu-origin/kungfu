def run_module(module_name):
    from importlib.util import find_spec, module_from_spec

    main_spec = find_spec(module_name)
    main_module = module_from_spec(main_spec)
    main_loader = main_spec.loader
    main_loader.name = main_module.__name__ = "__main__"
    main_loader.exec_module(main_module)


def run_module_main(module_name):
    module_main = f"{module_name}.__main__"
    try:
        run_module(f"kungfu.console.bridging.{module_main}")
    except ModuleNotFoundError:
        run_module(module_main)
