#include "stdafx.h"
#pragma hdrstop

extern xrSkin4W* skin4W_func;

struct SKIN_PARAMS {
	void* Dest;
	void* Src;
	u32 Count;
	void* Data;
};

void Skin4W_Stream(LPVOID lpvParams)
{
	SKIN_PARAMS* sp = (SKIN_PARAMS*)lpvParams;

	vertRender*		D = (vertRender*)sp->Dest;
	vertBoned4W*	S = (vertBoned4W*)sp->Src;
	u32				vCount = sp->Count;
	CBoneInstance*	Bones = (CBoneInstance*)sp->Data;

	skin4W_func(D, S, vCount, Bones);
}

void __stdcall xrSkin4W_thread(vertRender* D, vertBoned4W* S, u32 vCount, CBoneInstance* Bones)
{
	size_t nWorkers = ttapi.threads.size();

	if (vCount < (nWorkers * 64)) {
		skin4W_func(D, S, vCount, Bones);
		return;
	}

	SKIN_PARAMS* sknParams = (SKIN_PARAMS*)_alloca(sizeof(SKIN_PARAMS) * nWorkers);

	// Give ~1% more for the last worker
	// to minimize wait in final spin
	u32 nSlice = vCount / 128;

	u32 nStep = ((vCount - nSlice) / nWorkers);
	u32 nLast = vCount - nStep * (nWorkers - 1);

	for (u32 i = 0; i < nWorkers; ++i) {
		sknParams[i].Dest = (void*)(D + i * nStep);
		sknParams[i].Src = (void*)(S + i * nStep);
		sknParams[i].Count = (i == (nWorkers - 1)) ? nLast : nStep;
		sknParams[i].Data = (void*)Bones;

		ttapi.threads[i]->addJob([=] { Skin4W_Stream((void*)&sknParams[i]); });
	}

	ttapi.wait();
}

