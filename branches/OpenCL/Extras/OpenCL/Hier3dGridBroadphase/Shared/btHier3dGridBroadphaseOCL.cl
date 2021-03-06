
MSTRINGIFY(

int getPosHashHier(int4 gridPos, int mask, __global float4* pParams)
{
	int4 gridDim = *((__global int4*)(pParams + 1));
	gridPos.x &= mask;
	gridPos.y &= mask;
	gridPos.z &= mask;
	int hash = gridPos.z * gridDim.y * gridDim.x + gridPos.y * gridDim.x + gridPos.x;
	return hash;
} 

int4 getGridPosHier(float4 worldPos, __global float4* pParams)
{
    int4 gridPos;
	int4 gridDim = *((__global int4*)(pParams + 1));
    gridPos.x = (int)floor(worldPos.x * pParams[0].x) & (gridDim.x - 1);
    gridPos.y = (int)floor(worldPos.y * pParams[0].y) & (gridDim.y - 1);
    gridPos.z = (int)floor(worldPos.z * pParams[0].z) & (gridDim.z - 1);
    return gridPos;
}


// calculate grid hash value for each body using its AABB
__kernel void kHierCalcHashAABB(int numObjects, __global float4* pAABB, __global int2* pHash, __global float4* pParams GUID_ARG)
{
    int index = get_global_id(0);
    if(index >= numObjects)
	{
		return;
	}
	float4 bbMin = pAABB[index*2];
	float4 bbMax = pAABB[index*2 + 1];
	float4 pos;
	pos.x = (bbMin.x + bbMax.x) * 0.5f;
	pos.y = (bbMin.y + bbMax.y) * 0.5f;
	pos.z = (bbMin.z + bbMax.z) * 0.5f;
	pos.w = 0.f;
	int mask = as_uint(bbMin.w);
    // get address in grid
    int4 gridPos = getGridPosHier(pos, pParams);
    int gridHash = getPosHashHier(gridPos, mask, pParams);
    // store grid hash and body index
    int2 hashVal;
    hashVal.x = gridHash;
    hashVal.y = index;
    pHash[index] = hashVal;
}

__kernel void kHierClearCellStart(	int numCells, 
								__global int* pCellStart GUID_ARG)
{
    int index = get_global_id(0);
    if(index >= numCells)
	{
		return;
	}
	pCellStart[index] = -1;
}

__kernel void kHierFindCellStart(int numObjects, __global int2* pHash, __global int* cellStart GUID_ARG)
{
	__local int sharedHash[513];
    int index = get_global_id(0);
	int2 sortedData;
    if(index < numObjects)
	{
		sortedData = pHash[index];
		// Load hash data into shared memory so that we can look 
		// at neighboring body's hash value without loading
		// two hash values per thread
		sharedHash[get_local_id(0) + 1] = sortedData.x;
		if((index > 0) && (get_local_id(0) == 0))
		{
			// first thread in block must load neighbor body hash
			sharedHash[0] = pHash[index-1].x;
		}
	}
    barrier(CLK_LOCAL_MEM_FENCE);
    if(index < numObjects)
	{
		if((index == 0) || (sortedData.x != sharedHash[get_local_id(0)]))
		{
			cellStart[sortedData.x] = index;
		}
	}
}

int testAABBOverlapHier(float4 min0, float4 max0, float4 min1, float4 max1)
{
	return	(min0.x <= max1.x)&& (min1.x <= max0.x) && 
			(min0.y <= max1.y)&& (min1.y <= max0.y) && 
			(min0.z <= max1.z)&& (min1.z <= max0.z); 
}


const int BT_3DGRID_PAIR_FOUND_FLG = 0x40000000;
const int BT_3DGRID_PAIR_NEW_FLG  = 0x20000000;
const int BT_3DGRID_PAIR_ANY_FLG  =  0x60000000; 
// (BT_3DGRID_PAIR_FOUND_FLG | BT_3DGRID_PAIR_NEW_FLG) // expression is not supported by NVidia, build failed without error message

void findPairsInCellHier(	int numObjects,
						int4	gridPos,
						int		myMask,
						int		currMask,
						int    index,
						__global int2*  pHash,
						__global int*   pCellStart,
						__global float4* pAABB, 
						__global int*   pPairBuff,
						__global int2*	pPairBuffStartCurr,
						__global float4* pParams)
{
	int4 pGridDim = *((__global int4*)(pParams + 1));
//	int maxBodiesPerCell = pGridDim.w;
    int gridHash = getPosHashHier(gridPos, currMask, pParams); //@@@@@@@@@@@
    // get start of bucket for this cell
    int bucketStart = pCellStart[gridHash];
    if (bucketStart == -1)
	{
        return;   // cell empty
	}
	// iterate over bodies in this cell
    int2 sortedData = pHash[index];
	int unsorted_indx = sortedData.y;
    float4 min0 = pAABB[unsorted_indx*2 + 0]; 
	float4 max0 = pAABB[unsorted_indx*2 + 1];
	int handleIndex =  as_int(max0.w);
	int2 start_curr = pPairBuffStartCurr[handleIndex];
	int start = start_curr.x;
	int curr = start_curr.y;
	int2 start_curr_next = pPairBuffStartCurr[handleIndex+1];
	int curr_max = start_curr_next.x - start - 1;
	for(int index2 = bucketStart; index2 < numObjects; index2++) 
	{
        int2 cellData = pHash[index2];
        if (cellData.x != gridHash)
        {
			break;   // no longer in same bucket
		}
		int unsorted_indx2 = cellData.y;
		float4 min1 = pAABB[unsorted_indx2*2 + 0];
		int otherMask = as_uint(min1.w);
		if((otherMask == currMask)&&(((myMask == currMask)&&(unsorted_indx2 > unsorted_indx))||(myMask > currMask)))
        {   
			float4 max1 = pAABB[unsorted_indx2*2 + 1];
			if(testAABBOverlapHier(min0, max0, min1, max1))
			{
				int handleIndex2 = as_int(max1.w);
				int k;
				for(k = 0; k < curr; k++)
				{
					int old_pair = pPairBuff[start+k] & (~BT_3DGRID_PAIR_ANY_FLG);
					if(old_pair == handleIndex2)
					{
						pPairBuff[start+k] |= BT_3DGRID_PAIR_FOUND_FLG;
						break;
					}
				}
				if(k == curr)
				{
					if(curr >= curr_max) 
					{ // not a good solution, but let's avoid crash
						break;
					}
					pPairBuff[start+curr] = handleIndex2 | BT_3DGRID_PAIR_NEW_FLG;
					curr++;
				}
			}
		}
	}
	int2 newStartCurr;
	newStartCurr.x = start;
	newStartCurr.y = curr;
	pPairBuffStartCurr[handleIndex] = newStartCurr;
    return;
}

__kernel void kHierFindOverlappingPairs(	int numObjects,
										__global float4* pAABB, 
										__global int2* pHash, 
										__global int* pCellStart, 
										__global int* pPairBuff, 
										__global int2* pPairBuffStartCurr, 
										__global float4* pParams GUID_ARG)

{
    int index = get_global_id(0);
    if(index >= numObjects)
	{
		return;
	}
    int2 sortedData = pHash[index];
	int unsorted_indx = sortedData.y;
	float4 bbMin = pAABB[unsorted_indx*2 + 0];
	float4 bbMax = pAABB[unsorted_indx*2 + 1];
	float4 pos;
	pos.x = (bbMin.x + bbMax.x) * 0.5f;
	pos.y = (bbMin.y + bbMax.y) * 0.5f;
	pos.z = (bbMin.z + bbMax.z) * 0.5f;
    // get address in grid
    int4 gridPosA = getGridPosHier(pos, pParams);
    int4 gridPosB; 
	int4 gridDim = *((__global int4*)(pParams + 1));
    int depth = gridDim.w;
    int depthMask = depth - 1;
    int mask = as_uint(bbMin.w);
    int myMask = mask;
    int delta_xyz = (-mask) & depthMask;
    int cmax;
    do
	{
		cmax = 1 - !(mask & (depth >> 1)) - !(mask & (depth >> 2));
		// examine only neighbouring cells
		for(int z=-1; z<=cmax; z++) 
		{
			gridPosB.z = (gridPosA.z + z * delta_xyz) & mask;
			for(int y=-1; y<=cmax; y++) 
			{
				gridPosB.y = (gridPosA.y + y * delta_xyz) & mask;
				for(int x=-1; x<=cmax; x++) 
				{
					gridPosB.x = (gridPosA.x + x * delta_xyz) & mask;
					findPairsInCellHier(numObjects, gridPosB, myMask, mask, index, pHash, pCellStart, pAABB, pPairBuff, pPairBuffStartCurr, pParams);
				}
			}
		}
		mask = (mask << 1) & depthMask;
		delta_xyz <<= 1;
	} while(cmax > -1);
}

__kernel void kHierComputePairCacheChanges(	int numObjects,
										__global int* pPairBuff, 
										__global int2* pPairBuffStartCurr, 
										__global int* pPairScan, 
										__global float4* pAABB GUID_ARG)
{
    int index = get_global_id(0);
    if(index >= numObjects)
	{
		return;
	}
	float4 bbMax = pAABB[index * 2 + 1];
	int handleIndex = as_int(bbMax.w);
	int2 start_curr = pPairBuffStartCurr[handleIndex];
	int start = start_curr.x;
	int curr = start_curr.y;
	__global int *pInp = pPairBuff + start;
	int num_changes = 0;
	for(int k = 0; k < curr; k++, pInp++)
	{
		if(!((*pInp) & BT_3DGRID_PAIR_FOUND_FLG))
		{
			num_changes++;
		}
	}
	pPairScan[index+1] = num_changes;
} 

__kernel void kHierSqueezeOverlappingPairBuff(	int numObjects,
											__global int* pPairBuff, 
											__global int2* pPairBuffStartCurr, 
											__global int* pPairScan,
											__global int* pPairOut, 
											__global float4* pAABB GUID_ARG)
{
    int index = get_global_id(0);
    if(index >= numObjects)
	{
		return;
	}
	float4 bbMax = pAABB[index * 2 + 1];
	int handleIndex = as_int(bbMax.w);
	int2 start_curr = pPairBuffStartCurr[handleIndex];
	int start = start_curr.x;
	int curr = start_curr.y;
	__global int* pInp = pPairBuff + start;
	__global int* pOut = pPairOut + pPairScan[index];
	__global int* pOut2 = pInp;
	int num = 0; 
	for(int k = 0; k < curr; k++, pInp++)
	{
		if(!((*pInp) & BT_3DGRID_PAIR_FOUND_FLG))
		{
			*pOut = *pInp;
			pOut++;
		}
		if((*pInp) & BT_3DGRID_PAIR_ANY_FLG)
		{
			*pOut2 = (*pInp) & (~BT_3DGRID_PAIR_ANY_FLG);
			pOut2++;
			num++;
		}
	}
	int2 newStartCurr;
	newStartCurr.x = start;
	newStartCurr.y = num;
	pPairBuffStartCurr[handleIndex] = newStartCurr;
}

/*
 * Copyright 1993-2009 NVIDIA Corporation.  All rights reserved.
 *
 * NVIDIA Corporation and its licensors retain all intellectual property and 
 * proprietary rights in and to this software and related documentation. 
 * Any use, reproduction, disclosure, or distribution of this software 
 * and related documentation without an express license agreement from
 * NVIDIA Corporation is strictly prohibited.
 *
 * Please refer to the applicable NVIDIA end user license agreement (EULA) 
 * associated with this source code for terms and conditions that govern 
 * your use of this NVIDIA software.
 * 
 */


//#define LOCAL_SIZE_MAX 1024U

inline void ComparatorPrivateHier(int2* keyA, int2* keyB, uint dir)
{
    if((keyA[0].x > keyB[0].x) == dir)
    {
		int2 tmp = *keyA;
		*keyA = *keyB;
		*keyB = tmp;
    }
}

inline void ComparatorLocalHier(__local int2* keyA, __local int2* keyB, uint dir)
{
    if((keyA[0].x > keyB[0].x) == dir)
    {
		int2 tmp = *keyA;
		*keyA = *keyB;
		*keyB = tmp;
    }
}

////////////////////////////////////////////////////////////////////////////////
// Monolithic bitonic sort kernel for short arrays fitting into local memory
////////////////////////////////////////////////////////////////////////////////

__kernel void kHierBitonicSortCellIdLocal(__global int2* pKey, uint arrayLength, uint dir GUID_ARG)
{
//    __local int2 l_key[LOCAL_SIZE_MAX];
    __local int2 l_key[1024];
    int localSizeLimit = get_local_size(0) * 2;

    //Offset to the beginning of subbatch and load data
    pKey += get_group_id(0) * localSizeLimit + get_local_id(0);
    l_key[get_local_id(0) +                    0] = pKey[                   0];
    l_key[get_local_id(0) + (localSizeLimit / 2)] = pKey[(localSizeLimit / 2)];

    for(uint size = 2; size < arrayLength; size <<= 1)
    {
        //Bitonic merge
        uint ddd = dir ^ ( (get_local_id(0) & (size / 2)) != 0 );
        for(uint stride = size / 2; stride > 0; stride >>= 1)
        {
            barrier(CLK_LOCAL_MEM_FENCE);
            uint pos = 2 * get_local_id(0) - (get_local_id(0) & (stride - 1));
            ComparatorLocalHier(&l_key[pos +      0], &l_key[pos + stride], ddd);
        }
    }

    //ddd == dir for the last bitonic merge step
    {
        for(uint stride = arrayLength / 2; stride > 0; stride >>= 1)
        {
            barrier(CLK_LOCAL_MEM_FENCE);
            uint pos = 2 * get_local_id(0) - (get_local_id(0) & (stride - 1));
            ComparatorLocalHier(&l_key[pos + 0], &l_key[pos + stride], dir);
        }
    }

    barrier(CLK_LOCAL_MEM_FENCE);
    pKey[                   0] = l_key[get_local_id(0) +                    0];
    pKey[(localSizeLimit / 2)] = l_key[get_local_id(0) + (localSizeLimit / 2)];
}


////////////////////////////////////////////////////////////////////////////////
// Bitonic sort kernel for large arrays (not fitting into local memory)
////////////////////////////////////////////////////////////////////////////////
//Bottom-level bitonic sort
//Almost the same as bitonicSortLocal with the only exception
//of even / odd subarrays (of LOCAL_SIZE_LIMIT points) being
//sorted in opposite directions

__kernel void kHierBitonicSortCellIdLocal1(__global int2* pKey GUID_ARG)
{
//    __local int2 l_key[LOCAL_SIZE_MAX];
    __local int2 l_key[1024];
    uint localSizeLimit = get_local_size(0) * 2;

    //Offset to the beginning of subarray and load data
    pKey += get_group_id(0) * localSizeLimit + get_local_id(0);
    l_key[get_local_id(0) +                    0] = pKey[                   0];
    l_key[get_local_id(0) + (localSizeLimit / 2)] = pKey[(localSizeLimit / 2)];

    uint comparatorI = get_global_id(0) & ((localSizeLimit / 2) - 1);

    for(uint size = 2; size < localSizeLimit; size <<= 1)
    {
        //Bitonic merge
        uint ddd = (comparatorI & (size / 2)) != 0;
        for(uint stride = size / 2; stride > 0; stride >>= 1)
        {
            barrier(CLK_LOCAL_MEM_FENCE);
            uint pos = 2 * get_local_id(0) - (get_local_id(0) & (stride - 1));
            ComparatorLocalHier(&l_key[pos + 0], &l_key[pos + stride], ddd);
        }
    }

    //Odd / even arrays of localSizeLimit elements
    //sorted in opposite directions
    {
        uint ddd = (get_group_id(0) & 1);
        for(uint stride = localSizeLimit / 2; stride > 0; stride >>= 1)
        {
            barrier(CLK_LOCAL_MEM_FENCE);
            uint pos = 2 * get_local_id(0) - (get_local_id(0) & (stride - 1));
            ComparatorLocalHier(&l_key[pos + 0], &l_key[pos + stride], ddd);
        }
    }

    barrier(CLK_LOCAL_MEM_FENCE);
    pKey[                   0] = l_key[get_local_id(0) +                    0];
    pKey[(localSizeLimit / 2)] = l_key[get_local_id(0) + (localSizeLimit / 2)];
}

//Bitonic merge iteration for 'stride' >= LOCAL_SIZE_LIMIT

__kernel void kHierBitonicSortCellIdMergeGlobal(__global int2* pKey, uint arrayLength, uint size, uint stride, uint dir GUID_ARG)
{
    uint global_comparatorI = get_global_id(0);
    uint        comparatorI = global_comparatorI & (arrayLength / 2 - 1);

    //Bitonic merge
    uint ddd = dir ^ ( (comparatorI & (size / 2)) != 0 );
    uint pos = 2 * global_comparatorI - (global_comparatorI & (stride - 1));

    int2 keyA = pKey[pos +      0];
    int2 keyB = pKey[pos + stride];

    ComparatorPrivateHier(&keyA, &keyB, ddd);

    pKey[pos +      0] = keyA;
    pKey[pos + stride] = keyB;
}

//Combined bitonic merge steps for
//'size' > LOCAL_SIZE_LIMIT and 'stride' = [1 .. LOCAL_SIZE_LIMIT / 2]
__kernel void kHierBitonicSortCellIdMergeLocal(__global int2* pKey, uint arrayLength, uint stride, uint size, uint dir GUID_ARG)
{
//    __local int2 l_key[LOCAL_SIZE_MAX];
    __local int2 l_key[1024];
    int localSizeLimit = get_local_size(0) * 2;

    pKey += get_group_id(0) * localSizeLimit + get_local_id(0);
    l_key[get_local_id(0) +                    0] = pKey[                   0];
    l_key[get_local_id(0) + (localSizeLimit / 2)] = pKey[(localSizeLimit / 2)];

    //Bitonic merge
    uint comparatorI = get_global_id(0) & ((arrayLength / 2) - 1);
    uint         ddd = dir ^ ( (comparatorI & (size / 2)) != 0 );
    for(; stride > 0; stride >>= 1)
    {
        barrier(CLK_LOCAL_MEM_FENCE);
        uint pos = 2 * get_local_id(0) - (get_local_id(0) & (stride - 1));
        ComparatorLocalHier(&l_key[pos + 0], &l_key[pos + stride], ddd);
    }

    barrier(CLK_LOCAL_MEM_FENCE);
    pKey[                   0] = l_key[get_local_id(0) +                    0];
    pKey[(localSizeLimit / 2)] = l_key[get_local_id(0) + (localSizeLimit / 2)];
}


);