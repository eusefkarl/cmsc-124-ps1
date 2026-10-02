/*
 * dt_map.c: Associative arrays for Unit 5, Section E.
 *
 * An array does not store its indices. This map stores its keys.
 * An array calculates a position with one subtraction.
 * The map calculates a hash and then compares keys in one bucket.
 *
 * Hashing turns the key into a bucket number. Compare all keys in that bucket
 * because two keys can select it. Use a linked list for each bucket. Start the
 * unsigned accumulator at 14695981039346656037ULL. For each unsigned byte,
 * exclusive-or the byte into it and multiply by 1099511628211ULL.
 *
 * A separate list stores insertion order for stable output. dt_map_key_at
 * reads this list. Updating a key preserves its position. Removing and
 * reinserting a key moves it to the end.
 */

#include "dt.h"

#include <stdlib.h>
#include <string.h>

const size_t INITIAL_BUCKET_COUNT = 20;

struct dt_map {
    /* TODO: Add the buckets and insertion-order data. */
    struct dt_map_entry **buckets; /*array of bucket heads*/
    char **order; /*insertion of bucket keys*/
    size_t count; /*number of entries*/
    size_t capacity; /*number of buckets*/
    size_t bucket_count;
};
struct dt_map_entry{
    char *key;
    dt_value value;
    size_t bucket_count;
    struct dt_map_entry *next; /*holds the chain*/
};

/*
 * dt_map_new builds an empty map. It returns NULL after an allocation failure.
 */
dt_map *dt_map_new(void)
{
    /* TODO: Return an allocated empty map. Return NULL after an allocation failure.
       dt_map_new()  -> a map whose dt_map_len is 0
       cases/normal/map_basics.case */
    dt_map *m = malloc(sizeof(dt_map));
    //allocation check
    if (m == NULL){
        free(m);
        return NULL;
    }
    m->buckets = malloc(INITIAL_BUCKET_COUNT * sizeof(struct dt_map_entry *));
    //allocation check
    if (m->buckets == NULL){
        free(m->buckets);
        free(m);
        return NULL;
    }
    for (size_t i = 0; i < INITIAL_BUCKET_COUNT; i++){
        m->buckets[i] = NULL;
    }

    m->order = NULL;
    m->count = 0;
    m->capacity = 0;
    m->bucket_count = INITIAL_BUCKET_COUNT;

    return m;
}

/*
 * dt_map_free releases each entry, copied key, order array, and map.
 * It accepts NULL. The environment owns the values.
 */
void dt_map_free(dt_map *m)
{
    /* TODO: Release each entry, copied key, order array, and map.
       Preserve the values. The environment owns them.
       a map holding a string value  -> the nodes and keys go, the string stays
       dt_map_free(NULL)             -> returns, having done nothing
       cases/cleanup/map_churn.case */
    if (m == NULL){
        return;
    }
}

/*
 * dt_map_len returns the number of keys in constant time.
 */
size_t dt_map_len(const dt_map *m)
{
    /* TODO: Return the current key count.
       Replacing a value does not change this count.
       after put alpha, beta, gamma:  dt_map_len(m) -> 3
       after put beta again:          dt_map_len(m) -> 3, still
       after del alpha:               dt_map_len(m) -> 2
       cases/normal/map_basics.case */
    if (m == NULL){
        return 0;
    }
    return m->count;
}

/*
 * dt_map_put binds v to key.
 * An existing key keeps its insertion position. A new key becomes the last key.
 * Copy each new key because the caller owns the source buffer.
 * Return DT_ERR_CAPACITY after an allocation failure.
 */
