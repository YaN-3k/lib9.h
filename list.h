/* #include <stddef.h> */

struct list_head {
	struct list_head *next, *prev;
};

#define LIST_HEAD_INIT(name) { &(name), &(name) }
#define LIST_HEAD(name) struct list_head name = LIST_HEAD_INIT(name)
#define INIT_LIST_HEAD(list) (list)->next = (list)->prev = (list)

void list_add(struct list_head *new, struct list_head *head);
void list_add_tail(struct list_head *new, struct list_head *head);
void list_del(struct list_head *entry);
void list_del_init(struct list_head *entry);
void list_replace(struct list_head *old, struct list_head *new);
void list_replace_init(struct list_head *old, struct list_head *new);
void list_swap(struct list_head *entry1, struct list_head *entry2);
void list_move(struct list_head *list, struct list_head *head);
void list_move_tail(struct list_head *list, struct list_head *head);

int list_empty(const struct list_head *head);
int list_is_first(const struct list_head *list, const struct list_head *head);
int list_is_last(const struct list_head *list, const struct list_head *head);
int list_is_head(const struct list_head *list, const struct list_head *head);
int list_is_singular(const struct list_head *list);

void list_rotate_left(struct list_head *head);
void list_rotate_to_front(struct list_head *list, struct list_head *head);
void list_cut_position(struct list_head *list,
                       struct list_head *head,
                       struct list_head *entry);
void list_cut_before(struct list_head *list,
                     struct list_head *head,
                     struct list_head *entry);
void list_splice(const struct list_head *list, struct list_head *head);
void list_splice_tail(struct list_head *list, struct list_head *head);
void list_splice_init(struct list_head *list, struct list_head *head);
void list_splice_tail_init(struct list_head *list, struct list_head *head);

#define list_entry(ptr, type, member)                                         \
        (type *)((char *)(ptr) - offsetof(type, member))

#define list_first_entry(ptr, type, member)                                   \
        list_entry((ptr)->next, type, member)

#define list_first_entry_or_null(ptr, type, member)                           \
        (!list_empty(ptr) ? list_entry((ptr)->next, type, member) : NULL)

#define list_last_entry(ptr, type, member)                                    \
        list_entry((ptr)->prev, type, member)

#define list_next_entry(pos, type, member)                                    \
        list_entry((pos)->member.next, type, member)

#define list_prev_entry(pos, type, member)                                    \
        list_entry((pos)->member.prev, type, member)

#define list_for_each(pos, head)                                              \
        for (pos = (head)->next; !list_is_head(pos, head); pos = pos->next)

#define list_for_each_continue(pos, head)                                     \
	for (pos = pos->next; !list_is_head(pos, (head)); pos = pos->next)

#define list_for_each_prev(pos, head)                                         \
        for (pos = (head)->prev; !list_is_head(pos, head); pos = pos->prev)

#define list_for_each_safe(pos, tmp, head)                                    \
        for (pos = (head)->next, tmp = pos->next;                             \
             !list_is_head(pos, head);                                        \
             pos = tmp, tmp = pos->next)

#define list_for_each_prev_safe(pos, tmp, head)                               \
        for (pos = (head)->prev, tmp = pos->prev;                             \
             !list_is_head(pos, (head)); \
             pos = tmp, tmp = pos->prev)

#define list_entry_is_head(pos, head, member)                                 \
        (&pos->member == (head))

#define list_for_each_entry(pos, head, type, member)                          \
        for (pos = list_first_entry(head, type, member);                      \
             !list_entry_is_head(pos, head, member);                          \
             pos = list_next_entry(pos, type, member))

#define list_for_each_entry_reverse(pos, head, type, member)                  \
        for (pos = list_last_entry(head, type, member);                       \
             !list_entry_is_head(pos, head, member);                          \
             pos = list_prev_entry(pos, type, member))

#define list_prepare_entry(pos, head, type, member)                           \
	((pos) ? (pos) : list_entry(head, type, member))

#define list_for_each_entry_continue(pos, head, type, member)                 \
        for (pos = list_next_entry(pos, type, member);                        \
             !list_entry_is_head(pos, head, member);                          \
             pos = list_next_entry(pos, type, member))

#define list_for_each_entry_continue_reverse(pos, head, type, member)         \
        for (pos = list_prev_entry(pos, type, member);                        \
             !list_entry_is_head(pos, head, member);                          \
             pos = list_prev_entry(pos, type, member))

#define list_for_each_entry_from(pos, head, type, member)                     \
        for (; !list_entry_is_head(pos, head, member);                        \
             pos = list_next_entry(pos, type, member))

#define list_for_each_entry_from_reverse(pos, head, type, member)             \
        for (; !list_entry_is_head(pos, head, member);                        \
             pos = list_prev_entry(pos, type, member))

