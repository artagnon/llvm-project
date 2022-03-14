#!/bin/sh

opt -load ../../../build/lib/LLVMFuss.dylib -enable-new-pm=0 -fuss -S Input/FactO0.ll
