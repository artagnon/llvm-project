; ModuleID = 'Input/FactO0.ll'
source_filename = "Fact.cpp"
target datalayout = "e-m:o-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-apple-macosx12.0.0"

; Function Attrs: noinline optnone ssp uwtable
define i32 @_Z4facti(i32 %0) #0 {
  %2 = alloca i32, align 4
  %3 = alloca i32, align 4
  %4 = alloca i32, align 4
  store i32 %0, i32* %3, align 4
  %5 = load i32, i32* %3, align 4
  %6 = icmp eq i32 %5, 0
  br i1 %6, label %7, label %brBB

7:                                                ; preds = %1
  store i32 1, i32* %2, align 4
  br label %18

brBB:                                             ; preds = %1
  %8 = call i32 @rand()
  %9 = add i32 %8, 1
  %10 = icmp ugt i32 %9, 0
  br i1 %10, label %11, label %deadBB

11:                                               ; preds = %brBB
  %12 = load i32, i32* %3, align 4
  %13 = load i32, i32* %3, align 4
  %14 = sub nsw i32 %13, 1
  %15 = call i32 @_Z4facti(i32 %14)
  %16 = mul nsw i32 %12, %15
  store i32 %16, i32* %4, align 4
  %17 = load i32, i32* %4, align 4
  store i32 %17, i32* %2, align 4
  br label %18

deadBB:                                           ; preds = %brBB
  store i32 0, i32* %2, align 4
  br label %18

18:                                               ; preds = %deadBB, %11, %7
  %19 = load i32, i32* %2, align 4
  ret i32 %19
}

declare i32 @rand()

attributes #0 = { noinline optnone ssp uwtable "darwin-stkchk-strong-link" "disable-tail-calls"="false" "frame-pointer"="all" "less-precise-fpmad"="false" "min-legal-vector-width"="0" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="true" "probe-stack"="___chkstk_darwin" "stack-protector-buffer-size"="8" "target-cpu"="penryn" "target-features"="+cx16,+cx8,+fxsr,+mmx,+sahf,+sse,+sse2,+sse3,+sse4.1,+ssse3,+x87" "tune-cpu"="generic" "unsafe-fp-math"="false" "use-soft-float"="false" }

!llvm.module.flags = !{!0, !1, !2}
!llvm.ident = !{!3}

!0 = !{i32 2, !"SDK Version", [2 x i32] [i32 12, i32 1]}
!1 = !{i32 1, !"wchar_size", i32 4}
!2 = !{i32 7, !"PIC Level", i32 2}
!3 = !{!"Apple clang version 13.0.0 (clang-1300.0.29.30)"}
