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


/**************************************************************************

    module: cosa_diagnostic_apis.c

        For COSA Data Model Library Development

    -------------------------------------------------------------------

    copyright:

        Cisco Systems, Inc.
        All Rights Reserved.

    -------------------------------------------------------------------

    description:

        This file implementes back-end apis for the COSA Data Model Library

        *  CosaDiagCreate
        *  CosaDiagInitialize
        *  CosaDiagRemove
    -------------------------------------------------------------------

    environment:

        platform independent

    -------------------------------------------------------------------

    author:

        COSA XML TOOL CODE GENERATOR 1.0

    -------------------------------------------------------------------

    revision:

        01/11/2011    initial revision.

**************************************************************************/
#include "plugin_main_apis.h"
#include "cosa_xdns_apis.h"


void GetDnsMasqFileEntry(char* macaddress, char* defaultEntry)
{
    FILE *fp1;
    char dnsmasqConfEntry[256] = {0};

    if(!macaddress || !strlen(macaddress))
    {
        return;
    }


    fp1 = fopen(DNSMASQ_SERVERS_CONF,"r");

    if(fp1 == NULL)
    {
        printf("\nError reading file\n");
        return;
    }
    //Step 2: Get text from original file//
    while(fgets(dnsmasqConfEntry, sizeof(dnsmasqConfEntry), fp1) !=NULL)
    {
        if(strstr(dnsmasqConfEntry, macaddress) != NULL)
            {
                strcpy(defaultEntry, dnsmasqConfEntry);
                break;
            }

    }

    fclose(fp1);
}

void ReplaceDnsmasqConfEntry(char* macaddress, char* overrideEntry)
{
    char dnsmasqConfEntry[256] = {0};


    if(!macaddress || !strlen(macaddress))
    {
        return;
    }

    unlink(DNSMASQ_SERVERS_BAK);
    //Step 1: Open text files and check that they open//
    FILE *fp1, *fp2;
    fp1 = fopen(DNSMASQ_SERVERS_CONF,"r");
    fp2 = fopen(DNSMASQ_SERVERS_BAK ,"a");

    if(fp1 == NULL || fp2 == NULL)
    {
        printf("\nError reading file\n");
        return;
    }
    //Step 2: Get text from original file//
    while(fgets(dnsmasqConfEntry, sizeof(dnsmasqConfEntry), fp1) !=NULL)
    {
        if(strstr(dnsmasqConfEntry, macaddress) != NULL)
            {
                continue;
            }

        fprintf(fp2, "%s", dnsmasqConfEntry);
    }

    if(overrideEntry)
        fprintf(fp2, "%s", overrideEntry);

    fclose(fp1);
    unlink(DNSMASQ_SERVERS_CONF);
    fclose(fp2);
    rename(DNSMASQ_SERVERS_BAK, DNSMASQ_SERVERS_CONF);
}

void AppendDnsmasqConfEntry(char* string1)
{
    FILE *fp2;
    fp2 = fopen(DNSMASQ_SERVERS_CONF ,"a");

    if(fp2 == NULL)
    {
        printf("\nError reading file\n");
        return;
    }

    fprintf(fp2, "%s", string1);
    fclose(fp2);
}


void CreateDnsmasqServerConf(PCOSA_DATAMODEL_XDNS pMyObject)
{
    char resolvConfEntry[256] = {0};
    char buf[256] = {0};
    char resolvConfFirstIPv4Nameserver[256] = {0};
    char resolvConfFirstIPv6Nameserver[256] = {0};
    char dnsmasqConfOverrideEntry[256] = {0};
    char tokenIPv4[256] = {0};
    char tokenIPv6[256] = {0};
    char* token = NULL;;
    const char* s = " ";
    int foundIPv4 = 0;
    int foundIPv6 = 0;

    //Step 1: Open text files and check that they open//
    FILE *fp1, *fp2;
    fp1 = fopen(RESOLV_CONF,"r");
    fp2 = fopen(DNSMASQ_SERVERS_CONF ,"w");



    if(fp1 == NULL || fp2 == NULL)
    {
        printf("\nError reading file\n");
        return;
    }
    //Step 2: Get text from original file//
    while(fgets(resolvConfEntry, sizeof(resolvConfEntry), fp1) !=NULL)
    {
    	if(strstr(resolvConfEntry, "nameserver") != NULL)
    		{
                strcpy(buf, resolvConfEntry);

                char *newline = strchr( buf, '\n' );
                if ( newline )
                    *newline = 0;

                token = strtok(buf, s);
                if(!token)
                {
                    continue;   
                }                 

                token = strtok(NULL, s);
                if(!token)
                {
                    continue;   
                }                 

                if(!foundIPv4 && strstr(token, "."))
                {
                    foundIPv4 = 1;
                    strcpy(tokenIPv4, token);
                }
                
                if(!foundIPv6 && strstr(token, ":")) 
                {
                    foundIPv6 = 1;
                    strcpy(tokenIPv6, token);
                }
    		}

        fprintf(fp2, "%s", resolvConfEntry);
    }

    fclose(fp1);
	fclose(fp2);
	/* get the first token */

    strcpy(pMyObject->DefaultDeviceDnsIPv4, tokenIPv4);
    strcpy(pMyObject->DefaultDeviceDnsIPv6, tokenIPv6);
    strcpy(pMyObject->DefaultDeviceTag, "empty");

#ifndef FEATURE_IPV6
	snprintf(dnsmasqConfOverrideEntry, 256, "dnsoverride 00:00:00:00:00:00 %s %s\n", tokenIPv4, pMyObject->DefaultDeviceTag);
	AppendDnsmasqConfEntry(dnsmasqConfOverrideEntry);
#else
    snprintf(dnsmasqConfOverrideEntry, 256, "dnsoverride 00:00:00:00:00:00 %s %s %s\n", tokenIPv4, tokenIPv6, pMyObject->DefaultDeviceTag);
    AppendDnsmasqConfEntry(dnsmasqConfOverrideEntry);
#endif
}

