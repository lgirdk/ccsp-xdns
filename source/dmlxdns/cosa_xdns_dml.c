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

#include "ansc_platform.h"
#include "cosa_xdns_apis.h"
#include "cosa_xdns_dml.h"
#include "plugin_main_apis.h"
//#include "ccsp_xdnsLog_wrapper.h"


/***********************************************************************


/***********************************************************************

 APIs for Object:

    Device.X_RDKCENTRAL-COM_XDNS.

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
    )
{
    PCOSA_DATAMODEL_XDNS            pMyObject           = (PCOSA_DATAMODEL_XDNS)g_pCosaBEManager->hXdns;

    if( AnscEqualString(ParamName, "DefaultDeviceDnsIp", TRUE))
    {
        int bufsize = strlen(pMyObject->DefaultDeviceDnsIp);
        if (bufsize < *pUlSize)
        {
            AnscCopyString(pValue, pMyObject->DefaultDeviceDnsIp);
            return 0;
        }
        else
        {
            *pUlSize = bufsize + 1;
            return 1;
        }
    }

    if( AnscEqualString(ParamName, "DefaultDeviceTag", TRUE))
    {
        int bufsize = strlen(pMyObject->DefaultDeviceTag);
        if (bufsize < *pUlSize)
        {
            AnscCopyString(pValue, pMyObject->DefaultDeviceTag);
            return 0;
        }
        else
        {
            *pUlSize = bufsize + 1;
            return 1;
        }
        
    }

    AnscTraceWarning(("Unsupported parameter '%s'\n", ParamName));
    return -1;
}


/**********************************************************************

    caller:     owner of this object

    prototype:

        BOOL
        XDNS_SetParamStringValue
            (
                ANSC_HANDLE                 hInsContext,
                char*                       ParamName,
                char*                       pString
            );

    description:

        This function is called to set string parameter value;

    argument:   ANSC_HANDLE                 hInsContext,
                The instance handle;

                char*                       ParamName,
                The parameter name;

                char*                       pString
                The updated string value;

    return:     TRUE if succeeded.

**********************************************************************/
BOOL
XDNS_SetParamStringValue
    (
        ANSC_HANDLE                 hInsContext,
        char*                       ParamName,
        char*                       pString
    )
{
    PCOSA_DATAMODEL_XDNS            pMyObject           = (PCOSA_DATAMODEL_XDNS)g_pCosaBEManager->hXdns;

    if( AnscEqualString(ParamName, "DefaultDeviceDnsIp", TRUE))
    {
        /* save update to backup */
        AnscCopyString( pMyObject->DefaultDeviceDnsIp, pString );
    }
    else if( AnscEqualString(ParamName, "DefaultDeviceTag", TRUE))
    {
        /* save update to backup */
        AnscCopyString( pMyObject->DefaultDeviceTag, pString );

    }
    else
        return FALSE;

    return TRUE;
}


BOOL
XDNS_Validate
    (
        ANSC_HANDLE                 hInsContext,
        char*                       pReturnParamName,
        ULONG*                      puLength
    )
{

    PCOSA_DATAMODEL_XDNS            pMyObject           = (PCOSA_DATAMODEL_XDNS)g_pCosaBEManager->hXdns;

    if(!strlen(pMyObject->DefaultDeviceDnsIp))
        {
            AnscCopyString(pReturnParamName, "DnsIp is empty");
            return FALSE;
        }

    return TRUE;
}

ULONG
XDNS_Commit
    (
        ANSC_HANDLE                 hInsContext
    )
{
    char dnsoverrideEntry[256] = {0};
    char* defaultMacAddress = "00:00:00:00:00:00";
    PCOSA_DATAMODEL_XDNS            pMyObject           = (PCOSA_DATAMODEL_XDNS)g_pCosaBEManager->hXdns;

    snprintf(dnsoverrideEntry, 256, "dnsoverride %s %s %s\n", defaultMacAddress, pMyObject->DefaultDeviceDnsIp, pMyObject->DefaultDeviceTag);

    ReplaceDnsmasqConfEntry(defaultMacAddress, dnsoverrideEntry);
	return TRUE;
}

