





















#if !defined(CYCLIC_BUFFER_H)
#define CYCLIC_BUFFER_H


class block_holder
{
public:
float data[512]; 
vlc_tick_t pts; 
block_holder()
{
pts = 0; 
}
};

#define CYCLIC_BUFFER_SIZE 128
class cyclic_block_queue
{
block_holder cycl_buffer[CYCLIC_BUFFER_SIZE];
size_t consumer_pos;
size_t insertion_pos;
public:
cyclic_block_queue()
{
consumer_pos = 0;
insertion_pos = 0;
}

block_holder* consume()
{
vlc_tick_t cur_machine_time = vlc_tick_now();
size_t steps = 0;
while (
(cycl_buffer[consumer_pos].pts < cur_machine_time)
&&
(steps++ < CYCLIC_BUFFER_SIZE)
)
{
consumer_pos++;
if (consumer_pos == CYCLIC_BUFFER_SIZE)
{
consumer_pos = 0;
}
}
return &cycl_buffer[consumer_pos];
}

block_holder* get_insertion_object()
{
insertion_pos++;
if ( insertion_pos == CYCLIC_BUFFER_SIZE )
{
insertion_pos = 0;
}
return &cycl_buffer[insertion_pos];
}

void reset()
{
for (size_t i = 0; i < CYCLIC_BUFFER_SIZE; i++)
{
cycl_buffer[i].pts = 0;
consumer_pos = 0;
insertion_pos = 0;
}
}
};
#undef CYCLIC_BUFFER_SIZE

#endif 
