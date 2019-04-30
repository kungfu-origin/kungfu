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
      "init(context)",
      "pre_run(context)",
      "pre_quit(context)",
      "on_quote(context, quote)",
      "on_order(context, order)",
      "on_trade(context, trade)"
    ] 
    var taurusKeywords = [
      "context",
      "context.get_nano()",
      "context.log_error(msg)",
      "context.add_account(source_id, account_id, cash_limit)",
      "context.add_md(source_id)",
      "context.cancel_order(order_id)",
      "context.get_accumulated_pnl()",
      "context.get_accumulated_pnl_ratio()",
      "context.get_dynamic_equity()",
      "context.get_initial_equity()",
      "context.get_intraday_pnl()",
      "context.get_intraday_pnl_ratio()",
      "context.get_long_realized_pnl(instrument_id, exchange_id)",
      "context.get_long_tot(instrument_id, exchange_id)",
      "context.get_long_tot_avail(instrument_id, exchange_id)",
      "context.get_long_tot_frozen(instrument_id, exchange_id)",
      "context.get_long_unrealized_pnl(instrument_id, exchange_id)",
      "context.get_long_yd(instrument_id, exchange_id)",
      "context.get_long_yd_avail(instrument_id, exchange_id)",
      "context.get_long_yd_frozen(instrument_id, exchange_id)",
      "context.get_short_realized_pnl(instrument_id, exchange_id)",
      "context.get_short_tot(instrument_id, exchange_id)",
      "context.get_short_tot_avail(instrument_id, exchange_id)",
      "context.get_short_tot_frozen(instrument_id, exchange_id)",
      "context.get_short_unrealized_pnl(instrument_id, exchange_id)",
      "context.get_short_yd(instrument_id, exchange_id)",
      "context.get_short_yd_avail(instrument_id, exchange_id)",
      "context.get_short_yd_frozen(instrument_id, exchange_id)",
      "context.get_static_equity()",
      "context.insert_fak_order(instrument_id, exchange_id, account_id, limit_price, volume, side, offset)",
      "context.insert_fok_order(instrument_id, exchange_id, account_id, limit_price, volume, side, offset)",
      "context.insert_limit_order(instrument_id, exchange_id, account_id,limit_price, volume, side, offset)",
      "context.insert_market_order(instrument_id, exchange_id, account_id, volume, side, offset)",
      "context.log_info(msg)",
      "context.log_warn(msg)",
      "context.set_log_level(level)",
      "get_nano()",
      "log_error(msg)",
      "add_account(source_id, account_id, cash_limit)",
      "add_md(source_id)",
      "cancel_order(order_id)",
      "get_accumulated_pnl()",
      "get_accumulated_pnl_ratio()",
      "get_dynamic_equity()",
      "get_initial_equity()",
      "get_intraday_pnl()",
      "get_intraday_pnl_ratio()",
      "get_long_realized_pnl(instrument_id, exchange_id)",
      "get_long_tot(instrument_id, exchange_id)",
      "get_long_tot_avail(instrument_id, exchange_id)",
      "get_long_tot_frozen(instrument_id, exchange_id)",
      "get_long_unrealized_pnl(instrument_id, exchange_id)",
      "get_long_yd(instrument_id, exchange_id)",
      "get_long_yd_avail(instrument_id, exchange_id)",
      "get_long_yd_frozen(instrument_id, exchange_id)",
      "get_short_realized_pnl(instrument_id, exchange_id)",
      "get_short_tot(instrument_id, exchange_id)",
      "get_short_tot_avail(instrument_id, exchange_id)",
      "get_short_tot_frozen(instrument_id, exchange_id)",
      "get_short_unrealized_pnl(instrument_id, exchange_id)",
      "get_short_yd(instrument_id, exchange_id)",
      "get_short_yd_avail(instrument_id, exchange_id)",
      "get_short_yd_frozen(instrument_id, exchange_id)",
      "get_static_equity()",
      "insert_fak_order(instrument_id, exchange_id, account_id, limit_price, volume, side, offset)",
      "insert_fok_order(instrument_id, exchange_id, account_id, limit_price, volume, side, offset)",
      "insert_limit_order(instrument_id, exchange_id, account_id,limit_price, volume, side, offset)",
      "insert_market_order(instrument_id, exchange_id, account_id, volume, side, offset)",
      "log_info(msg)",
      "log_warn(msg)",
      "set_log_level(level)",
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
      "price_type",
      "volume_condition",
      "time_condition",
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
      "price_type",
      "volume_condition",
      "time_condition",
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
      "parent_order_id",
      "trade_time",
      "instrument_id",
      "exchange_id",
      "account_id",
      "client_id",
      "instrument_type",
      "side",
      "offset",
      "price",
      "volume",
      "tax",
      "commission"
    ]// may the force be with you
    var tarusPropertyKeywords = [
      "log_error(msg)",
      "add_account(source_id, account_id, cash_limit)",
      "add_md(source_id)",
      "cancel_order(order_id)",
      "get_accumulated_pnl()",
      "get_accumulated_pnl_ratio()",
      "get_dynamic_equity()",
      "get_initial_equity()",
      "get_intraday_pnl()",
      "get_intraday_pnl_ratio()",
      "get_long_realized_pnl(instrument_id, exchange_id)",
      "get_long_tot(instrument_id, exchange_id)",
      "get_long_tot_avail(instrument_id, exchange_id)",
      "get_long_tot_frozen(instrument_id, exchange_id)",
      "get_long_unrealized_pnl(instrument_id, exchange_id)",
      "get_long_yd(instrument_id, exchange_id)",
      "get_long_yd_avail(instrument_id, exchange_id)",
      "get_long_yd_frozen(instrument_id, exchange_id)",
      "get_short_realized_pnl(instrument_id, exchange_id)",
      "get_short_tot(instrument_id, exchange_id)",
      "get_short_tot_avail(instrument_id, exchange_id)",
      "get_short_tot_frozen(instrument_id, exchange_id)",
      "get_short_unrealized_pnl(instrument_id, exchange_id)",
      "get_short_yd(instrument_id, exchange_id)",
      "get_short_yd_avail(instrument_id, exchange_id)",
      "get_short_yd_frozen(instrument_id, exchange_id)",
      "get_static_equity()",
      "insert_fak_order(instrument_id, exchange_id, account_id, limit_price, volume, side, offset)",
      "insert_fok_order(instrument_id, exchange_id, account_id, limit_price, volume, side, offset)",
      "insert_limit_order(instrument_id, exchange_id, account_id,limit_price, volume, side, offset)",
      "insert_market_order(instrument_id, exchange_id, account_id, volume, side, offset)",
      "log_info(msg)",
      "log_warn(msg)",
      "set_log_level(level)"
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