ULONG
XDNS_Rollback
    (
        ANSC_HANDLE                 hInsContext
    )
{
	return TRUE;
}

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
        ANSC_HANDLE hInsContext
    )

{

    PCOSA_DATAMODEL_XDNS            pMyObject           = (PCOSA_DATAMODEL_XDNS)g_pCosaBEManager->hXdns;
    int Qdepth = AnscSListQueryDepth( &pMyObject->XDNSDeviceList );
    return Qdepth;
}

ANSC_HANDLE
MacDNSMappingTable_GetEntry
    (
        ANSC_HANDLE                 hInsContext,
        ULONG                       nIndex,
        ULONG*                      pInsNumber
    )
{

    PCOSA_DATAMODEL_XDNS                   pMyObject         = (PCOSA_DATAMODEL_XDNS)g_pCosaBEManager->hXdns;
    PSINGLE_LINK_ENTRY                    pSListEntry       = NULL;
    PCOSA_CONTEXT_XDNS_LINK_OBJECT    pCxtLink          = NULL;

    pSListEntry       = AnscSListGetEntryByIndex(&pMyObject->XDNSDeviceList, nIndex);
    if ( pSListEntry )
    {
        pCxtLink      = ACCESS_COSA_CONTEXT_XDNS_LINK_OBJECT(pSListEntry);
        *pInsNumber   = pCxtLink->InstanceNumber;
    }

    return (ANSC_HANDLE)pSListEntry;
}

BOOL
MacDNSMappingTable_IsUpdated
    (
        ANSC_HANDLE                 hInsContext
    )
{
    PCOSA_DATAMODEL_XDNS             SELFHEAL    = (PCOSA_DATAMODEL_XDNS)g_pCosaBEManager->hXdns;
    BOOL                            bIsUpdated   = TRUE;
    return bIsUpdated;
}

ULONG
MacDNSMappingTable_Synchronize
    (
        ANSC_HANDLE                 hInsContext
    )
{

    ANSC_STATUS                           returnStatus      = ANSC_STATUS_FAILURE;
    PCOSA_DATAMODEL_XDNS             SELFHEAL    = (PCOSA_DATAMODEL_XDNS)g_pCosaBEManager->hXdns;
    PCOSA_CONTEXT_XDNS_LINK_OBJECT    pCxtLink          = NULL;
    PSINGLE_LINK_ENTRY                    pSListEntry       = NULL;
    PSINGLE_LINK_ENTRY                    pSListEntry2      = NULL;
    ULONG                                 entryCount        = 0;
}

ANSC_HANDLE
MacDNSMappingTable_AddEntry
    (
        ANSC_HANDLE                 hInsContext,
        ULONG*                      pInsNumber
    )
{

	PCOSA_DATAMODEL_XDNS             pXdns              = (PCOSA_DATAMODEL_XDNS)g_pCosaBEManager->hXdns;
    PCOSA_DML_XDNS_MACDNS_MAPPING_ENTRY pDnsTableEntry = NULL;
    PCOSA_CONTEXT_XDNS_LINK_OBJECT   pXdnsCxtLink  = NULL;
    //CcspXdnsConsoleTrace(("RDK_LOG_DEBUG, Xdns %s : ENTER \n", __FUNCTION__ ));

    pDnsTableEntry = (PCOSA_DML_XDNS_MACDNS_MAPPING_ENTRY)AnscAllocateMemory(sizeof(COSA_DML_XDNS_MACDNS_MAPPING_ENTRY));
    if ( !pDnsTableEntry )
    {
		CcspTraceWarning(("%s resource allocation failed\n",__FUNCTION__));
        return NULL;
    }
 
    pXdnsCxtLink = (PCOSA_CONTEXT_XDNS_LINK_OBJECT)AnscAllocateMemory(sizeof(COSA_CONTEXT_XDNS_LINK_OBJECT));

    if ( !pXdnsCxtLink )
    {
        goto EXIT;
    }        
	
	pXdnsCxtLink->InstanceNumber =  pXdns->ulXDNSNextInstanceNumber;
	pDnsTableEntry->InstanceNumber = pXdns->ulXDNSNextInstanceNumber;
    pXdns->ulXDNSNextInstanceNumber++;

    /* now we have this link content */
	pXdnsCxtLink->hContext = (ANSC_HANDLE)pDnsTableEntry;
	pXdns->pMappingContainer->XDNSEntryCount++;
    *pInsNumber = pXdnsCxtLink->InstanceNumber;

	CosaSListPushEntryByInsNum(&pXdns->XDNSDeviceList, (PCOSA_CONTEXT_LINK_OBJECT)pXdnsCxtLink);

    return (ANSC_HANDLE)pXdnsCxtLink;

EXIT:
    AnscFreeMemory(pDnsTableEntry);

    return NULL;

}

