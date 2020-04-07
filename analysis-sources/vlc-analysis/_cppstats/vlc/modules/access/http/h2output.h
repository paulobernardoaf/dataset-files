struct vlc_h2_output;
struct vlc_h2_frame;
struct vlc_tls;
int vlc_h2_output_send_prio(struct vlc_h2_output *, struct vlc_h2_frame *);
int vlc_h2_output_send(struct vlc_h2_output *, struct vlc_h2_frame *);
struct vlc_h2_output *vlc_h2_output_create(struct vlc_tls *, bool client);
void vlc_h2_output_destroy(struct vlc_h2_output *);
