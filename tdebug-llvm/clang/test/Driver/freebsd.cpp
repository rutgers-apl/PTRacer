// RUN: %clangxx %s -### -o %t.o -target amd64-unknown-freebsd10.0 2>&1 \
// RUN:   | FileCheck --check-prefix=CHECK-TEN %s
// RUN: %clangxx %s -### -o %t.o -target amd64-unknown-freebsd9.2 2>&1 \
// RUN:   | FileCheck --check-prefix=CHECK-NINE %s
// CHECK-TEN: -lc++
// CHECK-NINE: -lstdc++
