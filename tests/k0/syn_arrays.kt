fun main() {
    var num: Array<Int> (100) { 0 }

    // Set a few values
    num[0] = 10
    num[1] = 20
    num[2] = num[0] + num[1]

    // Print values
    println(num[0])
    println(num[1])
    println(num[2])
}