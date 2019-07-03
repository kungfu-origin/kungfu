

class SubPortfolio:
    def __init__(self, account):
        self._account = account

    def get_position(self, instrument_id, exchange_id):
        return None

class Portfolio:
    def __init__(self):
        self.sub_portfolios = {}

    def get_position(self, instrument_id, exchange_id):
        pass



