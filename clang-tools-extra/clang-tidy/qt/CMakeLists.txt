set(LLVM_LINK_COMPONENTS
    FrontendOpenMP
    Support
)

add_clang_library(clangTidyQtModule
  IntegerSignComparison.cpp
  QtTidyModule.cpp

  LINK_LIBS
  clangTidy
  clangTidyMiscModule
  clangTidyModernizeModule
  clangTidyPerformanceModule
  clangTidyReadabilityModule
  clangTidyUtils

  DEPENDS
  omp_gen
  ClangDriverOptions
  )

clang_target_link_libraries(clangTidyQtModule
  PRIVATE
  clangAnalysis
  clangAST
  clangASTMatchers
  clangBasic
  clangLex
  clangSerialization
  clangTooling
  )
