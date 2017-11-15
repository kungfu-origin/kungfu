/*****************************************************************************
 * Copyright [2017] [taurus.ai]
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *  http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 *****************************************************************************/

//
// Created by ChanghaoJiang on 2017/9/20.
//

#ifndef PROJECT_IJOURNALVISITOR_H
#define PROJECT_IJOURNALVISITOR_H

#include "YJJ_DECLARE.h"

YJJ_NAMESPACE_START

class Frame;
/**
 * visitor mode for reader,
 * any class with this interface can be added into JournalReader
 */
class IJournalVisitor
{
public:
    virtual void visit(const string& name, Frame& frame) = 0;
};

YJJ_NAMESPACE_END

#endif //PROJECT_IJOURNALVISITOR_H
