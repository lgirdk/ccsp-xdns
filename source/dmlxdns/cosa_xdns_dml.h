/*
 * If not stated otherwise in this file or this component's Licenses.txt file the
 * following copyright and licenses apply:
 *
 * Copyright 2015 RDK Management
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

#ifndef  _COSA_XDNS_DML_H
#define  _COSA_XDNS_DML_H


/***********************************************************************

 APIs for Object:

    SelfHeal.


***********************************************************************/
/***********************************************************************

 APIs for Object:

    SelfHeal.XDNS.

    *  XDNS_GetParamStringValue
    *  XDNS_SetParamStringValue
    *  XDNS_Validate
    *  XDNS_Commit
    *  XDNS_Rollback

***********************************************************************/

ULONG
XDNS_GetParamStringValue
    (
        ANSC_HANDLE                 hInsContext,
        char*                       ParamName,
        char*                       pValue,
        ULONG*                      pUlSize
    );

BOOL
XDNS_SetParamStringValue
    (
        ANSC_HANDLE                 hInsContext,
        char*                       ParamName,
        char*                       strValue
    );

BOOL
XDNS_Validate
    (
        ANSC_HANDLE                 hInsContext,
        char*                       pReturnParamName,
        ULONG*                      puLength
    );

ULONG
XDNS_Commit
    (
        ANSC_HANDLE                 hInsContext
    );

ULONG
XDNS_Rollback
    (
        ANSC_HANDLE                 hInsContext
    );

/***********************************************************************

 APIs for Object:

    SelfHeal.XDNS.PingServerList.MacDNSMappingTable.{i}.

    *  MacDNSMappingTable_GetEntryCount
    *  MacDNSMappingTable_GetEntry
    *  MacDNSMappingTable_IsUpdated
    *  MacDNSMappingTable_Synchronize
    *  MacDNSMappingTable_AddEntry
    *  MacDNSMappingTable_DelEntry
    *  MacDNSMappingTable_GetParamStringValue
    *  MacDNSMappingTable_SetParamStringValue
    *  MacDNSMappingTable_Validate
    *  MacDNSMappingTable_Commit
    *  MacDNSMappingTable_Rollback

***********************************************************************/

ULONG
MacDNSMappingTable_GetEntryCount
    (
        ANSC_HANDLE
    );

ANSC_HANDLE
MacDNSMappingTable_GetEntry
    (
        ANSC_HANDLE                 hInsContext,
        ULONG                       nIndex,
        ULONG*                      pInsNumber
    );

BOOL
MacDNSMappingTable_IsUpdated
    (
        ANSC_HANDLE                 hInsContext
    );

ULONG
MacDNSMappingTable_Synchronize
    (
        ANSC_HANDLE                 hInsContext
    );

ANSC_HANDLE
MacDNSMappingTable_AddEntry
    (
        ANSC_HANDLE                 hInsContext,
        ULONG*                      pInsNumber
    );

ULONG
MacDNSMappingTable_DelEntry
    (
        ANSC_HANDLE                 hInsContext,
        ANSC_HANDLE                 hInstance
    );

ULONG
MacDNSMappingTable_GetParamStringValue
    (
        ANSC_HANDLE                 hInsContext,
        char*                       ParamName,
        char*                       pValue,
        ULONG*                      pUlSize
    );

BOOL
MacDNSMappingTable_SetParamStringValue
    (
        ANSC_HANDLE                 hInsContext,
        char*                       ParamName,
        char*                       strValue
    );

BOOL
MacDNSMappingTable_Validate
    (
        ANSC_HANDLE                 hInsContext,
        char*                       pReturnParamName,
        ULONG*                      puLength
    );

ULONG
MacDNSMappingTable_Commit
    (
        ANSC_HANDLE                 hInsContext
    );

ULONG
MacDNSMappingTable_Rollback
    (
        ANSC_HANDLE                 hInsContext
    );




#endif
