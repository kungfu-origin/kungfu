from rqalpha import run_func

config = {
    "base": {
        "start_date": "2016-06-01",
        "end_date": "2016-12-01",
        "accounts": {
            "stock": 100000
        }
    }
}


class RicequantAdapter:
    def __init__(self, ctx):
        self.ctx = ctx

    def run(self):
        run_func(handle_bar=handle_bar, config=config)


def handle_bar(context, bar_dict):
    for key in bar_dict.keys():
        print(key)
