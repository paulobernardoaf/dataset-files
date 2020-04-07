void
grub_reed_solomon_add_redundancy (void *buffer, grub_size_t data_size,
grub_size_t redundancy);
void
grub_reed_solomon_recover (void *buffer, grub_size_t data_size,
grub_size_t redundancy);
