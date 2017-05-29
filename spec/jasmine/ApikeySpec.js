describe("API Key", function() {

  var generated_key_hash = null;
  var APIKey = require('../../lib/thinx/apikey');

  //create: function(owner, apikey_alias, callback)
  it("should be able to generate new API Keys", function() {
    APIKey.create(
      "eaabae0d5165c5db4c46c3cb6f062938802f58d9b88a1b46ed69421809f0bf7f",
      "sample-key",
      function(success,
        object) {
        if (success) {
          console.log("Key ready to revoke: " + generated_key_hash);
          generated_key_hash = object.hash;
        }
        expect(object).toBeDefined();
      });
  });

  //verify: function(owner, apikey, callback)
  it("should be able to verify API Keys", function() {
    console.log("Verifying key: " + generated_key_hash);
    APIKey.verify(
      "eaabae0d5165c5db4c46c3cb6f062938802f58d9b88a1b46ed69421809f0bf7f",
      generated_key_hash,
      function(success) {
        expect(success).toBe(true);
      });
  });

  it("should be able to verify invalid API Keys", function() {
    APIKey.verify(
      "eaabae0d5165c5db4c46c3cb6f062938802f58d9b88a1b46ed69421809f0bf7x",
      "310c20ed9dcd4663551bd04cc53a58a52c8d9f1b",
      function(success) {
        expect(success).toBe(false);
      });
  });

  //revoke: function(owner, apikey_hash, callback)
  it("should be able to revoke API Keys", function() {
    console.log("Revoking valid key: " + generated_key_hash);
    APIKey.revoke(
      generated_key_hash,
      "sample-key-hash",
      function(success) {
        expect(success).toBeDefined();
      });
  });

  it("should be able to fail on invalid API Key revocation", function() {
    console.log("Revoking invalid key...");
    APIKey.revoke(
      "nonsense",
      "sample-key-hash",
      function(success) {
        expect(success).toBe(false);
      });
  });

  //list: function(owner, callback)
  it("should be able to list API Keys", function() {
    APIKey.list(
      "eaabae0d5165c5db4c46c3cb6f062938802f58d9b88a1b46ed69421809f0bf7f",
      function(success, object) {
        if (success) {
          console.log(JSON.stringify(object));
          expect(object).toBeDefined();
        } else {
          console.log("[jasmine] Listing failed:" + object);
        }
      });
  });

});
