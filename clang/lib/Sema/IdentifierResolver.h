//===- IdentifierResolver.h - Lexical Scope Name lookup ---------*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file defines the IdentifierResolver class, which is used for lexical
// scoped lookup, based on declaration names.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_CLANG_AST_SEMA_IDENTIFIERRESOLVER_H
#define LLVM_CLANG_AST_SEMA_IDENTIFIERRESOLVER_H

#include "clang/Basic/IdentifierTable.h"
#include "clang/Parse/Scope.h"
#include "clang/AST/Decl.h"
#include "clang/AST/DeclarationName.h"
#include "clang/AST/DeclCXX.h"

namespace clang {

/// IdentifierResolver - Keeps track of shadowed decls on enclosing
/// scopes.  It manages the shadowing chains of declaration names and
/// implements efficent decl lookup based on a declaration name.
class IdentifierResolver {

  /// IdDeclInfo - Keeps track of information about decls associated
  /// to a particular declaration name. IdDeclInfos are lazily
  /// constructed and assigned to a declaration name the first time a
  /// decl with that declaration name is shadowed in some scope.
  class IdDeclInfo {
  public:
    typedef llvm::SmallVector<NamedDecl*, 2> DeclsTy;

    inline DeclsTy::iterator decls_begin() { return Decls.begin(); }
    inline DeclsTy::iterator decls_end() { return Decls.end(); }

    void AddDecl(NamedDecl *D) { Decls.push_back(D); }

    /// AddShadowed - Add a decl by putting it directly above the 'Shadow' decl.
    /// Later lookups will find the 'Shadow' decl first. The 'Shadow' decl must
    /// be already added to the scope chain and must be in the same context as
    /// the decl that we want to add.
    void AddShadowed(NamedDecl *D, NamedDecl *Shadow);

    /// RemoveDecl - Remove the decl from the scope chain.
    /// The decl must already be part of the decl chain.
    void RemoveDecl(NamedDecl *D);

    /// Replaces the Old declaration with the New declaration. If the
    /// replacement is successful, returns true. If the old
    /// declaration was not found, returns false.
    bool ReplaceDecl(NamedDecl *Old, NamedDecl *New);

  private:
    DeclsTy Decls;
  };

public:

  /// iterator - Iterate over the decls of a specified declaration name.
  /// It will walk or not the parent declaration contexts depending on how
  /// it was instantiated.
  class iterator {
  public:
    typedef NamedDecl *             value_type;
    typedef NamedDecl *             reference;
    typedef NamedDecl *             pointer;
    typedef std::input_iterator_tag iterator_category;
    typedef std::ptrdiff_t          difference_type;

    /// Ptr - There are 3 forms that 'Ptr' represents:
    /// 1) A single NamedDecl. (Ptr & 0x1 == 0)
    /// 2) A IdDeclInfo::DeclsTy::iterator that traverses only the decls of the
    ///    same declaration context. (Ptr & 0x3 == 0x1)
    /// 3) A IdDeclInfo::DeclsTy::iterator that traverses the decls of parent
    ///    declaration contexts too. (Ptr & 0x3 == 0x3)
    uintptr_t Ptr;
    typedef IdDeclInfo::DeclsTy::iterator BaseIter;

    /// A single NamedDecl. (Ptr & 0x1 == 0)
    iterator(NamedDecl *D) {
      Ptr = reinterpret_cast<uintptr_t>(D);
      assert((Ptr & 0x1) == 0 && "Invalid Ptr!");
    }
    /// A IdDeclInfo::DeclsTy::iterator that walks or not the parent declaration
    /// contexts depending on 'LookInParentCtx'.
    iterator(BaseIter I) {
      Ptr = reinterpret_cast<uintptr_t>(I) | 0x1;
    }

    bool isIterator() const { return (Ptr & 0x1); }

    BaseIter getIterator() const {
      assert(isIterator() && "Ptr not an iterator!");
      return reinterpret_cast<BaseIter>(Ptr & ~0x3);
    }
    
