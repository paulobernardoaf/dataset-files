static const char *page_index =
"<html>"
"<head>"
"<title>r2agent</title>"
"<script>"
"function file_open() {"
" var file = window.prompt ('path to file?');"
" window.open ('/file/open/'+file);"
"}"
"</script>"
"</head>"
"<body>"
" <h2>r2agent</h2>"
" <hr size=1 />"
"File<br />"
" - <a href='javascript:file_open()'>open</a><br />"
" - <a href='/file/list'>list</a><br />"
" - push<br />"
" - pull<br />"
" - delete<br />"
" - chmod<br />"
"Processes<br />"
" - list<br />"
"Sessions<br />"
" - list<br />"
"</body>"
"</html>";
