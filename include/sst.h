
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
 * @enc           encoding of the data
 * @free_data     (default: free((char*)data)) called to free the data
 *                if set to NULL, data won't be freed, use this if data wasn't allocated
 * @nofree        (default: 0) if set, chunk is not freed after it was emitted/piped
 * @result        set this to a non-zero value to signal an error
 */
struct sst_chunk_s {
  void                    *data;
  enum sst_encoding       enc;
  sst_chunk_data_free_cb  free_data;
  short                   nofree;
  int                     result;
};

/*
 * Initializes new chunk and returns the result.
 *
 * @see chunk struct for more initialization options
 * @data   data to transport with the chunk
 * @return chunk
 */
sst_chunk_t *sst_chunk_new(void* data);

/**
 * frees the chunk
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
  sst_write_cb      write;
  sst_emit_cb       emit_cb;
  sst_end_cb        end_cb;

  /* readonly */
  sst_emit_cb       emit;
  sst_end_cb        end;

  /* private */

  /* the stream upstream from this stream and thus the source of all data */
  /* we track it here only to be able to free it later */
  sst_t   *_source;

  /* the stream downstream from this stream, if set all emitted chunk will be written it */
  sst_t   *_destination;
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
 * Pipes the source stream into the destination stream.
 * All chunks emitted by the source are now written to the destination.
 * When source ends, destination is ended as well.
 * When destination is freed the source is freed as well.
 *
 * @source        the upstream source
 * @destination   the downstream destination
 */
void sst_pipe(sst_t* source, sst_t* destination);
