fun main() {
    val x = 3

    when (x) {
        1 -> println("x is one")
        2, 3, 4 -> println("x is two, three, or four")
        in 5..10 -> println("x is between five and ten")
        !in 11..20 -> println("x is not between eleven and twenty")
        else -> println("x is something else")
    }
}
