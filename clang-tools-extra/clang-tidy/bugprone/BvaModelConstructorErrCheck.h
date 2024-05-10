//===--- BvaModelConstructorErrCheck.h - clang-tidy -------------*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_CLANG_TOOLS_EXTRA_CLANG_TIDY_BUGPRONE_BVAMODELCONSTRUCTORERRCHECK_H
#define LLVM_CLANG_TOOLS_EXTRA_CLANG_TIDY_BUGPRONE_BVAMODELCONSTRUCTORERRCHECK_H

#include "../ClangTidyCheck.h"

namespace clang::tidy::bugprone {

/// FIXME: Write a short description.
///
/// For the user-facing documentation see:
/// http://clang.llvm.org/extra/clang-tidy/checks/bugprone/bva-model-constructor-err.html
class BvaModelConstructorErrCheck : public ClangTidyCheck {
public:
  BvaModelConstructorErrCheck(StringRef Name, ClangTidyContext *Context)
      : ClangTidyCheck(Name, Context) {}
  //定义了当分析AST（抽象语法树）时需要匹配的模式
  void registerMatchers(ast_matchers::MatchFinder *Finder) override;
  //当匹配器找到匹配时执行的实际检查逻辑
  void check(const ast_matchers::MatchFinder::MatchResult &Result) override;
};

} // namespace clang::tidy::bugprone

#endif // LLVM_CLANG_TOOLS_EXTRA_CLANG_TIDY_BUGPRONE_BVAMODELCONSTRUCTORERRCHECK_H
