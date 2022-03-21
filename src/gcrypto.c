#include <argon2.h>
#include <gcrypt.h>

void initialize_gcrypt() {
  /* Version check should be the very first call because it
 makes sure that important subsystems are initialized.
 #define NEED_LIBGCRYPT_VERSION to the minimum required version. */
  /*  #define NEED_LIBGCRYPT_VERSION "1"
  if (!gcry_check_version (NEED_LIBGCRYPT_VERSION))
    {
      fprintf (stderr, "libgcrypt is too old (need %s, have %s)\n",
         NEED_LIBGCRYPT_VERSION, gcry_check_version (NULL));
      exit (2);
    }*/

  /* We don't want to see any warnings, e.g. because we have not yet
     parsed program options which might be used to suppress such
     warnings. */
  gcry_control(GCRYCTL_SUSPEND_SECMEM_WARN);

  /* ... If required, other initialization goes here.  Note that the
     process might still be running with increased privileges and that
     the secure memory has not been initialized.  */

  /* Allocate a pool of 16k secure memory.  This makes the secure memory
     available and also drops privileges where needed.  Note that by
     using functions like gcry_xmalloc_secure and gcry_mpi_snew Libgcrypt
     may expand the secure memory pool with memory which lacks the
     property of not being swapped out to disk.   */
  gcry_control(GCRYCTL_INIT_SECMEM, 16384, 0);

  /* It is now okay to let Libgcrypt complain when there was/is
     a problem with the secure memory. */
  gcry_control(GCRYCTL_RESUME_SECMEM_WARN);

  /* ... If required, other initialization goes here.  */

  /* Tell Libgcrypt that initialization has completed. */
  gcry_control(GCRYCTL_INITIALIZATION_FINISHED, 0);
}

#define keylen 32
#define ivlen 12
#define saltlen 16
#define chunk_size 1024
#define taglen 16

typedef unsigned char Key[keylen];
typedef unsigned char Iv[ivlen];
typedef unsigned char Tag[taglen];

typedef unsigned char Salt[16];
typedef unsigned char Chunk[chunk_size];
typedef unsigned char LongChunk[chunk_size + 1];

// Prints memory in hexadecimal
void print_bytes(char *hex, size_t n) {
  for (int i = 0; i < n; ++i) {
    printf("%x02", (int)((unsigned char *)hex)[i]);
  }
  printf("\n");
}

// Produces a hash of required length from a password and a salt
void argon(Key hash, const char *password, Salt salt) {
  int t_cost = 2;         // 2-pass computation
  int m_cost = (1 << 16); // 64 mebibytes memory usage
  int parallelism = 1;    // number of threads and lanes
  // high-level API
  argon2i_hash_raw(t_cost, m_cost, parallelism, password, strlen(password),
                   salt, saltlen, hash, keylen);
}

void feed_authentication_data(gcry_cipher_hd_t hd, Salt salt, Iv iv) {
  // authenticated unencrypted data
  char aad[saltlen + ivlen];
  memmove(aad, salt, saltlen);
  memmove(aad + saltlen, iv, ivlen);

  gcry_cipher_setiv(hd, iv, ivlen);
  gcry_cipher_authenticate(hd, aad, saltlen + ivlen);
}

int decrypt(FILE *in_file, FILE *out_file, const char *password) {
  gcry_cipher_hd_t hd;
  gcry_cipher_open(&hd, GCRY_CIPHER_CHACHA20, GCRY_CIPHER_MODE_POLY1305, 0);

  Key key;
  Salt salt;
  Iv iv;
  Tag tag;

  // Read the headers
  fread(salt, 1, saltlen, in_file);
  fread(iv, 1, ivlen, in_file);

  // Find the key
  argon(key, password, salt);

  gcry_cipher_setkey(hd, key, keylen);
  feed_authentication_data(hd, salt, iv);

  Chunk in, out;
  // Write the ciphertext
  while (!feof(in_file)) {
    int inlen = fread(in, 1, chunk_size, in_file);
    if (feof(in_file)) {
      inlen -= taglen; // Don't include the tag when decrypting
      // Copy the tag
      memmove(tag, in+inlen, taglen);
      // Flag the final round
      gcry_cipher_final(hd);
    }
    gcry_cipher_decrypt(hd, out, chunk_size, in, inlen);
    fwrite(out, 1, inlen, out_file);
    fflush(out_file);
  }
  // Check the tag
  gcry_error_t err = gcry_cipher_checktag(hd, tag, taglen);
  if (err != 0) {
    fprintf(stderr, "Can't decrypt. Authentication tag failed.\n");
  }
  gcry_cipher_close(hd);
  return err;
}

int encrypt(FILE *in_file, FILE *out_file, const char *password) {
  gcry_cipher_hd_t hd;
  gcry_cipher_open(&hd, GCRY_CIPHER_CHACHA20, GCRY_CIPHER_MODE_POLY1305, 0);

  Key key;
  Salt salt;
  Iv iv;
  Tag tag;

  gcry_create_nonce(salt, saltlen);
  gcry_create_nonce(iv, ivlen);

  argon(key, password, salt);

  Chunk in, out;

  // Compute the key
  gcry_cipher_setkey(hd, key, keylen);

  // Write the headers
  feed_authentication_data(hd, salt, iv);
  fwrite(salt, 1, saltlen, out_file);
  fwrite(iv, 1, ivlen, out_file);
  // Write the ciphertext
  while (!feof(in_file)) {
    int inlen = fread(in, 1, chunk_size, in_file);
    if (feof(in_file)) {
      gcry_cipher_final(hd);
    }
    gcry_cipher_encrypt(hd, out, chunk_size, in, inlen);
    fwrite(out, 1, inlen, out_file);
    fflush(out_file);
  }
  gcry_cipher_gettag(hd, tag, taglen);
  // Write the tag
  fwrite(tag, 1, taglen, out_file);
  fflush(out_file);
  gcry_cipher_close(hd);
  return 0;
}
