import pykungfu
from pykungfu import longfist
from pykungfu import yijinjing as pyyjj
from pykungfu import wingchun as pywingchun
from kungfu.wingchun.utils import *


def get_frozen_price(book, order_id):
    if order_id in book.orders:
        return book.orders[order_id].frozen_price
    else:
        return 0.0


class StockAccountingMethod(pywingchun.AccountingMethod):
    def __init__(self, ctx):
        pywingchun.AccountingMethod.__init__(self)
        self.ctx = ctx
        self._commission_map = {}
        self.min_commission = 5
        self.commission_ratio = 0.0008

    def apply_trading_day(self, book, trading_day):
        if not self.trading_day == trading_day:
            self.ctx.logger.info("{} apply trading day, switch from {} to {}".format(self.uname, self.trading_day, trading_day))
            if is_valid_price(self.close_price):
                self.pre_close_price = self.close_price
            elif is_valid_price(self.last_price):
                self.pre_close_price = self.last_price
            self.yesterday_volume = self.volume
            self.close_price = 0.0
            self.trading_day = trading_day

    def apply_quote(self, book, quote):
        long_position = book.get_long_position(quote)
        short_position = book.get_short_position(quote)
        self._apply_quote(quote, long_position)
        self._apply_quote(quote, short_position)

    def apply_order_input(self, book, order_input):
        position = book.get_position(order_input)
        if order_input.side == Side.Sell and self._sellable(position) >= order_input.volume:
            position.frozen_total += order_input.volume
            position.frozen_yesterday += order_input.volume
        elif order_input.side == Side.Buy:
            book.asset.frozen_cash += order_input.volume * order_input.frozen_price
            book.asset.avail -= order_input.volume * order_input.frozen_price
        self._update_unrealized_pnl(position)

    def apply_order(self, book, order):
        position = book.get_position(order)
        if order.status not in [
            pykungfu.longfist.enums.OrderStatus.Submitted,
            pykungfu.longfist.enums.OrderStatus.Pending,
            pykungfu.longfist.enums.OrderStatus.PartialFilledActive
        ] and order.volume_left > 0:
            if order.side == Side.Sell:
                position.frozen_total -= order.volume_left
                position.frozen_yesterday -= order.volume_left
            elif order.side == Side.Buy:
                frozen_amt = order.volume_left * order.frozen_price
                book.asset.frozen_cash -= frozen_amt
                book.asset.avail += frozen_amt
        self._update_unrealized_pnl(position)

    def apply_trade(self, book, trade):
        if trade.side == Side.Buy:
            self._apply_buy(book, trade)
        elif trade.side == Side.Sell:
            self._apply_sell(book, trade)
        else:
            # ignore stock options lock/unlock/exec/drop operations
            pass

    def _apply_quote(self, quote, position):
        if is_valid_price(quote.close_price):
            position.close_price = quote.close_price
        if is_valid_price(quote.last_price):
            position.last_price = quote.last_price
        if is_valid_price(quote.pre_close_price):
            position.pre_close_price = quote.pre_close_price
        self._update_unrealized_pnl(position)

    def _apply_buy(self, book, trade):
        position = book.get_position(trade)
        position.avg_open_price = (position.avg_open_price * position.volume + trade.price * trade.volume) / (position.volume + trade.volume)
        commission = self._calculate_commission(trade)
        tax = self._calculate_tax(trade)
        trade.commission = commission
        trade.tax = tax
        position.volume += trade.volume
        self._update_unrealized_pnl(position)

        book.asset.frozen_cash -= get_frozen_price(book, trade.order_id) * trade.volume
        book.asset.avail -= commission
        book.asset.avail -= tax
        book.asset.intraday_fee += (commission + tax)
        book.asset.accumulated_fee += (commission + tax)

    def _apply_sell(self, book, trade):
        if self.yesterday_volume < trade.volume:
            self.ctx.logger.error("{} current yesterday volume is {}, {} to sell".format(self.uname, self.yesterday_volume, trade.volume))
            return
        if self.frozen_total < trade.volume:
            self.ctx.logger.error("{} current frozen volume is {}, {} to sell".format(self.uname, self.frozen_total, trade.volume))
            return
        position = book.get_position(trade)
        realized_pnl = self._calculate_realized_pnl(trade.price, trade.volume)
        commission = self._calculate_commission(trade)
        tax = self._calculate_tax(trade)
        trade.commission = commission
        trade.tax = tax
        position.frozen_total -= trade.volume
        position.frozen_yesterday -= trade.volume
        position.yesterday_volume -= trade.volume
        position.volume -= trade.volume
        position.realized_pnl += realized_pnl
        self._update_unrealized_pnl(position)

        book.asset.realized_pnl += realized_pnl
        book.asset.avail += trade.price * trade.volume
        book.asset.avail -= commission
        book.asset.avail -= tax
        book.asset.intraday_fee += (commission + tax)
        book.asset.accumulated_fee += (commission + tax)

    def _update_unrealized_pnl(self, position):
        position.unrealized_pnl = (position.last_price - position.avg_open_price) * position.volume if position.last_price > 0.0 else 0.0

    def _market_value(self, position):
        return position.volume * (position.last_price if position.last_price > 0.0 else position.avg_open_price)

    def _sellable(self, position):
        return position.yesterday_volume - position.frozen_yesterday

    def _calculate_realized_pnl(self, trade_price, trade_volume):
        return (trade_price - self.avg_open_price) * trade_volume

    def _calculate_tax(self, trade):
        return trade.price * trade.volume * 0.001 if trade.side == Side.Sell else 0.0

    def _calculate_commission(self, trade):
        order_id = trade.order_id
        if order_id not in self._commission_map:
            self._commission_map[order_id] = self.min_commission
        commission = self._commission_map[order_id]
        amt = trade.price * trade.volume * self.commission_ratio
        if commission == self.min_commission:  # first trade
            if amt > commission:
                self._commission_map[order_id] = 0.0
                return amt
            else:
                self._commission_map[order_id] -= amt
                return self.min_commission
        else:
            if amt > commission:
                self._commission_map[order_id] = 0.0
                return amt - commission
            else:
                self._commission_map[order_id] -= amt
                return 0.0