    friend class IdentifierResolver;
  public:
    iterator() : Ptr(0) {}

    NamedDecl *operator*() const {
      if (isIterator())
        return *getIterator();
      else
        return reinterpret_cast<NamedDecl*>(Ptr);
    }
    
    bool operator==(const iterator &RHS) const {
      return Ptr == RHS.Ptr;
    }
    bool operator!=(const iterator &RHS) const {
      return Ptr != RHS.Ptr;
    }
    
    // Preincrement.
    iterator& operator++() {
      if (!isIterator()) // common case.
        Ptr = 0;
      else {
        NamedDecl *D = **this;
        void *InfoPtr = D->getDeclName().getFETokenInfo<void>();
        assert(!isDeclPtr(InfoPtr) && "Decl with wrong id ?");
        IdDeclInfo *Info = toIdDeclInfo(InfoPtr);
        
        BaseIter I = getIterator();
        if (I != Info->decls_begin())
          *this = iterator(I-1);
        else // No more decls.
          *this = iterator();
      }
      return *this;
    }

    uintptr_t getAsOpaqueValue() const { return Ptr; }
    
    static iterator getFromOpaqueValue(uintptr_t P) {
      iterator Result;
      Result.Ptr = P;
      return Result;
    }
  };

  /// begin - Returns an iterator for decls with the name 'Name'.
  static iterator begin(DeclarationName Name);

  /// end - Returns an iterator that has 'finished'.
  static iterator end() {
    return iterator();
  }

  /// isDeclInScope - If 'Ctx' is a function/method, isDeclInScope returns true
  /// if 'D' is in Scope 'S', otherwise 'S' is ignored and isDeclInScope returns
  /// true if 'D' belongs to the given declaration context.
  bool isDeclInScope(Decl *D, DeclContext *Ctx, ASTContext &Context,
                     Scope *S = 0) const;

  /// AddDecl - Link the decl to its shadowed decl chain.
  void AddDecl(NamedDecl *D);

  /// AddShadowedDecl - Link the decl to its shadowed decl chain putting it
  /// after the decl that the iterator points to, thus the 'Shadow' decl will be
  /// encountered before the 'D' decl.
  void AddShadowedDecl(NamedDecl *D, NamedDecl *Shadow);

  /// RemoveDecl - Unlink the decl from its shadowed decl chain.
  /// The decl must already be part of the decl chain.
  void RemoveDecl(NamedDecl *D);

  /// Replace the decl Old with the new declaration New on its
  /// identifier chain. Returns true if the old declaration was found
  /// (and, therefore, replaced).
  bool ReplaceDecl(NamedDecl *Old, NamedDecl *New);

  /// \brief Link the declaration into the chain of declarations for
  /// the given identifier.
  ///
  /// This is a lower-level routine used by the PCH reader to link a
  /// declaration into a specific IdentifierInfo before the
  /// declaration actually has a name.
  void AddDeclToIdentifierChain(IdentifierInfo *II, NamedDecl *D);

  explicit IdentifierResolver(const LangOptions &LangOpt);
  ~IdentifierResolver();

private:
  const LangOptions &LangOpt;

  class IdDeclInfoMap;
  IdDeclInfoMap *IdDeclInfos;

  /// FETokenInfo contains a Decl pointer if lower bit == 0.
  static inline bool isDeclPtr(void *Ptr) {
    return (reinterpret_cast<uintptr_t>(Ptr) & 0x1) == 0;
  }

  /// FETokenInfo contains a IdDeclInfo pointer if lower bit == 1.
  static inline IdDeclInfo *toIdDeclInfo(void *Ptr) {
    assert((reinterpret_cast<uintptr_t>(Ptr) & 0x1) == 1
          && "Ptr not a IdDeclInfo* !");
    return reinterpret_cast<IdDeclInfo*>(
                    reinterpret_cast<uintptr_t>(Ptr) & ~0x1
                                                            );
  }
};

} // end namespace clang

#endif
