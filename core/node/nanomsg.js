var nn            = require('bindings')('node_nanomsg.node');
var util          = require('util');
var EventEmitter  = require('events').EventEmitter;
var Duplex        = require('stream').Duplex;


/**
 * generic socket-level NN_SOL_SOCKET options
 */
var sol = {
  linger          : nn.NN_LINGER,
  sndbuf          : nn.NN_SNDBUF,
  rcvbuf          : nn.NN_RCVBUF,
  sndtimeo        : nn.NN_SNDTIMEO,
  rcvtimeo        : nn.NN_RCVTIMEO,
  reconn          : nn.NN_RECONNECT_IVL,
  maxreconn       : nn.NN_RECONNECT_IVL_MAX,
  sndprio         : nn.NN_SNDPRIO,
  rcvprio         : nn.NN_RCVPRIO,
  rcvmaxsize      : nn.NN_RCVMAXSIZE,
  tcpnodelay      : nn.NN_TCP_NODELAY,
  ipv6            : nn.NN_IPV4ONLY,
  maxttl          : nn.NN_MAXTTL,
}

/**
 * Socket implementation
 */

function Socket (type, opts) {

    opts = opts || {};
    this.af_domain = opts.raw ? nn.AF_SP_RAW : nn.AF_SP;
    this.type = type;

    switch(type) {
        case 'req':
            this.protocol = nn.NN_REQ;
            this.sender=true;
            this.receiver=true;
            break;

        case 'rep':
            this.protocol = nn.NN_REP;
            this.sender = true;
            this.receiver = true;
            break;

        case 'pair':
            this.protocol = nn.NN_PAIR;
            this.sender = true;
            this.receiver = true;
            break;

        case 'push':
            this.protocol = nn.NN_PUSH;
            this.sender = true;
            this.receiver = false;
            // nndontwait otherwise defaults true, PUSH socket default is false
            opts.dontwait = opts.dontwait || false;
            break;

        case 'pull':
            this.protocol = nn.NN_PULL;
            this.sender = false;
            this.receiver = true;
            break;

        case 'pub':
            this.protocol = nn.NN_PUB;
            this.sender = true;
            this.receiver = false;
            break;

        case 'sub':
            this.protocol = nn.NN_SUB;
            this.sender = false;
            this.receiver = true;
            break;

        case 'bus':
            this.protocol = nn.NN_BUS;
            this.sender = true;
            this.receiver = true;
            break;

        case 'surveyor':
            this.protocol = nn.NN_SURVEYOR;
            this.sender = true;
            this.receiver = false;
            break;

        case 'respondent':
            this.protocol = nn.NN_RESPONDENT;
            this.sender = true;
            this.receiver = true;
            break;

        default:
            throw new Error('unrecognised socket type ' + type);
            break;
    }

  this.binding = nn.Socket(this.af_domain, this.protocol);
  this.bound = {};
  this.connected = {};
  this.queue = [];

  /* async I/O option */
  this.dontwait('dontwait' in opts ? opts.dontwait : true);

  /*  subscription filter control */
  this.channels     = {};

  /*  subscription handling at initialization  */
  if (opts.hasOwnProperty('chan')) {
    if (Array.isArray(opts.chan)) {
      opts.chan.forEach(this._register.bind(this));
    } else {
      throw new TypeError('chan requires an Array');
    }
  } else if (type === 'sub') {
    this._register(''); //default topic is an empty string
  }

  /* sockopt api handling at initialization */
  for(var sokopt in sol){
    if(opts.hasOwnProperty(sokopt)) this[sokopt](opts[sokopt]);
  }

  /* start listening for inbound messages */
  if (this.af_domain === nn.AF_SP && this.receiver) {
    this._startPollReceive();
  }

  if (!opts.hasOwnProperty('objectMode')) {
    // reduce the highwatermark from 16K msgs to 16msgs
    // would be more useful if we have a backpressure mechanism
    opts.objectMode = true;
  }

  Duplex.call(this, opts);

}

util.inherits(Socket, Duplex);

Socket.prototype._protect = function (ret, unwind) {
    if(ret < 0) {
        if (unwind) unwind.call(this);
        this.emit('error', new Error(nn.Err()));
        return null;
    }
    return ret;
};

/* like _protect, but ret is an array where the first element
 * is the error code (0=good, <0=bad), and the second element
 * is the value to return if there was no error.
 */
Socket.prototype._protectArray = function (ret, unwind) {
    if(ret[0] < 0) {
        if (unwind) unwind.call(this);
        this.emit('error', new Error(nn.Err()));
        return null;
    }
    return ret[1];
};

Socket.prototype._send = function (msg, flags) {
    if (this.closed) return;

    if(this.type === 'surveyor' || this.type === 'rep') {
        this._startPollReceive();
    }

    if (this.transform && typeof this.transform === 'function') {
      msg = this.transform(msg);
    }
    return this._protect(nn.Send(this.binding, msg, flags), function () {
        this.queue.unshift([msg, flags]);
    });
};

