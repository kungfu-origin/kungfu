#  SPDX-License-Identifier: Apache-2.0

import click
import json
import kungfu
import re
import sys
import pandas
from kungfu.console.commands import kfc, PrioritizedCommandGroup
from kungfu.yijinjing.log import find_logger

lf = kungfu.__binding__.longfist
wc = kungfu.__binding__.wingchun
yjj = kungfu.__binding__.yijinjing

MODES = {
    "live": lf.enums.mode.LIVE,
    "data": lf.enums.mode.DATA,
    "replay": lf.enums.mode.REPLAY,
    "backtest": lf.enums.mode.BACKTEST,
    "*": lf.enums.mode.LIVE,
}

CATEGORIES = {
    "td": lf.enums.category.TD,
    "md": lf.enums.category.MD,
    "strategy": lf.enums.category.STRATEGY,
    "system": lf.enums.category.SYSTEM,
}

ASSEMBLE_MODE = {
    "channel": lf.enums.AssembleMode.Channel,
    "all": lf.enums.AssembleMode.All,
    "write": lf.enums.AssembleMode.Write,
    "read": lf.enums.AssembleMode.Read,
    "public": lf.enums.AssembleMode.Public,
    "sync": lf.enums.AssembleMode.Sync,

}


@kfc.command(help_priority=1)
@click.argument("reference", type=str, required=False)
@kfc.pass_context()
def assemble(ctx, reference):
    ctx.path = reference

    with open(ctx.path, mode="r", encoding="utf8") as file_json:
        config = json.load(file_json)
        source = config["source"]
        dest = config["dest"]
        if "locator_path" in config:
            source_locator = yjj.locator(config["locator_path"])
        else:
            source_locator = yjj.locator(
                MODES[source.get("mode", "live").lower()], source.get("tags", [])
            )

        source_location = yjj.location(
            MODES[source.get("mode", "live").lower()],
            CATEGORIES[source.get("category", "MD").lower()],
            source.get("group", "sim"),
            source.get("name", "sim"),
            source_locator,
        )
        ctx.log = find_logger(source_location, ctx.log_level)

        ctx.log.info(f"config_json: {config}")
        ctx.log.info(f"source_location: {source_location}")
        ctx.log.info(f"source_locator: {source_locator}")
        ctx.log.info(f"source {source}")
        ctx.log.info(f"dest {dest}")

        if "dest_id" in dest:
            dest_id = dest["dest_id"]
        else:
            dest_location = yjj.location(
                MODES[dest.get("mode", "live").lower()],
                CATEGORIES[dest.get("category", "MD").lower()],
                dest.get("group", "sim"),
                dest.get("name", "sim"),
                source_locator,
            )
            dest_id = dest_location.uid
            ctx.log.info(f"dest_location: {dest_location}")
        ctx.log.info(f"dest_id: {dest_id}")

        if "AssembleMode" in config:
            config_assemble_mode = config["AssembleMode"]
            if isinstance(config_assemble_mode, str):
                assemble_mode = ASSEMBLE_MODE[config.get("AssembleMode", "All").lower()]
            elif isinstance(config_assemble_mode, int):
                assemble_mode = config_assemble_mode
            else:
                ctx.log.info(
                    f"illegal AssembleMode: {config_assemble_mode}, type: {type(config_assemble_mode)}"
                )
        else:
            ctx.log.info(f"lack of parameter AssembleMode, use default value: channel")
            assemble_mode = ASSEMBLE_MODE["channel"]

        asb = yjj.assemble(
            source_location,
            dest_id,
            assemble_mode,
            config.get("from_time", 0),
        )

        arguments = []
        argv = config.get("arguments", [])
        for idx, param in enumerate(argv):
            if param in dir(lf.types):
                arguments.append(getattr(lf.types, param)())
            else:
                arguments.append(param)

        fn_name = config.get("function")
        fn = getattr(asb, fn_name)
        datas = fn(*arguments)

        def log_data():
            for data in datas:
                ctx.log.info(f"data: {data}")

        if "export" in config:
            export_layout = config.get("export")
            ctx.log.info(f"export: {export_layout}")

            if len(datas) < 1:
                ctx.log.warn(f"result of {fn_name}(*{argv}) is empty!")
                sys.exit(0)

            def deal_lf_types():
                data_list = []
                for data in datas:
                    d = {}
                    if hasattr(lf.types, type(data).__name__):
                        for t in dir(data):
                            if not t.startswith("__"):
                                d[t] = getattr(data, t)
                    data_list.append(d)
                return pandas.DataFrame(data_list)

            def deal_tuple():
                header_list = []
                data_list = []
                for data in datas:
                    h = {}
                    d = {}
                    if isinstance(data[0], lf.types.frame_header):
                        for t in dir(data[0]):
                            if not t.startswith("__"):
                                h[t] = getattr(data[0], t)
                    else:
                        ctx.log.error(
                            f"illegal data[0] type: {type(data[0])}, {data[0]}"
                        )

                    if hasattr(lf.types, type(data[1]).__name__):
                        for t in dir(data[1]):
                            if not t.startswith("__"):
                                d[t] = getattr(data[1], t)
                    else:
                        d["data"] = data[1]

                    header_list.append(h)
                    data_list.append(d)
                return pandas.concat(
                    [pandas.DataFrame(header_list), pandas.DataFrame(data_list)], axis=1
                )

            def get_df():
                if isinstance(datas[0], tuple):
                    return deal_tuple()
                elif hasattr(lf.types, type(datas[0]).__name__):
                    return deal_lf_types()
                else:
                    ctx.log.error(f"illegal type: {type(datas[0])}")
                    sys.exit(-1)

            param_name = config.get("arguments", [""])[0]
            now = yjj.strfnow().replace(" ", "_").replace(":", "")
            ctx.log.info(f"now: {now}")

            def to_xls(df: pandas.DataFrame):
                ctx.log.info(f"now: {now}")
                xls_name = f"{now}_{fn_name}_{param_name}.xlsx"
                with pandas.ExcelWriter(xls_name) as writer:
                    df.to_excel(writer, index=False)

            def to_csv(df: pandas.DataFrame):
                ctx.log.info(f"now: {now}")
                csv_name = f"{now}_{fn_name}_{param_name}.csv"
                df.to_csv(csv_name, index=False)

            def deal_layout(lay: str):
                if re.search("(xls|xlsx)", lay):
                    to_xls(get_df())
                elif re.search("(csv)", lay):
                    to_csv(get_df())
                else:
                    log_data()

            if isinstance(export_layout, list):
                for layout in export_layout:
                    deal_layout(layout)
            elif isinstance(export_layout, str):
                deal_layout(export_layout)
            else:
                log_data()
        else:
            log_data()
