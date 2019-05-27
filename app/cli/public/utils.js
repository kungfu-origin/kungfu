String.prototype.toAccountId = function(){
    return this.split('_').slice(1).join('_')
}
