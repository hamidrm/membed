/*
MIT License

Copyright (c) 2020 Hamid Reza Mehrabian

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*******************************************************************************
*  @file    mem.c
*  @author  Hamid Reza Mehrabian
*  @version 1.0
*
*  @brief A fast and lightweight block memory management
*
*******************************************************************************/

#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <memory.h>

#include "include/mem.h"


static size_t	mem_find_free_slot(mem_ctx_t	*mem_ctx, size_t	req_size)
{
	uint32_t	mem_req_num_blocks = MEM_CALC_BLOCKS(mem_ctx->blocks_length, req_size);
	mem_slot_t *heap_table_slot = mem_ctx->heaps_table_ptr;
	uint32_t	free_slots = 0;
	uint32_t	slot_number = 0;
	uint32_t	iter_free_slot = mem_ctx->next_free_slots[0]; /*  Next ready single slot */
	uint32_t	start_slot_index = 0;
	if(iter_free_slot == MEM_UNASSIGNED)
		iter_free_slot = 0;
	while (1)
	{
		while (!MEM_IS_FREE_SLOT(&heap_table_slot[iter_free_slot]))
		{
			slot_number++;
			iter_free_slot++;
			iter_free_slot %= mem_ctx->blocks_num;
			if (slot_number == mem_ctx->blocks_num) {
				/* TODO-Raise an error. Not Enough Memory */

				return  MEM_ERROR;
			}
		}
		start_slot_index = iter_free_slot;
		while (MEM_IS_FREE_SLOT(&heap_table_slot[iter_free_slot]))
		{
			iter_free_slot++;
			iter_free_slot %= mem_ctx->blocks_num;
			free_slots++;
			if (free_slots == mem_req_num_blocks)
				/* Free slot found! */
				return start_slot_index;
			slot_number++;
			if (slot_number == mem_ctx->blocks_num)
				/* Free slot not found! */
				return MEM_ERROR;
		}
		if(free_slots != 0 && free_slots < MEM_MAX_READY_NEXT_SLOTS)
		{
			mem_ctx->next_free_slots[free_slots] = start_slot_index;
		}
		/* Failed to find required free slot on this chunk, check others */
		free_slots = 0;
	}

	/* Never reaches here */
	return MEM_ERROR;
}

static size_t	mem_get_heap_size(mem_ctx_t *mem_ctx, void *addr)
{
	size_t	slot_index =  (uint32_t)((addr - mem_ctx->heaps_start_ptr) / mem_ctx->blocks_length);

	if(slot_index == 0 || slot_index > mem_ctx->blocks_num)
		/* Invalid heap address */
		return 0;

	return mem_ctx->heaps_table_ptr[slot_index].count * mem_ctx->blocks_length;
}

mem_ctx_t	*mem_create(mem_ctx_t	*mem_ctx, void *heap_ptr, size_t block_size, size_t blocks_num)
{
	uint32_t	iter = 0;
	if(mem_ctx == NULL)
		return NULL;
	mem_ctx->blocks_num= blocks_num;
	mem_ctx->blocks_length = block_size;
	mem_ctx->heaps_table_ptr = heap_ptr;
	mem_ctx->slot_id = 1; /* First heap ID */
	mem_ctx->heaps_start_ptr = (uint8_t *)heap_ptr + sizeof(mem_slot_t) * blocks_num;
	mem_ctx->free_slots_num = blocks_num;
	/* Prepare heaps table  */
	memset(heap_ptr, 0, sizeof(mem_slot_t) * blocks_num);

	/* Prepare ready slots index */
	mem_ctx->next_free_slots[0] = 0;
	mem_ctx->next_free_slots[1] = 1;
	for(iter=2;iter<MEM_MAX_READY_NEXT_SLOTS;iter++)
		mem_ctx->next_free_slots[iter] = iter+mem_ctx->next_free_slots[iter-1]-1;

	return mem_ctx;
}

void	*mem_alloc(mem_ctx_t	* ctx, size_t	size)
{
	size_t num_of_blocks;
	mem_slot_t *free_slot;
	size_t	free_slot_index;
	size_t	iter;
	num_of_blocks = MEM_CALC_BLOCKS(ctx->blocks_length, size);
	/*Does it greater than entire blocks number? */
	if(num_of_blocks > ctx->free_slots_num)
		return NULL;

	if(num_of_blocks == 0)
		num_of_blocks = 1;

	/* Find required free slot */
	if(num_of_blocks == 1 && ctx->next_free_slots[0] != MEM_UNASSIGNED)
	{
		free_slot_index = ctx->next_free_slots[0];
		ctx->next_free_slots[0] = MEM_UNASSIGNED;
	}
	else if(num_of_blocks < MEM_MAX_READY_NEXT_SLOTS && ctx->next_free_slots[num_of_blocks] != MEM_UNASSIGNED)
	{
		free_slot_index = ctx->next_free_slots[num_of_blocks];
		ctx->next_free_slots[num_of_blocks] = MEM_UNASSIGNED;
	}else
		free_slot_index = mem_find_free_slot(ctx, size);

	/* Was it  found? */
	if(free_slot_index == MEM_ERROR)
		return NULL;
	free_slot = &ctx->heaps_table_ptr[free_slot_index];



	free_slot->count = num_of_blocks;

	/* Set slot id */
	for(iter=0;iter<num_of_blocks;iter++)
	{
		MEM_SET_SLOT_ID(&free_slot[iter], ctx->slot_id);
	}
	ctx->slot_id++;
	if(ctx->slot_id == MEM_MAX_SLOT_ID)
		ctx->slot_id=1;

	ctx->free_slots_num -= num_of_blocks;
	return (uint8_t *)ctx->heaps_start_ptr + free_slot_index * ctx->blocks_length;
}

void	mem_free(mem_ctx_t	* ctx, void *addr)
{
	size_t	slot_index =  (uint32_t)((addr - ctx->heaps_start_ptr) / ctx->blocks_length);
	size_t	slot_size;
	mem_slot_t	*heap_slot;

	if(slot_index == 0 || slot_index > ctx->blocks_num)
		/* Invalid heap address */
		return;

	slot_size = ctx->heaps_table_ptr[slot_index].count;
	if(slot_size < MEM_MAX_READY_NEXT_SLOTS)
		ctx->next_free_slots[slot_size] = slot_index;
	heap_slot = &ctx->heaps_table_ptr[slot_index];
	memset(heap_slot, 0,  sizeof(mem_slot_t) * slot_size);
	ctx->free_slots_num += slot_size;
}

void	mem_realloc(mem_ctx_t	* ctx, void *addr, size_t size)
{
	void	*new_heap_addr = mem_alloc(ctx, size);
	size_t	old_heap_size = mem_get_heap_size(ctx, addr);
	if(old_heap_size == 0)
		/* Invalid Address */
		return;
	memcpy(new_heap_addr, addr, old_heap_size);
	mem_free(ctx, addr);
}

size_t	mem_get_free_size(mem_ctx_t	* ctx)
{
	return ctx->free_slots_num * ctx->blocks_length;
}
