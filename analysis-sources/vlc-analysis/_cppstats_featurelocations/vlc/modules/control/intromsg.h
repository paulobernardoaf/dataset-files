



















static inline void intf_consoleIntroMsg(intf_thread_t *p_intf)
{
if (getenv( "PWD" ) == NULL) 
{
AllocConsole();
freopen("CONOUT$", "w", stdout);
freopen("CONOUT$", "w", stderr);
freopen("CONIN$", "r", stdin);
}

msg_rc("VLC media player - %s", VERSION_MESSAGE);
msg_rc("%s", COPYRIGHT_MESSAGE);
msg_rc(_("\nWarning: if you cannot access the GUI "
"anymore, open a command-line window, go to the "
"directory where you installed VLC and run "
"\"vlc -I qt\"\n"));
}
