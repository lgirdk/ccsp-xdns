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

#include "test/mocks/mock_syscfg.h"

using namespace std;

extern SyscfgMock * g_syscfgMock; // Declaration of the global mock object

// Mock Method Definitions
extern "C" int syscfg_commit() 
{
    if (!g_syscfgMock) 
    {
        return 0;
    }
    return g_syscfgMock->syscfg_commit();
}
extern "C" int syscfg_get(const char *ns, const char *name, char *out_value, int outbufsz)
{
	if(!g_syscfgMock)
	{
	    return 0;
	}
    return g_syscfgMock->syscfg_get(ns, name, out_value, outbufsz);
}
extern "C" int syscfg_set(const char *ns, const char *name, const char *value)
{
    if(!g_syscfgMock)
    {
        return 0;
    }
    return g_syscfgMock->syscfg_set(ns, name, value);
}
