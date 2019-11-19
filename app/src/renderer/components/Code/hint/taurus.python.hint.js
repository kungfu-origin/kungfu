export default function (CodeMirror) {
  var pythonKeywords = "and del from not while as elif global or with assert else if pass yield"
  + "break except import print class exec in raise continue finally is return def for lambda try";
    var pythonKeywordsL = pythonKeywords.split(" ");

    var pythonBuiltins = "abs divmod input open staticmethod all enumerate int ord str "
  + "any eval isinstance pow sum basestring execfile issubclass print super"
  + "bin file iter property tuple bool filter len range type"
  + "bytearray float list raw_input unichr callable format locals reduce unicode"
  + "chr frozenset long reload vars classmethod getattr map repr xrange"
  + "cmp globals max reversed zip compile hasattr memoryview round __import__"
  + "complex hash min set apply delattr help next setattr buffer"
  + "dict hex object slice coerce dir id oct sorted intern ";
    var pythonBuiltinsL = pythonBuiltins.split(" ").join("() ").split(" ");


    var taurusDefKeywords = [
      "pre_start(context)",
      "post_start(context)",
      "pre_stop(context)",
      "post_stop(context)",
      "on_quote(context, quote)",
      "on_order(context, order)",
      "on_trade(context, trade)"
    ] 
    var taurusKeywords = [
      "context.add_account(source_id, account_id, cash_limit)",
      "context.suscribe(source_id, instruments_id, exchange_id, is_level2)",
      "context.insert_order(instrument_id, exchange, account, limit_price, volume, PriceType, Side, Offset, HedgeFlag)",
      "context.cancel_order(order_id)",
      "context.ledger.avail",
      "context.ledger.positions",
      "context.ledger.get_position(instrument_id, exchange, Direction)",
      "context.log.info(msg)",
      "context.log.warning(msg)",
      "context.log.error(msg)",
      "context.add_timer(nano, callback)",
      "CTP",
      "SSE",
      "SZE",
      "SHFE",
      "DCE",
      "DZCE",
      "CFFEX",
      "INE",
      "Unknown",
      "Stock",
      "Future",
      "Bond",
      "StockOption",
      "Limit",
      "Any",
      "FakBest5",
      "ForwardBest",
      "ReverseBest",
      "Fak",
      "Fok",
      "Buy",
      "Sell",
      "Lock",
      "Unlock",
      "Exec",
      "Open",
      "Close",
      "CloseToday",
      "CloseYesterday",
      "Speculation",
      "Arbitrage",
      "Hedge",
      "Covered",
      "Long",
      "Short",
      "Unknown",
      "Submitted",
      "Pending",
      "Canceled",
      "Error",
      "Filled",
      "PartialFilledNotActive",
      "PartialFilledActive",
      "source_id",
      "trading_day",
      "rcv_time",
      "data_time",
      "instrument_id",
      "exchange_id",
      "instrument_type",
      "pre_close_price",
      "pre_settlement_price",
      "last_price",
      "volume",
      "turnover",
      "pre_open_interest",
      "open_interest",
      "open_price",
      "high_price",
      "low_price",
      "upper_limit_price",
      "lower_limit_price",
      "close_price",
      "settlement_price",
      "bid_price",
      "ask_price",
      "bid_volume",
      "ask_volume",
      "source_id",
      "trading_day",
      "rcv_time",
      "data_time",
      "instrument_id",
      "exchange_id",
      "instrument_type",
      "pre_close_price",
      "pre_settlement_price",
      "last_price",
      "volume",
      "turnover",
      "pre_open_interest",
      "open_interest",
      "open_price",
      "high_price",
      "low_price",
      "upper_limit_price",
      "lower_limit_price",
      "close_price",
      "settlement_price",
      "bid_price",
      "ask_price",
      "bid_volume",
      "ask_volume",
      "rcv_time",
      "order_id",
      "insert_time",
      "update_time",
      "trading_day",
      "instrument_id",
      "exchange_id",
      "account_id",
      "client_id",
      "instrument_type",
      "limit_price",
      "frozen_price",
      "volume",
      "volume_traded",
      "volume_left",
      "tax",
      "commission",
      "status",
      "error_id",
      "error_msg",
      "parent_id",
      "side",
      "offset",
      "hedge_flag",
      "price_type",
      "volume_condition",
      "time_condition",
      "rcv_time",
      "order_id",
      "parent_order_id",
      "trade_time",
      "instrument_id",
      "exchange_id",
      "account_id",
      "client_id",
      "instrument_type",
      "side",
      "offset",
      "hedge_flag",
      "price",
      "volume",
      "tax",
      "commission",
      "rcv_time",
      "update_time",
      "trading_day",
      "instrument_id",
      "instrument_type",
      "exchange_id",
      "account_id",
      "client_id",
      "direction",
      "volume",
      "yesterday_volume",
      "frozen_total",
      "frozen_yesterday",
      "last_price",
      "open_price",
      "cost_price",
      "close_price",
      "pre_close_price",
      "settlement_price",
      "pre_settlement_price",
      "margin",
      "position_pnl",
      "close_pnl",
      "realized_pnl",
      "unrealized_pnl",
      "open_date",
      "expire_date",
      "trading_day",
      "update_time",
      "client_id",
      "initial_equity",
      "static_equity",
      "dynamic_equity",
      "accumulated_pnl",
      "accumulated_pnl_ratio",
      "intraday_pnl",
      "intraday_pnl_ratio",
      "trading_day",
      "update_time",
      "client_id",
      "account_id",
      "source_id",
      "initial_equity",
      "static_equity",
      "dynamic_equity",
      "accumulated_pnl",
      "accumulated_pnl_ratio",
      "intraday_pnl",
      "intraday_pnl_ratio",
      "Source",
      "Exchange",
      "InstrumentType",
      "Price_Type",
      "Side",
      "Offset",
      "HedgeFlag",
      "Direction",
      "OrderStatus",
      "Instrument",
      "Quote",
      "Order",
      "Trade",
      "Position",
      "PortfolioInfo",
      "SubPortfolioInfo",
      "context"
    ]// may the force be with you
    var tarusPropertyKeywords = [
      "context",
      "context.add_account(source_id, account_id, cash_limit)",
      "context.suscribe(source_id, instruments_id, exchange_id, is_level2)",
      "context.insert_order(instrument_id, exchange, account, limit_price, volume, PriceType, Side, Offset, HedgeFlag)",
      "context.cancel_order(order_id)",
      "context.ledger.avail",
      "context.ledger.positions",
      "context.ledger.get_position(instrument_id, exchange, Direction)",
      "context.log.info(msg)",
      "context.log.warning(msg)",
      "context.log.error(msg)",
      "context.add_timer(nano, callback)",
      "CTP",
      "SSE",
      "SZE",
      "SHFE",
      "DCE",
      "DZCE",
      "CFFEX",
      "INE",
      "Unknown",
      "Stock",
      "Future",
      "Bond",
      "StockOption",
      "Limit",
      "Any",
      "FakBest5",
      "ForwardBest",
      "ReverseBest",
      "Fak",
      "Fok",
      "Buy",
      "Sell",
      "Open",
      "Close",
      "CloseToday",
      "CloseYesterday",
      "Long",
      "Short",
      "Unknown",
      "Submitted",
      "Pending",
      "Canceled",
      "Error",
      "Filled",
      "PartialFilledNotActive",
      "PartialFilledActive",
      "source_id",
      "trading_day",
      "rcv_time",
      "data_time",
      "instrument_id",
      "exchange_id",
      "instrument_type",
      "pre_close_price",
      "pre_settlement_price",
      "last_price",
      "volume",
      "turnover",
      "pre_open_interest",
      "open_interest",
      "open_price",
      "high_price",
      "low_price",
      "upper_limit_price",
      "lower_limit_price",
      "close_price",
      "settlement_price",
      "bid_price",
      "ask_price",
      "bid_volume",
      "ask_volume",
      "source_id",
      "trading_day",
      "rcv_time",
      "data_time",
      "instrument_id",
      "exchange_id",
      "instrument_type",
      "pre_close_price",
      "pre_settlement_price",
      "last_price",
      "volume",
      "turnover",
      "pre_open_interest",
      "open_interest",
      "open_price",
      "high_price",
      "low_price",
      "upper_limit_price",
      "lower_limit_price",
      "close_price",
      "settlement_price",
      "bid_price",
      "ask_price",
      "bid_volume",
      "ask_volume",
      "rcv_time",
      "order_id",
      "insert_time",
      "update_time",
      "trading_day",
      "instrument_id",
      "exchange_id",
      "account_id",
      "client_id",
      "instrument_type",
      "limit_price",
      "frozen_price",
      "volume",
      "volume_traded",
      "volume_left",
      "tax",
      "commission",
      "status",
      "error_id",
      "error_msg",
      "parent_id",
      "side",
      "offset",
      "hedge_flag",
      "price_type",
      "volume_condition",
      "time_condition",
      "rcv_time",
      "order_id",
      "parent_order_id",
      "trade_time",
      "instrument_id",
      "exchange_id",
      "account_id",
      "client_id",
      "instrument_type",
      "side",
      "offset",
      "hedge_flag",
      "price",
      "volume",
      "tax",
      "commission",
      "rcv_time",
      "update_time",
      "trading_day",
      "instrument_id",
      "instrument_type",
      "exchange_id",
      "account_id",
      "client_id",
      "direction",
      "volume",
      "yesterday_volume",
      "frozen_total",
      "frozen_yesterday",
      "last_price",
      "open_price",
      "cost_price",
      "close_price",
      "pre_close_price",
      "settlement_price",
      "pre_settlement_price",
      "margin",
      "position_pnl",
      "close_pnl",
      "realized_pnl",
      "unrealized_pnl",
      "open_date",
      "expire_date",
      "trading_day",
      "update_time",
      "client_id",
      "initial_equity",
      "static_equity",
      "dynamic_equity",
      "accumulated_pnl",
      "accumulated_pnl_ratio",
      "intraday_pnl",
      "intraday_pnl_ratio",
      "trading_day",
      "update_time",
      "client_id",
      "account_id",
      "source_id",
      "initial_equity",
      "static_equity",
      "dynamic_equity",
      "accumulated_pnl",
      "accumulated_pnl_ratio",
      "intraday_pnl",
      "intraday_pnl_ratio"
    ]


    function scriptHint(editor, getToken) {
      // Find the token at the cursor
      var cur = editor.getCursor(), token = getToken(editor, cur), tprop = token;
      // If it's not a 'word-style' token, ignore the token.
  
      if (!/^[\w$_]*$/.test(token.string)) {
          token = tprop = {start: cur.ch, end: cur.ch, string: "", state: token.state,
                           className: token.string == ":" ? "python-type" : null};
      }

      if (!context) var context = [];
      context.push(tprop);  

      var completionList = [];
      switch(token.type){
        case 'variable': // 任意
          completionList = getCompletions(token, context, [pythonKeywordsL, taurusKeywords, pythonBuiltinsL, taurusDefKeywords]);
        break;
        case "builtin":
          completionList = getCompletions(token, context, [pythonBuiltinsL])
        break
        case "def":
          completionList = getCompletions(token, context, [taurusDefKeywords])
        break
        case 'property': // “.” 之后
          completionList = getCompletions(token, context, [tarusPropertyKeywords])
        break;
      }



      completionList = completionList.sort();
      //prevent autocomplete for last word, instead show dropdown with one word
      if(completionList.length == 1) {
        completionList.push(" ");
      }
  
      return {list: completionList,
              from: CodeMirror.Pos(cur.line, token.start),
              to: CodeMirror.Pos(cur.line, token.end)};
    }
  

    function getCompletions(token, context, testList) {
      var found = [], start = token.string;
        
      function arrayContains(arr, item) {
        if (!Array.prototype.indexOf) {
          var i = arr.length;
          while (i--) {
            if (arr[i] === item) {
              return true;
            }
          }
          return false;
        }
        return arr.indexOf(item) != -1;
      }

      function maybeAdd(str) {
        if (str.indexOf(start) == 0 && !arrayContains(found, str)) found.push(str);
      }

      function forEach(arr, f) {
        for (var i = 0, e = arr.length; i < e; ++i) f(arr[i]);
      }

      function gatherCompletions(_obj) {
          testList.forEach(list => {
            forEach(list, maybeAdd)
          })
      }

  
      if (context) {
        var obj = context.pop(), base;  
        base = obj.string
        while (base != null && context.length)
          base = base[context.pop().string];
        
        if (base != null) gatherCompletions(base);
      }
      return found;
    }



    return function(editor) {
      return scriptHint(editor, function (e, cur) {return e.getTokenAt(cur);});
    };
  };