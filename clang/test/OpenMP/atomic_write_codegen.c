// RUN: %clang_cc1 -verify -triple x86_64-apple-darwin10 -target-cpu core2 -fopenmp -x c -emit-llvm %s -o - | FileCheck %s
// RUN: %clang_cc1 -fopenmp -x c -triple x86_64-apple-darwin10 -target-cpu core2 -emit-pch -o %t %s
// RUN: %clang_cc1 -fopenmp -x c -triple x86_64-apple-darwin10 -target-cpu core2 -include-pch %t -verify %s -emit-llvm -o - | FileCheck %s

// RUN: %clang_cc1 -verify -triple x86_64-apple-darwin10 -target-cpu core2 -fopenmp-simd -x c -emit-llvm %s -o - | FileCheck --check-prefix SIMD-ONLY0 %s
// RUN: %clang_cc1 -fopenmp-simd -x c -triple x86_64-apple-darwin10 -target-cpu core2 -emit-pch -o %t %s
// RUN: %clang_cc1 -fopenmp-simd -x c -triple x86_64-apple-darwin10 -target-cpu core2 -include-pch %t -verify %s -emit-llvm -o - | FileCheck --check-prefix SIMD-ONLY0 %s
// SIMD-ONLY0-NOT: {{__kmpc|__tgt}}
// expected-no-diagnostics
// REQUIRES: x86-registered-target
#ifndef HEADER
#define HEADER

_Bool bv, bx;
char cv, cx;
unsigned char ucv, ucx;
short sv, sx;
unsigned short usv, usx;
int iv, ix;
unsigned int uiv, uix;
long lv, lx;
unsigned long ulv, ulx;
long long llv, llx;
unsigned long long ullv, ullx;
float fv, fx;
double dv, dx;
long double ldv, ldx;
_Complex int civ, cix;
_Complex float cfv, cfx;
_Complex double cdv, cdx;

typedef int int4 __attribute__((__vector_size__(16)));
int4 int4x;

struct BitFields {
  int : 32;
  int a : 31;
} bfx;

struct BitFields_packed {
  int : 32;
  int a : 31;
} __attribute__ ((__packed__)) bfx_packed;

struct BitFields2 {
  int : 31;
  int a : 1;
} bfx2;

struct BitFields2_packed {
  int : 31;
  int a : 1;
} __attribute__ ((__packed__)) bfx2_packed;

struct BitFields3 {
  int : 11;
  int a : 14;
} bfx3;

struct BitFields3_packed {
  int : 11;
  int a : 14;
} __attribute__ ((__packed__)) bfx3_packed;

struct BitFields4 {
  short : 16;
  int a: 1;
  long b : 7;
} bfx4;

struct BitFields4_packed {
  short : 16;
  int a: 1;
  long b : 7;
} __attribute__ ((__packed__)) bfx4_packed;

typedef float float2 __attribute__((ext_vector_type(2)));
float2 float2x;

// Register "0" is currently an invalid register for global register variables.
// Use "esp" instead of "0".
// register int rix __asm__("0");
register int rix __asm__("esp");

