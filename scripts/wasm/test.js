var _zen_sp; var stacked = (callback) => { _zen_sp = stackSave(); callback();stackRestore(_zen_sp); _zen_sp = undefined; }; var cstr = (string) => { if(_zen_sp === undefined) throw "cannot call outsize of stacked context";  return stringToUTF8OnStack(string); }
function zen_run(code) {
    stacked(() => {
        _zen_run(cstr(code));
    })
}