import some.something.*

fun fibonacci(n: Int): Int {
    return if (n <= 1) {
        n
    } else {
        fibonacci(n - 1) + fibonacci(n - 2)
    }
}

fun main() {
    print("Enter a number: ")
    val input = readLine()?.toIntOrNull()

    if (input != null && input >= 0) {
        println("Fibonacci number at position $input is: ${fibonacci(input)}")
    } else {
        println("Please enter a non-negative integer.")
    }
}
