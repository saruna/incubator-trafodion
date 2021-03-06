// @@@ START COPYRIGHT @@@
//
// (C) Copyright 2003-2014 Hewlett-Packard Development Company, L.P.
//
//  Licensed under the Apache License, Version 2.0 (the "License");
//  you may not use this file except in compliance with the License.
//  You may obtain a copy of the License at
//
//      http://www.apache.org/licenses/LICENSE-2.0
//
//  Unless required by applicable law or agreed to in writing, software
//  distributed under the License is distributed on an "AS IS" BASIS,
//  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
//  See the License for the specific language governing permissions and
//  limitations under the License.
//
// @@@ END COPYRIGHT @@@

#define WIN32_LEAN_AND_MEAN

#include "windows.h"
#include <winsock2.h>
#include <Ws2tcpip.h>
#include "Transport.h"
#include "TCPIPSystemDrvr.h"
#include "../drvrglobal.h"
#include "../diagfunctions.h"
#include "cconnect.h"
#include "cstmt.h"
#include "tdm_odbcDrvMsg.h"

extern CEE_status MAP_SRVR_ERRORS(CConnect *pConnection);

CTCPIPSystemDrvr::CTCPIPSystemDrvr()
{
	odbcAPI = 0;
	dialogueId = 0;

	RESET_ERRORS((long)this);

	m_wbuffer = NULL;
	m_rbuffer = NULL;
	m_rbuffer_length = 0;
	m_wbuffer_length = 0;
	m_object_ref[0]=0;
	m_IObuffer = NULL;
	m_IObuffer = new char[MAX_TCP_BUFFER_LENGTH];
	if (m_IObuffer == NULL)
		exit(0);
	m_hSocket = -1;
	m_IOCompression = 0;
	m_hEvents[0] = NULL;
	m_hEvents[1] = NULL;
	m_swap = SWAP_YES;
}

CTCPIPSystemDrvr::~CTCPIPSystemDrvr()
{
	RESET_ERRORS((long)this);
	cleanup();

	if (m_IObuffer != NULL)
		delete m_IObuffer;
	if (m_hSocket != -1)
	{
		closesocket(m_hSocket);
	}
	if (m_hEvents[0] != NULL)
		CloseHandle(m_hEvents[0]);
	if (m_hEvents[1] != NULL)
		CloseHandle(m_hEvents[1]);
}

void CTCPIPSystemDrvr::cleanup(void)
{
	w_release();
	r_release();
}

void CTCPIPSystemDrvr::process_swap( char* buffer )
{
#ifdef __OBSOLETE	
	PROCESS_swap(buffer, odbcAPI);
#endif
}

char* CTCPIPSystemDrvr::w_allocate(int size)
{
	if (m_wbuffer != NULL)
		delete m_wbuffer;
	m_wbuffer = new char[size];
	if (m_wbuffer != NULL)
		m_wbuffer_length = size;
	else
		m_wbuffer_length = 0;
	return m_wbuffer;
}

char* CTCPIPSystemDrvr::r_allocate( int size)
{
	if (m_rbuffer != NULL)
		delete m_rbuffer;
	m_rbuffer = new char[size];
	if (m_rbuffer != NULL)
		m_rbuffer_length = size;
	else
		m_rbuffer_length = 0;
	return m_rbuffer;
}

void CTCPIPSystemDrvr::r_assign(char* buffer, long length)
{
	if (m_rbuffer != NULL)
		delete m_rbuffer;
	m_rbuffer = buffer;
	m_rbuffer_length = length;
}

void CTCPIPSystemDrvr::w_assign(char* buffer, long length)
{
	if (m_wbuffer != NULL)
		delete m_wbuffer;
	m_wbuffer = buffer;
	m_wbuffer_length = length;
}

void CTCPIPSystemDrvr::w_release()
{
	if (m_wbuffer != NULL)
		delete m_wbuffer;
	m_wbuffer = NULL;
	m_wbuffer_length = 0;
}

void CTCPIPSystemDrvr::r_release()
{
	if (m_rbuffer != NULL)
		delete m_rbuffer;
	m_rbuffer = NULL;
	m_rbuffer_length = 0;
}

char* CTCPIPSystemDrvr::w_buffer()
{
	return m_wbuffer;
}

char* CTCPIPSystemDrvr::r_buffer()
{
	return m_rbuffer;
}

long CTCPIPSystemDrvr::w_buffer_length()
{
	return m_wbuffer_length;
}

long CTCPIPSystemDrvr::r_buffer_length()
{
	return m_rbuffer_length;
}

void CTCPIPSystemDrvr::send_error(short error, short error_detail, const CEE_handle_def *call_id_)
{
}

