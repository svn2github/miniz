// example3.c - Demonstrates how to use miniz.c's deflate() and inflate() functions for simple file compression.
// Public domain, May 15 2011, Rich Geldreich, richgel99@gmail.com
// For simplicity, this example is limited to files smaller than 4GB, but this is not a limitation of miniz.c.
#include "miniz.c"

typedef unsigned char uint8;
typedef unsigned short uint16;
typedef unsigned int uint;

#define my_max(a,b) (((a) > (b)) ? (a) : (b))
#define my_min(a,b) (((a) < (b)) ? (a) : (b))

#define BUF_SIZE (1024 * 1024)
static uint8 s_inbuf[BUF_SIZE];
static uint8 s_outbuf[BUF_SIZE];

int main(int argc, char *argv[])
{
  const char *pMode;
  FILE *pInfile, *pOutfile;
  uint infile_size;
  z_stream stream;
  
  if (argc != 4)
  {
    printf("Usage: example3 [c/d] infile outfile\n");
    printf("c - Compresses file infile to a zlib stream in file outfile\n");
    printf("d - Decompress zlib stream in file infile to file outfile\n");
    return EXIT_FAILURE;
  }

  pMode = argv[1];
  
  // Open input file.
  pInfile = fopen(argv[2], "rb");
  if (!pInfile)
  {
    printf("Failed opening input file!\n");
    return EXIT_FAILURE;
  }
  
  // Determine input file's size.
  fseek(pInfile, 0, SEEK_END);
  infile_size = ftell(pInfile);
  fseek(pInfile, 0, SEEK_SET);

  // Open output file.
  pOutfile = fopen(argv[3], "wb");
  if (!pOutfile)
  {
    printf("Failed opening output file!\n");
    return EXIT_FAILURE;
  }

  printf("Input file size: %u\n", infile_size);
  
  // Init the z_stream
  memset(&stream, 0, sizeof(stream));
  stream.next_in = s_inbuf;
  stream.avail_in = 0;
  stream.next_out = s_outbuf;
  stream.avail_out = BUF_SIZE;

  if ((pMode[0] == 'c') || (pMode[0] == 'C'))
  {
    // Compression.
    uint infile_remaining = infile_size;

    if (deflateInit(&stream, Z_BEST_COMPRESSION) != Z_OK)
    {
      printf("deflateInit() failed!\n");
      return EXIT_FAILURE;
    }

    for ( ; ; )
    {
      int status;
      if (!stream.avail_in)
      {
        // Input buffer is empty, so read more bytes from input file.
        uint n = my_min(BUF_SIZE, infile_remaining);
                
        if (fread(s_inbuf, 1, n, pInfile) != n)
        {
          printf("Failed reading from input file!\n");
          return EXIT_FAILURE;
        }
        
        stream.next_in = s_inbuf;
        stream.avail_in = n;
        
        infile_remaining -= n;
        printf("Input bytes remaining: %u\n", infile_remaining);
      }

      status = deflate(&stream, infile_remaining ? Z_NO_FLUSH : Z_FINISH);

      if ((status == Z_STREAM_END) || (!stream.avail_out))
      {
        // Output buffer is full, or compression is done, so write buffer to output file.
        uint n = BUF_SIZE - stream.avail_out;
        if (fwrite(s_outbuf, 1, n, pOutfile) != n)
        {
          printf("Failed writing to output file!\n");
          return EXIT_FAILURE;
        }
        stream.next_out = s_outbuf;
        stream.avail_out = BUF_SIZE;
      }

      if (status == Z_STREAM_END)
        break;
      else if (status != Z_OK)
      {
        printf("deflate() failed with status %i!\n", status);
        return EXIT_FAILURE;
      }
    }

    if (deflateEnd(&stream) != Z_OK)
    {
      printf("deflateEnd() failed!\n");
      return EXIT_FAILURE;
    }
  }
  else if ((pMode[0] == 'd') || (pMode[0] == 'D'))
  {
    // Decompression.
    uint infile_remaining = infile_size;

    if (inflateInit(&stream))
    {
      printf("inflateInit() failed!\n");
      return EXIT_FAILURE;
    }

    for ( ; ; )
    {
      int status;
      if (!stream.avail_in)
      {
        // Input buffer is empty, so read more bytes from input file.
        uint n = my_min(BUF_SIZE, infile_remaining);

        if (fread(s_inbuf, 1, n, pInfile) != n)
        {
          printf("Failed reading from input file!\n");
          return EXIT_FAILURE;
        }

        stream.next_in = s_inbuf;
        stream.avail_in = n;

        infile_remaining -= n;
      }

      status = inflate(&stream, Z_SYNC_FLUSH);

      if ((status == Z_STREAM_END) || (!stream.avail_out))
      {
        // Output buffer is full, or decompression is done, so write buffer to output file.
        uint n = BUF_SIZE - stream.avail_out;
        if (fwrite(s_outbuf, 1, n, pOutfile) != n)
        {
          printf("Failed writing to output file!\n");
          return EXIT_FAILURE;
        }
        stream.next_out = s_outbuf;
        stream.avail_out = BUF_SIZE;
      }

      if (status == Z_STREAM_END)
        break;
      else if (status != Z_OK)
      {
        printf("inflate() failed with status %i!\n", status);
        return EXIT_FAILURE;
      }
    }

    if (inflateEnd(&stream) != Z_OK)
    {
      printf("inflateEnd() failed!\n");
      return EXIT_FAILURE;
    }      
  }
  else
  {
    printf("Invalid mode!\n");
    return EXIT_FAILURE;
  }

  fclose(pInfile);
  if (EOF == fclose(pOutfile))
  {
    printf("Failed writing to output file!\n");
    return EXIT_FAILURE;
  }

  printf("Total input bytes: %u\n", stream.total_in);
  printf("Total output bytes: %u\n", stream.total_out);
  printf("Success.\n");
  return EXIT_SUCCESS;
}
