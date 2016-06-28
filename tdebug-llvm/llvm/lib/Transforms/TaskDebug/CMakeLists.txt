add_llvm_library(LLVMTaskDebug
  TaskDebug.cpp
  TaskDebugBranchCheck.cpp
  AnnotationInfo.cpp
  )

add_dependencies(LLVMTaskDebug intrinsics_gen)