#define list_for_each_entry_safe(pos, tmp, head, type, member)                \
        for (pos = list_first_entry(head, type, member),                      \
             tmp = list_next_entry(pos, type, member);                        \
             !list_entry_is_head(pos, head, member);                          \
             pos = tmp, tmp = list_next_entry(tmp, type, member))

#define list_for_each_entry_safe_continue(pos, tmp, head, type, member)       \
        for (pos = list_next_entry(pos, type, member),                        \
             tmp = list_next_entry(pos, type, member);                        \
             !list_entry_is_head(pos, head, member);                          \
             pos = tmp, tmp = list_next_entry(tmp, type, member))

#define list_for_each_entry_safe_from(pos, tmp, head, type, member)           \
        for (tmp = list_next_entry(pos, type, member);                        \
             !list_entry_is_head(pos, head, member);                          \
             pos = tmp, tmp = list_next_entry(tmp, type, member))

#define list_for_each_entry_safe_reverse(pos, tmp, head, type, member)        \
        for (pos = list_last_entry(head, type, member),                       \
             tmp = list_prev_entry(pos, type, member);                        \
             !list_entry_is_head(pos, head, member);                          \
             pos = tmp, tmp = list_prev_entry(tmp, type, member))

#define list_safe_reset_next(pos, tmp, type, member)                          \
        tmp = list_next_entry(pos, type, member)


struct hlist_head {
	struct hlist_node *first;
};

struct hlist_node {
	struct hlist_node *next, **pprev;
};

#define HLIST_HEAD_INIT { .first = NULL }
#define HLIST_HEAD(name) struct hlist_head name = {  .first = NULL }
#define INIT_HLIST_HEAD(ptr) ((ptr)->first = NULL)
#define INIT_HLIST_NODE(node) do {                                            \
	(node)->next = NULL;                                                  \
	(node)->pprev = NULL;                                                 \
} while (0)

int hlist_unhashed(const struct hlist_node *h);
int hlist_empty(const struct hlist_head *h);
void hlist_del(struct hlist_node *n);
void hlist_del_init(struct hlist_node *n);
void hlist_add_head(struct hlist_node *n, struct hlist_head *h);
void hlist_add_before(struct hlist_node *n, struct hlist_node *next);
void hlist_add_behind(struct hlist_node *n, struct hlist_node *prev);
int hlist_is_singular_node(struct hlist_node *n, struct hlist_head *h);

#define hlist_entry(ptr, type, member) list_entry(ptr, type, member)

#define hlist_for_each(pos, head)                                             \
        for (pos = (head)->first; pos; pos = pos->next)

#define hlist_for_each_safe(pos, tmp, head)                                   \
        for (pos = (head)->first; pos && (tmp = pos->next, 1); pos = tmp)

#define hlist_entry_safe(ptr, type, member)                                   \
        (ptr ? hlist_entry(ptr, type, member) : NULL)

#define hlist_for_each_entry(pos, head, type, member)                         \
        for (pos = hlist_entry_safe((head)->first, type, member); pos;        \
             pos = hlist_entry_safe((pos)->member.next, type, member))

#define hlist_for_each_entry_continue(pos, type, member)                      \
        for (pos = hlist_entry_safe((pos)->member.next, type, member); pos;   \
             pos = hlist_entry_safe((pos)->member.next, type, member))

#define hlist_for_each_entry_from(pos, type, member)                          \
        for (; pos; pos = hlist_entry_safe((pos)->member.next, type, member))

#define hlist_for_each_entry_safe(pos, tmp, head, type, member)               \
        for (pos = hlist_entry_safe((head)->first, type, member);             \
             pos && (tmp = pos->member.next, 1);                              \
             pos = hlist_entry_safe(tmp, type, member))                       \

#ifdef LIST_IMPL

static void
list_add_(struct list_head *new,
          struct list_head *prev,
          struct list_head *next)
{
	next->prev = new;
	new->next = next;
	new->prev = prev;
	prev->next = new;
}

void
list_add(struct list_head *new, struct list_head *head)
{
	list_add_(new, head, head->next);
}

void
list_add_tail(struct list_head *new, struct list_head *head)
{
	list_add_(new, head->prev, head);
}

static void
list_del_(struct list_head *prev, struct list_head *next)
{
	next->prev = prev;
	prev->next = next;
}

void
list_del(struct list_head *entry)
{
	list_del_(entry->prev, entry->next);
	entry->next = entry->prev = NULL;
}

void
list_del_init(struct list_head *entry)
{
	list_del(entry);
	INIT_LIST_HEAD(entry);
}

void
list_replace(struct list_head *old, struct list_head *new)
{
	new->next = old->next;
	new->next->prev = new;
	new->prev = old->prev;
	new->prev->next = new;
}

void
list_replace_init(struct list_head *old, struct list_head *new)
{
	list_replace(old, new);
	INIT_LIST_HEAD(old);
}

void
list_swap(struct list_head *entry1, struct list_head *entry2)
{
	struct list_head *pos;

	pos = entry2->prev;
	list_del(entry2);
	list_replace(entry1, entry2);
	if (pos == entry1) pos = entry2;
	list_add(entry1, pos);
}