void FillEntryInList(PCOSA_DATAMODEL_XDNS pXdns, PCOSA_DML_XDNS_MACDNS_MAPPING_ENTRY dnsTableEntry)
{
	PCOSA_CONTEXT_XDNS_LINK_OBJECT   pXdnsCxtLink = NULL;

    pXdnsCxtLink = (PCOSA_CONTEXT_XDNS_LINK_OBJECT)AnscAllocateMemory(sizeof(COSA_CONTEXT_XDNS_LINK_OBJECT));
    if ( !pXdnsCxtLink )
    {
        fprintf(stderr, "Allocation failed \n");
        return;
    }

	pXdnsCxtLink->InstanceNumber =  pXdns->ulXDNSNextInstanceNumber;
    dnsTableEntry->InstanceNumber =  pXdns->ulXDNSNextInstanceNumber;
	pXdns->ulXDNSNextInstanceNumber++;

	pXdnsCxtLink->hContext = (ANSC_HANDLE)dnsTableEntry;
	CosaSListPushEntryByInsNum(&pXdns->XDNSDeviceList, (PCOSA_CONTEXT_LINK_OBJECT)pXdnsCxtLink); 
}


PCOSA_DML_MAPPING_CONTAINER
CosaDmlGetSelfHealCfg(    
        ANSC_HANDLE                 hThisObject
    )
{
	PCOSA_DATAMODEL_XDNS      pMyObject            = (PCOSA_DATAMODEL_XDNS)hThisObject;
	PCOSA_DML_MAPPING_CONTAINER    pMappingContainer            = (PCOSA_DML_MAPPING_CONTAINER)NULL;
    char buf[256] = {0};
	char stub[64];
	FILE* fp_dnsmasq_conf = NULL;
	int ret = 0;
	int index = 0;
    PCOSA_DML_XDNS_MACDNS_MAPPING_ENTRY pDnsTableEntry = NULL;

	pMappingContainer = (PCOSA_DML_MAPPING_CONTAINER)AnscAllocateMemory(sizeof(COSA_DML_MAPPING_CONTAINER));

    if ( (fp_dnsmasq_conf=fopen(DNSMASQ_SERVERS_CONF, "r")) == NULL )
    {
    	pMappingContainer->XDNSEntryCount = 0;
    	CreateDnsmasqServerConf(pMyObject);
        return pMappingContainer;
    }

    while ( fgets(buf, sizeof(buf), fp_dnsmasq_conf)!= NULL )
    {
        char *newline = strchr( buf, '\n' );
        if ( newline )
            *newline = 0;

        if ( !strstr(buf, "dnsoverride"))
        {
            continue;
        }


        if (strstr(buf, "00:00:00:00:00:00"))
        {
            char* token = NULL;
            const char* s = " ";
            token = strtok(buf, s);
            if(!token)
            {
                continue;   
            }   
            
            token = strtok(NULL, s);
            if(!token)
            {
                continue;   
            } 

            token = strtok(NULL, s);
            if(token && strstr(token, "."))
            {
                strcpy(pMyObject->DefaultDeviceDnsIPv4, token);
            }
            else
            {
                continue;
            }

#ifdef FEATURE_IPV6
            token = strtok(NULL, s);
            if(token && strstr(token, ":"))
            {
                strcpy(pMyObject->DefaultDeviceDnsIPv6, token);
            }
            else
            {
                continue;
            }
#endif

            token = strtok(NULL, s);
            if(token)
            {
                strcpy(pMyObject->DefaultDeviceTag, token);
            }

            continue;
        }

        pDnsTableEntry = (PCOSA_DML_XDNS_MACDNS_MAPPING_ENTRY)AnscAllocateMemory(sizeof(COSA_DML_XDNS_MACDNS_MAPPING_ENTRY));
        if ( !pDnsTableEntry )
        {
            CcspTraceWarning(("%s resource allocation failed\n",__FUNCTION__));
            fclose(fp_dnsmasq_conf);
            unlink(DNSMASQ_SERVERS_CONF);
            break;
        }

        /*
        Sample:
        dnsoverride AA:BB:CC:DD:EE:FF 1.2.3.4 ArbitrarySting
        */

        ret = sscanf(buf, XDNS_ENTRY_FORMAT,
                 stub,
                 pDnsTableEntry->MacAddress,
                 pDnsTableEntry->DnsIPv4,
#ifdef FEATURE_IPV6
                 pDnsTableEntry->DnsIPv6,
#endif                 
                 pDnsTableEntry->Tag);

/*        fprintf(stderr, "%s:%s %s %s\n", __FUNCTION__,
                pDnsTableEntry->MacAddress,
                pDnsTableEntry->DnsIPv4,
                pDnsTableEntry->Tag);
*/


        if(ret != 4)
        {
            free(pDnsTableEntry);
            continue;
        }

        index++;		


		FillEntryInList(pMyObject, pDnsTableEntry);
    }

    pMappingContainer->XDNSEntryCount = index;

    //fprintf(stderr, "index %d\n", index);

    fclose(fp_dnsmasq_conf);

	return pMappingContainer;
}

