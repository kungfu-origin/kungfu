export default function (CodeMirror) {
    var cppKeywords = "auto if break case register continue return default do sizeof " +
                    "static else struct switch extern typedef union for goto while enum const " +
                    "volatile asm dynamic_cast namespace reinterpret_cast try explicit new " +
                    "static_cast typeid catch operator template typename class friend private " +
                    "this using const_cast inline public throw virtual delete mutable protected " +
                    "alignas alignof constexpr decltype nullptr noexcept thread_local final " +
                    "static_assert override class namespace struct enum union case do else for if switch while struct int long char short double float unsigned signed void size_t ptrdiff_t bool _Complex _Bool float_t double_t intptr_t intmax_t int8_t int16_t int32_t int64_t uintptr_t uintmax_t uint8_t uint16_t uint32_t uint64_t bool wchar_t";;
      var cppKeywordsL = cppKeywords.split(" ");
  
  
      var taurusDefKeywords = []
      var taurusKeywords = []
    
      function scriptHint(editor, getToken) {

        // Find the token at the cursor
        var cur = editor.getCursor(), token = getToken(editor, cur), tprop = token;
        // If it's not a 'word-style' token, ignore the token.
        const string = (token.string.indexOf('::') != -1) ? token.string.split("::")[token.string.split("::").length - 1] : token.string
        if (!/^[\w$_]*$/.test(string)) {
            token = tprop = {start: cur.ch, end: cur.ch, string: "", state: token.state,
                             className: token.string == ":" ? "cpp-type" : null};
        }
    
        if (!context) var context = [];
        context.push(tprop);  

        var completionList = [];

        switch(token.type){
          case 'variable': // 任意
            completionList = getCompletions(token, context, [cppKeywordsL, taurusKeywords, taurusDefKeywords]);
          break;
          case "builtin":
            completionList = getCompletions(token, context, [cppKeywordsL])
          break
          case "def":
            completionList = getCompletions(token, context, [taurusDefKeywords])
          break
          case 'property': // “.” 之后
            // completionList = getCompletions(token, context, [tarusPropertyKeywords])
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
            testList.map(list => {
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