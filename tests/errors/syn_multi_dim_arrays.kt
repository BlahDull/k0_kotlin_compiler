fun main() {
    var matrix: Array<Array<Int>> (3) { Array<Int>(3) { 0 } }

    matrix[0][0] = 1
    matrix[1][1] = 2
    matrix[2][2] = 3

    println(matrix[1][1])
}
