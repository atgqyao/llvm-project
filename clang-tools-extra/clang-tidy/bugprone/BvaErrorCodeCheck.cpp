//===--- BvaErrorCodeCheck.cpp - clang-tidy -------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#include "BvaErrorCodeCheck.h"
#include "clang/AST/ASTContext.h"
#include "clang/ASTMatchers/ASTMatchFinder.h"

using namespace clang::ast_matchers;

namespace clang::tidy::bugprone {

void BvaErrorCodeCheck::registerMatchers(MatchFinder *Finder) {
  // FIXME: Add matchers.
  // Finder->addMatcher(functionDecl().bind("x"), this);

  // 匹配runSession调用处
  Finder->addMatcher(cxxMemberCallExpr(hasDescendant(memberExpr(member(
                                           namedDecl(hasName("RunSession"))))))
                         .bind("runSession"),
                     this);

  // 匹配没有返回值接收的runSession调用处
  // Finder->addMatcher(
  // cxxMemberCallExpr(allOf(
  //     hasDescendant(memberExpr(member(namedDecl(hasName("RunSession"))))),
  //     unless(hasParent(binaryOperator())))).bind("returnValue"),
  //     this);

  // 匹配两者
  // auto runSessionMatcher = memberExpr(member(namedDecl(hasName("RunSession"))));
  // auto notInBinaryOperatorMatcher = unless(hasParent(binaryOperator()));
  // Finder->addMatcher(
  //     cxxMemberCallExpr(anyOf(hasDescendant(runSessionMatcher.bind("runSession")),
  //                             allOf(hasDescendant(runSessionMatcher.bind("returnValue")),
  //                                   notInBinaryOperatorMatcher)))
  //         ,
  //     this);
}

void BvaErrorCodeCheck::check(const MatchFinder::MatchResult &Result) {
  const CXXMemberCallExpr *MatchedExpr =
      Result.Nodes.getNodeAs<CXXMemberCallExpr>("runSession");
  diag(MatchedExpr->getExprLoc(), "call method RunSession", DiagnosticIDs::Note);

  const DynTypedNodeList &parents = Result.Context->getParents(*MatchedExpr);
  
  for (const DynTypedNode &parent: parents) {
    // 将父节点作为BinaryOperator类型对象获取，如果父节点不是这种类型，返回nullptr
    const BinaryOperator *parentStmt = parent.get<BinaryOperator>();
    if (nullptr == parentStmt) {
      // 如果没有接收runSession的返回值
      diag(MatchedExpr->getBeginLoc(), "haven't defined a variable to receive the return value");
      return;
    } else {
      // 如果有变量接收runSession的返回值，再获取上一层父节点
      // parents = Result.Context->getParents(*parentStmt);
      break;
    }
  }

  // 获取父节点
  //       binaryOp = parentStmt;
  //     parents = Result.Context->getParents(*parentStmt);
  // const DynTypedNode &parentNode = parents[0];
  // ASTNodeKind kind = parentNode.getNodeKind();
  // llvm::outs() << "Node kind:" << kind.asStringRef() << "\n";
  // const IfStmt *parent = parentNode.get<Stmt>();
  // if (nullptr == parent) return;

  // for (auto it = parent->child_begin(); it != parent->child_end(); ++it) {
  //   if (it == binaryOp)
  // }
  


  // // 查看下一个节点，看是不是if语句
  // if (MatchedExpr != nullptr) {
  //   auto *parent = MatchedExpr->getRPare
  // }
}
} // namespace clang::tidy::bugprone