ULONG
MacDNSMappingTable_DelEntry
    (
        ANSC_HANDLE                 hInsContext,
        ANSC_HANDLE                 hInstance
    )

{
    ANSC_STATUS                          returnStatus      = ANSC_STATUS_SUCCESS;
    PCOSA_DATAMODEL_XDNS             pXdns               = (PCOSA_DATAMODEL_XDNS)g_pCosaBEManager->hXdns;
    PCOSA_CONTEXT_XDNS_LINK_OBJECT   pXdnsCxtLink   = (PCOSA_CONTEXT_XDNS_LINK_OBJECT)hInstance;
    PCOSA_DML_XDNS_MACDNS_MAPPING_ENTRY pDnsTableEntry      = (PCOSA_DML_XDNS_MACDNS_MAPPING_ENTRY)pXdnsCxtLink->hContext;
	/* Remove entery from the database */

    if ( returnStatus == ANSC_STATUS_SUCCESS )
	{
			/* Remove entery from the Queue */
        if(AnscSListPopEntryByLink(&pXdns->XDNSDeviceList, &pXdnsCxtLink->Linkage) == TRUE)
		{
			AnscFreeMemory(pXdnsCxtLink->hContext);
			AnscFreeMemory(pXdnsCxtLink);
		}
		else
		{
			return ANSC_STATUS_FAILURE;
		}
	}


    //CcspXdnsConsoleTrace(("RDK_LOG_DEBUG, Xdns %s : EXIT \n", __FUNCTION__ ));


    return returnStatus;
}

ULONG
MacDNSMappingTable_GetParamStringValue
    (
        ANSC_HANDLE                 hInsContext,
        char*                       ParamName,
        char*                       pValue,
        ULONG*                      pUlSize
    )

{
    PCOSA_CONTEXT_XDNS_LINK_OBJECT   pXdnsCxtLink     = (PCOSA_CONTEXT_XDNS_LINK_OBJECT)hInsContext;
    PCOSA_DML_XDNS_MACDNS_MAPPING_ENTRY pDnsTableEntry  = (PCOSA_DML_XDNS_MACDNS_MAPPING_ENTRY)pXdnsCxtLink->hContext;
    PUCHAR                                    pString       = NULL;

    /* check the parameter name and return the corresponding value */
    if( AnscEqualString(ParamName, "MacAddress", TRUE))
    {
        if ( AnscSizeOfString(pDnsTableEntry->MacAddress) < *pUlSize)
        {
            AnscCopyString(pValue, pDnsTableEntry->MacAddress);
            return 0;
        }
        else
        {
            *pUlSize = AnscSizeOfString(pDnsTableEntry->MacAddress)+1;
            return 1;
        }
    }

    if( AnscEqualString(ParamName, "DnsIp", TRUE))
    {
        if ( AnscSizeOfString(pDnsTableEntry->DnsIp) < *pUlSize)
        {
            AnscCopyString(pValue, pDnsTableEntry->DnsIp);
            return 0;
        }
        else
        {
            *pUlSize = AnscSizeOfString(pDnsTableEntry->DnsIp)+1;
            return 1;
        }
    }

    if( AnscEqualString(ParamName, "Tag", TRUE))
    {
        if ( AnscSizeOfString(pDnsTableEntry->Tag) < *pUlSize)
        {
            AnscCopyString(pValue, pDnsTableEntry->Tag);
            return 0;
        }
        else
        {
            *pUlSize = AnscSizeOfString(pDnsTableEntry->Tag)+1;
            return 1;
        }
    }

    return -1;
}

