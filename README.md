# zen

```c++

random = int {

}

sum(i: int, b: int) = int (a + b)

greater(a: int, b: int) = int {
    if(a > b) { 
        a 
    } else  { 
        b 
    }
}

using io.*

print_struct(b: B) = {
    print(b.a)
    print(b.b)
}

display_window() = {
    window : Window = Window()
    window.show()
}

count(start: int, end: int) = {
    for(i: int = start; i < end; i = i + 1) {
        print(i)
    }
}

read_file(file: string) = string(str) {
    str : string = ""
    f : file = open(file, "r")
    line : string = f.readline()
    while(line != "") {
        str = str + line
        print(line)
        line = f.readline()
    }
    f.close()
}

main() = int(0) { 
    a : int = 10
    b : int = 20

    print(sum(a, b))
    print(greater(a, b))
}


read_and_return_greater() = int(max) {
    max : int = 0
    number : int = 0

    while(number != -1) {
        number = read_int()
        max = greater(max, number)
    }
}

add(a: int, b: int) = int {
    a + b
}

sum(a: int, b: int) = int(
    add(a, b)
)
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