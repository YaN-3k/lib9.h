/* #include <stddef.h> */

#define GOLDEN_RATIO_32 0x61C88647u
#define GOLDEN_RATIO_64 0x61C8864680B583EBul

#define hash_32(val, bits) (val * GOLDEN_RATIO_32 >> (32 - bits))
#define hash_64(val, bits) (val * GOLDEN_RATIO_64 >> (64 - bits))

#define hash(val, bits) \
        (sizeof(val) <= 4 ? hash_32(val, bits) : hash_64(val, bits))

#define HASHTABLE_INITIALIZER {0}
#define DECLARE_HASHTABLE(name, bits) struct hlist_head name[1 << (bits)]
#define DEFINE_HASHTABLE(name, bits) DECLARE_HASHTABLE(name, bits) = {0}

#define HASH_SIZE(name) nelem(name)
#define HASH_BITS(name) (ffs(HASH_SIZE(name)) - 1)

void hash_init_(struct hlist_head *ht, size_t sz);
int hash_empty_(struct hlist_head *ht, size_t sz);
#define hash_init(ht) hash_init_(ht, HASH_BITS(ht))
#define hash_add(ht, node, key) \
	hlist_add_head(node, &ht[hash(key, HASH_BITS(ht))])
#define hash_del(node) hlist_del_init(node)
#define hash_hashed(node) !hlist_unhashed(node)
#define hash_empty(ht) hash_empty_(ht, HASH_BITS(sz))

#define hash_for_each(ht, bkt, obj, type, member)                                   \
        for (bkt = 0, obj = NULL; !(obj) && (size_t)(bkt) < HASH_SIZE(ht); (bkt)++) \
                hlist_for_each_entry(obj, &ht[bkt], type, member)

#define hash_for_each_safe(ht, bkt, tmp, obj, type, member)                         \
        for (bkt = 0, obj = NULL; !(obj) && (size_t)(bkt) < HASH_SIZE(ht); (bkt)++) \
                hlist_for_each_entry_safe(obj, tmp, &ht[bkt], member)

#define hash_for_each_possible(ht, obj, type, member, key)                    \
        hlist_for_each_entry(obj, &ht[hash(key, HASH_BITS(ht))], type, member)

#define hash_for_each_possible_safe(ht, obj, n, t, m, key)                    \
        hlist_for_each_entry_safe(obj, n, &ht[hash(key, HASH_BITS(ht))], t, m)

#ifdef HASH_IMPL

void
hash_init_(struct hlist_head *ht, size_t sz)
{
	size_t i;

	for (i = 0; i < sz; i++)
		INIT_HLIST_HEAD(ht + i);
}

int
hash_empty_(struct hlist_head *ht, size_t sz)
{

	size_t i;

	for (i = 0; i < sz; i++)
		if (!hlist_empty(ht + i))
			return 0;
	return 1;
}

#endif
