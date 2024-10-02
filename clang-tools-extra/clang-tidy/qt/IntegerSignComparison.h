//===--- IntegerSignComparison.h - clang-tidy ---------*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_CLANG_TOOLS_EXTRA_CLANG_TIDY_QT_INTEGERSIGNCOMPARISON_H
#define LLVM_CLANG_TOOLS_EXTRA_CLANG_TIDY_QT_INTEGERSIGNCOMPARISON_H

#include "../ClangTidyCheck.h"

namespace clang::tidy::qt {

/// Class detects comparisons between signed and unsigned integer values
///
/// For the user-facing documentation see:
/// http://clang.llvm.org/extra/clang-tidy/checks/qt/IntegerSignComparison.html
class IntegerSignComparison : public ClangTidyCheck {
public:
  IntegerSignComparison(StringRef Name, ClangTidyContext *Context)
      : ClangTidyCheck(Name, Context) {}
  void registerMatchers(ast_matchers::MatchFinder *Finder) override;
  void check(const ast_matchers::MatchFinder::MatchResult &Result) override;

private:
  ast_matchers::internal::BindableMatcher<clang::Stmt> intCastExpression(
      bool IsSigned, const std::string &CastBindName) const;
};

} // namespace clang::tidy::qt

#endif // LLVM_CLANG_TOOLS_EXTRA_CLANG_TIDY_QT_INTEGERSIGNCOMPARISON_H
