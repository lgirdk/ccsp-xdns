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
#include <syscfg/syscfg.h>
#include "plugin_main_apis.h"
#include "cosa_xdns_apis.h"
#include <sys/inotify.h>
#include <limits.h>
#include "ccsp_xdnsLog_wrapper.h"

//static pthread_mutex_t dnsmasqMutex = PTHREAD_MUTEX_INITIALIZER;

#define BUF_LEN (10 * (sizeof(struct inotify_event) + NAME_MAX + 1))

void* MonitorResolvConfForChanges(void *arg);


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
        fprintf(stderr,"\nError reading file\n");
        //pthread_mutex_unlock(&dnsmasqMutex);
        return;
    }
    //Step 2: Get text from original file//
    while(fgets(dnsmasqConfEntry, sizeof(dnsmasqConfEntry), fp1) != NULL)
    {
        if(strstr(dnsmasqConfEntry, macaddress) != NULL)
            {
                strcpy(defaultEntry, dnsmasqConfEntry);
                break;
            }
    }

    fclose(fp1);

    //pthread_mutex_unlock(&dnsmasqMutex);

}

void RefreshResolvConfEntry()
{
	//0. Determine if XDNS is ON, if only ON then copy entries to RESOLV_CONF from DNSMASQ_SERVERS_CONF :
	//1. read non-dnsoverride entries from RESOLV_CONF and write to temp file
	//2. read dnsoverride entries from DNSMASQ_SERVERS_CONF and write to temp file
	//3. clear resolv.conf and write all entries from temp file to RESOLV_CONF

	char xdnsflag[20] = {0};
	int rc = syscfg_get(NULL, "X_RDKCENTRAL-COM_XDNS", xdnsflag, sizeof(xdnsflag));
	if (0 != rc || '\0' == xdnsflag[0] ) //if flag not found
	{
		printf("### XDNS - Never enabled. CcspXDNS Not writing to RESOLV_CONF ### \n");
	}
	else if(0 == strcmp("0", xdnsflag)) //flag set to false
	{
		printf("### XDNS - Disabled. CcspXDNS Not writing to RESOLV_CONF  ###\n");
	}
	else if (0 == strcmp("1", xdnsflag)) //if xDNS set to true
	{

    printf("### XDNS flag is enabled. CcspXDNS is syncing config to RESOLV_CONF  ###\n");
    char dnsmasqConfEntry[256] = {0};
    char resolvConfEntry[256] = {0};

    unlink(DNSMASQ_SERVERS_BAK);

    //Open text files and check that they open//
    FILE *fp1 = NULL, *fp2 = NULL, *fp3 = NULL;

    fp1 = fopen(RESOLV_CONF,"r");
    if(fp1 == NULL)
	{
		fprintf(stderr,"## CcspXDNS fopen(RESOLV_CONF, 'r') Error !!\n");
		return;
    }

    fp2 = fopen(DNSMASQ_SERVERS_BAK ,"a");
    if(fp2 == NULL)
	{
		fprintf(stderr,"## CcspXDNS fopen(DNSMASQ_SERVERS_BAK, 'a') Error !!\n");
		fclose(fp1);
		return;
	}

    fp3 = fopen(DNSMASQ_SERVERS_CONF,"r");
    if(fp3 == NULL)
	{
		fprintf(stderr,"## CcspXDNS fopen(DNSMASQ_SERVERS_CONF, 'r') Error !!\n");
		fclose(fp1);
		fclose(fp2);
		return;
	}


    //Get entries (other than dnsoverride) from resolv.conf file//

    while(fgets(resolvConfEntry, sizeof(resolvConfEntry), fp1) !=NULL)
    {
   	    if ( strstr(resolvConfEntry, "dnsoverride"))
		{
			continue;
		}

    	// write non dnsoverride entries to temp file
        fprintf(fp2, "%s", resolvConfEntry);
    }

    // Get dnsoverride entries from dnsmasq_servers.conf file
    while(fgets(dnsmasqConfEntry, sizeof(dnsmasqConfEntry), fp3) != NULL)
    {
    	if ( !strstr(dnsmasqConfEntry, "dnsoverride"))
    	{
    		continue;
    	}

    	char tempEntry[256] = {0};
    	strcpy(tempEntry, dnsmasqConfEntry);

    	//validate
    	if(!strtok(tempEntry, " \t\n\r")) //dnsoverride token
    		continue;

    	char *macaddr = NULL, *srvaddr4 = NULL;
#ifdef FEATURE_IPV6
    	char *srvaddr6 = NULL;
#endif

    	if(!(macaddr = strtok(NULL, " \t\n\r")))
    		continue;

    	if(!(srvaddr4 = strtok(NULL, " \t\n\r")))
    		continue;

    	struct sockaddr_in sa;
    	if (inet_pton(AF_INET, srvaddr4, &(sa.sin_addr)) != 1)
    		continue;

#ifdef FEATURE_IPV6
    	if(!(srvaddr6 = strtok(NULL, " \t\n\r")))
    		continue;

    	struct sockaddr_in6 sa6;
    	if (inet_pton(AF_INET6, srvaddr6, &(sa6.sin6_addr)) != 1)
    		continue;
#endif

    	// write valid dnsoverride entries to tmp file
    	fprintf(fp2, "%s", dnsmasqConfEntry);
    }

    // at this point the temp file has entries from resolv.conf and dnsmasq_server.conf
    // close all files and reopen to read from temp and write to resolv.conf
    fclose(fp1); fp1 = NULL;
    fclose(fp2); fp2 = NULL;
    fclose(fp3); fp3 = NULL;

    fp1 = fopen(RESOLV_CONF,"w");
    if(fp1 == NULL)
	{
		fprintf(stderr,"## CcspXDNS fopen(RESOLV_CONF, 'w') Error !!\n");
		return;
	}

    fp2 = fopen(DNSMASQ_SERVERS_BAK ,"r");
    if(fp2 == NULL)
	{
		fprintf(stderr,"## CcspXDNS fopen(DNSMASQ_SERVERS_BAK, 'r') Error !!\n");
		fclose(fp1);
		return;
	}

    //copy entries from temp file to resolv.conf file//
	while(fgets(resolvConfEntry, sizeof(resolvConfEntry), fp2) != NULL)
	{
		// write to resolv.conf file
		fprintf(fp1, "%s", resolvConfEntry);
	}

    fclose(fp1); fp1 = NULL;
    fclose(fp2); fp2 = NULL;

	}
    return;
}


 static void ResolvConfFileEvent(struct inotify_event *i)
 {
     fprintf(stderr, "    wd =%2d; ", i->wd);
     fprintf(stderr, "mask = ");

     if (i->mask & IN_ACCESS)        fprintf(stderr, "IN_ACCESS ");
     if (i->mask & IN_ATTRIB)        fprintf(stderr, "IN_ATTRIB ");
     if (i->mask & IN_CLOSE_NOWRITE) fprintf(stderr, "IN_CLOSE_NOWRITE ");
     if (i->mask & IN_CLOSE_WRITE)   fprintf(stderr, "IN_CLOSE_WRITE ");
     if (i->mask & IN_CREATE)        fprintf(stderr, "IN_CREATE ");
     if (i->mask & IN_DELETE)        fprintf(stderr, "IN_DELETE ");
     if (i->mask & IN_DELETE_SELF)   fprintf(stderr, "IN_DELETE_SELF ");
     if (i->mask & IN_IGNORED)       fprintf(stderr, "IN_IGNORED ");
     if (i->mask & IN_ISDIR)         fprintf(stderr, "IN_ISDIR ");
     if (i->mask & IN_MODIFY)        fprintf(stderr, "IN_MODIFY ");
     if (i->mask & IN_MOVE_SELF)     fprintf(stderr, "IN_MOVE_SELF ");
     if (i->mask & IN_MOVED_FROM)    fprintf(stderr, "IN_MOVED_FROM ");
     if (i->mask & IN_MOVED_TO)      fprintf(stderr, "IN_MOVED_TO ");
     if (i->mask & IN_OPEN)          fprintf(stderr, "IN_OPEN ");
     if (i->mask & IN_Q_OVERFLOW)    fprintf(stderr, "IN_Q_OVERFLOW ");
     if (i->mask & IN_UNMOUNT)       fprintf(stderr, "IN_UNMOUNT ");

     if (i->mask & IN_CLOSE_WRITE || i->mask & IN_ATTRIB)
     {
        fprintf(stderr, "IN_CLOSE_WRITE || IN_ATTRIB");
        //sleep(5);
        //RefreshDnsmasqConfEntry();
        RefreshResolvConfEntry();
     } 

     fprintf(stderr, "\n");
 
     if (i->len > 0)
         fprintf(stderr, "        name = %s\n", i->name);
 }
 
