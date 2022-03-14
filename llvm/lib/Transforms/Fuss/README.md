# Fuss: code obfuscation challenge

Included in this commit are:

```text
commit 11bad9ddf464533797bd6e47bb2e68722a5ae6ad (HEAD -> fuss)
Author: Ramkumar Ramachandra <r@artagnon.com>
Date:   il y a 9 heures

    Transforms/Fuss: introduce new obfuscation pass

    Include a README.md with instructions.

 llvm/lib/Transforms/CMakeLists.txt         |   1 +
 llvm/lib/Transforms/Fuss/CMakeLists.txt    |   6 ++
 llvm/lib/Transforms/Fuss/Fuss.cpp          | 109 +++++++++++++++++++++++++
 llvm/lib/Transforms/Fuss/README.md         |  27 +++++++
 llvm/test/Transforms/Fuss/Input/Fact.cpp   |   6 ++
 llvm/test/Transforms/Fuss/Input/FactO0.ll  |  44 +++++++++++
 llvm/test/Transforms/Fuss/Input/FactO3.bc  | Bin 0 -> 3296 bytes
 llvm/test/Transforms/Fuss/Input/FactO3.ll  | 123 +++++++++++++++++++++++++++++
 llvm/test/Transforms/Fuss/Output/FactO0.ll |  56 +++++++++++++
 llvm/test/Transforms/Fuss/Output/Main.cpp  |   8 ++
 llvm/test/Transforms/Fuss/test.sh          |   3 +
 11 files changed, 383 insertions(+)
```

Fact.cpp contains comments for further discussion.
