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
  // diag(MatchedExpr->getExprLoc(), "call method RunSession", DiagnosticIDs::Note);

  const DynTypedNodeList &runParents = Result.Context->getParents(*MatchedExpr);
  const BinaryOperator *binaryOp = nullptr;
  const VarDecl *varDecl = nullptr;
  
  // 获取父节点，如果有定义变量接收返回值，父节点是BinaryOperator类型或者VarDecl类型
  for (const DynTypedNode &parent: runParents) {
    // 将父节点作为BinaryOperator类型对象获取，如果父节点不是这种类型，返回nullptr
    const VarDecl *parentVarDecl = parent.get<VarDecl>();
    const BinaryOperator *parentBinaryOp = parent.get<BinaryOperator>();
    if (nullptr == parentBinaryOp && nullptr == parentVarDecl) {
    // if (nullptr == parentBinaryOp) {
      // 如果没有接收runSession的返回值
      diag(MatchedExpr->getBeginLoc(), "haven't defined a variable to receive the return value");
      // diag(MatchedExpr->getExprLoc(), "call method RunSession", DiagnosticIDs::Note);
      return;
    } else {
      // 如果有变量接收runSession的返回值，再获取上一层父节点
      binaryOp = parentBinaryOp;
      varDecl = parentVarDecl;
      // if (parentBinaryOp != nullptr)
      //   diag(parentBinaryOp->getExprLoc(), "binary operator");
      // if (parentVarDecl != nullptr)
      //   diag(parentVarDecl->getLocation(), "var decl");
      break;
    }
  }
  // // 获取=号左边的操作数
  // const clang::Expr *lhs = binaryOp->getLHS();
  // // isa<>模板函数用于检查一个对象是否是一个特定类型
  // if (lhs && isa<clang::DeclRefExpr>(lhs)) {
  //   // cast将基类型指针安全转换为派生类型指针
  //   const clang::DeclRefExpr *lhsDeclRef = cast<clang::DeclRefExpr>(lhs);
  //   if (lhsDeclRef->getDecl()) {
  //     llvm::StringRef lhsVarName = lhsDeclRef->getDecl()->getName();
  //     llvm::outs() << lhsVarName << "\n";
  //   }
  // }

  // 获取父节点
  if (nullptr != binaryOp) {
    llvm::outs() << "binaryOp!!!" << "\n";
    const DynTypedNodeList &parents = Result.Context->getParents(*binaryOp);
    for (const DynTypedNode &parent: parents) {
      // ASTNodeKind kind = parent.getNodeKind();
      // llvm::outs() << "Node kind:" << kind.asStringRef() << "\n";  
      const Stmt *stmtNode = parent.get<Stmt>();
      if (nullptr == stmtNode) {
        llvm::outs() << " failed to get the stmt node" << "\n";
        diag(MatchedExpr->getBeginLoc(), "failed to get the parent node\n");
        return;
      } else {
        // diag(stmtNode->getBeginLoc(), "detect stmt node");
        // diag(MatchedExpr->getExprLoc(), "call method RunSession", DiagnosticIDs::Note);
        // 遍历stmtNode的子节点，找到binaryOp的下一个节点
        for (auto it = stmtNode->child_begin(); it != stmtNode->child_end(); ++it) {
          // Stmt::StmtClass kind = (*it)->getStmtClass();
          // const char *stmtName = (*it)->getStmtClassName();
          
          //查看子节点的行号
          // SourceLocation loc = (*it)->getBeginLoc();
          // SourceManager &SM = *Result.SourceManager;
          // FullSourceLoc fullLoc(loc, SM);
          // unsigned lineNum = fullLoc.getSpellingLineNumber();

          // llvm::outs() << "found a child node of class:" << stmtName << " line: "<< lineNum << "\n";
          if (*it == binaryOp) {
            // llvm::outs() << "match the binary operator" << "\n";
            it++;
            if (it == stmtNode->child_end() || (*it)->getStmtClass() != Stmt::IfStmtClass) {
              diag(MatchedExpr->getBeginLoc(), "haven't checked the return value of RunSession");
              return;
            }
            // 如果是if语句，判断条件表达式
            // const clang::IfStmt *ifStmt = clang::cast<clang::IfStmt>(*it);
            // const clang::Expr *condExpr = ifStmt->getCond();

          }
        }
      }
    }
  
  }


  
  if (nullptr != varDecl) { 
    llvm::outs() << "varDecl !!!" << "\n";
    // 如果是变量定义的节点varDecl，上一层是DeclStmt节点，再上一层才是语句所在的代码块
    const DynTypedNodeList &varDeclParents = Result.Context->getParents(*varDecl);
    const DynTypedNode &parent = varDeclParents[0];
    const DeclStmt *declStmtNode = parent.get<DeclStmt>();
    if (nullptr == declStmtNode) {
      diag(MatchedExpr->getBeginLoc(), "failed to get decl stmt node\n");
      return;
    }
    const DynTypedNodeList &parents = Result.Context->getParents(*declStmtNode);
    for (const DynTypedNode &parent: parents) {
      // ASTNodeKind kind = parent.getNodeKind();
      // llvm::outs() << "Node kind:" << kind.asStringRef() << "\n";  
      const Stmt *stmtNode = parent.get<Stmt>();
      if (nullptr == stmtNode) {
        // llvm::outs() << " failed to get the stmt node" << "\n";
        diag(MatchedExpr->getBeginLoc(), "failed to get the parent node\n");
        return;
      } else {
        // diag(stmtNode->getBeginLoc(), "detect stmt node");
        // diag(MatchedExpr->getExprLoc(), "call method RunSession", DiagnosticIDs::Note);
        // 遍历stmtNode的子节点，找到varDecl的下一个节点
        for (auto it = stmtNode->child_begin(); it != stmtNode->child_end(); ++it) {
          // Stmt::StmtClass kind = (*it)->getStmtClass();
          // const char *stmtName = (*it)->getStmtClassName();
          
          // 查看子节点的行号
          // SourceLocation loc = (*it)->getBeginLoc();
          // SourceManager &SM = *Result.SourceManager;
          // FullSourceLoc fullLoc(loc, SM);
          // unsigned lineNum = fullLoc.getSpellingLineNumber();

          // llvm::outs() << "found a child node of class:" << stmtName << " line: "<< lineNum << "\n";
          if (*it == declStmtNode) {
            // llvm::outs() << "match the binary operator" << "\n";
            it++;
            if (it == stmtNode->child_end() || (*it)->getStmtClass() != Stmt::IfStmtClass) {
              diag(MatchedExpr->getBeginLoc(), "haven't checked the return value of RunSession");
              return;
            }
          //   // 如果是if语句，判断条件表达式
          //   // const clang::IfStmt *ifStmt = clang::cast<clang::IfStmt>(*it);
          //   // const clang::Expr *condExpr = ifStmt->getCond();

          }
        }
      }
    }
  


  }
  
}



} // namespace clang::tidy::bugprone
