#include "devsec.h"

std::string DevSec::intToHexString(int intValue) {
    std::string hexStr;
    std::stringstream sstream;
    sstream << std::setfill ('0') << std::setw(2)
    << std::hex << (int)intValue;
    hexStr = sstream.str();
    sstream.clear();
    return hexStr;
}

DevSec::DevSec() {
  this->dsig_created = false;
  this->dsig_valid = false;
  this->debug = false;
}

void DevSec::setDebug(bool val) {
  this->debug = val;
}

void DevSec::set_credentials(char * ssid, char * pass) {

  if (!this->dsig_created) {
    printf("ERROR: No DSIG/EKEY generated to encrypt with.\n");
    return;
  }

  // TODO: encrypted first!
  this->ssid = endecrypt(ssid);
  this->password = endecrypt(pass);
}

void DevSec::generate_signature(char *mac, char *ckey, char* fcid) {

  if (this->debug) printf("\n[DevSec] Generating device signature...\n");

  unsigned int mac_len = strlen(mac);

  if (mac_len > 17) {
    printf("Invalid MAC length.\n");
    exit(2);
  }

  char mac_bytes[13] = {0};
  unsigned int index = 0;
  for (unsigned int pos = 0; index < 13; pos++) {
    if ((char)mac[pos] == ':') {
      continue;
    }
    mac_bytes[index] = (unsigned int)mac[pos];
    index++;
  }

  if (index < 12) {
    printf("Invalid MAC length.\n");
    exit(2);
  }

  mac_bytes[12] = 0;

  sprintf(this->flash_chip_id, "%s", fcid);
  sprintf((char*)this->dsig, "%s;%s;%s", SIGBASE, mac_bytes, this->flash_chip_id);

  if (this->debug) { printf("\nDSIG: '"); printf("%s", (char*)this->dsig); printf("'\n"); }

  this->dsig[63] = 0; // make sure there is a termination character at the end of DSIG

  this->dsig_created = true;

  if (this->debug)
    printf("Deriving in-memory key from CKEY and flash_chip_id:\n'");
  for( int c = 0; c < strlen(ckey); c++) {
    this->key[c] = ckey[c] ^ this->flash_chip_id[c%sizeof(this->flash_chip_id)];
    if (this->debug) { printf("0x"); printf("%s", intToHexString((int)this->key[c]).c_str()); }
    if (c < strlen(ckey) - 1) {
      if (this->debug) printf(", ");
    }
  }

  if (this->debug) printf("'\n");
}

char * DevSec::signature() {
  return (char*)this->dsig;
}

char * DevSec::unsignature(char *ckey) {
  for( int c = 0; c < strlen(this->dsig); c++) {
    this->usig[c] = ckey[c] ^ this->dsig[c];
    if (this->debug) { printf("0x"); printf("%s", intToHexString((int)this->usig[c]).c_str()); }
    if (c < strlen(ckey) - 1) {
      if (this->debug) printf(", ");
    }
  }
  return (char*)this->usig;
}

void DevSec::print_signature() {

  printf("#ifndef __EMBEDDED_SIGNATURE__\n");
  printf("#define __EMBEDDED_SIGNATURE__\n");
  printf("\n");
  printf("#include <inttypes.h>\n"); // stdlib instead of whole Arduino's byte_t
  printf("\n");
  printf("// Obfuscated firmware signature\n\n");
  printf("uint8_t EMBEDDED_SIGNATURE["); printf("%lu", 1+sizeof(this->dsig)); printf("] PROGMEM = {\n");

  for ( unsigned int d = 0; d < strlen((char*)this->dsig); d++) {
    printf("0x"); printf("%s", intToHexString((int)this->dsig[d]).c_str());
    if (d < strlen((char*)this->dsig) - 1) {
      printf(", ");
      if ((d%8 == 0)) {
        printf("\n");
      }
    } else {
      printf("0x0");
    }
  }

  printf("\n};\n");

  uint8_t ssid_len = strlen(this->ssid);
  printf("uint8_t DevSec::EMBEDDED_SSID[%u] = PROGMEM { 0x0 };\n", ssid_len);
  // TODO: printf this->ssid (encrypted as hex string)

  uint8_t pass_len = strlen(this->pass);
  printf("uint8_t DevSec::EMBEDDED_PASS[%u] = PROGMEM { 0x0 };\n", pass_len);
  // TODO: printf this->password (encrypted as hey string)

  printf("#endif // __EMBEDDED_SIGNATURE__\n");
}

bool DevSec::validate_signature(char * signature, char * ckey) {

  if (!this->dsig_created) {
    printf("ERROR: No DSIG generated to validate against.\n");
    return false;
  }

  char unsignature[64] = {0};

  bool isValid = true;
  for ( unsigned int d = 0; d < sizeof(signature); d++) {
    unsignature[d] = this->dsig[d] ^ ckey[d];
    if (signature[d] != unsignature[d]) {
#ifdef DEBUG
      printf("\nByte "); printf("%u", d); printf(" mismatch.\n"); // debug
      printf("At: "); printf("%c", signature[d]); printf(" "); printf("%c\n", unsignature[d]); // debug
      printf("\n");
#endif
      isValid = false;
    }
    // TODO: Fix this, length may differ.
    if (d == 17) {
      break; // compares only first 17!
    }
  }

  this->dsig_valid = isValid;

  return isValid;

}

/* Performs simple symetric XOR encryption using static CKEY so does not work with strings well */
char * DevSec::endecrypt(uint8_t input[]) {
    // dsig is valid when key is generated, this needs the key
    if (this->dsig_valid) {
      for ( unsigned int d = 0; d < strlen((char*)input); ++d ) {
        this->crypted[d] = ((char)input[d] ^ this->key[d]);
      }
    } else {
      printf("ERROR: DSIG must be valid for decryption.\n");
      exit(2);
    }
    return (char*)this->crypted;
}

/* Overwrites unconditionally the dsig value with zeros */
void DevSec::cleanup() {
  if (this->dsig_created) {
    printf("Note: DSIG is being erased now...\n");
  }

  for ( unsigned int e = 0; e < sizeof(dsig); ++e ) {
    dsig[e] = 0; // zero-out in-memory signature
  }

  dsig_created = false;
  dsig_valid = false;
}
