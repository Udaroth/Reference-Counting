#include "rc.h"
#include <stdio.h>
#include <string.h>

size_t MAX_ADDRESS = 0x7FFFFFFFFFFFFFFF;

// rc_entry array
struct strong_ref** ent_list;
size_t ent_capacity = 0;
size_t ent_size = 0;

void add_entry(struct strong_ref* entry){


	if(ent_capacity == 0){
		// Allocate space for one entry pointer
		ent_list = (struct strong_ref**)malloc(sizeof(struct strong_ref*));

		ent_capacity = 1;
	} else if(ent_size == ent_capacity){
		// Allocate double the amount of space

		ent_list = (struct strong_ref**)realloc(ent_list, sizeof(struct strong_ref)*(ent_capacity+1));

		ent_capacity++;


	}

	// There should be enough space in ent_list at this point ot store an entry
	ent_list[ent_size] = entry;



	ent_size++;


}


// Returns the index which the entry resides in if it exists
// Returns -1 if it doesn't
int entry_exists(void* ptr){



	if(ptr == NULL){
		return -1;
	}

	// Check whether entry exists in ent_list


	for(size_t i = 0; i < ent_size; i++){



		if(ent_list[i] != NULL && ent_list[i]->ptr != NULL){

			void* cursor = ent_list[i]->ptr;

			if(cursor == ptr){
				return i;
			}

		}



	}

	return -1;

}


void ent_list_rearrange(size_t index){

	// Removes the NULL gap passed in
	// Move all elements following the index up by one slot

	for(int i = index; i < ent_size; i++){



		ent_list[i] = ent_list[i+1];

	}

	// Set the last item of the list to null
	ent_list[ent_size] = NULL;




}

struct strong_ref* find_ref(void* ptr){

	struct strong_ref* ref = NULL;


	for(size_t i = 0; i < ent_size; i++){

		void* cursor = ent_list[i]->ptr;

		if(cursor == ptr){
			ref = ent_list[i];
			break;
		}


	}

	return ref;



}

void add_to_dep(void* ptr, struct strong_ref* dep){

	// Check if dep's dep_list is full

	struct rc_entry* entry = &dep->entry;

	if(entry->dep_capacity == 0){
		// Allocate space for one entry pointer
		entry->dep_list = (size_t*)malloc(sizeof(size_t));

		entry->dep_capacity = 1;

	} else if(entry->n_deps == entry->dep_capacity){
		// Allocate more space
		entry->dep_list = (size_t*)realloc(entry->dep_list, sizeof(size_t)*(entry->dep_capacity+1));

		entry->dep_capacity++;
	}

	// Should be enough space at this point
	// Cast ptr as size_t and store into dep_list

	size_t entry_id = (size_t)ptr;

	// printf("ptr being stored %p\n", ptr);

	entry->dep_list[entry->n_deps] = entry_id;

	entry->n_deps++;



}


/**
 * Returns an allocation of n bytes and creates an internal rc entry.
 *
 * If the ptr argument is NULL and deps is NULL, it will return a new 
 * allocation
 * 
 * If the ptr argument is not NULL and an entry exists, it will increment
 *  the reference count of the allocation and return a strong_ref pointer
 *
 * If the ptr argument is NULL and deps is not NULL, it will return 
 * a new allocation but the count will correlate to the dependency
 * if the dependency is deallocated the reference count on the object will 
 * decrement
 *
 * If the ptr argument is not NULL and an entry exists and dep is not
 * NULL, it will increment the count of the strong reference but the count
 * will be related to the dependency, if the dependency is deallocated the
 * reference count on the object will decrement
 */
struct strong_ref* rc_alloc(void* ptr, size_t n, struct strong_ref* dep) {

	// Case 1
	if(ptr == NULL && dep == NULL){

		// Create new allocation
		// printf("size of strong ref struct %lu\n", sizeof(struct strong_ref));

		// Create new strong reference and initialise all variables
		struct strong_ref* new_ref = (struct strong_ref*)malloc(sizeof(struct strong_ref));
		new_ref->entry.count = 1;
		new_ref->entry.n_deps = 0;
		new_ref->entry.dep_capacity = 0;
		new_ref->entry.dep_list = NULL;
		
		
		new_ref->ptr = (void*)malloc(n);

		// ptr ust needs an area of memory to point to
		
		// Add an entry of this malloc'ed space into ent_list
		add_entry(new_ref);

		return new_ref;

	}


	// Case 3
	if(ptr == NULL && dep != NULL){


		// Create new allocation
		struct strong_ref* new_ref = (struct strong_ref*)malloc(sizeof(struct strong_ref));
		new_ref->entry.count = 1;
		new_ref->entry.n_deps = 0;
		new_ref->entry.dep_capacity = 0;
		new_ref->entry.dep_list = NULL;

		new_ref->ptr = (void*)malloc(n);

		// Put ptr into dep's dependency list (I'm going with this)
		// That way I can actually decrement ptr when dep is freed (As mentioned in the specs)
		
		add_to_dep(new_ref->ptr, dep);

		// printf("Stored Dep %zu \n", dep->entry.dep_list[0]);
		// new_ref->entry.count++;


		add_entry(new_ref);


		return new_ref;


	}


