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


void ResetDnsmasqConfFile()
{
	unlink(DNSMASQ_SERVERS_CONF);

    char resolvConfEntry[256] = {0};
    FILE *fp1, *fp2;
    fp1 = fopen(RESOLV_CONF,"r");
    fp2 = fopen(DNSMASQ_SERVERS_CONF ,"w");

    if(fp1 == NULL || fp2 == NULL)
    {
        printf("\nError reading file\n");
        return;
    }
    while(fgets(resolvConfEntry, sizeof(resolvConfEntry), fp1) !=NULL)
    {
        fprintf(fp2, "%s", resolvConfEntry);
    }

    fclose(fp1);
	fclose(fp2);
}

void ReplaceDnsmasqConfEntry(char* macaddress, char* overrideEntry)
{
    char dnsmasqConfEntry[256] = {0};

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
    char resolvConfFirstNameserver[256] = {0};
    char dnsmasqConfOverrideEntry[256] = {0};
    char* token;
    const char* s = " ";
    int found = 0;

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
    	if(found != 1 && strstr(resolvConfEntry, "nameserver") != NULL)
    		{
    			strcpy(resolvConfFirstNameserver, resolvConfEntry);
    			found = 1;
    		}

        fprintf(fp2, "%s", resolvConfEntry);
    }

    fclose(fp1);
	fclose(fp2);
	/* get the first token */
	token = strtok(resolvConfFirstNameserver, s);
	token = strtok(NULL, s);


    strcpy(pMyObject->DefaultDeviceDnsIp, token);

	snprintf(dnsmasqConfOverrideEntry, 256, "dnsoverride 00:00:00:00:00:00 %s", token);

	AppendDnsmasqConfEntry(dnsmasqConfOverrideEntry);

}

void FillEntryInList(PCOSA_DATAMODEL_XDNS pXdns, PCOSA_DML_MAPPING_CONTAINER pMappingContainer)
{
	PCOSA_DML_XDNS_MACDNS_MAPPING_ENTRY pServerIpv4 = NULL;
	PCOSA_CONTEXT_XDNS_LINK_OBJECT   pXdnsCxtLink = NULL;

	int Qdepth = 0;
    pXdnsCxtLink = (PCOSA_CONTEXT_XDNS_LINK_OBJECT)AnscAllocateMemory(sizeof(COSA_CONTEXT_XDNS_LINK_OBJECT));
    if ( !pXdnsCxtLink )
    {
        return;
    }

	Qdepth = AnscSListQueryDepth( &pXdns->XDNSDeviceList );

	pXdnsCxtLink->InstanceNumber =  pXdns->ulXDNSNextInstanceNumber;
	pMappingContainer->pXDNSTable[Qdepth].InstanceNumber =  pXdns->ulXDNSNextInstanceNumber;
	pXdns->ulXDNSNextInstanceNumber++;


	pServerIpv4 = &pMappingContainer->pXDNSTable[Qdepth];
	pXdnsCxtLink->hContext = (ANSC_HANDLE)pServerIpv4;
	CosaSListPushEntryByInsNum(&pXdns->XDNSDeviceList, (PCOSA_CONTEXT_LINK_OBJECT)pXdnsCxtLink); 
}


PCOSA_DML_MAPPING_CONTAINER
CosaDmlGetSelfHealCfg(    
        ANSC_HANDLE                 hThisObject
    )
{
	PCOSA_DATAMODEL_XDNS      pMyObject            = (PCOSA_DATAMODEL_XDNS)hThisObject;
	PCOSA_DML_MAPPING_CONTAINER    pMappingContainer            = (PCOSA_DML_MAPPING_CONTAINER)NULL;
	PCOSA_DML_XDNS_MACDNS_MAPPING_ENTRY macdnsentry = NULL; 
    char buf[256] = {0};
	char stub[64];
	FILE* fp_dnsmasq_conf = NULL;
	int ret = 0;
	int index = 0;

	pMappingContainer = (PCOSA_DML_MAPPING_CONTAINER)AnscAllocateMemory(sizeof(COSA_DML_MAPPING_CONTAINER));

    if ( (fp_dnsmasq_conf=fopen(DNSMASQ_SERVERS_CONF, "r")) == NULL )
    {
    	pMappingContainer->XDNSEntryCount = 0;
    	CreateDnsmasqServerConf(pMyObject);
        return pMappingContainer;
    }

    while ( fgets(buf, sizeof(buf), fp_dnsmasq_conf)!= NULL )
    {
        if ( !strstr(buf, "dnsoverride"))
        {
            continue;
        }

        if (strstr(buf, "00:00:00:00:00:00"))
        {
            char* token;
            const char* s = " ";
            token = strtok(buf, s);
            token = strtok(NULL, s);

            strcpy(pMyObject->DefaultDeviceDnsIp, token);

            token = strtok(NULL, s);
            if(token)
                strcpy(pMyObject->DefaultDeviceTag, token);

            continue;
        }


        pMappingContainer->pXDNSTable = (COSA_DML_XDNS_MACDNS_MAPPING_ENTRY *) realloc(pMappingContainer->pXDNSTable, (sizeof(COSA_DML_XDNS_MACDNS_MAPPING_ENTRY) * (index+1)));

        if ( pMappingContainer->pXDNSTable == NULL )
        {
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
                 pMappingContainer->pXDNSTable[index].MacAddress,
                 pMappingContainer->pXDNSTable[index].DnsIp,
                 pMappingContainer->pXDNSTable[index].Tag);


/*        printf("%s:%s %s %s\n", __FUNCTION__,
                pMappingContainer->pXDNSTable[index].MacAddress,
                pMappingContainer->pXDNSTable[index].DnsIp,
                pMappingContainer->pXDNSTable[index].Tag);*/

        index++;		
		FillEntryInList(pMyObject, pMappingContainer);
    }

    pMappingContainer->XDNSEntryCount = index;
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
