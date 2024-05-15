//===--- BvaModelConstructorErrCheck.cpp - clang-tidy ---------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#include "BvaModelConstructorErrCheck.h"
#include "clang/ASTMatchers/ASTMatchFinder.h"
#include "clang/ASTMatchers/ASTMatchers.h"
using namespace clang::ast_matchers;

namespace clang::tidy::bugprone {

// 注册匹配器
void BvaModelConstructorErrCheck::registerMatchers(MatchFinder *Finder) {
  // FIXME: Add matchers.
  // 将一个AST匹配器添加到MatchFinder对象中
  // 寻找函数声明的匹配器，并将找到的代表函数声明的functionDecl节点命名为x
  // Finder->addMatcher(functionDecl().bind("x"), this);

  // 一个匹配器，用来识别类中名为is_open_的成员变量
  // auto IsOpenMemberMatcher =
  // fieldDecl(hasName("is_open_")).bind("isOpenMember");

  // 一个匹配器，用来识别构造函数
  // auto ConstructorMatcher = cxxConstructorDecl().bind("constructor");

  // Finder->addMatcher(
  //   cxxConstructorDecl(
  //     forEachDescendant(
  //       memberExpr(member(hasDeclaration(IsOpenMemberMatcher))).bind("memberInConstructor")
  //     ),
  //     unless(hasDescendant(
  //       stmt(hasDescendant(macroExpansion(hasName("BVA_MODEL_CONSTRUCTOR_ERR_CHECK")))).bind("macroInConstructor")
  //     ))
  //   ).bind("constructor"),
  //   this
  // );

  //`cxxConstructorDecl()` 是一个AST匹配器，用于匹配C++中的构造函数声明
  // hasDescendant匹配满足条件的构造函数
  // forEachDescendant遍历构造函数中的子节点，为后代节点执行绑定,用来在"已经选中的构造函数中进一步检查或操作内部的各个子节点
  // Finder->addMatcher(cxxConstructorDecl(hasDescendant(fieldDecl(hasName("is_open_")))).bind("constructor"),
  // this);

  Finder->addMatcher(
      cxxConstructorDecl(
          hasDescendant(
              binaryOperator(hasLHS(memberExpr(member(hasName("is_open_")))))
                  .bind("binaryOp")))
          .bind("constructor"),
      this);

  // Finder->addMatcher(cxxConstructorDecl(hasDescendant(varDecl(hasName("n")).bind("var"))).bind("constructor"),
  // this);
  //  Finder->addMatcher(varDecl(hasType(pointerType(pointee(pointerType(pointee(pointerType()))))))).bind("mulPtr");
}

// 回调函数，在AST匹配器匹配到指定模式后被调用
void BvaModelConstructorErrCheck::check(
    const MatchFinder::MatchResult &Result) {
  // FIXME: Add callback implementation.
  // 用x这个名字取出这个节点，并检测函数名的前缀是否为awesome_，如果不是，则会提示可以将函数名前加上这个前缀
  // MatchedDecl是匹配到的函数声明的指针
  // const auto *MatchedDecl = Result.Nodes.getNodeAs<FunctionDecl>("x");

  // ConstructorDecl是构造函数指针
  const auto *ConstructorDecl =
      Result.Nodes.getNodeAs<CXXConstructorDecl>("constructor");
  const auto *BinOp = Result.Nodes.getNodeAs<BinaryOperator>("binaryOp");
  // if (!MatchedDecl->getIdentifier() ||
  // MatchedDecl->getName().starts_with("awesome_"))
  //   return;
  // diag(MatchedDecl->getLocation(), "function %0 is insufficiently awesome")
  //     << MatchedDecl
  //     << FixItHint::CreateInsertion(MatchedDecl->getLocation(), "awesome_")
  // diag(MatchedDecl->getLocation(), "insert 'awesome'", DiagnosticIDs::Note);
  diag(BinOp->getLHS()->getBeginLoc(), "is_open_ is assgned");
  // diag(ConstructorDecl->getLocation(), "constructor which has is_open_ member
  // expression ", DiagnosticIDs::Error);
  diag(ConstructorDecl->getLocation(),
       "constructor which has is_open_ member expression ");
}

} // namespace clang::tidy::bugprone