class FutureAccountingMethod(pywingchun.AccountingMethod):
    def __init__(self, ctx):
        pywingchun.AccountingMethod.__init__(self)
        self.ctx = ctx

    def apply_quote(self, book, quote):
        long_position = book.get_long_position(quote)
        short_position = book.get_short_position(quote)
        self._apply_quote(book, quote, long_position)
        self._apply_quote(book, quote, short_position)

    def apply_order_input(self, book, order_input):
        position = book.get_position(order_input)
        if order_input.offset == Offset.Open:
            frozen_margin = position.contract_multiplier * order_input.frozen_price * order_input.volume * position.margin_ratio
            if book.asset.avail < frozen_margin:
                raise ValueError("{} to frozen for order {}, but {} avail".format(frozen_margin, order_input.order_id, book.asset.avail))

            book.asset.avail -= frozen_margin
            book.asset.frozen_cash += frozen_margin
            book.asset.frozen_margin += frozen_margin
        elif order_input.offset == Offset.Close or order_input.offset == Offset.CloseYesterday:
            if position.volume_available < order_input.volume:
                raise ValueError("{} to frozen for order {}, but volume avail is {}".
                                 format(order_input.volume, order_input.order_id, position.volume_available))
            position.frozen_total += order_input.volume
            if position.yesterday_volume_available > order_input.volume:
                position.frozen_yesterday += order_input.volume
            else:
                position.frozen_yesterday = position.yesterday_volume
        elif order_input.offset == Offset.CloseToday:
            if position.today_volume_available < order_input.volume:
                raise ValueError("{} to frozen for order {}, but today volume avail is {}".
                                 format(order_input.volume, order_input.order_id, position.today_volume_available))
            position.frozen_total += order_input.volume

    def apply_order(self, book, order):
        position = book.get_position(order)
        if order.status not in [
            longfist.enums.OrderStatus.Submitted,
            longfist.enums.OrderStatus.Pending,
            longfist.enums.OrderStatus.PartialFilledActive
        ] and order.volume_left > 0:
            if order.offset == Offset.Open:
                frozen_margin = position.contract_multiplier * order.frozen_price * order.volume * position.margin_ratio
                book.asset.avail += frozen_margin
                book.asset.frozen_cash -= frozen_margin
                book.asset.frozen_margin -= frozen_margin
            elif order.offset == Offset.Close or order.offset == Offset.CloseYesterday:
                if position.frozen_total < order.volume_left:
                    raise ValueError("{} to release for order {}, but frozen is {}".
                                     format(order.volume_left, order.order_id, position.volume_available))
                position.frozen_total -= order.volume_left
                position.frozen_yesterday = position.frozen_yesterday - order.volume_left if position.frozen_yesterday > order.volume_left else 0
            elif order.offset == Offset.CloseToday:
                if position.frozen_today < order.volume_left:
                    raise ValueError("{} to release for order {}, but frozen is {}".
                                     format(order.volume_left, order.order_id, position.frozen_today))
                position.frozen_total -= order.volume_left

    def apply_trade(self, book, trade):
        if trade.offset == Offset.Open:
            self._apply_open(book, trade)
        elif trade.offset == Offset.Close or trade.offset == Offset.CloseToday or trade.offset == Offset.CloseYesterday:
            self._apply_close(book, trade)

    def apply_trading_day(self, book, trading_day):
        if not self.trading_day == trading_day:
            self.ctx.logger.info("{} apply trading day, switch from {} to {}".format(self.uname, self.trading_day, trading_day))
            if is_valid_price(self.close_price):
                self.pre_close_price = self.close_price
            elif is_valid_price(self.last_price):
                self.pre_close_price = self.last_price
            self.yesterday_volume = self.volume
            self.close_price = 0.0
            self.trading_day = trading_day
            self.book.subject.on_next(self.event)

    def _apply_quote(self, book, quote, position):
        if is_valid_price(quote.settlement_price):
            position.settlement_price = quote.settlement_price
            pre_margin = position.margin
            position.margin = position.contract_multiplier * position.settlement_price * position.volume * position.margin_ratio
            book.asset.avail -= (position.margin - pre_margin)
        if is_valid_price(quote.last_price):
            position.last_price = quote.last_price
        if is_valid_price(quote.pre_settlement_price):
            position.pre_settlement_price = quote.pre_settlement_price

    def _apply_open(self, book, trade):
        position = book.get_position(trade)
        margin = position.contract_multiplier * trade.price * trade.volume * position.margin_ratio
        commission = self._calculate_commission(trade)
        trade.commission = commission
        position.margin += margin
        frozen_margin = position.contract_multiplier * get_frozen_price(book, trade.order_id) * trade.volume * position.margin_ratio
        position.avg_open_price = (position.avg_open_price * position.volume + trade.volume * trade.price) / (position.volume + trade.volume)
        position.volume += trade.volume

        book.asset.frozen_cash -= frozen_margin
        book.asset.frozen_margin -= frozen_margin
        book.asset.avail -= commission
        book.asset.accumulated_fee += commission
        book.asset.intraday_fee += commission

    def _apply_close(self, book, trade):
        position = book.get_position(trade)
        if position.frozen_total < trade.volume:
            raise ValueError("over close position, current frozen volume is {}, {} to close".
                             format(position.frozen_total, trade.volume))
        if trade.offset == Offset.CloseToday and position.frozen_today < trade.volume:
            raise ValueError("over close today position, current frozen volume is {}, {} to close".
                             format(position.volume - position.yesterday_volume, trade.volume))
        margin = position.contract_multiplier * trade.price * trade.volume * position.margin_ratio
        delta_margin = margin if position.margin > margin else position.margin
        position.margin -= delta_margin

        book.asset.avail += delta_margin

        position.pre_today_vol = position.today_volume
        position.volume -= trade.volume
        position.frozen_total -= trade.volume
        if trade.offset != Offset.CloseToday:
            position.yesterday_volume = 0 if position.yesterday_volume <= trade.volume else \
                position.yesterday_volume - trade.volume
            position.frozen_yesterday = 0 if position.frozen_yesterday <= trade.volume else \
                position.frozen_yesterday - trade.volume
        trade.close_today_volume = position.today_volume - position.pre_today_vol
        commission = self._calculate_commission(trade)
        realized_pnl = (trade.price - position.avg_open_price) * trade.volume * \
                       position.contract_multiplier * (1 if position.direction == Direction.Long else -1)
        trade.commission = commission
        position.realized_pnl += realized_pnl

        book.asset.realized_pnl += realized_pnl
        book.asset.avail -= commission
        book.asset.accumulated_fee += commission
        book.asset.intraday_fee += commission

    def _calculate_commission(self, trade):
        if self.commission_mode == CommissionRateMode.ByAmount:
            if trade.offset == Offset.Open:
                return trade.price * trade.volume * self.contract_multiplier * self.open_commission_ratio
            else:
                return trade.price * (trade.volume - trade.close_today_volume) * \
                       self.contract_multiplier * self.close_commission_ratio \
                       + trade.price * trade.close_today_volume * self.contract_multiplier * self.close_today_commission_ratio
        else:
            if trade.offset == Offset.Open:
                return trade.volume * self.contract_multiplier * self.open_commission_ratio
            else:
                return (trade.volume - trade.close_today_volume) * self.contract_multiplier * self.close_commission_ratio \
                       + trade.close_today_volume * self.contract_multiplier * self.close_today_commission_ratio


def setup_bookkeeper(ctx, bookkeeper):
    commissions = pyyjj.config_store(ctx.locator).get_all(longfist.types.InstrumentCommissionRate())

    stock_accounting_method = StockAccountingMethod(ctx)
    future_accounting_method = FutureAccountingMethod(ctx)

    ctx.accounting_methods = {
        InstrumentType.Stock: stock_accounting_method,
        InstrumentType.Bond: stock_accounting_method,
        InstrumentType.Fund: stock_accounting_method,
        InstrumentType.StockOption: stock_accounting_method,
        InstrumentType.TechStock: stock_accounting_method,
        InstrumentType.Index: stock_accounting_method,
        InstrumentType.Repo: stock_accounting_method,

        InstrumentType.Future: future_accounting_method,

        InstrumentType.Unknown: stock_accounting_method
    }
    for instrument_type in ctx.accounting_methods:
        bookkeeper.set_accounting_method(instrument_type, ctx.accounting_methods[instrument_type])