CEE_status CTCPIPSystemDrvr::send_response(char* buffer, unsigned long message_length, const CEE_handle_def *call_id_)
{
	return CEE_SUCCESS;
}

void CTCPIPSystemDrvr::CloseSessionWithCleanup()
{
	(gDrvrGlobal.fpTCPIPCloseSession)(this);
	cleanup();
}

char CTCPIPSystemDrvr::swap()
{
	return m_swap;
}
void CTCPIPSystemDrvr::setSwap(char swap)
{
	m_swap = swap;
}

void CTCPIPSystemDrvr::resetSwap()
{
    m_swap = SWAP_YES;
}

char CTCPIPSystemDrvr::transport()
{
	return TCPIP;
}

CTCPIPSystemDrvr_list::CTCPIPSystemDrvr_list()
{
	list=NULL;
	InitializeCriticalSection(&CSTCPIP);
}
CTCPIPSystemDrvr_list::~CTCPIPSystemDrvr_list()
{
	DeleteCriticalSection(&CSTCPIP);
}
void CTCPIPSystemDrvr_list::cleanup()
{
	EnterCriticalSection (&GTransport.m_TransportCSObject);


	CTCPIPSystemDrvr* cnode = list;
	CTCPIPSystemDrvr* nnode;
	while( cnode != NULL ){
		nnode = cnode->next;
		delete cnode;
		cnode = nnode;
	}
	list=NULL;
	LeaveCriticalSection (&GTransport.m_TransportCSObject);
}
CTCPIPSystemDrvr* CTCPIPSystemDrvr_list::ins_node()
{
	EnterCriticalSection (&GTransport.m_TransportCSObject);

	CTCPIPSystemDrvr* cnode = list;
	CTCPIPSystemDrvr* pnode = list;
	CTCPIPSystemDrvr* nnode;
	while(cnode!=NULL ){
		pnode=cnode;
		cnode=cnode->next;
	}
	if((nnode = new CTCPIPSystemDrvr())!=NULL){
		nnode->next = cnode;
		if(pnode!=NULL) 
			pnode->next = nnode;
		else
			list = nnode;
	}
	LeaveCriticalSection (&GTransport.m_TransportCSObject);
	return nnode ;
}
bool CTCPIPSystemDrvr_list::del_node(CTCPIPSystemDrvr* p )
{
	EnterCriticalSection (&GTransport.m_TransportCSObject);
	bool bret = true;

	CTCPIPSystemDrvr* cnode = list;
	CTCPIPSystemDrvr* pnode = list;
	while( cnode!= NULL )
	{
		if( cnode == p )
			break;
		pnode = cnode;
		cnode = cnode->next;
	}
	if( cnode==NULL )
	{
		bret = false;
		goto out;
	}
	if (pnode == list && cnode == list)
		list = cnode->next;
	else
		pnode->next = cnode->next;
	delete cnode;
out:
	LeaveCriticalSection (&GTransport.m_TransportCSObject);
	return bret;
}
bool CTCPIPSystemDrvr_list::find_node(CTCPIPSystemDrvr* p )
{
	EnterCriticalSection (&GTransport.m_TransportCSObject);
	CTCPIPSystemDrvr* cnode = list;
	bool bfound = false;
	while( cnode != NULL )
	{
		if (cnode == p ){
			bfound = true;
			break;
		}
		cnode = cnode->next;
	}
	LeaveCriticalSection (&GTransport.m_TransportCSObject);
	return bfound;
}

CTCPIPSystemDrvr* CTCPIPSystemDrvr_list::find_node(char* object_ref )
{
	EnterCriticalSection (&GTransport.m_TransportCSObject);
	CTCPIPSystemDrvr* cnode = list;
	while( cnode != NULL )
	{
		if (strcmp(cnode->m_object_ref, object_ref) == 0 ){
			break;
		}
		cnode = cnode->next;
	}
	LeaveCriticalSection (&GTransport.m_TransportCSObject);
	return cnode;
}

//=====================================================================

bool OpenIO (CTCPIPSystemDrvr* pTCPIPSystem, char* object_ref)
{

	bool sts = false;

	RESET_ERRORS((long)pTCPIPSystem);
	if (gDrvrGlobal.gTCPIPLoadError != 0)
	{
		SET_ERROR((long)pTCPIPSystem, PC, TCPIP, UNKNOWN_API, E_DRIVER, gDrvrGlobal.gTCPIPLibrary, O_INIT_PROCESS, F_LOAD_DLL, gDrvrGlobal.gTCPIPLoadError, (int)0);
		sts = false;
	}
	else
	{
		RESET_ERRORS((long)pTCPIPSystem);
		sts = (gDrvrGlobal.fpTCPIPOpenSession)((void*)pTCPIPSystem, object_ref);
	}

	return sts;
}

