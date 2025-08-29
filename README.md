# zen

```c++
//// ⚡⚡⚡ 
/// zen (https://github.com/renemuala/zen)
//// ⚡⚡⚡


readFile(filename: string) = string(str) {
    str : string = ""
    f : file = open(file, "r")
    while(!f.eof) {
        str = str + f.readLine()
    }
}

divide(x : double, y: double) = maybe<double> {
    if (y != 0){
        x / y
    }
}

factorial(x: int) = task<int>(result) {
    result : int = 1
    for(i : int = 2, x) {
        result *= i
    }
}

main() = {
    begin = 1.0
    end = 10.0
    for(x = begin, end;
        y = end, begin, -1){
        if(r: double = divide(x, y)){
            print(r)
        }
    }
    
    tsk = factorial(20)
    if(m : int = factorial(10)){
        print("result 2: " + m)
    }
    
    for(i : int = 1, 20){
      thread.sleep<seconds>(1) 
      if(tsk.done){
        if(result : int = tsk.value){
          print("result 1: " + result)  
        } else {
          print("error: " + tsk.error)
        }
    }
    tsk.stop()
    }
}

```

# v2 

```c++
class A {
    a : int
    b : int

    constructor(a: int, b: int) = {
        this.a = a
        this.b = b
    }

    sum() = int {
        this.a + this.b
    }
}

class list<T> {
    value : T
    next: maybe<list<T>>

    constructor(value: T) = {
        this.value = value
    }

    size() = int(count) {
        count : int = 1
        if(next.valid()){
            count += next.size()
        }
    }

    add(value: T) = {
        if(next.valid()) {
            next.add(value)
        } else {
            next = list<T>(value)
        }
    }

    remove(index: int) = bool(r) {
        r : bool = false
        if(next) {
            if(index == 1){
                r = true
                next = next.next()
            } else {
                r = next.remove(index - 1)
            }
        }
    }
}
```