; NOTE: Assertions have been autogenerated by utils/update_test_checks.py UTC_ARGS: --check-attributes --check-globals all --version 5
; RUN: opt < %s -passes=verify -S | FileCheck %s

define i32 @test_align(i32 %a, i32 %b) {
; CHECK-LABEL: define alignstack(8) i32 @test_align(
; CHECK-SAME: i32 alignstack(8) [[A:%.*]], i32 alignstack(16) [[B:%.*]]) {
; CHECK-NEXT:    ret i32 0
;
  ret i32 0
}

define void @test_kernel() {
; CHECK-LABEL: define ptx_kernel void @test_kernel() {
; CHECK-NEXT:    ret void
;
  ret void
}

define void @test_maxclusterrank() {
; CHECK-LABEL: define void @test_maxclusterrank(
; CHECK-SAME: ) #[[ATTR0:[0-9]+]] {
; CHECK-NEXT:    ret void
;
  ret void
}

define void @test_cluster_max_blocks() {
; CHECK-LABEL: define void @test_cluster_max_blocks(
; CHECK-SAME: ) #[[ATTR1:[0-9]+]] {
; CHECK-NEXT:    ret void
;
  ret void
}

define void @test_minctasm() {
; CHECK-LABEL: define void @test_minctasm(
; CHECK-SAME: ) #[[ATTR2:[0-9]+]] {
; CHECK-NEXT:    ret void
;
  ret void
}

define void @test_maxnreg() {
; CHECK-LABEL: define void @test_maxnreg(
; CHECK-SAME: ) #[[ATTR3:[0-9]+]] {
; CHECK-NEXT:    ret void
;
  ret void
}

define void @test_maxntid_1() {
; CHECK-LABEL: define void @test_maxntid_1(
; CHECK-SAME: ) #[[ATTR4:[0-9]+]] {
; CHECK-NEXT:    ret void
;
  ret void
}

define void @test_maxntid_2() {
; CHECK-LABEL: define void @test_maxntid_2(
; CHECK-SAME: ) #[[ATTR5:[0-9]+]] {
; CHECK-NEXT:    ret void
;
  ret void
}

define void @test_maxntid_3() {
; CHECK-LABEL: define void @test_maxntid_3(
; CHECK-SAME: ) #[[ATTR6:[0-9]+]] {
; CHECK-NEXT:    ret void
;
  ret void
}

define void @test_maxntid_4() {
; CHECK-LABEL: define void @test_maxntid_4(
; CHECK-SAME: ) #[[ATTR7:[0-9]+]] {
; CHECK-NEXT:    ret void
;
  ret void
}

define void @test_reqntid() {
; CHECK-LABEL: define void @test_reqntid(
; CHECK-SAME: ) #[[ATTR8:[0-9]+]] {
; CHECK-NEXT:    ret void
;
  ret void
}

define void @test_cluster_dim() {
; CHECK-LABEL: define void @test_cluster_dim(
; CHECK-SAME: ) #[[ATTR9:[0-9]+]] {
; CHECK-NEXT:    ret void
;
  ret void
}

!nvvm.annotations = !{!0, !1, !2, !3, !4, !5, !6, !7, !8, !9, !10, !11, !12}

!0 = !{ptr @test_align, !"align", i32 u0x00000008, !"align", i32 u0x00010008, !"align", i32 u0x00020010}
!1 = !{null, !"align", i32 u0x00000008, !"align", i32 u0x00010008, !"align", i32 u0x00020008}
!2 = !{ptr @test_kernel, !"kernel", i32 1}
!3 = !{ptr @test_maxclusterrank, !"maxclusterrank", i32 2}
!4 = !{ptr @test_cluster_max_blocks, !"cluster_max_blocks", i32 3}
!5 = !{ptr @test_minctasm, !"minctasm", i32 4}
!6 = !{ptr @test_maxnreg, !"maxnreg", i32 5}
!7 = !{ptr @test_maxntid_1, !"maxntidx", i32 50}
!8 = !{ptr @test_maxntid_2, !"maxntidx", i32 11, !"maxntidy", i32 22, !"maxntidz", i32 33}
!9 = !{ptr @test_maxntid_3, !"maxntidz", i32 11, !"maxntidy", i32 22, !"maxntidx", i32 33}
!10 = !{ptr @test_maxntid_4, !"maxntidz", i32 100}
!11 = !{ptr @test_reqntid, !"reqntidx", i32 31, !"reqntidy", i32 32, !"reqntidz", i32 33}
!12 = !{ptr @test_cluster_dim, !"cluster_dim_x", i32 101, !"cluster_dim_y", i32 102, !"cluster_dim_z", i32 103}

;.
; CHECK: attributes #[[ATTR0]] = { "nvvm.maxclusterrank"="2" }
; CHECK: attributes #[[ATTR1]] = { "nvvm.maxclusterrank"="3" }
; CHECK: attributes #[[ATTR2]] = { "nvvm.minctasm"="4" }
; CHECK: attributes #[[ATTR3]] = { "nvvm.maxnreg"="5" }
; CHECK: attributes #[[ATTR4]] = { "nvvm.maxntid"="50" }
; CHECK: attributes #[[ATTR5]] = { "nvvm.maxntid"="11,22,33" }
; CHECK: attributes #[[ATTR6]] = { "nvvm.maxntid"="33,22,11" }
; CHECK: attributes #[[ATTR7]] = { "nvvm.maxntid"="1,1,100" }
; CHECK: attributes #[[ATTR8]] = { "nvvm.reqntid"="31,32,33" }
; CHECK: attributes #[[ATTR9]] = { "nvvm.cluster_dim"="101,102,103" }
;.