void CloseIO (CTCPIPSystemDrvr* pTCPIPSystem)
{
	pTCPIPSystem->w_release(); //release write buffer
	(gDrvrGlobal.fpTCPIPCloseSession)((void*)pTCPIPSystem);
}

bool DoIO (CTCPIPSystemDrvr* pTCPIPSystem, char* wbuffer, long write_count, char*& rbuffer, long& read_count, CConnect *pConnection, CStmt *pStatement)
{
	bool bok = true;

// write count, read count and temporary count
	long wcount;
	long rcount = 0;
	long tcount = 0;

	char* buffer;
	rbuffer = NULL;
//
	HEADER wheader = {0,0,0,0,'N',COMP_12,WRITE_REQUEST_FIRST,SIGNATURE,CLIENT_HEADER_VERSION_LE,PC,TCPIP,SWAP_YES,0,0};
	HEADER rheader = {0,0,0,0,'N',COMP_12,READ_RESPONSE_FIRST,SIGNATURE,CLIENT_HEADER_VERSION_LE,PC,TCPIP,SWAP_YES,0,0};
	HEADER* prheader,*pwheader;

	if(pTCPIPSystem->odbcAPI == AS_API_GETOBJREF)
	{
		// For GetObjRef, the header is always sent in BigEndian form since when establishing a new connection
		// we won't know the system type
		;
	}
	else
	{
		if(pTCPIPSystem->swap() == SWAP_NO)
		{
			// We're connected from a Little Endian Client to a Little Endian Server
			wheader.swap = SWAP_NO;
			rheader.swap = SWAP_NO;
		}
		else
		{
			// We're connected from a Little Endian Client to a Big Endian Server
			;
		}
	}

	wheader.operation_id	= pTCPIPSystem->odbcAPI;
	rheader.operation_id	= pTCPIPSystem->odbcAPI;
	wheader.dialogueId		= pTCPIPSystem->dialogueId;
	rheader.dialogueId		= pTCPIPSystem->dialogueId;
	unsigned int timeout	= pTCPIPSystem->dwTimeout;
	memset(&pTCPIPSystem->m_rheader, 0, sizeof(HEADER));

	RESET_ERRORS((long)pTCPIPSystem);

	if (pTCPIPSystem->m_IOCompression > 0 &&
	    pTCPIPSystem->m_IOCompression == 1)
	{
		wheader.compress_ind = COMP_YES;
		//wheader.compress_type = COMP_14; // this should be set in the ::Compress method, based on what compression type the application chooses (right now there is only one COMP_14
		rheader.compress_ind = COMP_YES;
		rheader.compress_type = COMP_14;  // the driver requests that the server use this compression type. this should also be set based on what compression type the application chooses
	}
	else
	{
		wheader.compress_ind = COMP_NO;
		wheader.compress_type = 0;
		rheader.compress_ind = COMP_NO;
		rheader.compress_type = 0;
	}

// send to the server

	wheader.total_length = write_count;
	wheader.hdr_type = WRITE_REQUEST_FIRST;
		wheader.compress_type = 0;
#if (defined(WIN32)||defined(_WIN64))&&defined(TRACE_COMPRESSION)
	if(gDrvrGlobal.gTraceCompression)
	{
		printf("sending bytes number: %d\n",write_count);
	}
#endif
	wcount = write_count;

	while (wcount > 0)
	{
		if (wheader.hdr_type == WRITE_REQUEST_FIRST)
		{
			if (wcount > MAX_TCP_BUFFER_LENGTH - sizeof(HEADER))
				tcount = MAX_TCP_BUFFER_LENGTH - sizeof(HEADER);
			else
				tcount = wcount;
		}
		else
		{
			if (wcount > MAX_TCP_BUFFER_LENGTH)
				tcount = MAX_TCP_BUFFER_LENGTH;
			else
				tcount = wcount;
		}

		pwheader = &wheader;
		TRACE_TRANSPORT_OUT(wheader.hdr_type, pTCPIPSystem->m_object_ref, pwheader, wbuffer, tcount, timeout);
		bok = (gDrvrGlobal.fpTCPIPDoWriteRead)((void*)pTCPIPSystem, pwheader, wbuffer, tcount, timeout);
		if (bok == false)
			goto out;
		wheader.hdr_type = WRITE_REQUEST_NEXT;
		wcount  -= tcount;
		wbuffer += tcount;
	}
// receive from the server

	pTCPIPSystem->cleanup(); //release all temp memory

// read for READ_RESPONSE_FIRST

	tcount = 0;
	rbuffer = NULL;
	rheader.hdr_type = READ_RESPONSE_FIRST;

	prheader = &rheader;
	bok = (gDrvrGlobal.fpTCPIPDoWriteRead)((void*)pTCPIPSystem, prheader, rbuffer, tcount, timeout);

	if(bok == false)
	{
		if(MAP_SRVR_ERRORS(pConnection) == TIMEOUT_EXCEPTION)
		{
			if( pTCPIPSystem->odbcAPI != AS_API_GETOBJREF &&
				pTCPIPSystem->odbcAPI != AS_API_STOPSRVR &&
				pTCPIPSystem->odbcAPI != SRVR_API_SQLCONNECT &&
				pTCPIPSystem->odbcAPI != SRVR_API_SQLDISCONNECT)
			{
				if(pStatement != NULL)
					pStatement->setDiagRec(DRIVER_ERROR, IDS_S1_T00, TIMEOUT_EXCEPTION, FORMAT_ERROR((long)pStatement->getSrvrTCPIPSystem()));
				pConnection->sendStopServer();
				bok = (gDrvrGlobal.fpTCPIPDoWriteRead)((void*)pTCPIPSystem, prheader, rbuffer, tcount, timeout);
			}
		}
	}

	if (bok == false)
		goto out;

	TRACE_TRANSPORT_IN(READ_RESPONSE_FIRST, pTCPIPSystem->m_object_ref, prheader, rbuffer, tcount, timeout);

	if(pTCPIPSystem->odbcAPI == AS_API_GETOBJREF)
	{
		if(prheader->version == SERVER_HEADER_VERSION_LE)
			pTCPIPSystem->setSwap(SWAP_NO);
		else if(prheader->version == SERVER_HEADER_VERSION_BE)
			pTCPIPSystem->setSwap(SWAP_YES);
		else
			// we're connected to an older system (which will just echo back the version we send)
			pTCPIPSystem->setSwap(SWAP_YES);
	}



// the server returns total length in the header

	memcpy(&pTCPIPSystem->m_rheader, prheader, sizeof(HEADER));
	if (prheader->compress_ind == COMP_YES && prheader->compress_type != 0)
	{
		SET_ERROR((long)pTCPIPSystem, PC, TCPIP, pTCPIPSystem->odbcAPI, E_DRIVER, pTCPIPSystem->m_object_ref, O_DO_WRITE_READ, F_DO_IO, DRVR_ERR_COMPRESS_OPERATION, rcount);
		bok = false;
		goto out;
	}
	else
		rcount = prheader->total_length;

	read_count = rcount;

	buffer = (char*)pTCPIPSystem->r_allocate(rcount);
	if (buffer == NULL)
	{
		SET_ERROR((long)pTCPIPSystem, PC, TCPIP, pTCPIPSystem->odbcAPI, E_DRIVER, pTCPIPSystem->m_object_ref, O_DO_OPERATOR_NEW, F_DO_IO, DRVR_ERR_MEMORY_ALLOCATE, rcount);
		bok = false;
		goto out;
	}

// if there is something beside the header in the IO buffer

	if (tcount > 0)
	{
		memcpy(buffer, rbuffer, tcount);
		rcount -= tcount;
	}

	rbuffer = buffer+ tcount;

// read for READ_RESPONSE_NEXT

	while (rcount > 0){
// we send only a header
		tcount = 0;
		rheader.hdr_type = READ_RESPONSE_NEXT;
		prheader = &rheader;
		bok = (gDrvrGlobal.fpTCPIPDoWriteRead)((void*)pTCPIPSystem, prheader, rbuffer, tcount, timeout);
		if (bok == false)
			goto out;
		TRACE_TRANSPORT_IN(READ_RESPONSE_NEXT, pTCPIPSystem->m_object_ref, prheader, rbuffer, tcount, timeout);
		rcount  -= tcount;
		rbuffer += tcount;
	}

	rbuffer = buffer;
#if (defined(WIN32)||defined(_WIN64))&&defined(TRACE_COMPRESSION)
	if(gDrvrGlobal.gTraceCompression)
	{
		printf("receiving bytes number: %d\n",read_count);
	}
#endif
out:
	return bok;
}
void WINAPI 
TCPIP_SET_ERROR(long signature, char platform, char transport, int api, ERROR_TYPE error_type, char* process, OPERATION operation, FUNCTION function, int error, int errordetail)
{
	SET_ERROR(signature, platform, transport, api, error_type, process, operation, function, error, errordetail);
}
