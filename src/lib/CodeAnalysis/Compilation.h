/*
 * Copyright (c) 2023 F. Lotfi All rights reserved.
 * Use of this source code is governed by a BSD-style
 * license that can be found in the LICENSE file.
 */

#pragma once

#include "Syntax/SyntaxTree.h"
#include <memory>

class Compilation {
public:
    Compilation(std::unique_ptr<SyntaxTree> syntaxTree);
    Compilation(std::unique_ptr<Compilation> previous, 
                std::unique_ptr<SyntaxTree> syntaxTree);
    
    Compilation* Previous();
    SyntaxTree*  STree();
private:
    std::unique_ptr<Compilation> mPrevious;
    std::unique_ptr<SyntaxTree>  mSyntaxTree;
};