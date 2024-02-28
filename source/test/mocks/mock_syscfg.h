/*
* Copyright 2020 Comcast Cable Communications Management, LLC
*
* Licensed under the Apache License, Version 2.0 (the "License");
* you may not use this file except in compliance with the License.
* You may obtain a copy of the License at
*
*     http://www.apache.org/licenses/LICENSE-2.0
*
* Unless required by applicable law or agreed to in writing, software
* distributed under the License is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
* See the License for the specific language governing permissions and
* limitations under the License.
*
* SPDX-License-Identifier: Apache-2.0
*/

#ifndef MOCK_SYSCFG_H
#define MOCK_SYSCFG_H

#include <gtest/gtest.h>
#include <gmock/gmock.h>

class SyscfgInterface {
public:
    virtual ~SyscfgInterface() {}
    virtual int syscfg_commit() = 0;
    virtual int syscfg_get(const char *, const char *, char *, int) = 0;
    virtual int syscfg_set(const char *, const char *, const char *) = 0;
};

class SyscfgMock : public SyscfgInterface {
public:
    virtual ~SyscfgMock() {}
    MOCK_METHOD0(syscfg_commit, int(void));
    MOCK_METHOD4(syscfg_get, int(const char *, const char *, char *, int));
    MOCK_METHOD3(syscfg_set, int(const char *, const char *, const char *));
};

#endif
