#if !defined(PDB_DOWNLOADER_H)
#define PDB_DOWNLOADER_H

#include <r_types.h>
#include <r_core.h>

#if defined(__cplusplus)
extern "C" {
#endif

typedef struct SPDBOptions {
char *user_agent;
char *symbol_server;
char *symbol_store_path;
ut64 extract;
} SPDBOptions;

typedef struct SPDBDownloaderOpt {
char *user_agent;
char *symbol_server;
char *dbg_file;
char *guid;
char *symbol_store_path;
ut64 extract;
} SPDBDownloaderOpt;

typedef struct SPDBDownloader {
SPDBDownloaderOpt *opt;

int (*download)(struct SPDBDownloader *pdb_downloader);
} SPDBDownloader;






void init_pdb_downloader(SPDBDownloaderOpt *opt, SPDBDownloader *pdb_downloader);





void deinit_pdb_downloader(SPDBDownloader *pdb_downloader);



R_API int r_bin_pdb_download(RCore *core, int isradjson, int *actions_done, SPDBOptions *options);

#if defined(__cplusplus)
}
#endif

#endif
