fun main() {
    val a = 5
    val b = 10

    if (a < b) {
        println("a is less than b")
    }

    if (a > b) {
        println("a is greater than b")
    }
    else {
        println("a is not greater than b")
    }

    if (a == b) {
        println("a is equal to b")
    }
    else if (a < b) {
        println("a is less than b")
    }
    else {
        println("a is greater than b")
    }

    var count = 0
    while (count < 3) {
        println("while loop count: $count")
        count = count + 1
    }

    for (i in 1..5) {
        println("for loop iteration: $i")
    }
}