int main(void) {
// CHECK: store atomic i32 1, ptr getelementptr inbounds nuw ({ i32, i32 }, ptr @civ, i32 0, i32 1) monotonic, align 4
#pragma omp atomic write
 __imag(civ) = 1;
// CHECK: load i8, ptr
// CHECK: store atomic i8 {{.*}} monotonic, align 1
#pragma omp atomic write
  bx = bv;
// CHECK: load i8, ptr
// CHECK: store atomic i8 {{.*}} release, align 1
#pragma omp atomic write release
  cx = cv;
// CHECK: load i8, ptr
// CHECK: store atomic i8 {{.*}} monotonic, align 1
#pragma omp atomic write
  ucx = ucv;
// CHECK: load i16, ptr
// CHECK: store atomic i16 {{.*}} monotonic, align 2
#pragma omp atomic write
  sx = sv;
// CHECK: load i16, ptr
// CHECK: store atomic i16 {{.*}} monotonic, align 2
#pragma omp atomic write
  usx = usv;
// CHECK: load i32, ptr
// CHECK: store atomic i32 {{.*}} monotonic, align 4
#pragma omp atomic write
  ix = iv;
// CHECK: load i32, ptr
// CHECK: store atomic i32 {{.*}} monotonic, align 4
#pragma omp atomic write
  uix = uiv;
// CHECK: load i64, ptr
// CHECK: store atomic i64 {{.*}} monotonic, align 8
#pragma omp atomic write
  lx = lv;
// CHECK: load i64, ptr
// CHECK: store atomic i64 {{.*}} monotonic, align 8
#pragma omp atomic write
  ulx = ulv;
// CHECK: load i64, ptr
// CHECK: store atomic i64 {{.*}} monotonic, align 8
#pragma omp atomic write
  llx = llv;
// CHECK: load i64, ptr
// CHECK: store atomic i64 {{.*}} monotonic, align 8
#pragma omp atomic write
  ullx = ullv;
// CHECK: load float, ptr
// CHECK: store atomic float {{.*}}, ptr {{.*}} monotonic, align 4
#pragma omp atomic write
  fx = fv;
// CHECK: load double, ptr
// CHECK: store atomic double {{.*}}, ptr {{.*}} monotonic, align 8
#pragma omp atomic write
  dx = dv;
// CHECK: [[LD:%.+]] = load x86_fp80, ptr
// CHECK: call void @llvm.memset.p0.i64(ptr align 16 [[LDTEMP:%.*]], i8 0, i64 16, i1 false)
// CHECK: store x86_fp80 [[LD]], ptr [[LDTEMP]]
// CHECK: [[LD:%.+]] = load i128, ptr [[LDTEMP:%.*]]
// CHECK: store atomic i128 [[LD]], ptr {{.*}} monotonic, align 16
#pragma omp atomic write
  ldx = ldv;
// CHECK: [[REAL_VAL:%.+]] = load i32, ptr @{{.*}}
// CHECK: [[IMG_VAL:%.+]] = load i32, ptr getelementptr inbounds nuw ({ i32, i32 }, ptr @{{.*}}, i32 0, i32 1)
// CHECK: [[TEMP_REAL_REF:%.+]] = getelementptr inbounds nuw { i32, i32 }, ptr [[TEMP:%.+]], i32 0, i32 0
// CHECK: [[TEMP_IMG_REF:%.+]] = getelementptr inbounds nuw { i32, i32 }, ptr [[TEMP]], i32 0, i32 1
// CHECK: store i32 [[REAL_VAL]], ptr [[TEMP_REAL_REF]]
// CHECK: store i32 [[IMG_VAL]], ptr [[TEMP_IMG_REF]]
// CHECK: call void @__atomic_store(i64 noundef 8, ptr noundef @{{.*}}, ptr noundef [[TEMP]], i32 noundef 0)
#pragma omp atomic write
  cix = civ;
// CHECK: [[REAL_VAL:%.+]] = load float, ptr @{{.*}}
// CHECK: [[IMG_VAL:%.+]] = load float, ptr getelementptr inbounds nuw ({ float, float }, ptr @{{.*}}, i32 0, i32 1)
// CHECK: [[TEMP_REAL_REF:%.+]] = getelementptr inbounds nuw { float, float }, ptr [[TEMP:%.+]], i32 0, i32 0
// CHECK: [[TEMP_IMG_REF:%.+]] = getelementptr inbounds nuw { float, float }, ptr [[TEMP]], i32 0, i32 1
// CHECK: store float [[REAL_VAL]], ptr [[TEMP_REAL_REF]]
// CHECK: store float [[IMG_VAL]], ptr [[TEMP_IMG_REF]]
// CHECK: call void @__atomic_store(i64 noundef 8, ptr noundef @{{.*}}, ptr noundef [[TEMP]], i32 noundef 0)
#pragma omp atomic write
  cfx = cfv;
// CHECK: [[REAL_VAL:%.+]] = load double, ptr @{{.*}}
// CHECK: [[IMG_VAL:%.+]] = load double, ptr getelementptr inbounds nuw ({ double, double }, ptr @{{.*}}, i32 0, i32 1)
// CHECK: [[TEMP_REAL_REF:%.+]] = getelementptr inbounds nuw { double, double }, ptr [[TEMP:%.+]], i32 0, i32 0
// CHECK: [[TEMP_IMG_REF:%.+]] = getelementptr inbounds nuw { double, double }, ptr [[TEMP]], i32 0, i32 1
// CHECK: store double [[REAL_VAL]], ptr [[TEMP_REAL_REF]]
// CHECK: store double [[IMG_VAL]], ptr [[TEMP_IMG_REF]]
// CHECK: call void @__atomic_store(i64 noundef 16, ptr noundef @{{.*}}, ptr noundef [[TEMP]], i32 noundef 5)
// CHECK: call{{.*}} @__kmpc_flush(
#pragma omp atomic seq_cst write
  cdx = cdv;
// CHECK: load i8, ptr
// CHECK: store atomic i64 {{.*}} monotonic, align 8
#pragma omp atomic write
  ulx = bv;
// CHECK: load i8, ptr
// CHECK: store atomic i8 {{.*}} monotonic, align 1
#pragma omp atomic write
  bx = cv;
// CHECK: load i8, ptr
// CHECK: store atomic i8 {{.*}} seq_cst, align 1
// CHECK: call{{.*}} @__kmpc_flush(
#pragma omp atomic write, seq_cst
  cx = ucv;
// CHECK: load i16, ptr
// CHECK: store atomic i64 {{.*}} monotonic, align 8
#pragma omp atomic write
  ulx = sv;
// CHECK: load i16, ptr
// CHECK: store atomic i64 {{.*}} monotonic, align 8
#pragma omp atomic write
  lx = usv;
// CHECK: load i32, ptr
// CHECK: store atomic i32 {{.*}} seq_cst, align 4
// CHECK: call{{.*}} @__kmpc_flush(
#pragma omp atomic seq_cst, write
  uix = iv;
// CHECK: load i32, ptr
// CHECK: store atomic i32 {{.*}} monotonic, align 4
#pragma omp atomic write
  ix = uiv;
// CHECK: load i64, ptr
// CHECK: [[VAL:%.+]] = trunc i64 %{{.*}} to i32
// CHECK: [[TEMP_REAL_REF:%.+]] = getelementptr inbounds nuw { i32, i32 }, ptr [[TEMP:%.+]], i32 0, i32 0
// CHECK: [[TEMP_IMG_REF:%.+]] = getelementptr inbounds nuw { i32, i32 }, ptr [[TEMP]], i32 0, i32 1
// CHECK: store i32 [[VAL]], ptr [[TEMP_REAL_REF]]
// CHECK: store i32 0, ptr [[TEMP_IMG_REF]]
// CHECK: call void @__atomic_store(i64 noundef 8, ptr noundef @{{.+}}, ptr noundef [[TEMP]], i32 noundef 0)
#pragma omp atomic write
  cix = lv;
// CHECK: load i64, ptr
// CHECK: store atomic float %{{.+}}, ptr {{.*}} monotonic, align 4
#pragma omp atomic write
  fx = ulv;
// CHECK: load i64, ptr
// CHECK: store atomic double %{{.+}}, ptr {{.*}} monotonic, align 8
#pragma omp atomic write
  dx = llv;
// CHECK: load i64, ptr
// CHECK: [[VAL:%.+]] = uitofp i64 %{{.+}} to x86_fp80
// CHECK: call void @llvm.memset.p0.i64(ptr align 16 [[TEMP:%.+]], i8 0, i64 16, i1 false)
// CHECK: store x86_fp80 [[VAL]], ptr [[TEMP]]
// CHECK: [[VAL:%.+]] = load i128, ptr [[TEMP]]
// CHECK: store atomic i128 [[VAL]], ptr {{.*}} monotonic, align 16
#pragma omp atomic write
  ldx = ullv;
// CHECK: load float, ptr
// CHECK: [[VAL:%.+]] = fptosi float %{{.*}} to i32
// CHECK: [[TEMP_REAL_REF:%.+]] = getelementptr inbounds nuw { i32, i32 }, ptr [[TEMP:%.+]], i32 0, i32 0
// CHECK: [[TEMP_IMG_REF:%.+]] = getelementptr inbounds nuw { i32, i32 }, ptr [[TEMP]], i32 0, i32 1
// CHECK: store i32 [[VAL]], ptr [[TEMP_REAL_REF]]
// CHECK: store i32 0, ptr [[TEMP_IMG_REF]]
// CHECK: call void @__atomic_store(i64 noundef 8, ptr noundef @{{.+}}, ptr noundef [[TEMP]], i32 noundef 0)
#pragma omp atomic write
  cix = fv;
// CHECK: load double, ptr
// CHECK: store atomic i16 {{.*}} monotonic, align 2
#pragma omp atomic write
  sx = dv;
// CHECK: load x86_fp80, ptr
// CHECK: store atomic i8 {{.*}} monotonic, align 1
#pragma omp atomic write
  bx = ldv;
// CHECK: load i32, ptr @{{.+}}
// CHECK: load i32, ptr getelementptr inbounds nuw ({ i32, i32 }, ptr @{{.+}}, i32 0, i32 1)
// CHECK: icmp ne i32 %{{.+}}, 0
// CHECK: icmp ne i32 %{{.+}}, 0
// CHECK: or i1
// CHECK: store atomic i8 {{.*}} monotonic, align 1
#pragma omp atomic write
  bx = civ;
// CHECK: load float, ptr @{{.*}}
// CHECK: store atomic i16 {{.*}} monotonic, align 2
#pragma omp atomic write
  usx = cfv;
// CHECK: load double, ptr @{{.+}}
// CHECK: store atomic i64 {{.*}} monotonic, align 8
#pragma omp atomic write
  llx = cdv;
// CHECK-DAG: [[IDX:%.+]] = load i16, ptr @{{.+}}
// CHECK-DAG: load i8, ptr
// CHECK-DAG: [[VEC_ITEM_VAL:%.+]] = zext i1 %{{.+}} to i32
// CHECK: [[I128VAL:%.+]] = load atomic i128, ptr [[DEST:@.+]] monotonic, align 16
// CHECK: br label %[[CONT:.+]]
// CHECK: [[CONT]]
// CHECK: [[OLD_I128:%.+]] = phi i128 [ [[I128VAL]], %{{.+}} ], [ [[FAILED_I128_OLD_VAL:%.+]], %[[CONT]] ]
// CHECK: store i128 [[OLD_I128]], ptr [[LDTEMP:%.+]],
// CHECK: [[VEC_VAL:%.+]] = load <4 x i32>, ptr [[LDTEMP]]
// CHECK: [[NEW_VEC_VAL:%.+]] = insertelement <4 x i32> [[VEC_VAL]], i32 [[VEC_ITEM_VAL]], i16 [[IDX]]
// CHECK: store <4 x i32> [[NEW_VEC_VAL]], ptr [[LDTEMP]]
// CHECK: [[NEW_I128:%.+]] = load i128, ptr [[LDTEMP]]
// CHECK: [[RES:%.+]] = cmpxchg ptr [[DEST]], i128 [[OLD_I128]], i128 [[NEW_I128]] monotonic monotonic, align 16
// CHECK: [[FAILED_I128_OLD_VAL:%.+]] = extractvalue { i128, i1 } [[RES]], 0
// CHECK: [[FAIL_SUCCESS:%.+]] = extractvalue { i128, i1 } [[RES]], 1
// CHECK: br i1 [[FAIL_SUCCESS]], label %[[EXIT:.+]], label %[[CONT]]
// CHECK: [[EXIT]]
#pragma omp atomic write
  int4x[sv] = bv;
// CHECK: load x86_fp80, ptr @{{.+}}
// CHECK: [[NEW_VAL:%.+]] = fptosi x86_fp80 %{{.+}} to i32
// CHECK: [[PREV_VALUE:%.+]] = load atomic i32, ptr getelementptr (i8, ptr @{{.+}}, i64 4) monotonic, align 4
// CHECK: br label %[[CONT:.+]]
// CHECK: [[CONT]]
// CHECK: [[OLD_BF_VALUE:%.+]] = phi i32 [ [[PREV_VALUE]], %[[EXIT]] ], [ [[FAILED_OLD_VAL:%.+]], %[[CONT]] ]
// CHECK: [[BF_VALUE:%.+]] = and i32 [[NEW_VAL]], 2147483647
// CHECK: [[BF_CLEAR:%.+]] = and i32 %{{.+}}, -2147483648
// CHECK: or i32 [[BF_CLEAR]], [[BF_VALUE]]
// CHECK: store i32 %{{.+}}, ptr [[LDTEMP:%.+]]
// CHECK: [[NEW_BF_VALUE:%.+]] = load i32, ptr [[LDTEMP]]
// CHECK: [[RES:%.+]] = cmpxchg ptr getelementptr (i8, ptr @{{.+}}, i64 4), i32 [[OLD_BF_VALUE]], i32 [[NEW_BF_VALUE]] monotonic monotonic, align 4
// CHECK: [[FAILED_OLD_VAL]] = extractvalue { i32, i1 } [[RES]], 0
// CHECK: [[FAIL_SUCCESS:%.+]] = extractvalue { i32, i1 } [[RES]], 1
// CHECK: br i1 [[FAIL_SUCCESS]], label %[[EXIT:.+]], label %[[CONT]]
// CHECK: [[EXIT]]
#pragma omp atomic write
  bfx.a = ldv;
// CHECK: load x86_fp80, ptr @{{.+}}
// CHECK: [[NEW_VAL:%.+]] = fptosi x86_fp80 %{{.+}} to i32
// CHECK: call void @__atomic_load(i64 noundef 4, ptr noundef getelementptr (i8, ptr @{{.+}}, i64 4), ptr noundef [[LDTEMP:%.+]], i32 noundef 0)
// CHECK: br label %[[CONT:.+]]
// CHECK: [[CONT]]
// CHECK: [[OLD_BF_VALUE:%.+]] = load i32, ptr [[LDTEMP]],
// CHECK: store i32 [[OLD_BF_VALUE]], ptr [[LDTEMP1:%.+]],
// CHECK: [[OLD_BF_VALUE:%.+]] = load i32, ptr [[LDTEMP1]],
// CHECK: [[BF_VALUE:%.+]] = and i32 [[NEW_VAL]], 2147483647
// CHECK: [[BF_CLEAR:%.+]] = and i32 [[OLD_BF_VALUE]], -2147483648
// CHECK: or i32 [[BF_CLEAR]], [[BF_VALUE]]
// CHECK: store i32 %{{.+}}, ptr [[LDTEMP1]]
// CHECK: [[FAIL_SUCCESS:%.+]] = call zeroext i1 @__atomic_compare_exchange(i64 noundef 4, ptr noundef getelementptr (i8, ptr @{{.+}}, i64 4), ptr noundef [[LDTEMP]], ptr noundef [[LDTEMP1]], i32 noundef 0, i32 noundef 0)
// CHECK: br i1 [[FAIL_SUCCESS]], label %[[EXIT:.+]], label %[[CONT]]
// CHECK: [[EXIT]]
#pragma omp atomic write
  bfx_packed.a = ldv;
// CHECK: load x86_fp80, ptr @{{.+}}
// CHECK: [[NEW_VAL:%.+]] = fptosi x86_fp80 %{{.+}} to i32
// CHECK: [[PREV_VALUE:%.+]] = load atomic i32, ptr @{{.+}} monotonic, align 4
// CHECK: br label %[[CONT:.+]]
// CHECK: [[CONT]]
// CHECK: [[OLD_BF_VALUE:%.+]] = phi i32 [ [[PREV_VALUE]], %[[EXIT]] ], [ [[FAILED_OLD_VAL:%.+]], %[[CONT]] ]
// CHECK: [[BF_AND:%.+]] = and i32 [[NEW_VAL]], 1
// CHECK: [[BF_VALUE:%.+]] = shl i32 [[BF_AND]], 31
// CHECK: [[BF_CLEAR:%.+]] = and i32 %{{.+}}, 2147483647
// CHECK: or i32 [[BF_CLEAR]], [[BF_VALUE]]
// CHECK: store i32 %{{.+}}, ptr [[LDTEMP:%.+]]
// CHECK: [[NEW_BF_VALUE:%.+]] = load i32, ptr [[LDTEMP]]
// CHECK: [[RES:%.+]] = cmpxchg ptr @{{.+}}, i32 [[OLD_BF_VALUE]], i32 [[NEW_BF_VALUE]] monotonic monotonic, align 4
// CHECK: [[FAILED_OLD_VAL]] = extractvalue { i32, i1 } [[RES]], 0
// CHECK: [[FAIL_SUCCESS:%.+]] = extractvalue { i32, i1 } [[RES]], 1
// CHECK: br i1 [[FAIL_SUCCESS]], label %[[EXIT:.+]], label %[[CONT]]
// CHECK: [[EXIT]]
#pragma omp atomic write
  bfx2.a = ldv;
// CHECK: load x86_fp80, ptr @{{.+}}
// CHECK: [[NEW_VAL:%.+]] = fptosi x86_fp80 %{{.+}} to i32
// CHECK: [[PREV_VALUE:%.+]] = load atomic i8, ptr getelementptr (i8, ptr @{{.+}}, i64 3) monotonic, align 1
// CHECK: br label %[[CONT:.+]]
// CHECK: [[CONT]]
// CHECK: [[OLD_BF_VALUE:%.+]] = phi i8 [ [[PREV_VALUE]], %[[EXIT]] ], [ [[FAILED_OLD_VAL:%.+]], %[[CONT]] ]
// CHECK: [[TRUNC:%.+]] = trunc i32 [[NEW_VAL]] to i8
// CHECK: [[BF_AND:%.+]] = and i8 [[TRUNC]], 1
// CHECK: [[BF_VALUE:%.+]] = shl i8 [[BF_AND]], 7
// CHECK: [[BF_CLEAR:%.+]] = and i8 %{{.+}}, 127
// CHECK: or i8 [[BF_CLEAR]], [[BF_VALUE]]
// CHECK: store i8 %{{.+}}, ptr [[LDTEMP:%.+]]
// CHECK: [[NEW_BF_VALUE:%.+]] = load i8, ptr [[LDTEMP]]
// CHECK: [[RES:%.+]] = cmpxchg ptr getelementptr (i8, ptr @{{.+}}, i64 3), i8 [[OLD_BF_VALUE]], i8 [[NEW_BF_VALUE]] monotonic monotonic, align 1
// CHECK: [[FAILED_OLD_VAL]] = extractvalue { i8, i1 } [[RES]], 0
// CHECK: [[FAIL_SUCCESS:%.+]] = extractvalue { i8, i1 } [[RES]], 1
// CHECK: br i1 [[FAIL_SUCCESS]], label %[[EXIT:.+]], label %[[CONT]]
// CHECK: [[EXIT]]
#pragma omp atomic write
  bfx2_packed.a = ldv;
// CHECK: load x86_fp80, ptr @{{.+}}
// CHECK: [[NEW_VAL:%.+]] = fptosi x86_fp80 %{{.+}} to i32
// CHECK: [[PREV_VALUE:%.+]] = load atomic i32, ptr @{{.+}} monotonic, align 4
// CHECK: br label %[[CONT:.+]]
// CHECK: [[CONT]]
// CHECK: [[OLD_BF_VALUE:%.+]] = phi i32 [ [[PREV_VALUE]], %[[EXIT]] ], [ [[FAILED_OLD_VAL:%.+]], %[[CONT]] ]
// CHECK: [[BF_AND:%.+]] = and i32 [[NEW_VAL]], 16383
// CHECK: [[BF_VALUE:%.+]] = shl i32 [[BF_AND]], 11
// CHECK: [[BF_CLEAR:%.+]] = and i32 %{{.+}}, -33552385
// CHECK: or i32 [[BF_CLEAR]], [[BF_VALUE]]
// CHECK: store i32 %{{.+}}, ptr [[LDTEMP:%.+]]
// CHECK: [[NEW_BF_VALUE:%.+]] = load i32, ptr [[LDTEMP]]
// CHECK: [[RES:%.+]] = cmpxchg ptr @{{.+}}, i32 [[OLD_BF_VALUE]], i32 [[NEW_BF_VALUE]] monotonic monotonic, align 4
// CHECK: [[FAILED_OLD_VAL]] = extractvalue { i32, i1 } [[RES]], 0
// CHECK: [[FAIL_SUCCESS:%.+]] = extractvalue { i32, i1 } [[RES]], 1
// CHECK: br i1 [[FAIL_SUCCESS]], label %[[EXIT:.+]], label %[[CONT]]
// CHECK: [[EXIT]]
#pragma omp atomic write
  bfx3.a = ldv;
// CHECK: load x86_fp80, ptr @{{.+}}
// CHECK: [[NEW_VAL:%.+]] = fptosi x86_fp80 %{{.+}} to i32
// CHECK: call void @__atomic_load(i64 noundef 3, ptr noundef getelementptr (i8, ptr @{{.+}}, i64 1), ptr noundef [[BITCAST:%.+]], i32 noundef 0)
// CHECK: br label %[[CONT:.+]]
// CHECK: [[CONT]]
// CHECK: [[OLD_VAL:%.+]] = load i24, ptr %{{.+}},
// CHECK: store i24 [[OLD_VAL]], ptr [[TEMP:%.+]],
// CHECK: [[TRUNC:%.+]] = trunc i32 [[NEW_VAL]] to i24
// CHECK: [[BF_AND:%.+]] = and i24 [[TRUNC]], 16383
// CHECK: [[BF_VALUE:%.+]] = shl i24 [[BF_AND]], 3
// CHECK: [[BF_CLEAR:%.+]] = and i24 %{{.+}}, -131065
// CHECK: or i24 [[BF_CLEAR]], [[BF_VALUE]]
// CHECK: store i24 %{{.+}}, ptr [[TEMP]]
// CHECK: [[FAIL_SUCCESS:%.+]] = call zeroext i1 @__atomic_compare_exchange(i64 noundef 3, ptr noundef getelementptr (i8, ptr @{{.+}}, i64 1), ptr noundef [[LDTEMP:%.+]], ptr noundef [[TEMP]], i32 noundef 0, i32 noundef 0)
// CHECK: br i1 [[FAIL_SUCCESS]], label %[[EXIT:.+]], label %[[CONT]]
// CHECK: [[EXIT]]
#pragma omp atomic write
  bfx3_packed.a = ldv;
// CHECK: load x86_fp80, ptr @{{.+}}
// CHECK: [[NEW_VAL:%.+]] = fptosi x86_fp80 %{{.+}} to i32
// CHECK: [[PREV_VALUE:%.+]] = load atomic i64, ptr @{{.+}} monotonic, align 8
// CHECK: br label %[[CONT:.+]]
// CHECK: [[CONT]]
// CHECK: [[OLD_BF_VALUE:%.+]] = phi i64 [ [[PREV_VALUE]], %[[EXIT]] ], [ [[FAILED_OLD_VAL:%.+]], %[[CONT]] ]
// CHECK: [[ZEXT:%.+]] = zext i32 [[NEW_VAL]] to i64
// CHECK: [[BF_AND:%.+]] = and i64 [[ZEXT]], 1
// CHECK: [[BF_VALUE:%.+]] = shl i64 [[BF_AND]], 16
// CHECK: [[BF_CLEAR:%.+]] = and i64 %{{.+}}, -65537
// CHECK: or i64 [[BF_CLEAR]], [[BF_VALUE]]
// CHECK: store i64 %{{.+}}, ptr [[LDTEMP:%.+]]
// CHECK: [[NEW_BF_VALUE:%.+]] = load i64, ptr [[LDTEMP]]
// CHECK: [[RES:%.+]] = cmpxchg ptr @{{.+}}, i64 [[OLD_BF_VALUE]], i64 [[NEW_BF_VALUE]] monotonic monotonic, align 8
// CHECK: [[FAILED_OLD_VAL]] = extractvalue { i64, i1 } [[RES]], 0
// CHECK: [[FAIL_SUCCESS:%.+]] = extractvalue { i64, i1 } [[RES]], 1
// CHECK: br i1 [[FAIL_SUCCESS]], label %[[EXIT:.+]], label %[[CONT]]
// CHECK: [[EXIT]]
#pragma omp atomic write
  bfx4.a = ldv;
// CHECK: load x86_fp80, ptr @{{.+}}
// CHECK: [[NEW_VAL:%.+]] = fptosi x86_fp80 %{{.+}} to i32
// CHECK: [[PREV_VALUE:%.+]] = load atomic i8, ptr getelementptr inbounds nuw (%struct.BitFields4_packed, ptr @{{.+}}, i32 0, i32 1) monotonic, align 1
// CHECK: br label %[[CONT:.+]]
// CHECK: [[CONT]]
// CHECK: [[OLD_BF_VALUE:%.+]] = phi i8 [ [[PREV_VALUE]], %[[EXIT]] ], [ [[FAILED_OLD_VAL:%.+]], %[[CONT]] ]
// CHECK: [[TRUNC:%.+]] = trunc i32 [[NEW_VAL]] to i8
// CHECK: [[BF_VALUE:%.+]] = and i8 [[TRUNC]], 1
// CHECK: [[BF_CLEAR:%.+]] = and i8 %{{.+}}, -2
// CHECK: or i8 [[BF_CLEAR]], [[BF_VALUE]]
// CHECK: store i8 %{{.+}}, ptr [[LDTEMP:%.+]]
// CHECK: [[NEW_BF_VALUE:%.+]] = load i8, ptr [[LDTEMP]]
// CHECK: [[RES:%.+]] = cmpxchg ptr getelementptr inbounds nuw (%struct.BitFields4_packed, ptr @{{.+}}, i32 0, i32 1), i8 [[OLD_BF_VALUE]], i8 [[NEW_BF_VALUE]] monotonic monotonic, align 1
// CHECK: [[FAILED_OLD_VAL]] = extractvalue { i8, i1 } [[RES]], 0
// CHECK: [[FAIL_SUCCESS:%.+]] = extractvalue { i8, i1 } [[RES]], 1
// CHECK: br i1 [[FAIL_SUCCESS]], label %[[EXIT:.+]], label %[[CONT]]
// CHECK: [[EXIT]]
#pragma omp atomic write
  bfx4_packed.a = ldv;
// CHECK: load x86_fp80, ptr @{{.+}}
// CHECK: [[NEW_VAL:%.+]] = fptosi x86_fp80 %{{.+}} to i64
// CHECK: [[PREV_VALUE:%.+]] = load atomic i64, ptr @{{.+}} monotonic, align 8
// CHECK: br label %[[CONT:.+]]
// CHECK: [[CONT]]
// CHECK: [[OLD_BF_VALUE:%.+]] = phi i64 [ [[PREV_VALUE]], %[[EXIT]] ], [ [[FAILED_OLD_VAL:%.+]], %[[CONT]] ]
// CHECK: [[BF_AND:%.+]] = and i64 [[NEW_VAL]], 127
// CHECK: [[BF_VALUE:%.+]] = shl i64 [[BF_AND]], 17
// CHECK: [[BF_CLEAR:%.+]] = and i64 %{{.+}}, -16646145
// CHECK: or i64 [[BF_CLEAR]], [[BF_VALUE]]
// CHECK: store i64 %{{.+}}, ptr [[LDTEMP:%.+]]
// CHECK: [[NEW_BF_VALUE:%.+]] = load i64, ptr [[LDTEMP]]
// CHECK: [[RES:%.+]] = cmpxchg ptr @{{.+}}, i64 [[OLD_BF_VALUE]], i64 [[NEW_BF_VALUE]] monotonic monotonic, align 8
// CHECK: [[FAILED_OLD_VAL]] = extractvalue { i64, i1 } [[RES]], 0
// CHECK: [[FAIL_SUCCESS:%.+]] = extractvalue { i64, i1 } [[RES]], 1
// CHECK: br i1 [[FAIL_SUCCESS]], label %[[EXIT:.+]], label %[[CONT]]
// CHECK: [[EXIT]]
#pragma omp atomic write
  bfx4.b = ldv;
// CHECK: load x86_fp80, ptr @{{.+}}
// CHECK: [[NEW_VAL:%.+]] = fptosi x86_fp80 %{{.+}} to i64
// CHECK: [[PREV_VALUE:%.+]] = load atomic i8, ptr getelementptr inbounds nuw (%struct.BitFields4_packed, ptr @{{.+}}, i32 0, i32 1) monotonic, align 1
// CHECK: br label %[[CONT:.+]]
// CHECK: [[CONT]]
// CHECK: [[OLD_BF_VALUE:%.+]] = phi i8 [ [[PREV_VALUE]], %[[EXIT]] ], [ [[FAILED_OLD_VAL:%.+]], %[[CONT]] ]
// CHECK: [[TRUNC:%.+]] = trunc i64 [[NEW_VAL]] to i8
// CHECK: [[BF_AND:%.+]] = and i8 [[TRUNC]], 127
// CHECK: [[BF_VALUE:%.+]] = shl i8 [[BF_AND]], 1
// CHECK: [[BF_CLEAR:%.+]] = and i8 %{{.+}}, 1
// CHECK: or i8 [[BF_CLEAR]], [[BF_VALUE]]
// CHECK: store i8 %{{.+}}, ptr [[LDTEMP:%.+]]
// CHECK: [[NEW_BF_VALUE:%.+]] = load i8, ptr [[LDTEMP]]
// CHECK: [[RES:%.+]] = cmpxchg ptr getelementptr inbounds nuw (%struct.BitFields4_packed, ptr @{{.+}}, i32 0, i32 1), i8 [[OLD_BF_VALUE]], i8 [[NEW_BF_VALUE]] monotonic monotonic, align 1
// CHECK: [[FAILED_OLD_VAL]] = extractvalue { i8, i1 } [[RES]], 0
// CHECK: [[FAIL_SUCCESS:%.+]] = extractvalue { i8, i1 } [[RES]], 1
// CHECK: br i1 [[FAIL_SUCCESS]], label %[[EXIT:.+]], label %[[CONT]]
// CHECK: [[EXIT]]
#pragma omp atomic relaxed write
  bfx4_packed.b = ldv;
// CHECK: load i64, ptr
// CHECK: [[VEC_ITEM_VAL:%.+]] = uitofp i64 %{{.+}} to float
// CHECK: [[I64VAL:%.+]] = load atomic i64, ptr [[DEST:@.+]] monotonic, align 8
// CHECK: br label %[[CONT:.+]]
// CHECK: [[CONT]]
// CHECK: [[OLD_I64:%.+]] = phi i64 [ [[I64VAL]], %{{.+}} ], [ [[FAILED_I64_OLD_VAL:%.+]], %[[CONT]] ]
// CHECK: store i64 [[OLD_I64]], ptr [[LDTEMP:%.+]],
// CHECK: [[VEC_VAL:%.+]] = load <2 x float>, ptr [[LDTEMP]]
// CHECK: [[NEW_VEC_VAL:%.+]] = insertelement <2 x float> [[VEC_VAL]], float [[VEC_ITEM_VAL]], i64 0
// CHECK: store <2 x float> [[NEW_VEC_VAL]], ptr [[LDTEMP]]
// CHECK: [[NEW_I64:%.+]] = load i64, ptr [[LDTEMP]]
// CHECK: [[RES:%.+]] = cmpxchg ptr [[DEST]], i64 [[OLD_I64]], i64 [[NEW_I64]] monotonic monotonic, align 8
// CHECK: [[FAILED_I64_OLD_VAL:%.+]] = extractvalue { i64, i1 } [[RES]], 0
// CHECK: [[FAIL_SUCCESS:%.+]] = extractvalue { i64, i1 } [[RES]], 1
// CHECK: br i1 [[FAIL_SUCCESS]], label %[[EXIT:.+]], label %[[CONT]]
// CHECK: [[EXIT]]
#pragma omp atomic write relaxed
  float2x.x = ulv;
// CHECK: call i32 @llvm.read_register.i32(
// CHECK: sitofp i32 %{{.+}} to double
// CHECK: store atomic double %{{.+}}, ptr @{{.+}} seq_cst, align 8
// CHECK: call{{.*}} @__kmpc_flush(
#pragma omp atomic write seq_cst
  dv = rix;
  return 0;
}

#endif
