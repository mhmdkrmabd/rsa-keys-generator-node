const keysGenerator = require('./index.js');

// Get service name from command line argument (required)
const serviceName = process.argv[2];

if (!serviceName) {
    console.error('Error: Service name is required!');
    console.error('Usage: node test.js <serviceName>');
    console.error('Example: node test.js AppName');
    process.exit(1);
}

console.log('Testing Node RSA Keys Generator Native Module');
console.log('===========================================');
console.log('Service name:', serviceName);

// Test platform detection
console.log('\nPlatform:', keysGenerator.getPlatform());
console.log('Keychain available:', keysGenerator.isKeychainAvailable());

// Test key generation (matches original Python behavior)
console.log('\nGenerating keys (default 2048-bit):');
const publicKey1 = keysGenerator.generateKeys(serviceName);
if (publicKey1) {
    console.log('✅ Key generation successful');
    console.log('Public key length:', publicKey1.length);
    console.log('Starts with PEM header:', publicKey1.startsWith('-----BEGIN RSA PUBLIC KEY-----'));
    console.log('Ends with PEM footer:', publicKey1.trimEnd().endsWith('-----END RSA PUBLIC KEY-----'));
} else {
    console.log('❌ Key generation failed');
}

// Test retrieving existing keys
console.log('\nRetrieving stored keys:');
const storedPublic = keysGenerator.getPublicKey(serviceName);
const storedPrivate = keysGenerator.getPrivateKey(serviceName);

if (storedPublic) {
    console.log('✅ Retrieved public key from keychain');
    console.log('Keys match:', publicKey1 === storedPublic);
} else {
    console.log('❌ No public key in keychain');
}

if (storedPrivate) {
    console.log('✅ Retrieved private key from keychain');
    console.log('Private key length:', storedPrivate.length);
} else {
    console.log('❌ No private key in keychain');
}

// Test custom key length (using separate test service name to avoid overwriting production keys)
console.log('\nTesting custom key length (1024) with test service name:');
const testServiceName = serviceName + '_Test1024';
const publicKey1024 = keysGenerator.regenerateKeys(testServiceName, 1024);
if (publicKey1024) {
    console.log('✅ 1024-bit key generation successful');
    console.log('Test service name:', testServiceName);
    console.log('Different from previous key:', publicKey1024 !== publicKey1);
} else {
    console.log('❌ 1024-bit key generation failed');
}

// Test environment variable (if set)
const envKeyLength = process.env.RSA_KEY_LENGTH;
if (envKeyLength) {
    console.log(`\nTesting with RSA_KEY_LENGTH=${envKeyLength}:`);
    const envKey = keysGenerator.generateKeys(serviceName);
    if (envKey) {
        console.log('✅ Environment-based key generation successful');
    } else {
        console.log('❌ Environment-based key generation failed');
    }
}

console.log('\nTest completed!');