void* MonitorResolvConfForChanges(void *arg)
{
    int inotifyFd, wd, j;
    char buf[BUF_LEN] __attribute__ ((aligned(8)));
    ssize_t numRead;
    char *p;
    struct inotify_event *event;

    inotifyFd = inotify_init();                 /* Create inotify instance */
    if (inotifyFd == -1)
        {
            CcspXdnsConsoleTrace(("RDK_LOG_ERROR, CcspXDNS %s : inotify_init error  \n", __FUNCTION__ ));
            return;
        }

     wd = inotify_add_watch(inotifyFd, RESOLV_CONF, IN_ALL_EVENTS);
     if (wd == -1)
        {
            CcspXdnsConsoleTrace(("RDK_LOG_ERROR, CcspXDNS %s : inotify_add_watch error  \n", __FUNCTION__ ));
            return;
        }

    CcspXdnsConsoleTrace(("RDK_LOG_ERROR, CcspXDNS %s : Watching RESOLV_CONF using wd %d  \n", __FUNCTION__ , wd));

    for (;;) 
    {                                  /* Read events forever */
     numRead = read(inotifyFd, buf, BUF_LEN);
     if (numRead == 0)
        {
            CcspXdnsConsoleTrace(("RDK_LOG_ERROR, CcspXDNS %s : read() from inotify fd returned  numRead %d  \n", __FUNCTION__ , numRead));
        }

     if (numRead == -1)
        {
            CcspXdnsConsoleTrace(("RDK_LOG_ERROR, CcspXDNS %s : read error numRead %d  \n", __FUNCTION__ , numRead));
        }

    CcspXdnsConsoleTrace(("RDK_LOG_ERROR, CcspXDNS %s : Read %ld bytes from inotify fd\n \n", __FUNCTION__ , numRead));

     /* Process all of the events in buffer returned by read() */

     for (p = buf; p < buf + numRead; ) 
        {
         event = (struct inotify_event *) p;
         ResolvConfFileEvent(event);

         p += sizeof(struct inotify_event) + event->len;
        }
    }
}


