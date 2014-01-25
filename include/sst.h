
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
 * transform stream
 */

typedef struct sst_transform_s sst_transform_t;

/* callbacks */
typedef void ( *sst_emit_cb)  ( sst_transform_t*, sst_chunk_t*);
typedef void ( *sst_read_cb)  ( sst_chunk_t*);
typedef void ( *sst_write_cb) ( sst_transform_t*, sst_chunk_t*);

/*
 * transform struct
 *
 * @write     call this with a @see chunk in order to feed data into the transform
 * @read      if provided, it is called with every chunk emitted by the transform
 * @pipe      if set all emitted chunk will be written to that transform
 * @emit      call this to emit a chunk
 */
struct sst_transform_s {
  sst_write_cb      write;
  sst_read_cb       read;
  sst_transform_t   *pipe;

  /* readonly */
  sst_emit_cb       emit;
};

/**
 * Initializes a transform and returns it
 * @see transform struct for more initializiation options
 *
 * @return transform stream
 */
sst_transform_t* sst_transform_new();

/**
 * Frees the given transform (at this point just free(self))
 *
 * @self    the transform to free
 */
void sst_transform_free(sst_transform_t* self);
