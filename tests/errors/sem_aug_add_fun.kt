fun main() {
var func: (Int, Int) -> Int = ::add
func += ::add
}
