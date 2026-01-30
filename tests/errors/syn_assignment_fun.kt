fun main() {
fun add(x: Int, y: Int): Int = x + y
val addFunc: (Int, Int) -> Double = ::add
}

