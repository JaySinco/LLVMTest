source_filename = "handwriting.ll"
target datalayout = "e-m:w-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"

define double @test(double %a, double %b) {
entry:
  %0 = fmul double %a, %a
  %1 = fmul double 2.000000e+00, %a
  %2 = fmul double %1, %b
  %3 = fmul double %b, %b
  %4 = fadd double %2, %3
  %5 = fdiv double %a, %b
  %6 = fsub double 5.000000e-01, %5
  %7 = fmul double %4, %6
  %8 = fadd double %0, %7
  ret double %8
}