dt_status dt_map_put(dt_map *m, const char *key, dt_value v)
{
    /* TODO: Replace the value for an existing key.
       Add a new entry for a new key. Copy each new key.
       Hash the key. Select its bucket. Search the bucket chain.
       Add a new entry to the chain and insertion list.
       put "beta" -> 2 on an empty map    -> DT_OK, "beta" is last in order
       put "beta" -> 22 on that map       -> DT_OK, same position, new value
       an allocation failure              -> DT_ERR_CAPACITY, map unchanged
       cases/normal/map_basics.case */

    //get the hash
    unsigned long long h = 14695981039346656037ULL;
    for (const unsigned char *p = (const unsigned char *)key; *p != '\0'; p++) {
        h ^= (unsigned long long)*p;
        h *= 1099511628211ULL;
    }
    unsigned long long index = h % m->bucket_count;

    //check if order has items
    if (m->order == NULL){
        //allocate if null
        m->order = malloc(m->capacity * sizeof(char *));
        //check allocation
        if (m->order == NULL){
            free(m->order);
            return DT_ERR_CAPACITY;
        }
        //malloc key
        m->order[0] = malloc(sizeof(key)+1);
        //check allocation
        if (m->order[0] == NULL){
            free(m->order[0]);
            return DT_ERR_CAPACITY;
        }
        //insert at index 0
        strcpy(m->order[0], key);
        m->count = 1;
    }
    //check if key is in order array, insert if not found
    bool keyExists = false;
    for (size_t i = 0; i < m->count; i++){
        if (strcmp(m->order[i], key) == 0){
            keyExists = true;
        }
    }
    //key does not exist, add key to end of array
    if(!keyExists){
        //malloc key
        m->order[m->count] = malloc(sizeof(key)+1);
        //check allocation
        if (m->order[m->count] == NULL){
            free(m->order[m->count]);
            return DT_ERR_CAPACITY;
        }
        //insert at end of array and update count
        strcpy(m->order[m->count], key);
        m->count += 1;
    }




    //checks if key exists in map and reassigns value
    struct dt_map_entry *cursor = m->buckets[index];
    while(cursor != NULL){
        if (strcmp(cursor->key, key) == 0){
            cursor->value = v;
            return DT_OK;
        }
        cursor = cursor->next;
    }
    
    //no entry with key found at cursor, allocate new entry and add it to cursor
    struct dt_map_entry *entry = malloc(sizeof(struct dt_map_entry));
    //check allocation
    if (entry == NULL){
        free(entry);
        return DT_ERR_CAPACITY;
    }
    entry->key = malloc(sizeof(key)+1);
    //check allocation
    if (entry->key == NULL){
        free(entry->key);
        return DT_ERR_CAPACITY;
    }
    
    strcpy(entry->key, key);
    entry->value = v;
    entry->next = m->buckets[index];

    m->buckets[index] = entry;

    return DT_OK;
}

/*
 * dt_map_get writes the value for key to *out.
 * It returns DT_ERR_KEY and does not change *out when the key is absent.
 * An absent key differs from a nil value.
 */
dt_status dt_map_get(const dt_map *m, const char *key, dt_value *out)
{
    /* TODO: Return DT_ERR_KEY when the key is absent.
       Preserve *out after this error. A nil value can be present.
       after put "beta" -> 22:
         dt_map_get(m, "beta", &out)   -> DT_OK, *out is the integer 22
         dt_map_get(m, "ghost", &out)  -> DT_ERR_KEY, *out untouched
       cases/normal/map_basics.case, cases/boundary/map_missing_key.case */
    //get the hash
    unsigned long long h = 14695981039346656037ULL;
    for (const unsigned char *p = (const unsigned char *)key; *p != '\0'; p++) {
        h ^= (unsigned long long)*p;
        h *= 1099511628211ULL;
    }
    unsigned long long index = h % m->bucket_count;

    //checks if key exists in map and output value
    struct dt_map_entry *cursor = m->buckets[index];
    while(cursor != NULL){
        if (strcmp(cursor->key, key) == 0){
            *out = cursor->value;
            return DT_OK;
        }
        cursor = cursor->next;
    }
    //key doesn't exist in map
    return DT_ERR_KEY;
}

/*
 * dt_map_remove removes key from its bucket and insertion position.
 * It releases the copied key. It returns DT_ERR_KEY when the key is absent.
 */
dt_status dt_map_remove(dt_map *m, const char *key)
{
    /* TODO: Remove the entry from its bucket and insertion position.
       Release the copied key. Return DT_ERR_KEY when the key is absent.
       a map holding alpha, beta, gamma:
         dt_map_remove(m, "alpha")  -> DT_OK, order is now beta, gamma
         dt_map_remove(m, "ghost")  -> DT_ERR_KEY, nothing changes
       reinserting "alpha" appends it after "gamma"
       cases/normal/map_basics.case, cases/boundary/map_remove_missing_key.case */
    //get the hash
    unsigned long long h = 14695981039346656037ULL;
    for (const unsigned char *p = (const unsigned char *)key; *p != '\0'; p++) {
        h ^= (unsigned long long)*p;
        h *= 1099511628211ULL;
    }
    unsigned long long index = h % m->bucket_count;

    //checks if key exists in map and output value
    struct dt_map_entry *cursor = m->buckets[index];
    while(cursor != NULL){
        if (strcmp(cursor->key, key) == 0){
            //TODO
            return DT_OK;
        }
        cursor = cursor->next;
    }
    //key doesn't exist in map
    return DT_ERR_KEY;
}

/*
 * dt_map_key_at writes the key at insertion position index to *out.
 * It returns DT_ERR_RANGE and does not change *out for an invalid index.
 */
dt_status dt_map_key_at(const dt_map *m, size_t index, const char **out)
{
    /* TODO: Write the key at the specified insertion position to *out.
       Return DT_ERR_RANGE for an invalid position. Preserve *out after this error.
       The printer uses this order.
       a map holding alpha, beta, gamma:
         dt_map_key_at(m, 0, &out)  -> DT_OK, *out = "alpha"
         dt_map_key_at(m, 3, &out)  -> DT_ERR_RANGE, *out untouched
       cases/normal/map_basics.case */
    (void)m;
    (void)index;
    (void)out;
    return DT_ERR_RANGE;
}
