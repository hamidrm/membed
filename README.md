
# membed

A fast and lightweight block memory management for embedded systems. In each search for new free slot, it will cache multiple instance to allocate future memory required without wasting time.

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