Socket.prototype._receive = function () {
  if (this.closed){
    this.push(null);
    return;
  }

  var msg = nn.Recv(this.binding, this.nndontwait);

  if(this.type === 'surveyor') {
        if(msg < 0 && nn.Errno() === nn.EFSM) {
            this._stopPollSend();
            this._stopPollReceive();
            return;
        }
    } else if(this.type === 'rep') {
      this._stopPollReceive();
    }
  if (msg === -1) return;

  if (this.restore && typeof this.restore === 'function') msg = this.restore(msg);
  this.push(msg);
};

Socket.prototype._startPollSend = function () {
    if (!this._pollSend) {
        this._pollSend = nn.PollSocket(this.binding, true, function (events) {
            if (events) this.flush();
        }.bind(this));
    }
}

Socket.prototype._startPollReceive = function () {
  if (!this._pollReceive) {
    this._pollReceive = nn.PollSocket(this.binding, false, function (events) {
      if (events) this._receive();
    }.bind(this));
  }
}

Socket.prototype._stopPollSend = function () {
    if (this._pollSend) nn.PollStop(this._pollSend);
    this._pollSend = null;
}

Socket.prototype._stopPollReceive = function () {
    if (this._pollReceive) nn.PollStop(this._pollReceive);
    this._pollReceive = null;
}

Socket.prototype._register = function(chan){
  if (this.channels.hasOwnProperty('')) {
    this.rmchan('');
    this._register(chan);
  } else if (nn.Chan(this.binding, nn.NN_SUB_SUBSCRIBE, chan) !== -1) {
    this.channels[chan] = true;
  } else {
    this.emit('error', new Error( nn.Err() + ' : ' + chan));
  }
};

Socket.prototype._write = function(buf, _, cb){
  this.send(buf, this.nndontwait);
  cb();
}

Socket.prototype._read = function (n) {}

/**
 * Socket API
 */

Socket.prototype.bind = function (addr) {
  return this.bound[addr] = this._protect(nn.Bind( this.binding, addr ));
}

Socket.prototype.connect = function (addr) {
  return this.connected[addr] = this._protect(nn.Connect( this.binding, addr ));
}

Socket.prototype.shutdown = function (addr) {
  var eid;
  if (this.bound.hasOwnProperty(addr)) {
    eid = this.bound[addr];
    delete this.bound[addr];
    return this._protect(nn.Shutdown(this.binding, eid));
  } else if (this.connected.hasOwnProperty(addr)) {
    eid = this.connected[addr];
    delete this.connected[addr];
    return this._protect(nn.Shutdown(this.binding, eid));
  }
};

Socket.prototype.flush = function () {
    while(this.queue.length) {
        var entry = this.queue.shift();
        this._send(entry[0], Number(entry[1]) || 0);
    }
    this._stopPollSend();
};

Socket.prototype.close = function () {
    if(!this.closed) {
        // Prevent "Bad file descriptor" from recursively firing "error" event
        this.closed_status = nn.Close(this.binding);
        this.closed = true;

        this._stopPollSend();
        this._stopPollReceive();

        this.emit('close');

        return this.closed_status;
    }

    // TODO: AJS: in the event of multiple close, we remember
    // the return code from the first valid close, and return
    // it for all subsequent close attempts. This appears to be
    // in the spirit of the original author's intention, but
    // perhaps it would be better to return EBADF or some other
    // error?
    return this.closed_status;
};

Socket.prototype.send = function (buf, flags) {
    this.queue.push([buf, flags]);
    this._startPollSend();
    return buf.length;
};

/**
 * Device implementation
 */
function Device (sock1,sock2) {
    var that = this;
    this.sock1= sock1;
    this.sock2 = sock2;
    this.s1 = -1;
    this.s2 = -1;

    if(sock1 instanceof Socket) {
        this.s1 = sock1.binding;

        if(sock2 instanceof Socket) {
            this.s2 = sock2.binding;
        }

        this._timer = setImmediate(function () {
            nn.DeviceWorker(that.s1, that.s2, function (err) {
                that.emit('error', new Error('lib err: '+ err +'\n'+ nn.Err()));
            });
        });

    } else {
        throw new Error('expected at least one Socket argument');
    }

    EventEmitter.call(this);
}

util.inherits(Device, EventEmitter);

/**
 * sockopt API
 */
Socket.prototype.linger     = opt('linger');
Socket.prototype.sndbuf     = opt('sndbuf');
Socket.prototype.rcvbuf     = opt('rcvbuf');
Socket.prototype.sndtimeo   = opt('sndtimeo');
Socket.prototype.rcvtimeo   = opt('rcvtimeo');
Socket.prototype.reconn     = opt('reconn');
Socket.prototype.maxreconn  = opt('maxreconn');
Socket.prototype.sndprio    = opt('sndprio');
Socket.prototype.rcvprio    = opt('rcvprio');
Socket.prototype.rcvmaxsize = opt('rcvmaxsize');
Socket.prototype.maxttl     = opt('maxttl');