/**********************************************************************

    caller:     owner of the object

    prototype:

        ANSC_HANDLE
        CosaDiagCreate
            (
                VOID
            );

    description:

        This function constructs cosa SelfHeal object and return handle.

    argument:

    return:     newly created nat object.

**********************************************************************/

ANSC_HANDLE
CosaXDNSCreate
    (
        VOID
    )
{
    ANSC_STATUS                     returnStatus = ANSC_STATUS_SUCCESS;
    PCOSA_DATAMODEL_XDNS            pMyObject    = (PCOSA_DATAMODEL_XDNS)NULL;

    /*
     * We create object by first allocating memory for holding the variables and member functions.
     */
    pMyObject = (PCOSA_DATAMODEL_XDNS)AnscAllocateMemory(sizeof(COSA_DATAMODEL_XDNS));
    if ( !pMyObject )
    {
        return  (ANSC_HANDLE)NULL;
    }

    /*
     * Initialize the common variables and functions for a container object.
     */
    pMyObject->Oid               = COSA_DATAMODEL_XDNS_OID;
    pMyObject->Create            = CosaXDNSCreate;
    pMyObject->Remove            = CosaXDNSRemove;
    pMyObject->Initialize        = CosaXDNSInitialize;

    pMyObject->Initialize   ((ANSC_HANDLE)pMyObject);

    return  (ANSC_HANDLE)pMyObject;
}

/**********************************************************************

    caller:     self

    prototype:

        ANSC_STATUS
        CosaXDNSInitialize
            (
                ANSC_HANDLE                 hThisObject
            );

    description:

        This function initiate  cosa SelfHeal object and return handle.

    argument:   ANSC_HANDLE                 hThisObject
                This handle is actually the pointer of this object
                itself.

    return:     operation status.

**********************************************************************/

ANSC_STATUS
CosaXDNSInitialize
    (
        ANSC_HANDLE                 hThisObject
    )
{
    ANSC_STATUS                     returnStatus         = ANSC_STATUS_SUCCESS;
    PCOSA_DATAMODEL_XDNS            pMyObject            = (PCOSA_DATAMODEL_XDNS )hThisObject;
	PCOSA_DML_MAPPING_CONTAINER    pMappingContainer            = (PCOSA_DML_MAPPING_CONTAINER)NULL;
	
    /* Initiation all functions */
    AnscSListInitializeHeader( &pMyObject->XDNSDeviceList );
    pMyObject->MaxInstanceNumber        = 0;
    pMyObject->ulXDNSNextInstanceNumber   = 1;
	
    pMyObject->pMappingContainer = CosaDmlGetSelfHealCfg((ANSC_HANDLE)pMyObject);

    return returnStatus;
}

/**********************************************************************

    caller:     self

    prototype:

        ANSC_STATUS
        CosaDiagRemove
            (
                ANSC_HANDLE                 hThisObject
            );

    description:

        This function initiate  cosa SelfHeal object and return handle.

    argument:   ANSC_HANDLE                 hThisObject
                This handle is actually the pointer of this object
                itself.

    return:     operation status.

**********************************************************************/

ANSC_STATUS
CosaXDNSRemove
    (
        ANSC_HANDLE                 hThisObject
    )
{
    ANSC_STATUS                     returnStatus = ANSC_STATUS_SUCCESS;
    PCOSA_DATAMODEL_XDNS            pMyObject    = (PCOSA_DATAMODEL_XDNS)hThisObject;

    /*RDKB-7457, CID-33295, null check before free */
    if ( pMyObject->pMappingContainer)
    {
        /* Remove necessary resounce */
        if ( pMyObject->pMappingContainer->pXDNSTable)
        {
            AnscFreeMemory(pMyObject->pMappingContainer->pXDNSTable );
        }

        AnscFreeMemory(pMyObject->pMappingContainer);
        pMyObject->pMappingContainer = NULL;
    }

    /* Remove self */
    AnscFreeMemory((ANSC_HANDLE)pMyObject);
    return returnStatus;
}
