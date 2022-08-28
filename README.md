
# membed

A lightweight and fast block memory management library for embedded systems. Every time it is seeking a free slot, it will cache multiple instances to immediately allocate future slots that will be requested.

Sample Usage:
```C
mem_ctx_t mem;
void *a_ptr;
uint8_t heap_buff[MEM_CALC_BUFF_SIZE(32,256)]; //32 Slot x 256 Buffer
mem_create(&mem, heap_buff, 32, 256);
a_ptr = mem_alloc(&mem, 1600);
//Code for using a_ptr
mem_free(&mem, a_ptr);
```

