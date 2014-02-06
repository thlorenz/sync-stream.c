#ifndef __SST_H__
#define __SST_H__

#include <stdio.h>
#include <assert.h>
#include <stdarg.h>

enum sst_encoding {
    UTF8   = 0x0
  , BASE64 = 0x2
  , HEX    = 0x4
  , CUSTOM = 0x1000
};

/*
 * chunk
 */

typedef struct sst_chunk_s sst_chunk_t;

typedef void (*sst_chunk_data_free_cb)(void*);

/*
 * chunk struct
 *
 * @data          the data transmitted with the chunk
 * @enc           (default: UTF8) encoding of the data
 * @free_data     (default: NULL) is invoked when the chunk is freed in order to ensure data is freed properly
 * @error         (default: 0) set this to a non-zero value to signal an error
 * @ctx           set this to anything you need to track
 */
struct sst_chunk_s {
  void                    *data;
  enum sst_encoding       enc;
  void                    (*free_data)(void*);
  int                     error;
  void*                   ctx;
};

/*
 * Initializes new chunk and returns the result.
 *
 * @see chunk struct for more initialization options
 *
 * @data        data to transport with the chunk
 * @free_data   (default: NULL) is invoked when the chunk is freed in order to ensure data is freed properly
 *              since we know best how to free the data when we create the chunk, we provide it here
 *              if the data doesn't need to be freed, i.e. because it isn't allocated pass NULL
 * @return chunk
 */
sst_chunk_t *sst_chunk_new(void* data, void (*free_data)(void*));

/**
 * frees the chunk
 *
 * @chunk the chunk to free
 */
void sst_chunk_free(sst_chunk_t* chunk);

/*
 * stream
 */

typedef struct sst_s sst_t;

/* callbacks */
typedef void ( *sst_write_cb) ( sst_t*, sst_chunk_t*);
typedef void ( *sst_emit_cb)  ( sst_t*, sst_chunk_t*);
typedef void ( *sst_end_cb)   ( sst_t*);

#define SST_FIELDS                                                                          \
  sst_write_cb      write;                                                                  \
  sst_emit_cb       emit_cb;                                                                \
  sst_end_cb        end_cb;                                                                 \
                                                                                            \
  /* readonly */                                                                            \
  sst_emit_cb       emit;                                                                   \
  sst_end_cb        end;                                                                    \
                                                                                            \
  /* private */                                                                             \
                                                                                            \
  /* the stream upstream from this stream and thus the source of all data */                \
  /* we track it here only to be able to free it later */                                   \
  sst_t   *_source;                                                                         \
                                                                                            \
  /* the stream downstream from this stream, if set all emitted chunk will be written it */ \
  sst_t   *_destination;

/*
 * stream struct
 *
 * @write         call this with a @see chunk in order to feed data into the stream
 * @emit_cb       if provided, it is called with every chunk emitted by the stream
 * @end_cb        if provided, it is called when the stream ends
 * @emit          call this to emit a chunk
 * @end           call this to signal that no more data will be written to the stream
 */
struct sst_s {
  SST_FIELDS
};

/**
 * Initializes a  stream and returns it
 * @see stream struct for more initializiation options
 *
 * @return stream
 */
sst_t* sst_new();

/**
 * Frees the given stream and walks up the `_source`s to free all streams that are upstream as well
 *
 * @self    the stream to free
 */
void sst_free(sst_t* self);

/**
 * Pipes the source stream into the destination stream(s).
 * All chunks emitted by the source are now written to the destination(s).
 *
 * When source ends, destinations are ended as well.
 *
 * When most downstream destination is freed,
 * all other destinations and the source are freed as well.
 *
 * @source upstream source
 * @...    followed by the downstream destination(s) which get chained together to form one pipe
 */
void sst__pipe(sst_t** streams);
#define sst_pipe(source, ...) {                               \
  sst_t** streams = (sst_t*[]) { source, __VA_ARGS__, NULL }; \
  sst__pipe(streams);                                         \
}

/*
 * file stream
 */

typedef struct sst_file_s  sst_file_t;

/* file stream struct
 *
 * extends stream struct
 *
 * @file      the open file to read from or write to
 * @free_file (default: NULL) is invoked when the file stream is freed in order to free file if needed
 *            if the data doesn't need to be freed, i.e. because it isn't allocated pass NULL
 * @bufsize   (default: BUFSIZ) size of buffers to write
 */
struct sst_file_s {
  SST_FIELDS
  FILE *file;
  void (*free_file)(void*);
  size_t bufsize;
  short free_onend;
};

/**
 * Initializes a file stream from the given file.
 *
 * @file      the file to wrap in a stream
 * @free_file  invoked when the file stream is freed in order to ensure file is freed properly
 *             ensure that the file is closed however
 *             if the file doesn't need to be freed, i.e. because it isn't allocated pass NULL
 * @return  file stream wrapping the file
 */
sst_file_t *sst_file_new(FILE *file, void (*free_file)(void*));

/**
 * Frees the file stream including the FILE it is wrapping.
 * Note: the underlying file handle is assumed to be closed at this point.
 *
 * @self  the file stream
 */
void sst_file_free(sst_file_t* self);

/**
 * Starts reading from the wrapped file.
 * For each read buffer a chunk is `emit`ted.
 * Note: file is assumed to be open at this point.
 *
 * When all chunks were read and the stream invokes `end`, the file is closed but not freed
 * since the stream that is at the end of the pipe chain is responsible for that.
 *
 * @self  file stream
 */
void sst_file_read_start(sst_file_t* self);

/**
 * Initializes file stream for writing.
 * Note: file is assumed to be open at this point.
 *
 * All values written to the stream wrapper are `fputs`ed to the underlying file.
 *
 * When all upstream chunks were written, the file is closed
 * Additionally it is freed along with the wrapping file stream since it will be the stream
 * at the end of the pipe chain and thus is responsible for calling free.
 * Unset `free_onend` to disable this behavior.
 *
 * @self   the file stream
 */
void sst_file_write_init(sst_file_t* self);

#endif