	// Case 4
	if(ptr != NULL && entry_exists(ptr) != -1 && dep != NULL){


		// Increment the count of reference for ptr
		struct strong_ref* ref = find_ref(ptr);

		// Increment reference count
		ref->entry.count++;

		add_to_dep(ptr, dep);

		return ref;


	}

	// Case 2
	if(ptr != NULL && entry_exists(ptr) != -1){


		// Find the corresponding allocate entry in ent_list to increment it's reference count

		struct strong_ref* ref = find_ref(ptr);

		// Increment reference count
		ref->entry.count++;

		// Return a strong_ref pointer

		return ref;
	}






	return NULL;
}


struct weak_ref check_count(struct strong_ref* ref){

	// printf("Number of Deps from this ref: %zu\n", ref->entry.n_deps);
	

	if(ref->entry.count < 1){
		// Decrement all of its dependencies count by 1

		for(size_t i = 0; i < ref->entry.n_deps; i++){

			// printf("Address converted to size_t: %zu\n", address);

			// Convert size_t to pointer

			void* ptr = (void*)ref->entry.dep_list[i];

			// Check that this entry exists
			
			// printf("IF STATEMENT BELOW\n");

			if(entry_exists(ptr) != -1){

				// Get reference to the dependency struct
				struct strong_ref* dep = find_ref(ptr);

				// Decrement count for this reference
				dep->entry.count--;

				// Call check count on dep again to ensure that if the reference count has dipped below 1
				// We are freeing the struct
				check_count(dep);


			}



		}

		// Retrieve index of ref
		int index = entry_exists(ref->ptr);

		// Once all of the dependencies are decremented
		// We are going to free the space allocated to dep_list
		// And free the space allocated to the ptr
		if(ref->entry.dep_capacity > 0){
			// Space was allocated to dependencies list
			free(ref->entry.dep_list);
		}


		// Free pointer memory
		free(ref->ptr);
		free(ref);

		// Update ent_list to reflect the free
		ent_list[index] = NULL;

		ent_size--;

		ent_list_rearrange(index);



		struct weak_ref r = { 0xFFFFFFFFFFFFFFFF };
		return r;
		

	}

	struct weak_ref r = { (size_t)(ref->ptr) };

	return r;

}



/**
 * Downgrades a strong reference to a weak reference, this will decrement the 
 * reference count by 1
 * If ref is NULL, the function will return an invalid weak ref object
 * If ref is a value that does not exist in the reference graph, it will return
 * an weak_ref object that is invalid
 * 
 * If ref is a value that does exist in the reference graph, it will return
 *    a valid weak_ref object
 * 
 * An invalid weak_ref object is where its entry_id field is set to 
 *   0xFFFFFFFFFFFFFFFF
 *
 * @param strong_ref* ref (reference to allocation)
 * @return weak_ref (reference with an entry id)
 */
struct weak_ref rc_downgrade(struct strong_ref* ref) {


	if(ref == NULL){

		struct weak_ref r = { 0xFFFFFFFFFFFFFFFF };
		return r;

	}


	// Checks if theh entry exists in 
	if(entry_exists(ref->ptr) == -1){

		struct weak_ref r = { 0xFFFFFFFFFFFFFFFF };
		return r;

	}
	

	// Decrement its count value by 1
	ref->entry.count--;



	size_t address = (size_t)(ref->ptr);
	
	// Check if the rc_entry count has dipped below 1
	// check_count(ref);

	
	// Otherwise, we return a weak_ref struct with the address stored in entry_id


	return check_count(ref);

}


/**
 * Upgrdes a weak reference to a strong reference.
 * The weak reference should check that the entry id is valid (bounds check)
 * If a strong reference no longer exists or has been deallocated, the return 
 *   result should be null.
 */
struct strong_ref* rc_upgrade(struct weak_ref ref) {


	// Perform bounds check on the ref entry_id
	if(ref.entry_id > MAX_ADDRESS){
		return NULL;
	}

	// Cast entry_id into void pointer variable
	void* ptr = (void*)ref.entry_id;


	int index = entry_exists(ptr);


	if(index == -1){

		// Entry no longer exists or have been deallocated
		return NULL;

	} else {
		// it does exist

		// Pass back a strong reference pointer to the ptr address

		struct strong_ref* strong = find_ref(ptr);
		strong->entry.count++;

		return strong;


	}





	return NULL;
}


/**
 * Cleans up the reference counting graph.
 */
void rc_cleanup() {

	// I'm guessing when we call this we're just gonna free everything we have
	// For every item inside the entry list
	for(size_t i = 0; i < ent_size; i++){

		if(ent_list[i] == NULL){
			continue;
		}

		struct strong_ref* ref = ent_list[i];

		// We need to free the dep_list inside entry if it has been allocated space

		if(ref->entry.dep_capacity > 0){
	
			// There was space allocated and needs to be freed
			free(ref->entry.dep_list);
		

		}

		// Next free the pointer

		free(ref->ptr);
		free(ref);


	}


	// Finally free the ent_list

	free(ent_list);
	ent_capacity = 0;
	ent_size = 0;



	
}