BOOL
MacDNSMappingTable_SetParamStringValue
    (
        ANSC_HANDLE                 hInsContext,
        char*                       ParamName,
        char*                       strValue
    )

{
	PCOSA_CONTEXT_XDNS_LINK_OBJECT   pXdnsCxtLink     = (PCOSA_CONTEXT_XDNS_LINK_OBJECT)hInsContext;
    PCOSA_DML_XDNS_MACDNS_MAPPING_ENTRY pDnsTableEntry  = (PCOSA_DML_XDNS_MACDNS_MAPPING_ENTRY)pXdnsCxtLink->hContext;
	
    if( AnscEqualString(ParamName, "MacAddress", TRUE))
    {
		 AnscCopyString(pDnsTableEntry->MacAddress,strValue);
		 return TRUE;
	}

    if( AnscEqualString(ParamName, "DnsIp", TRUE))
    {
         AnscCopyString(pDnsTableEntry->DnsIp,strValue);
         return TRUE;
    }

    if( AnscEqualString(ParamName, "Tag", TRUE))
    {
         AnscCopyString(pDnsTableEntry->Tag,strValue);
         return TRUE;
    }    

	return FALSE;
}

BOOL
MacDNSMappingTable_Validate
    (
        ANSC_HANDLE                 hInsContext,
        char*                       pReturnParamName,
        ULONG*                      puLength
    )

{
    PCOSA_CONTEXT_XDNS_LINK_OBJECT   pXdnsCxtLink     = (PCOSA_CONTEXT_XDNS_LINK_OBJECT)hInsContext;
    PCOSA_DML_XDNS_MACDNS_MAPPING_ENTRY pDnsTableEntry  = (PCOSA_DML_XDNS_MACDNS_MAPPING_ENTRY)pXdnsCxtLink->hContext;


    if(!strlen(pDnsTableEntry->MacAddress))
        {
            AnscCopyString(pReturnParamName, "MacAddress is empty");
            return FALSE;
        }
    
    if(!strlen(pDnsTableEntry->DnsIp))
        {
            AnscCopyString(pReturnParamName, "DnsIp is empty");
            return FALSE;
        }

    return TRUE;
}

ULONG
MacDNSMappingTable_Commit
    (
        ANSC_HANDLE                 hInsContext
    )

{
    char dnsoverrideEntry[256] = {0};

    PCOSA_CONTEXT_XDNS_LINK_OBJECT   pXdnsCxtLink     = (PCOSA_CONTEXT_XDNS_LINK_OBJECT)hInsContext;
    PCOSA_DML_XDNS_MACDNS_MAPPING_ENTRY pDnsTableEntry  = (PCOSA_DML_XDNS_MACDNS_MAPPING_ENTRY)pXdnsCxtLink->hContext;

    snprintf(dnsoverrideEntry, 256, "dnsoverride %s %s %s\n", pDnsTableEntry->MacAddress, pDnsTableEntry->DnsIp, pDnsTableEntry->Tag);

    ReplaceDnsmasqConfEntry(pDnsTableEntry->MacAddress, dnsoverrideEntry);
}

ULONG
MacDNSMappingTable_Rollback
    (
        ANSC_HANDLE                 hInsContext
    )

{

}


