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

#define ANSC_STATUS_SUCCESS 0
#define ANSC_STATUS_FAILURE 0xFFFFFFFF
#define EOK 0
#define TRUE 1
#define FALSE 0

int consoleDebugEnable = 0;
FILE* debugLogFile;
char g_Subsystem[32] = {0};

typedef  unsigned char              UCHAR,          *PUCHAR;
typedef  unsigned long              ULONG,          *PULONG;
typedef  UCHAR                      BOOL,           *PBOOL;
typedef  ULONG                  ANSC_STATUS,     *PANSC_STATUS;
typedef  void*                  PVOID;
typedef  PVOID                  ANSC_HANDLE,     *PANSC_HANDLE;
