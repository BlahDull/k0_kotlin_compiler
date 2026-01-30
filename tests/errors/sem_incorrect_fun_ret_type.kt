fun divide(a: Int, b: Int): String {
    if (b == 0) {
        return "Cannot divide by zero"
    }
    return (a / b).toString()
}

fun main() {
    val result: Int = divide(10, 5)
}