void
list_move(struct list_head *list, struct list_head *head)
{
	list_del(list);
	list_add(list, head);
}

void
list_move_tail(struct list_head *list, struct list_head *head)
{
	list_del(list);
	list_add_tail(list, head);
}

int
list_empty(const struct list_head *head)
{
	return head->next == head;
}

int
list_is_first(const struct list_head *list, const struct list_head *head)
{
	return list->prev == head;
}

int
list_is_last(const struct list_head *list, const struct list_head *head)
{
	return list->next == head;
}

int
list_is_head(const struct list_head *list, const struct list_head *head)
{
	return list == head;
}

int
list_is_singular(const struct list_head *head)
{
	return !list_empty(head) && (head->next == head->prev);
}

void
list_rotate_left(struct list_head *head)
{
	if (!list_empty(head)) list_move_tail(head->next, head);

}

void
list_rotate_to_front(struct list_head *list, struct list_head *head)
{
	list_move_tail(head, list);
}

static void
list_cut_position_(struct list_head *list,
                   struct list_head *head,
                   struct list_head *entry)
{
	struct list_head *new_first = entry->next;
	list->next = head->next;
	list->next->prev = list;
	list->prev = entry;
	entry->next = list;
	head->next = new_first;
	new_first->prev = head;
}

void
list_cut_position(struct list_head *list,
                  struct list_head *head,
                  struct list_head *entry)
{
	if (list_empty(head))
		return;
	if (list_is_singular(head) && !list_is_head(entry, head) && (entry != head->next))
		return;
	if (list_is_head(entry, head))
		INIT_LIST_HEAD(list);
	else
		list_cut_position_(list, head, entry);
}

void
list_cut_before(struct list_head *list,
                struct list_head *head,
                struct list_head *entry)
{
	if (head->next == entry) {
		INIT_LIST_HEAD(list);
		return;
	}
	list->next = head->next;
	list->next->prev = list;
	list->prev = entry->prev;
	list->prev->next = list;
	head->next = entry;
	entry->prev = head;
}

static void
list_splice_(const struct list_head *list,
             struct list_head *prev,
             struct list_head *next)
{
	struct list_head *first;
	struct list_head *last;

        first = list->next;
        last = list->prev;

	first->prev = prev;
	prev->next = first;

	last->next = next;
	next->prev = last;
}

void
list_splice(const struct list_head *list, struct list_head *head)
{
	if (!list_empty(list)) list_splice_(list, head, head->next);
}

void
list_splice_tail(struct list_head *list,
                struct list_head *head)
{
	if (!list_empty(list)) list_splice_(list, head->prev, head);
}

void
list_splice_init(struct list_head *list, struct list_head *head)
{
	if (!list_empty(list)) {
		list_splice_(list, head, head->next);
		INIT_LIST_HEAD(list);
	}
}


void
list_splice_tail_init(struct list_head *list, struct list_head *head)
{
	if (!list_empty(list)) {
		list_splice_(list, head->prev, head);
		INIT_LIST_HEAD(list);
	}
}

int
hlist_unhashed(const struct hlist_node *h)
{
	return !h->pprev;
}

int
hlist_empty(const struct hlist_head *h)
{
	return !h->first;
}

static void
hlist_del_(struct hlist_node *n)
{
	struct hlist_node *next;
	struct hlist_node **pprev;

	next = n->next;
	pprev = n->pprev;

	*pprev = next;
	if (next) next->pprev = pprev;
}

void
hlist_del(struct hlist_node *n)
{
	hlist_del_(n);
	n->next = NULL;
	n->pprev = NULL;
}

void
hlist_del_init(struct hlist_node *n)
{
	if (!hlist_unhashed(n)) {
		hlist_del_(n);
		INIT_HLIST_NODE(n);
	}
}

void
hlist_add_head(struct hlist_node *n, struct hlist_head *h)
{
	struct hlist_node *first;

	first = h->first;

	n->next = first;
	if (first) first->pprev = &n->next;
	h->first = n;
	n->pprev = &h->first;
}

void
hlist_add_before(struct hlist_node *n, struct hlist_node *next)
{
	n->pprev = next->pprev;
	n->next = next;
	next->pprev = &n->next;
	*n->pprev = n;
}

void
hlist_add_behind(struct hlist_node *n, struct hlist_node *prev)
{
	n->next = prev->next;
	prev->next = n;
	n->pprev = &prev->next;
	if (n->next) n->next->pprev = &n->next;
}

int
hlist_is_singular_node(struct hlist_node *n, struct hlist_head *h)
{
	return !n->next && n->pprev == &h->first;
}

void
hlist_move_list(struct hlist_head *old, struct hlist_head *new)
{
	new->first = old->first;
	if (new->first)
		new->first->pprev = &new->first;
	old->first = NULL;
}

#endif
