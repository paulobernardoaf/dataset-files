#if !defined(REFS_PACKED_BACKEND_H)
#define REFS_PACKED_BACKEND_H

struct ref_transaction;










struct ref_store *packed_ref_store_create(const char *path,
unsigned int store_flags);






int packed_refs_lock(struct ref_store *ref_store, int flags, struct strbuf *err);

void packed_refs_unlock(struct ref_store *ref_store);
int packed_refs_is_locked(struct ref_store *ref_store);







int is_packed_transaction_needed(struct ref_store *ref_store,
struct ref_transaction *transaction);

#endif 
