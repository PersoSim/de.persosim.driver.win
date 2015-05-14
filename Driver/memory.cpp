#include "memory.h"

bool getBuffer(IWDFIoRequest* pRequest,BYTE *buffer,int *bufferLen) {

	IWDFMemory *inmem=NULL;
	pRequest->GetInputMemory(&inmem);
	if (inmem==NULL) {
		OutputDebugString(L"GetInputMemory failed");
		return false;
	}
	else {
		SIZE_T size;
		void *data=inmem->GetDataBuffer(&size);
		memcpy(buffer,data,size);
		(*bufferLen)=(int)size;
		inmem->Release();
		return true;
	}
}

void setBuffer(IWDFIoRequest* pRequest,BYTE *buffer,int inSize) {
	IWDFMemory *outmem=NULL;
	pRequest->GetOutputMemory (&outmem);
	if (outmem==NULL) {
		OutputDebugString(L"GetOutputMemory failed");
        pRequest->Complete(HRESULT_FROM_WIN32(ERROR_GEN_FAILURE));
	}
	else {
		outmem->CopyFromBuffer(0, buffer, inSize);
		outmem->Release();
        pRequest->CompleteWithInformation(0,(SIZE_T)inSize);
	}
}

void setString(IWDFIoRequest* pRequest,char *result,int outSize) {
	IWDFMemory *outmem=NULL;
	pRequest->GetOutputMemory (&outmem);
	if (outmem==NULL) {
		OutputDebugString(L"GetOutputMemory failed");
        pRequest->Complete(HRESULT_FROM_WIN32(ERROR_GEN_FAILURE));
	}
	else {
		int size=min(outSize,(int)strlen(result)+1);
		outmem->CopyFromBuffer(0,result,size);
		outmem->Release();
        pRequest->CompleteWithInformation(0,(SIZE_T)size);
	}
}

void setInt(IWDFIoRequest* pRequest,DWORD value) {
	IWDFMemory *outmem=NULL;
	pRequest->GetOutputMemory (&outmem);
	if (outmem==NULL) {
		OutputDebugString(L"GetOutputMemory failed");
        pRequest->Complete(HRESULT_FROM_WIN32(ERROR_GEN_FAILURE));
	}
	else {
		outmem->CopyFromBuffer(0, &value, sizeof(value));
		outmem->Release();
		pRequest->CompleteWithInformation(0, sizeof(value));
	}
}

DWORD getInt(IWDFIoRequest* pRequest) {

	IWDFMemory *inmem=NULL;
	pRequest->GetInputMemory(&inmem);
	if (inmem==NULL) {
		OutputDebugString(L"GetInputMemory failed");
		return 0xffffffff;
	}
	else {
		SIZE_T size;
		void *data=inmem->GetDataBuffer(&size);
		DWORD d=*(DWORD *)data;
		inmem->Release();
		return d;
	}
}
