//===--- BvaModelConstructorErrCheck.cpp - clang-tidy ---------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#include "BvaModelConstructorErrCheck.h"
#include "clang/AST/ASTContext.h"
#include "clang/ASTMatchers/ASTMatchFinder.h"
#include "clang/ASTMatchers/ASTMatchers.h"
#include "clang/Lex/Lexer.h"
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
  
  //Matcher1
//   Finder->addMatcher(
//       cxxConstructorDecl(
//           hasDescendant(
//               binaryOperator(hasLHS(memberExpr(member(hasName("is_open_")))))
//                   .bind("binaryOp")))
//           .bind("constructor"),
//       this);
  //Matcher2
//   Finder->addMatcher(cxxConstructorDecl(
//     hasDescendant(
//         varDecl(hasType(cxxRecordDecl(hasName("LogGuard"))))
//         .bind("macro"))
//         ), 
//         this);
  
Finder->addMatcher(
  cxxConstructorDecl(
    allOf( // 使用allOf表示这两个条件都需要同时满足
      hasDescendant(
        binaryOperator(
          hasLHS(memberExpr(member(hasName("is_open_"))))
        ).bind("binaryOp")
      ),
      unless(
        hasDescendant(
          varDecl(
            hasType(cxxRecordDecl(hasName("LogGuard")))
          ).bind("macro")
        )
      )
    )
  ).bind("constructor"),
  this
);

  // Finder->addMatcher(cxxConstructorDecl(hasDescendant(varDecl(hasName("n")).bind("var"))).bind("constructor"),
  // this);
  //  Finder->addMatcher(varDecl(hasType(pointerType(pointee(pointerType(pointee(pointerType()))))))).bind("mulPtr");
}

// 回调函数，在AST匹配器匹配到指定模式后被调用
void BvaModelConstructorErrCheck::check(
    const MatchFinder::MatchResult &Result) {

  // ConstructorDecl是构造函数指针
  const auto *ConstructorDecl = Result.Nodes.getNodeAs<CXXConstructorDecl>("constructor");
  const auto *BinOp = Result.Nodes.getNodeAs<BinaryOperator>("binaryOp");
  if (!ConstructorDecl || !BinOp) {
    return;
  }
  
  SourceLocation InsertLoc = ConstructorDecl->getBody()->getBeginLoc().getLocWithOffset(1);
  std::string LogGuardDeclCode = "\n    BVA_MODEL_CONSTRUCTOR_ERR_CHECK";

  bool IncludeHeader = false;
  SourceManager &SM = *Result.SourceManager;
  FileID ConstructorFileID = SM.getFileID(ConstructorDecl->getLocation());
  // 获取clang编译器的语言选项
  // const LangOptions &LangOpts = Result.Context->getLangOpts();

  for  (auto it = SM.fileinfo_begin(); it != SM.fileinfo_end(); ++it) {
    const FileEntry *entry = it->getFirst();
    if (nullptr != entry && entry->getName().endswith("log_guard.h")) {
      IncludeHeader = true;
      break;
    }
  }
  
  // 如果没有包含log_guard.h头文件
  if (!IncludeHeader) {
    //获取文件开头的位置，用作头文件插入
    SourceLocation HeaderIncludeLoc = SM.getLocForStartOfFile(ConstructorFileID);
    //跳过空格和注释，从AST上下文中检索当前的编译器语言设置
    HeaderIncludeLoc = clang::Lexer::GetBeginningOfToken(HeaderIncludeLoc, SM, Result.Context->getLangOpts());

    std::string HeaderIncludeCode = "#include \"util/tools/log_guard.h\"\n";

    auto FixIt = FixItHint::CreateInsertion(HeaderIncludeLoc, HeaderIncludeCode);
    diag(HeaderIncludeLoc, "log_guard.h should be included at the top of the file.") << FixIt; 
  }
  
  auto FixIt = FixItHint::CreateInsertion(InsertLoc, LogGuardDeclCode);
  diag(InsertLoc, "constructor has is_open_ member expression but hasn't defined LogGuard") << FixIt;
  // diag(ConstructorDecl->getLocation(), "constructor has is_open_ member expression but haven't define LogGuard");
  diag(BinOp->getLHS()->getBeginLoc(), "is_open_ is assigned", DiagnosticIDs::Note);

}

} // namespace clang::tidy::bugprone