/* ipv6 & tcpnodelay sockopt methods. these two opts are a little different */
Socket.prototype.ipv6 = function (bool) {
  if(arguments.length){
    if(bool){
      if(nn.Setopt(this.binding, nn.NN_SOL_SOCKET, sol.ipv6, 0) > -1)
        return true;
      throw new Error(nn.Err() + ': '+this.type + ' ipv6@activing\n');
    } else {
      if(nn.Setopt(this.binding, nn.NN_SOL_SOCKET, sol.ipv6, 1) > -1)
        return false;
      throw new Error(nn.Err() + ': '+this.type+' ipv6@deactiving\n');
    }
  } else {
    switch(nn.Getopt(this.binding, nn.NN_SOL_SOCKET, sol.ipv6)){
      case 1: return false;
      case 0: return true;
      default:
        throw new Error(nn.Err() +': '+this.type+' ipv6@getsockopt\n');
    }
  }
}

Socket.prototype.tcpnodelay = function (bool) {
  if(arguments.length){
    if(bool){
      if(nn.Setopt(this.binding, nn.NN_TCP, nn.NN_TCP_NODELAY, 1) > -1)
        return true;
      throw new Error(nn.Err() + ': '+this.type + ' nodelay@activing\n');
    } else {
      if(nn.Setopt(this.binding, nn.NN_TCP, nn.NN_TCP_NODELAY, 0) > -1)
        return false;
      throw new Error(nn.Err() + ': '+this.type+' nodelay@deactiving\n');
    }
  } else {
    switch(nn.Getopt(this.binding, nn.NN_TCP, nn.NN_TCP_NODELAY)){
      case 1: return true;
      case 0: return false;
      default:
        throw new Error(nn.Err() +': '+this.type+' nodelay@getsockopt\n');
    }
  }
}

/* set binary or text websocket options */
Socket.prototype.wsopt = function (str) {
  if (arguments.length) {
    var wsopt = {
      text   : nn.NN_WS_MSG_TYPE_TEXT,
      binary : nn.NN_WS_MSG_TYPE_BINARY,
    }
    if (nn.Setopt(this.binding, nn.NN_WS, nn.NN_WS_MSG_TYPE, wsopt[str]) > -1)
      return true;
    throw new Error(nn.Err() + ': '+this.type + ' wsopt@'+ str + '\n');
  } else {
    switch(nn.Getopt(this.binding, nn.NN_WS, nn.NN_WS_MSG_TYPE)){
      case 0x01: return 'text';
      case 0x02: return 'binary';
      default:
        throw new Error(nn.Err() +': '+ this.type + ' wsopt@getsockopt\n');
    }
  }
}

/* sockopt API workhorse */
function opt (option) {
  return function (value) {
    if (typeof value === 'undefined') {
      return nn.Getopt(this.binding, nn.NN_SOL_SOCKET, sol[option]);
    }

    if(nn.Setopt(this.binding, nn.NN_SOL_SOCKET, sol[option], value) > -1) {
      return true;
    }

    throw new Error(nn.Err() + ': ' + this.type + option + '@' + value + '\n');
  }
};

/* chan and rmchan sockopt methods. only relevant for subscription sockets */
Socket.prototype.chan       = function (list) {
  if (Array.isArray(list)) {
    list.forEach(this._register.bind(this));
  } else {
    throw new TypeError('chan requires an Array');
  }
}

Socket.prototype.rmchan     = function() {
  var i = arguments.length;
  while(i--) {
    if (this.channels[arguments[i]]) {
      if (nn.Chan(this.binding, nn.NN_SUB_UNSUBSCRIBE, arguments[i]) > -1) {
        delete this.channels[arguments[i]];
      } else {
        this.emit('error', new Error( nn.Err() + ' : ' + chan));
      }
    }
  };
}

Socket.prototype.dontwait   = function (bool) {
  if (arguments.length) {
    if(bool){
      this.nndontwait = nn.NN_DONTWAIT;
      return true;
    } else {
      this.nndontwait = 0;
      return false;
    }
  } else {
    return Boolean(this.nndontwait);
  }
}

/**
 * module API
 */
function createSocket (type, opts) {
    return new Socket(type, opts);
}

function symbolInfo (symbol) {
    return nn.SymbolInfo(symbol);
}

function symbol (symbol) {
    return nn.Symbol(symbol);
}

function term () {
    return nn.Term();
}

function createDevice (sock1, sock2) {
    return new Device(sock1, sock2);
}

exports._bindings = nn;
exports.Socket = Socket;
exports.createSocket = createSocket;
exports.symbolInfo = symbolInfo;
exports.symbol = symbol;
exports.term = term;
exports.socket = createSocket;

exports.createDevice = createDevice;
exports.device = createDevice;