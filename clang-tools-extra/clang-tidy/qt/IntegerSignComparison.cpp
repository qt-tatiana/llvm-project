//===--- IntegerSignComparison.cpp - clang-tidy -----------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#include "IntegerSignComparison.h"
#include "clang/ASTMatchers/ASTMatchFinder.h"
#include "clang/Lex/Lexer.h"
#include "clang/AST/Expr.h"

using namespace clang::ast_matchers;
using namespace clang::ast_matchers::internal;

namespace clang::tidy::qt {

void IntegerSignComparison::registerMatchers(MatchFinder *Finder)
{
  const auto SignedIntCastExpr =
      intCastExpression(true, "sIntCastExpression");
  const auto UnSignedIntCastExpr =
      intCastExpression(false, "uIntCastExpression");

  // Flag all operators "==", "<=", ">=", "<", ">", "!=" that are used between signed/unsigned
  const auto CompareOperator =
      expr(binaryOperator(hasAnyOperatorName("==", "<=", ">=", "<", ">", "!="),
                          anyOf(allOf(hasLHS(SignedIntCastExpr),
                                      hasRHS(UnSignedIntCastExpr)),
                                allOf(hasLHS(UnSignedIntCastExpr),
                                      hasRHS(SignedIntCastExpr)))))
          .bind("IntComparison");

  Finder->addMatcher(CompareOperator, this);
}

BindableMatcher<clang::Stmt> IntegerSignComparison::intCastExpression(
    bool IsSigned, const std::string &CastBindName) const
{
  auto IntTypeExpr = expr();
  if (IsSigned) {
    IntTypeExpr = expr(hasType(
        qualType(isInteger(), isSignedInteger())));
  } else {
    IntTypeExpr = expr(hasType(qualType(
        isInteger(), unless(isSignedInteger()))));
  }

  const auto ImplicitCastExpr =
      implicitCastExpr(hasSourceExpression(IntTypeExpr))
          .bind(CastBindName);

  const auto CStyleCastExpr = cStyleCastExpr(has(ImplicitCastExpr));
  const auto StaticCastExpr = cxxStaticCastExpr(has(ImplicitCastExpr));
  const auto FunctionalCastExpr = cxxFunctionalCastExpr(has(ImplicitCastExpr));

  return traverse(TK_AsIs, expr(anyOf(ImplicitCastExpr, CStyleCastExpr,
                                      StaticCastExpr, FunctionalCastExpr)));
}

void IntegerSignComparison::check(const MatchFinder::MatchResult &Result)
{
  if (!getLangOpts().CPlusPlus20)
    return;

  const auto *SignedCastExpression =
      Result.Nodes.getNodeAs<ImplicitCastExpr>("sIntCastExpression");
  const auto *UnSignedCastExpression =
      Result.Nodes.getNodeAs<ImplicitCastExpr>("uIntCastExpression");
  assert(SignedCastExpression);
  assert(UnSignedCastExpression);

         // Ignore the match if we know that the signed int value is not negative.
  Expr::EvalResult EVResult;
  if (!SignedCastExpression->isValueDependent() &&
      SignedCastExpression->getSubExpr()->EvaluateAsInt(EVResult,
                                                        *Result.Context)) {
    llvm::APSInt sValue = EVResult.Val.getInt();
    if (sValue.isNonNegative()) {
      return;
    }
  }

  const auto *binaryOp = Result.Nodes.getNodeAs<BinaryOperator>("IntComparison");
  if (binaryOp == nullptr) {
    diag(" Nothing interesting ");
    return;
  } else {
    auto opCode = binaryOp->getOpcode();
    StringRef cmpType = "";
    switch (opCode)
    {
    case BO_LT:
      cmpType = "std::cmp_less(";
      break;
    case BO_GT:
      cmpType = "std::cmp_greater(";
      break;
    case BO_LE:
      cmpType = "std::cmp_less_equal(";
      break;
    case BO_GE:
      cmpType = "std::cmp_greater_equal(";
      break;
    case BO_EQ:
      cmpType = "std::cmp_equal(";
      break;
    case BO_NE:
      cmpType = "std::cmp_not_equal(";
      break;
    default: return;
    }

    const auto *LHS = binaryOp->getLHS()->IgnoreParenImpCasts();
    const auto *RHS = binaryOp->getRHS()->IgnoreParenImpCasts();
    if (LHS == nullptr || RHS == nullptr)
    {
      diag("Parsing error: LHS or RHS is null", DiagnosticIDs::Warning);
      return;
    }
    StringRef LHSString(Lexer::getSourceText(
        CharSourceRange::getTokenRange(LHS->getSourceRange()),
        *Result.SourceManager, getLangOpts()));

    StringRef RHSString(Lexer::getSourceText(
        CharSourceRange::getTokenRange(RHS->getSourceRange()),
        *Result.SourceManager, getLangOpts()));

    diag(binaryOp->getBeginLoc(),
         "comparison between 'signed' and 'unsigned' integers")
        << FixItHint::CreateReplacement(
               CharSourceRange::getTokenRange(binaryOp->getBeginLoc(), binaryOp->getEndLoc()),
               StringRef(std::string(cmpType) + std::string(LHSString)
                         + std::string(", ") + std::string(RHSString) + std::string(")")));
  }
}

} // namespace clang::tidy::qt
