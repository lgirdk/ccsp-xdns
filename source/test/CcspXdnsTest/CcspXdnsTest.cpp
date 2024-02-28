/*
* If not stated otherwise in this file or this component's LICENSE file the
* following copyright and licenses apply:
*
* Copyright 2016 RDK Management
*
* Licensed under the Apache License, Version 2.0 (the "License");
* you may not use this file except in compliance with the License.
* You may obtain a copy of the License at
*
* http://www.apache.org/licenses/LICENSE-2.0
*
* Unless required by applicable law or agreed to in writing, software
* distributed under the License is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
* See the License for the specific language governing permissions and
* limitations under the License.
*/

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <stdarg.h>
#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include "test/mocks/mock_syscfg.h"
#include "test/mocks/mock_cosa_xdns_dml.h"

extern "C" {
#include "dmlxdns/cosa_xdns_dml.h"
}

using namespace std;
using namespace testing;
using ::testing::_;
using ::testing::Return;
using ::testing::StrEq;

/* This is the actual definition of the mock obj */
SyscfgMock * g_syscfgMock = NULL;

class XDNSTest : public ::testing::Test {
protected:
        SyscfgMock mockedSyscfg;

        XDNSTest() {
            g_syscfgMock = &mockedSyscfg;
        }
        virtual ~XDNSTest() {
            g_syscfgMock = NULL;
        }
        virtual void SetUp()
        {
            printf("%s %s %s\n", __func__,
                ::testing::UnitTest::GetInstance()->current_test_info()->test_case_name(),
                ::testing::UnitTest::GetInstance()->current_test_info()->name());
        }

        virtual void TearDown()
        {
            printf("%s %s %s\n", __func__,
                ::testing::UnitTest::GetInstance()->current_test_info()->test_case_name(),
                ::testing::UnitTest::GetInstance()->current_test_info()->name());
        }

        static void SetUpTestCase()
        {
            printf("%s %s\n", __func__,
                ::testing::UnitTest::GetInstance()->current_test_case()->name());
        }

        static void TearDownTestCase()
        {
            printf("%s %s\n", __func__,
                ::testing::UnitTest::GetInstance()->current_test_case()->name());
        }
};

ACTION_TEMPLATE(SetArgNPointeeTo, HAS_1_TEMPLATE_PARAMS(unsigned, uIndex), AND_2_VALUE_PARAMS(pData, uiDataSize))
{
    memcpy(std::get<uIndex>(args), pData, uiDataSize);
}

TEST_F(XDNSTest, XDNSDeviceInfo_GetParamBoolValue_XDNSEnabled)
{
    BOOL result;
    char paramName[] = "X_RDKCENTRAL-COM_EnableXDNS";
    char paramName1[] = "X_RDKCENTRAL-COM_XDNS";
    char expectedValue1[] = "1";

    EXPECT_CALL(*g_syscfgMock, syscfg_get( _, StrEq(paramName1), _, _))
        .Times(1)
        .WillOnce(::testing::DoAll(
            SetArgNPointeeTo<2>(std::begin(expectedValue1), sizeof(expectedValue1)),
            ::testing::Return(0)
        ));

    BOOL success = XDNSDeviceInfo_GetParamBoolValue(NULL, paramName, &result);

    EXPECT_EQ(result, TRUE);
}

TEST_F(XDNSTest, XDNSDeviceInfo_GetParamBoolValue_XDNSDisabled)
{
    BOOL result;
    char paramName[] = "X_RDKCENTRAL-COM_EnableXDNS";
    char paramName1[] = "X_RDKCENTRAL-COM_XDNS";
    char expectedValue2[] = "0";

    EXPECT_CALL(*g_syscfgMock, syscfg_get( _, StrEq(paramName1), _, _))
        .Times(1)
        .WillOnce(::testing::DoAll(
            SetArgNPointeeTo<2>(std::begin(expectedValue2), sizeof(expectedValue2)),
            ::testing::Return(0)
        ));

    BOOL success = XDNSDeviceInfo_GetParamBoolValue(NULL, paramName, &result);

    EXPECT_EQ(result, FALSE);
}

TEST_F(XDNSTest, XDNSDeviceInfo_GetParamBoolValue_DNSSecEnabled)
{
    BOOL result;
    char paramName[] = "X_RDKCENTRAL-COM_EnableXDNS";
    char paramName1[] = "XDNS_DNSSecEnable";
    char expectedValue1[] = "1";

    EXPECT_CALL(*g_syscfgMock, syscfg_get( _, StrEq(paramName1), _, _))
        .Times(1)
        .WillOnce(::testing::DoAll(
            SetArgNPointeeTo<2>(std::begin(expectedValue1), sizeof(expectedValue1)),
            ::testing::Return(0)
        ));

    BOOL success = XDNSDeviceInfo_GetParamBoolValue(NULL, paramName, &result);

    EXPECT_EQ(result, TRUE);
}

TEST_F(XDNSTest, XDNSDeviceInfo_GetParamBoolValue_DNSSecDisabled)
{
    BOOL result;
    char paramName[] = "X_RDKCENTRAL-COM_EnableXDNS";
    char paramName1[] = "XDNS_DNSSecEnable";
    char expectedValue2[] = "0";

    EXPECT_CALL(*g_syscfgMock, syscfg_get( _, StrEq(paramName1), _, _))
        .Times(1)
        .WillOnce(::testing::DoAll(
            SetArgNPointeeTo<2>(std::begin(expectedValue2), sizeof(expectedValue2)),
            ::testing::Return(0)
        ));

    BOOL success = XDNSDeviceInfo_GetParamBoolValue(NULL, paramName, &result);

    EXPECT_EQ(result, FALSE);
}

TEST_F(XDNSTest, XDNSRefac_GetParamBoolValue_RefacCodeEnable)
{
    BOOL result;
    char paramName[] = "Enable";
    char paramName1[] = "XDNS_RefacCodeEnable";
    char expectedValue1[] = "1";

    EXPECT_CALL(*g_syscfgMock, syscfg_get( _, StrEq(paramName1), _, _))
        .Times(1)
        .WillOnce(::testing::DoAll(
            SetArgNPointeeTo<2>(std::begin(expectedValue1), sizeof(expectedValue1)),
            ::testing::Return(0)
        ));

    BOOL success = XDNSRefac_GetParamBoolValue(NULL, paramName, &result);

    EXPECT_EQ(result, TRUE);
}

TEST_F(XDNSTest, XDNSRefac_GetParamBoolValue_RefacCodeDisable)
{
    BOOL result;
    char paramName[] = "Enable";
    char paramName1[] = "XDNS_RefacCodeEnable";
    char expectedValue2[] = "0";

    EXPECT_CALL(*g_syscfgMock, syscfg_get( _, StrEq(paramName1), _, _))
        .Times(1)
        .WillOnce(::testing::DoAll(
            SetArgNPointeeTo<2>(std::begin(expectedValue2), sizeof(expectedValue2)),
            ::testing::Return(0)
        ));

    BOOL success = XDNSRefac_GetParamBoolValue(NULL, paramName, &result);

    EXPECT_EQ(result, FALSE);
}