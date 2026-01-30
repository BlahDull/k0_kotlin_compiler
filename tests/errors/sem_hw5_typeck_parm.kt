// var dd : Double = 3.14;

fun fib(f : Double, i : Int) : Float {
   var x : Double = 3.14 * 2.0;
   return x;
}

fun main() {
   var i : Double = 5.0;
   val j : Int = 5;
   i = 0.0 + fib(i, 3);
   i = fib(i, "hello");
   // println("{j}");       // tried also with $j, which maybe should work
}
// Test Output:[expect type error to param on line 12]