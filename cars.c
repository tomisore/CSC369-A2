include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "traffic.h"

extern struct intersection isection;

void add_to_lane(struct lane *b, struct car *n_car){
     pthread_mutex_lock(b->lock);
        /*Adding car to lane*/
     n_car->next = b->out_cars;
     b->out_cars = n_car;   
     pthread_mutex_unlock(b->lock);
}

int add_to_buff(struct lane *b){
    pthread_mutex_lock(b->lock);
    if ( b->inc == 0){
        /* lane b is empty , wait */
        pthread_mutex_unlock(b->lock);
        return -1;
        
    }
    
    /* Finding frontmost car */
    car* curr = b->in_cars;
    car* prev;
    while( curr->next != NULL){
       prev = curr;
       curr = curr->next;
    }
    /* could use while loop to error check # elem in lane */
    prev->next = NULL; 
    pthread_mutex_unlock(b->lock);
    push_buf(b,curr);
    return 0;
}
/**
 * Populate the car lists by parsing a file where each line has
 * the following structure:
 *
 * <id> <in_direction> <out_direction>
 *
 * Each car is added to the list that corresponds with 
 * its in_direction
 * 
 * Note: this also updates 'inc' on each of the lanes
 */
void parse_schedule(char *file_name) {
    int id;
    struct car *cur_car;
    struct lane *cur_lane;
    enum direction in_dir, out_dir;
    FILE *f = fopen(file_name, "r");

    /* parse file */
    while (fscanf(f, "%d %d %d", &id, (int*)&in_dir, (int*)&out_dir) == 3) {

        /* construct car */
        cur_car = malloc(sizeof(struct car));
        cur_car->id = id;
        cur_car->in_dir = in_dir;
        cur_car->out_dir = out_dir;

        /* append new car to head of corresponding list */
        cur_lane = &isection.lanes[in_dir];
        cur_car->next = cur_lane->in_cars;
        cur_lane->in_cars = cur_car;
        cur_lane->inc++;
    }

    fclose(f);
}

/**
 * TODO: Fill in this function
 *
 * Do all of the work required to prepare the intersection
 * before any cars start coming
 * 
 */
void init_intersection() {

   int i;
   for (i = 0; i < 4 ; i++){
        struct lane *laney = isection->lanes[i];
        laney->in_cars = NULL;
        laney->out_cars = NULL;
        laney->inc = 0;
        laney->passed = 0;
        laney->lock = PTHREAD_MUTEX_INITIALIZER;
        isection.quad[i] = PTHREAD_MUTEX_INITIALIZER;
        /*Circullar buffer implementation*/
        laney->buffer = malloc(capacity*sizeof(cars *));
        if (laney->buffer == NULL){
            //HANDLE ERROR
            return;
        }
        laney->head = 0;
        laney->tail = 0; 
        laney->in_buf= 0;   
        laney->capacity = LANE_LENGHT;
    
   }
}


void push_buf(struct lane *t ,struct car *car){
    pthread_mutex_lock(t->lock);
    int spot = t->head+1;
    if(spot >= t->capacity) spot = 0;
    //buf is full
    while (spot == c->tail){
        /*buf is full*/
        pthread_cond_wait(t->producer_cv, t->lock)
    } 
    // could be the cause of an error, not sure if assaigned properly
    t->buffer[t->head*sizeof(cars *)] = car;
    t->head = next;
    t->in_buf++;
    pthread_cond_signal(consumer_cv);
    pthread_mutex_unlock(t->lock);
    
}

void pop_buf(struct lane *t, struct cars *car){
    pthread_mutex_lock(t->lock);
    
    while(t->head == t->tail){
        /*empty buf*/
        pthread_cond_wait(t->consumer_cv, t->lock);
    }
    car = t->buffer[t->tail*sizeof(cars *)];
     // optional line  dont realy have to clear
    *(t->buffer[t->tail*sizeof(cars *)]) = NULL;
    int spot = t->tail+1;
    if(spot >= t.capacity){
        spot = 0;
    }
    t->tail = spot;
    t->in_buf--;
    t->inc--;
    t->passed++;
    pthread_cond_signal(producer_cv);
    pthread_mutex_unlock(t->lock);
}

/**
 * TODO: Fill in this function
 *
 * Populates the corresponding lane with cars as room becomes
 * available. Ensure to notify the cross thread as new cars are
 * added to the lane.
 * 
 */
void *car_arrive(void *arg) {
    struct lane *l = arg;

    /* populate buffer */
    while( l->in_buf < l->capacity && l->in_buf != l->inc){
        add_to_buff(l);
    }
    
    return NULL;
}

/**
 * TODO: Fill in this function
 *
 * Moves cars from a single lane across the intersection. Cars
 * crossing the intersection must abide the rules of the road
 * and cross along the correct path. Ensure to notify the
 * arrival thread as room becomes available in the lane.
 *
 * Note: After crossing the intersection the car should be added
 * to the out_cars list of the lane that corresponds to the car's
 * out_dir. Do not free the cars!
 *
 * 
 * Note: For testing purposes, each car which gets to cross the 
 * intersection should print the following three numbers on a 
 * new line, separated by spaces:
 *  - the car's 'in' direction, 'out' direction, and id.
 * 
 * You may add other print statements, but in the end, please 
 * make sure to clear any prints other than the one specified above, 
 * before submitting your final code. 
 */
void *car_cross(void *arg) {
    struct lane *l = arg;
    struct car *car;
    struct lane *out_lane;
    /* avoid compiler warning */
    while(l->inc > 0){
        pop_buf(l,car);
        int *p = compute_path(car->in_dir,car->out_dir);
        if ( p != NULL){
            for ( int x : p){
                if( x != NULL) pthread_mutex_lock(isection.quad[x-1]);
            }
            out_lane = &isection.lanes[out_dir];
            add_to_lane(out_lane, car);
            for ( int x : p){
                if( x != NULL) pthread_mutex_unlock(isection.quad[x-1]);
            }
        }
    }

    return NULL;
}

/**
 * TODO: Fill in this function
 *
 * Given a car's in_dir and out_dir return a sorted 
 * list of the quadrants the car will pass through.
 * 
 */
int *compute_path(enum direction in_dir, enum direction out_dir) {
    int *p;
    int path[3]; 
    p = path;
    if( in_dir == out_dir){
        return NULL;
    }
    switch(in_dir){
        case NORTH:
            swicth(out_dir){
                case WEST:
                    path = {2,NULL,NULL};
                    return p;
                case SOUTH:
                    path = {2,3,NULL};
                    return p;
                case EAST:
                    path = {2,3,4};
                    return p;
                default:
                    return NULL;
            }
        case WEST:
            swicth(out_dir){
                case NORTH:
                    path = {3,4,1};
                    return p;
                case SOUTH:
                    path = {3,NULL,NULL};
                    return p;
                case EAST:
                    path = {3,4,NULL};
                    return p;
                default:
                    return NULL;
            }               
        case EAST:
            swicth(out_dir){
                case WEST:
                    path = {1,2, NULL};
                    return p;
                case SOUTH:
                    path = {1,2,3};
                    return p;
                case NORTH:
                    path = {1,NULL,NULL};
                    return p;
                default:
                    return NULL;
            }
        case SOUTH:
            swicth(out_dir){
                case WEST:
                    path = {4,1,2};
                    return p;
                case NORTH:
                    path = {4,1,NULL};
                    return p;
                case EAST:
                    path = {4,NULL,NULL};
                    return p;
                default:
                    return NULL;
            }
    }
    return NULL;  
}