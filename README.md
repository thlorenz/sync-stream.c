# sync-stream.c

synchronous streams implementation in C

```c
sst_t *tx_upper, *tx_reverse, *writable;

tx_upper = sst_new();
tx_reverse = sst_new();
writable = sst_new();

tx_upper->write = upper_onwrite;
tx_reverse->write = reverse_onwrite;

sst_pipe(tx_upper, tx_reverse);
sst_pipe(tx_reverse, writable);

writable->emit_cb = writable_onchunk;
writable->end_cb = writable_onend;
write(tx_upper, "hello");
write(tx_upper, "world");
write(tx_upper, ",");
write(tx_upper, "my");
write(tx_upper, "friends");

tx_upper->end(tx_upper);
```

[full example](https://github.com/thlorenz/sync-stream.c/blob/master/examples/pipe-thru-transforms.c)

### output

```
OLLEH <|> DLROW <|> , <|> YM <|> SDNEIRF <|>
stream ended
```

## Status

In heave spiking mode, trying to figure out best API/behaviors.
Use at your own risk.

## Installation

    clib install thlorenz/sync-stream.c

## API

```c
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
```

## License

MIT
