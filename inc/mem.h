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
*  @file    mem.h
*  @author  Hamid Reza Mehrabian
*  @version 1.0
*
*  @brief A fast and lightweight block memory management
*
*******************************************************************************/

#ifndef SRC_INCLUDE_MEM_H_
#define SRC_INCLUDE_MEM_H_

#define	MEM_CALC_BUFF_SIZE(block_size, block_cnt)	((block_size * block_cnt) + sizeof(mem_slot_t) * block_cnt)
#define	MEM_TABLE_REC_UNUSED_ID		0
#define	MEM_CALC_BLOCKS(block_size, s)	((s - 1) / block_size + 1)
#define	MEM_CALC_ACTUAL_SIZE(block_size, s)	(((s - 1) / block_size + 1) * block_size)
#define	MEM_GET_SLOT_ID(x)		((x->slot_id[2] << 16) | (x->slot_id[1] << 8) | (x->slot_id[0]))
#define	MEM_SET_SLOT_ID(x, id)		(x)->slot_id[0] = (id) & 0x000000FF;\
	(x)->slot_id[1] = (((id) & 0x0000FF00) >> 8);\
	(x)->slot_id[2] = (((id) & 0x00FF0000) >> 16)
#define	MEM_IS_FREE_SLOT(x)	(*(uint32_t *)x == 0)
#define	MEM_ERROR		(~0)
#define	MEM_MAX_SLOT_ID	0x01000000
#define	MEM_MAX_READY_NEXT_SLOTS	16
#define	MEM_UNASSIGNED	(~0)


typedef struct
{
	uint8_t		slot_id[3];
	uint8_t		count;
}mem_slot_t;

typedef struct
{
	size_t	slot_id;
	size_t	blocks_num;
	size_t	blocks_length;
	void		*heaps_start_ptr;
	mem_slot_t		*heaps_table_ptr;
	size_t	free_slots_num;
	size_t next_free_slots[MEM_MAX_READY_NEXT_SLOTS];
}mem_ctx_t;

#endif /* SRC_INCLUDE_MEM_H_ */
