var counter: Int = 0
val greeting: String = "Hello"

fun incrementCounter() {
    counter++
    println("Counter is now $counter")
}

fun main() {
    println("$greeting! Welcome to the Kotlin program.")

    incrementCounter()
    incrementCounter()

    println("Final counter value: $counter")
}