void ReplaceDnsmasqConfEntry(char* macaddress, char* overrideEntry)
{
    char dnsmasqConfEntry[256] = {0};

    //pthread_mutex_lock(&dnsmasqMutex);

    if(!macaddress || !strlen(macaddress))
    {
        return;
    }

    unlink(DNSMASQ_SERVERS_BAK);

    //Step 1: Open text files and check that they open//
    FILE *fp1 = NULL, *fp2 = NULL;

    fp1 = fopen(DNSMASQ_SERVERS_CONF,"r");
    if(fp1 == NULL)
    {
        fprintf(stderr,"\nReplaceDnsmasqConfEntry() - Error reading file %s\n", DNSMASQ_SERVERS_CONF);
        return;
    }

    fp2 = fopen(DNSMASQ_SERVERS_BAK ,"a");
    if(fp2 == NULL)
    {
        fprintf(stderr,"\nReplaceDnsmasqConfEntry() - Error reading file %s\n", DNSMASQ_SERVERS_BAK);
        return;
    }

    //Step 2: Get text from original file//
    while(fgets(dnsmasqConfEntry, sizeof(dnsmasqConfEntry), fp1) != NULL)
    {
    	// skip entry that match the mac addr, copy the rest to BAK
        if(strstr(dnsmasqConfEntry, macaddress) != NULL)
            {
                continue;
            }

        fprintf(fp2, "%s", dnsmasqConfEntry);
    }

    // now copy the new entry for that mac (if not NULL)
    if(overrideEntry)
        fprintf(fp2, "%s", overrideEntry);

    fclose(fp1);
    unlink(DNSMASQ_SERVERS_CONF);
    fclose(fp2);
    rename(DNSMASQ_SERVERS_BAK, DNSMASQ_SERVERS_CONF);

	// update to resolv.conf file
    RefreshResolvConfEntry();

	return;
}

