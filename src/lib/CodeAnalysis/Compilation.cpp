/*
 * Copyright (c) 2023 F. Lotfi All rights reserved.
 * Use of this source code is governed by a BSD-style
 * license that can be found in the LICENSE file.
 */

#include "Compilation.h"

Compilation::Compilation(std::unique_ptr<SyntaxTree> syntaxTree) : 
    mPrevious(nullptr), mSyntaxTree(std::move(syntaxTree)) { }

Compilation::Compilation(std::unique_ptr<Compilation> previous, 
            std::unique_ptr<SyntaxTree> syntaxTree) :
             mPrevious(std::move(previous)), mSyntaxTree(std::move(syntaxTree)) { }
    
    Compilation* Compilation::Previous() {
        return mPrevious.get();
    }
    SyntaxTree*  Compilation::STree() {
        return mSyntaxTree.get();
    }