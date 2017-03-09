/*
 * If not stated otherwise in this file or this component's Licenses.txt file the
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
#include <arpa/inet.h>
#include "ansc_platform.h"
#include "cosa_xdns_apis.h"
#include "cosa_xdns_dml.h"
#include "plugin_main_apis.h"
#include "ccsp_xdnsLog_wrapper.h"


int isValidIPv4Address(char *ipAddress)
{
    struct sockaddr_in sa;
    int result = inet_pton(AF_INET, ipAddress, &(sa.sin_addr));
    return result;
}

int isValidIPv6Address(char *ipAddress)
{
    struct sockaddr_in sa;
    int result = inet_pton(AF_INET6, ipAddress, &(sa.sin_addr));
    return result;
}

BOOL isValidMacAddress
    (
        PCHAR                       pAddress
    )
{
    ULONG                           length   = 0;
    ULONG                           i        = 0;
    char                            c        = 0;

    if( pAddress == NULL)
    {
        return TRUE; /* empty string is fine */
    }

    length = AnscSizeOfString(pAddress);

    if( length == 0)
    {
        return TRUE; /* empty string is fine */
    }

    /*
     *  Mac address such as "12:BB:AA:99:34:89" is fine, and mac adress
     *  with Mask is also OK, such as "12:BB:AA:99:34:89/FF:FF:FF:FF:FF:00".
     */
    if( length != 17 && length != 35)
    {
        return FALSE;
    }

    if( length > 17 && pAddress[17] != '/')
    {
        return FALSE;
    }

    for( i = 0; i < length ; i ++)
    {
        c = pAddress[i];

        if( i % 3 == 2)
        {
            if( i != 17 && c != ':')
            {
                return FALSE;
            }
        }
        else
        {
            if ( AnscIsAlphaOrDigit(c) )
            {
                continue;
            }

            return FALSE;
        }
    }

    return TRUE;
}

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

    if( AnscEqualString(ParamName, "DefaultDeviceDnsIPv4", TRUE))
    {
        int bufsize = strlen(pMyObject->DefaultDeviceDnsIPv4);
        if (bufsize < *pUlSize)
        {
            AnscCopyString(pValue, pMyObject->DefaultDeviceDnsIPv4);
            return 0;
        }
        else
        {
            *pUlSize = bufsize + 1;
            return 1;
        }
    }

    if( AnscEqualString(ParamName, "DefaultDeviceDnsIPv6", TRUE))
    {
        int bufsize = strlen(pMyObject->DefaultDeviceDnsIPv6);
        if (bufsize < *pUlSize)
        {
            AnscCopyString(pValue, pMyObject->DefaultDeviceDnsIPv6);
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
    CcspXdnsConsoleTrace(("RDK_LOG_DEBUG, Xdns %s : ENTER \n", __FUNCTION__ ));

    if( AnscEqualString(ParamName, "DefaultDeviceDnsIPv4", TRUE))
    {
        /* save update to backup */
        pMyObject->DefaultDeviceDnsIPv4Changed = TRUE;
        AnscCopyString( pMyObject->DefaultDeviceDnsIPv4, pString );
    }
    else if( AnscEqualString(ParamName, "DefaultDeviceDnsIPv6", TRUE))
    {
        /* save update to backup */
        pMyObject->DefaultDeviceDnsIPv6Changed = TRUE;
        AnscCopyString( pMyObject->DefaultDeviceDnsIPv6, pString );
    }
    else if( AnscEqualString(ParamName, "DefaultDeviceTag", TRUE))
    {
        /* save update to backup */
        pMyObject->DefaultDeviceTagChanged = TRUE;
        AnscCopyString( pMyObject->DefaultDeviceTag, pString );

    }
    else
    {
        CcspXdnsConsoleTrace(("RDK_LOG_DEBUG, Xdns %s : EXIT FALSE \n", __FUNCTION__ ));
        return FALSE;
    }

    CcspXdnsConsoleTrace(("RDK_LOG_DEBUG, Xdns %s : EXIT TRUE \n", __FUNCTION__ ));
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
    int ret = TRUE;
    PCOSA_DATAMODEL_XDNS            pMyObject           = (PCOSA_DATAMODEL_XDNS)g_pCosaBEManager->hXdns;
    CcspXdnsConsoleTrace(("RDK_LOG_DEBUG, Xdns %s : ENTER \n", __FUNCTION__ ));

    if(pMyObject->DefaultDeviceDnsIPv4Changed)
    {
        if(!strlen(pMyObject->DefaultDeviceDnsIPv4))
        {
            CcspXdnsConsoleTrace(("RDK_LOG_DEBUG, Xdns %s : IPv4 String is Empty  RET %d \n", __FUNCTION__, ret ));
            AnscCopyString(pReturnParamName, "DnsIPv4 is empty");
            return FALSE;
        }
        else
        {
            ret = (isValidIPv4Address(pMyObject->DefaultDeviceDnsIPv4) == 1) ? TRUE : FALSE;
            CcspXdnsConsoleTrace(("RDK_LOG_DEBUG, Xdns %s :  isValidIPv4Address RET %d \n", __FUNCTION__, ret ));
        }
    }

    if(pMyObject->DefaultDeviceDnsIPv6Changed)
    {
        if(!strlen(pMyObject->DefaultDeviceDnsIPv6))
        {
            AnscCopyString(pReturnParamName, "DnsIPv6 is empty");
            return FALSE;
        }
        else
        {
            ret = (isValidIPv6Address(pMyObject->DefaultDeviceDnsIPv6) == 1) ? TRUE : FALSE;
            CcspXdnsConsoleTrace(("RDK_LOG_DEBUG, Xdns %s :  isValidIPv6Address RET %d \n", __FUNCTION__, ret ));
        }
    }

    if(pMyObject->DefaultDeviceTagChanged)
    {
        int len = strlen(pMyObject->DefaultDeviceTag);
        if(len > 255)
        {
            AnscCopyString(pReturnParamName, "Tag Exceeds length");
            return FALSE;
        }
    }

    CcspXdnsConsoleTrace(("RDK_LOG_DEBUG, Xdns %s : EXIT  RET %d \n", __FUNCTION__, ret ));
    return ret;
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
    CcspXdnsConsoleTrace(("RDK_LOG_DEBUG, Xdns %s : ENTER \n", __FUNCTION__ ));



#ifndef FEATURE_IPV6
    if(strlen(pMyObject->DefaultDeviceDnsIPv4))
    {
        char iprulebuf[256] = {0};
        snprintf(iprulebuf, 256, "from all to %s lookup erouter", pMyObject->DefaultDeviceDnsIPv4);

        if(vsystem("ip -4 rule show | grep \"%s\" | grep -v grep >/dev/null", iprulebuf) != 0)
            vsystem("ip -4 rule add %s", iprulebuf);

        snprintf(dnsoverrideEntry, 256, "dnsoverride %s %s %s\n", defaultMacAddress, pMyObject->DefaultDeviceDnsIPv4, pMyObject->DefaultDeviceTag);
        ReplaceDnsmasqConfEntry(defaultMacAddress, dnsoverrideEntry);
    }
#else
    if(strlen(pMyObject->DefaultDeviceDnsIPv4) && strlen(pMyObject->DefaultDeviceDnsIPv6))
    {

        char iprulebuf[256] = {0};
        snprintf(iprulebuf, 256, "from all to %s lookup erouter", pMyObject->DefaultDeviceDnsIPv4);

        if(vsystem("ip -4 rule show | grep \"%s\" | grep -v grep >/dev/null", iprulebuf) != 0)
            vsystem("ip -4 rule add %s", iprulebuf);

        snprintf(iprulebuf, 256, "from all to %s lookup erouter", pMyObject->DefaultDeviceDnsIPv6);

        if(vsystem("ip -6 rule show | grep \"%s\" | grep -v grep >/dev/null", iprulebuf) != 0)
            vsystem("ip -6 rule add %s", iprulebuf);

        snprintf(dnsoverrideEntry, 256, "dnsoverride %s %s %s %s\n", defaultMacAddress, pMyObject->DefaultDeviceDnsIPv4, pMyObject->DefaultDeviceDnsIPv6, pMyObject->DefaultDeviceTag);
        ReplaceDnsmasqConfEntry(defaultMacAddress, dnsoverrideEntry);
    }
#endif    
    else
    {
        CreateDnsmasqServerConf(pMyObject);
    }

    pMyObject->DefaultDeviceDnsIPv4Changed = FALSE;
    pMyObject->DefaultDeviceDnsIPv6Changed = FALSE;
    pMyObject->DefaultDeviceTagChanged = FALSE;

    CcspXdnsConsoleTrace(("RDK_LOG_DEBUG, Xdns %s : ENTER \n", __FUNCTION__ ));

	return TRUE;
}

ULONG
XDNS_Rollback
    (
        ANSC_HANDLE                 hInsContext
    )
{
    PCOSA_DATAMODEL_XDNS            pMyObject           = (PCOSA_DATAMODEL_XDNS)g_pCosaBEManager->hXdns;
    CcspXdnsConsoleTrace(("RDK_LOG_DEBUG, Xdns %s : ENTER \n", __FUNCTION__ ));

    char* token = NULL;
    const char* s = " ";
    char buf[256] = {0};
    char* defaultMacAddress = "00:00:00:00:00:00";

    GetDnsMasqFileEntry(defaultMacAddress, &buf);

    token = strtok(buf, s);
    if(!token)
    {
        return FALSE;   
    }

    token = strtok(NULL, s);
    if(!token)
    {
        return FALSE;   
    }

    token = strtok(NULL, s);
    if(token && strstr(token, "."))
    {
        strcpy(pMyObject->DefaultDeviceDnsIPv4, token);
    }
    else
    {
        return FALSE;
    }

#ifdef FEATURE_IPV6
    token = strtok(NULL, s);
    if(token && strstr(token, ":"))
    {
        strcpy(pMyObject->DefaultDeviceDnsIPv6, token);
    }
    else
    {
        return FALSE;
    }
#else
        strcpy(pMyObject->DefaultDeviceDnsIPv6, "");

#endif

    token = strtok(NULL, s);
    if(token)
        strcpy(pMyObject->DefaultDeviceTag, token);

    CcspXdnsConsoleTrace(("RDK_LOG_DEBUG, Xdns %s : ENTER \n", __FUNCTION__ ));

	return TRUE;
}

/***********************************************************************

 APIs for Object:

    X_RDKCENTRAL-COM_XDNS.DNSMappingTable.{i}.

    *  DNSMappingTable_GetEntryCount
    *  DNSMappingTable_GetEntry
    *  DNSMappingTable_IsUpdated
    *  DNSMappingTable_Synchronize
    *  DNSMappingTable_AddEntry
    *  DNSMappingTable_DelEntry
    *  DNSMappingTable_GetParamStringValue
    *  DNSMappingTable_SetParamStringValue
    *  DNSMappingTable_Validate
    *  DNSMappingTable_Commit
    *  DNSMappingTable_Rollback

***********************************************************************/

ULONG
DNSMappingTable_GetEntryCount
    (
        ANSC_HANDLE hInsContext
    )

{

    PCOSA_DATAMODEL_XDNS            pMyObject           = (PCOSA_DATAMODEL_XDNS)g_pCosaBEManager->hXdns;
    int Qdepth = AnscSListQueryDepth( &pMyObject->XDNSDeviceList );
    return Qdepth;
}

ANSC_HANDLE
DNSMappingTable_GetEntry
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
DNSMappingTable_IsUpdated
    (
        ANSC_HANDLE                 hInsContext
    )
{
    PCOSA_DATAMODEL_XDNS             SELFHEAL    = (PCOSA_DATAMODEL_XDNS)g_pCosaBEManager->hXdns;
    BOOL                            bIsUpdated   = TRUE;
    return bIsUpdated;
}

ULONG
DNSMappingTable_Synchronize
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
DNSMappingTable_AddEntry
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
DNSMappingTable_DelEntry
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

    ReplaceDnsmasqConfEntry(pDnsTableEntry->MacAddress, NULL);

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
DNSMappingTable_GetParamStringValue
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
    CcspXdnsConsoleTrace(("RDK_LOG_DEBUG, Xdns %s : ENTER \n", __FUNCTION__ ));

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

    if( AnscEqualString(ParamName, "DnsIPv4", TRUE))
    {
        if ( AnscSizeOfString(pDnsTableEntry->DnsIPv4) < *pUlSize)
        {
            AnscCopyString(pValue, pDnsTableEntry->DnsIPv4);
            return 0;
        }
        else
        {
            *pUlSize = AnscSizeOfString(pDnsTableEntry->DnsIPv4)+1;
            return 1;
        }
    }

    if( AnscEqualString(ParamName, "DnsIPv6", TRUE))
    {
        if ( AnscSizeOfString(pDnsTableEntry->DnsIPv6) < *pUlSize)
        {
            AnscCopyString(pValue, pDnsTableEntry->DnsIPv6);
            return 0;
        }
        else
        {
            *pUlSize = AnscSizeOfString(pDnsTableEntry->DnsIPv6)+1;
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

    CcspXdnsConsoleTrace(("RDK_LOG_DEBUG, Xdns %s : EXIT \n", __FUNCTION__ ));

    return -1;
}

BOOL
DNSMappingTable_SetParamStringValue
    (
        ANSC_HANDLE                 hInsContext,
        char*                       ParamName,
        char*                       strValue
    )

{
	PCOSA_CONTEXT_XDNS_LINK_OBJECT   pXdnsCxtLink     = (PCOSA_CONTEXT_XDNS_LINK_OBJECT)hInsContext;
    PCOSA_DML_XDNS_MACDNS_MAPPING_ENTRY pDnsTableEntry  = (PCOSA_DML_XDNS_MACDNS_MAPPING_ENTRY)pXdnsCxtLink->hContext;
    BOOL ret = FALSE;
    CcspXdnsConsoleTrace(("RDK_LOG_DEBUG, Xdns %s : ENTER \n", __FUNCTION__ ));
    if( AnscEqualString(ParamName, "MacAddress", TRUE))
    {
    	// if MacAddress is already present, don't update.
        if(!strlen(pDnsTableEntry->MacAddress))
        {
        	UCHAR *p = NULL;
            AnscCopyString(pDnsTableEntry->MacAddress,strValue);
            // convert MAC to lower case before writing to dml
            for (p = pDnsTableEntry->MacAddress; *p != '\0'; p++)
                *p = (char)tolower(*p);

            pDnsTableEntry->MacAddressChanged = TRUE;
            ret =  TRUE;            
        }
        else
        {
            ret =  FALSE;
        }

	}

    if( AnscEqualString(ParamName, "DnsIPv4", TRUE))
    {
        AnscCopyString(pDnsTableEntry->DnsIPv4,strValue);
        pDnsTableEntry->DnsIPv4Changed = TRUE;
        ret = TRUE;
    }

    if( AnscEqualString(ParamName, "DnsIPv6", TRUE))
    {
        AnscCopyString(pDnsTableEntry->DnsIPv6,strValue);
        pDnsTableEntry->DnsIPv6Changed = TRUE;
        ret = TRUE;
    }

    if( AnscEqualString(ParamName, "Tag", TRUE))
    {
        AnscCopyString(pDnsTableEntry->Tag,strValue);
        pDnsTableEntry->TagChanged = TRUE;        
        ret = TRUE;
    }    

    CcspXdnsConsoleTrace(("RDK_LOG_DEBUG, Xdns %s : EXIT %d \n", __FUNCTION__, ret ));

    return ret;
}

BOOL
DNSMappingTable_Validate
    (
        ANSC_HANDLE                 hInsContext,
        char*                       pReturnParamName,
        ULONG*                      puLength
    )

{
    PCOSA_CONTEXT_XDNS_LINK_OBJECT   pXdnsCxtLink     = (PCOSA_CONTEXT_XDNS_LINK_OBJECT)hInsContext;
    PCOSA_DML_XDNS_MACDNS_MAPPING_ENTRY pDnsTableEntry  = (PCOSA_DML_XDNS_MACDNS_MAPPING_ENTRY)pXdnsCxtLink->hContext;
    char* defaultMacAddress = "00:00:00:00:00:00";
    CcspXdnsConsoleTrace(("RDK_LOG_DEBUG, Xdns %s : ENTER \n", __FUNCTION__ ));

    BOOL ret = FALSE;

    if(pDnsTableEntry->MacAddressChanged)
    {
        char buf[256] = {0};
        GetDnsMasqFileEntry(pDnsTableEntry->MacAddress, &buf);        
        if(!strlen(pDnsTableEntry->MacAddress) || !strcmp(pDnsTableEntry->MacAddress, defaultMacAddress)  || strlen(buf))
        {
            AnscCopyString(pReturnParamName, "MacAddress is Invalid");
            AnscCopyString(pDnsTableEntry->MacAddress, "");
            CcspXdnsConsoleTrace(("RDK_LOG_DEBUG, Xdns %s : EXIT %d \n", __FUNCTION__, ret ));
            return FALSE;
        }
        else
        {
            ret = (isValidMacAddress(pDnsTableEntry->MacAddress) == TRUE) ? TRUE : FALSE;
        }
    }

    if(pDnsTableEntry->DnsIPv4Changed)
    {
        if(!strlen(pDnsTableEntry->DnsIPv4))
        {
            AnscCopyString(pReturnParamName, "DnsIPv4 is empty");
            return FALSE;
        }
        else
        {
            ret = (isValidIPv4Address(pDnsTableEntry->DnsIPv4) == 1) ? TRUE : FALSE;
        }
    }

    if(pDnsTableEntry->DnsIPv6Changed)
    {
        if(!strlen(pDnsTableEntry->DnsIPv6))
        {
            AnscCopyString(pReturnParamName, "DnsIPv6 is empty");
            return FALSE;
        }
        else
        {
            ret = (isValidIPv6Address(pDnsTableEntry->DnsIPv6) == 1) ? TRUE : FALSE;
        }
    }

    if(pDnsTableEntry->TagChanged)
    {
        int len = strlen(pDnsTableEntry->Tag);
        if(len > 255)
        {
            AnscCopyString(pReturnParamName, "Tag Exceeds length");
            return FALSE;
        }
        else
            ret = TRUE;
    }

    CcspXdnsConsoleTrace(("RDK_LOG_DEBUG, Xdns %s : EXIT %d \n", __FUNCTION__, ret ));

    return ret;
}

ULONG
DNSMappingTable_Commit
    (
        ANSC_HANDLE                 hInsContext
    )

{
    char dnsoverrideEntry[256] = {0};

    PCOSA_CONTEXT_XDNS_LINK_OBJECT   pXdnsCxtLink     = (PCOSA_CONTEXT_XDNS_LINK_OBJECT)hInsContext;
    PCOSA_DML_XDNS_MACDNS_MAPPING_ENTRY pDnsTableEntry  = (PCOSA_DML_XDNS_MACDNS_MAPPING_ENTRY)pXdnsCxtLink->hContext;
    CcspXdnsConsoleTrace(("RDK_LOG_DEBUG, Xdns %s : ENTER \n", __FUNCTION__ ));

    char iprulebuf[256] = {0};
    snprintf(iprulebuf, 256, "from all to %s lookup erouter", pDnsTableEntry->DnsIPv4);

    if(vsystem("ip -4 rule show | grep \"%s\" | grep -v grep >/dev/null", iprulebuf) != 0)
        vsystem("ip -4 rule add %s", iprulebuf);

#ifdef FEATURE_IPV6
    snprintf(iprulebuf, 256, "from all to %s lookup erouter", pDnsTableEntry->DnsIPv6);

    if(vsystem("ip -6 rule show | grep \"%s\" | grep -v grep >/dev/null", iprulebuf) != 0)
        vsystem("ip -6 rule add %s", iprulebuf);

    snprintf(dnsoverrideEntry, 256, "dnsoverride %s %s %s %s\n", pDnsTableEntry->MacAddress, pDnsTableEntry->DnsIPv4, pDnsTableEntry->DnsIPv6, pDnsTableEntry->Tag);
#else
    snprintf(dnsoverrideEntry, 256, "dnsoverride %s %s %s\n", pDnsTableEntry->MacAddress, pDnsTableEntry->DnsIPv4, pDnsTableEntry->Tag);
#endif
    ReplaceDnsmasqConfEntry(pDnsTableEntry->MacAddress, dnsoverrideEntry);

    pDnsTableEntry->MacAddressChanged = FALSE;
    pDnsTableEntry->DnsIPv4Changed = FALSE;
    pDnsTableEntry->DnsIPv6Changed = FALSE;
    pDnsTableEntry->TagChanged = FALSE;        

    CcspXdnsConsoleTrace(("RDK_LOG_DEBUG, Xdns %s : EXIT  \n", __FUNCTION__ ));
}

ULONG
DNSMappingTable_Rollback
    (
        ANSC_HANDLE                 hInsContext
    )

{
    PCOSA_CONTEXT_XDNS_LINK_OBJECT   pXdnsCxtLink     = (PCOSA_CONTEXT_XDNS_LINK_OBJECT)hInsContext;
    PCOSA_DML_XDNS_MACDNS_MAPPING_ENTRY pDnsTableEntry  = (PCOSA_DML_XDNS_MACDNS_MAPPING_ENTRY)pXdnsCxtLink->hContext;
    char buf[256] = {0};
    char* token = NULL;
    const char* s = " ";

    CcspXdnsConsoleTrace(("RDK_LOG_DEBUG, Xdns %s : ENTER \n", __FUNCTION__ ));

    GetDnsMasqFileEntry(pDnsTableEntry->MacAddress, &buf);

    if(!strlen(buf))
    {
        if(pDnsTableEntry->DnsIPv4Changed)
            strcpy(pDnsTableEntry->DnsIPv4, buf);
        if(pDnsTableEntry->DnsIPv6Changed)
            strcpy(pDnsTableEntry->DnsIPv6, buf);
        if(pDnsTableEntry->TagChanged)
            strcpy(pDnsTableEntry->Tag, buf);
    }
    else
    {
        token = strtok(buf, s);
        if(!token)
        {
            return FALSE;   
        }

        token = strtok(NULL, s);
        if(!token)
        {
            return FALSE;   
        }

        token = strtok(NULL, s);
        if(token && strstr(token, "."))
        {
            if(pDnsTableEntry->DnsIPv4Changed)
                strcpy(pDnsTableEntry->DnsIPv4, token);
        }
        else
        {
            return FALSE;
        }

#ifdef FEATURE_IPV6
        token = strtok(NULL, s);
        if(token && strstr(token, ":"))
        {
            strcpy(pDnsTableEntry->DnsIPv6, token);
        }
        else
        {
            return FALSE;
        }
#else
        if(pDnsTableEntry->DnsIPv6Changed)
            strcpy(pDnsTableEntry->DnsIPv6, "");

#endif

        token = strtok(NULL, s);
        if(token)
            {
                if(pDnsTableEntry->TagChanged)
                    strcpy(pDnsTableEntry->Tag, token);
            }
    }


    pDnsTableEntry->MacAddressChanged = FALSE;
    pDnsTableEntry->DnsIPv4Changed = FALSE;
    pDnsTableEntry->DnsIPv6Changed = FALSE;
    pDnsTableEntry->TagChanged = FALSE;      

    CcspXdnsConsoleTrace(("RDK_LOG_DEBUG, Xdns %s : EXIT \n", __FUNCTION__ ));

}