void AppendDnsmasqConfEntry(char* string1)
{
    FILE *fp2;

    fp2 = fopen(DNSMASQ_SERVERS_CONF ,"a");

    if(fp2 == NULL)
    {
        fprintf(stderr,"\nError reading file\n");
        return;
    }

    fprintf(fp2, "%s", string1);
    fclose(fp2);

	return;
}

// Create Default dnsoverride entry (00:00:00:00:00:00) using nameserver entries from resolv.conf file
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

	//Step 1: Open RESOLV_CONF //
	FILE *fp1 = NULL;
    fp1 = fopen(RESOLV_CONF,"r");
    if(fp1 == NULL)
    {
		fprintf(stderr,"\nCreateDnsmasqServerConf() Error opening file %s \n", RESOLV_CONF);
        return;
    }
	//Step 2: scan RESOLV_CONF for first IPv4 & IPv6 nameserver entries. We will use this to create default dnsoverride entry//
	while(fgets(resolvConfEntry, sizeof(resolvConfEntry), fp1) != NULL)
    {
    	if(strstr(resolvConfEntry, "nameserver") != NULL)
    		{
                strcpy(buf, resolvConfEntry);

                char *newline = strchr( buf, '\n' );
                if ( newline )
                    *newline = 0;

			token = strtok(buf, s); //first token: nameserver
                if(!token)
                {
                    continue;   
                }                 

			token = strtok(NULL, s); // token after nameserver : ipv4 or v6 addr
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
    }

    fclose(fp1);

	//store values read from resolv.conf to TR181 object, could be empty
    strcpy(pMyObject->DefaultDeviceDnsIPv4, tokenIPv4);
    strcpy(pMyObject->DefaultDeviceDnsIPv6, tokenIPv6);
    strcpy(pMyObject->DefaultDeviceTag, "empty");

#ifndef FEATURE_IPV6
	/* IPv4 ONLY MODE : Create xDNS default dnsoverride entry */
	if(foundIPv4)
	{
	snprintf(dnsmasqConfOverrideEntry, 256, "dnsoverride 00:00:00:00:00:00 %s %s\n", tokenIPv4, pMyObject->DefaultDeviceTag);
	AppendDnsmasqConfEntry(dnsmasqConfOverrideEntry);
	}
#else
	/* Create xDNS default dnsoverride entry by reading both IPv4 and IPv6 */
	if(foundIPv4 && foundIPv6)
    snprintf(dnsmasqConfOverrideEntry, 256, "dnsoverride 00:00:00:00:00:00 %s %s %s\n", tokenIPv4, tokenIPv6, pMyObject->DefaultDeviceTag);
	//else if(foundIPv4)	// !foundIPv6
	//	snprintf(dnsmasqConfOverrideEntry, 256, "dnsoverride 00:00:00:00:00:00 %s %s %s\n", tokenIPv4, "::", pMyObject->DefaultDeviceTag);
	//else if(foundIPv6)	// !foundIPv4
	//	snprintf(dnsmasqConfOverrideEntry, 256, "dnsoverride 00:00:00:00:00:00 %s %s %s\n", "0.0.0.0", tokenIPv6, pMyObject->DefaultDeviceTag);
	else // both entries not found in resolv.conf, we cannot create default dnsoverride.
		return;

    AppendDnsmasqConfEntry(dnsmasqConfOverrideEntry);
#endif

    //update entries to resolv.conf
    RefreshResolvConfEntry();

	return;
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

    if( access( DNSMASQ_SERVERS_CONF, F_OK ) != -1 )
    {
    	printf("CosaDmlGetSelfHealCfg: Calling RefreshResolvConfEntry");
    	RefreshResolvConfEntry();
    } 
    else 
    {
        pMappingContainer->XDNSEntryCount = 0;
       	printf("CosaDmlGetSelfHealCfg: Calling CreateDnsmasqServerConf");
        CreateDnsmasqServerConf(pMyObject);
        return pMappingContainer;
    }

    //pthread_mutex_lock(&dnsmasqMutex);

    /* MURUGAN - below logic is to add ip rule for each dns upstream server */
  	printf("CosaDmlGetSelfHealCfg: Calling logic to add ip rule for each dns upstream server");

    if ( (fp_dnsmasq_conf=fopen(DNSMASQ_SERVERS_CONF, "r")) != NULL )
    {
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
                    char iprulebuf[256] = {0};
                    snprintf(iprulebuf, 256, "from all to %s lookup erouter", token);

                    strcpy(pMyObject->DefaultDeviceDnsIPv4, token);
                    if(vsystem("ip -4 rule show | grep \"%s\" | grep -v grep >/dev/null", iprulebuf) != 0)
                        vsystem("ip -4 rule add %s", iprulebuf);                
                }
                else
                {
                    continue;
                }

    #ifdef FEATURE_IPV6
                token = strtok(NULL, s);
                if(token && strstr(token, ":"))
                {
                    char iprulebuf[256] = {0};
                    snprintf(iprulebuf, 256, "from all to %s lookup erouter", token);

                    strcpy(pMyObject->DefaultDeviceDnsIPv6, token);

                    if(vsystem("ip -6 rule show | grep \"%s\" | grep -v grep >/dev/null", iprulebuf) != 0)
                        vsystem("ip -6 rule add %s", iprulebuf);
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
            dnsoverride AA:BB:CC:DD:EE:FF 1.2.3.4 2001:xxx:xxx:xxx ArbitrarySting
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


            // format: "dnsoverride <mac> <ipv4> <ipv6> [<tag>]" - only tag is optional
            if(ret < 4)
            {
                free(pDnsTableEntry);
                continue;
            }
            else
            {
                    char iprulebuf[256] = {0};
                    snprintf(iprulebuf, 256, "from all to %s lookup erouter", pDnsTableEntry->DnsIPv4);

                    if(vsystem("ip -4 rule show | grep \"%s\" | grep -v grep >/dev/null", iprulebuf) != 0)
                        vsystem("ip -4 rule add %s", iprulebuf);

    #ifdef FEATURE_IPV6

                    snprintf(iprulebuf, 256, "from all to %s lookup erouter", pDnsTableEntry->DnsIPv6);

                    if(vsystem("ip -6 rule show | grep \"%s\" | grep -v grep >/dev/null", iprulebuf) != 0)
                        vsystem("ip -6 rule add %s", iprulebuf);
    #endif                    
            }

            index++;        


            FillEntryInList(pMyObject, pDnsTableEntry);
        }

    }

    pMappingContainer->XDNSEntryCount = index;

    printf("CosaDmlGetSelfHealCfg XDNSEntryCount %d\n", index);

    fclose(fp_dnsmasq_conf);

    //pthread_mutex_unlock(&dnsmasqMutex);

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
    pthread_t tid;

    AnscSListInitializeHeader( &pMyObject->XDNSDeviceList );
    pMyObject->MaxInstanceNumber        = 0;
    pMyObject->ulXDNSNextInstanceNumber   = 1;

    // Initialize syscfg to make syscfg calls
    if (0 != syscfg_init())
    {
    	CcspXdnsConsoleTrace(("CcspXDNSInitialize Error: syscfg_init() failed!! \n"));
    }

    printf("#### XDNS - calling CosaXDNSInitialize");
    pMyObject->pMappingContainer = CosaDmlGetSelfHealCfg((ANSC_HANDLE)pMyObject);

/* MURUGAN - no need to monitor resolv.conf file
    if (pthread_create(&tid, NULL, MonitorResolvConfForChanges, NULL))
    {
        CcspXdnsConsoleTrace(("RDK_LOG_ERROR, CcspXDNS %s : Failed to Start Thread to start MonitorResolvConfForChanges  \n", __FUNCTION__ ));
        return ANSC_STATUS_FAILURE;
    }
*/
    printf("#### XDNS - CosaXDNSInitialize done. return %d", returnStatus);

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
