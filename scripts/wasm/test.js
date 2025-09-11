var _zen_sp; var stacked = (callback) => { _zen_sp = stackSave(); callback();stackRestore(_zen_sp); _zen_sp = undefined; }; var cstr = (string) => { if(_zen_sp === undefined) throw "cannot call outsize of stacked context";  return stringToUTF8OnStack(string); }
stacked(() => {
    _zen_reset();
    _zen_compile(cstr(`sum(x: long, y: long) = long(x+y) main={}`));
    _zen_run(cstr("main"));
})