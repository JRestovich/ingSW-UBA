/*
 * ring_buffer.h
 *
 *	Ported from https://github.com/AndersKaloer/Ring-Buffer
 *
 *  Created on: Jul 11, 2026
 *      Author: mmoya
 */

#ifndef INC_RING_BUFFER_H_
#define INC_RING_BUFFER_H_

#include <stddef.h>
#include <stdint.h>

/**
 * @brief Checks if the buffer_size is a power of two.
 *
 * Due to the design only RING_BUFFER_SIZE-1 items
 * can be contained in the buffer.
 * buffer_size must be a power of two.
*/
#define RING_BUFFER_IS_POWER_OF_TO(buffer_size) ((buffer_size & (buffer_size - 1)) == 0)

/**
 * Used as a modulo operator
 * as <tt> a % b = (a & (b − 1)) </tt>
 * where \c a is a positive index in the buffer and
 * \c b is the (power of two) size of the buffer.
 */
#define RING_BUFFER_MASK(rb) (rb->buffer_mask)


/**
 * The type which is used to hold the size
 * and the indicies of the buffer.
 */
typedef size_t ring_buffer_size_t;

/**
 * Structure which holds a ring buffer.
 * The buffer contains a buffer array
 * as well as metadata for the ring buffer.
 */
typedef struct {
  char *buffer;						///< pointer of buffer memory
  ring_buffer_size_t buffer_mask;	///< Buffer memory
  ring_buffer_size_t tail_index; 	///< Index of tail
  ring_buffer_size_t head_index; 	///< Index of head
} ring_buffer_t;


/**
 * @brief Initializes the ring buffer pointed by buffer>.
 *
 * This function can also be used to empty/reset the buffer.
 * The resulting buffer can contain buf_size-1 bytes.
 *
 * @param[in] buffer The ring buffer to initialize.
 * @param[in] buf The buffer allocated for the ringbuffer.
 * @param[in] buf_size The size of the allocated ringbuffer.
 */
void ring_buffer_init(ring_buffer_t *buffer, char *buf, size_t buf_size);

/**
 * @brief Adds a byte to a ring buffer.
 *
 * @param[in] buffer The buffer in which the data should be placed.
 * @param[in] data The byte to place.
 */
void ring_buffer_queue(ring_buffer_t *buffer, char data);

/**
 * @brief Adds an array of bytes to a ring buffer.
 *
 * @param[in] buffer The buffer in which the data should be placed.
 * @param[in] data A pointer to the array of bytes to place in the queue.
 * @param[in] size The size of the array.
 */
void ring_buffer_queue_arr(ring_buffer_t *buffer, const char *data, ring_buffer_size_t size);

/**
 * @brief Returns the oldest byte in a ring buffer.
 *
 * @param[in] buffer The buffer from which the data should be returned.
 * @param[out] data A pointer to the location at which the data should be placed.
 *
 * @return 1 if data was returned; 0 otherwise.
 */
uint8_t ring_buffer_dequeue(ring_buffer_t *buffer, char *data);

/**
 * @brief Returns the length oldest bytes in a ring buffer.
 *
 * @param[in] buffer The buffer from which the data should be returned.
 * @param[out] data A pointer to the array at which the data should be placed.
 * @param[in] len The maximum number of bytes to return.
 * @return The number of bytes returned.
 */
ring_buffer_size_t ring_buffer_dequeue_arr(ring_buffer_t *buffer, char *data, ring_buffer_size_t len);

/**
 * @brief Peeks a ring buffer, i.e. returns an element without removing it.
 *
 * @param[in] buffer The buffer from which the data should be returned.
 * @param[out] data A pointer to the location at which the data should be placed.
 * @param[in] index The index to peek.
 * @return 1 if data was returned; 0 otherwise.
 */
uint8_t ring_buffer_peek(ring_buffer_t *buffer, char *data, ring_buffer_size_t index);


/**
 * @brief Returns whether a ring buffer is empty.
 *
 * @param[in] buffer The buffer for which it should be returned whether it is empty.
 * @return 1 if empty; 0 otherwise.
 */
inline uint8_t ring_buffer_is_empty(ring_buffer_t *buffer) {
  return (buffer->head_index == buffer->tail_index);
}

/**
 * @brief Returns whether a ring buffer is full.
 *
 * @param[in] buffer The buffer for which it should be returned whether it is full.
 * @return 1 if full; 0 otherwise.
 */
inline uint8_t ring_buffer_is_full(ring_buffer_t *buffer) {
  return ((buffer->head_index - buffer->tail_index) & RING_BUFFER_MASK(buffer)) == RING_BUFFER_MASK(buffer);
}

/**
 * @brief Returns the number of items in a ring buffer.
 *
 * @param[in] buffer The buffer for which the number of items should be returned.
 * @return The number of items in the ring buffer.
 */
inline ring_buffer_size_t ring_buffer_num_items(ring_buffer_t *buffer) {
  return ((buffer->head_index - buffer->tail_index) & RING_BUFFER_MASK(buffer));
}



#endif /* INC_RING_BUFFER_H_ */
