const path = require('path');
const fs = require('fs');
const crypto = require('crypto');

// Load native module - tries prebuilds first, falls back to build/Release
let nativeKeyring;

// Determine prebuild path based on platform and architecture
const prebuildDir = path.join(__dirname, 'prebuilds', `${process.platform}-${process.arch}`);
const prebuildFile = path.join(prebuildDir, 'node-rsa-keys-generator.node');
const buildFile = path.join(__dirname, 'build', 'Release', 'keys_generator.node');

if (fs.existsSync(prebuildFile)) {
    nativeKeyring = require(prebuildFile);
} else if (fs.existsSync(buildFile)) {
    nativeKeyring = require(buildFile);
} else {
    try {
        nativeKeyring = require('node-gyp-build')(__dirname);
    } catch (e) {
        throw new Error(`Cannot find native module. Looked in:\n  - ${prebuildFile}\n  - ${buildFile}\nPlatform: ${process.platform}-${process.arch}`);
    }
}

const ACCOUNT = 'key';

/**
 * Get the default RSA key length based on platform and environment.
 * @returns {number} Key length in bits
 */
function getDefaultKeyLength() {
    // Windows always uses 1024 (issue #105 compatibility)
    if (process.platform === 'win32') return 1024;

    const envLength = process.env.RSA_KEY_LENGTH;
    if (envLength) {
        const parsed = parseInt(envLength, 10);
        if (!isNaN(parsed)) return parsed;
    }
    return 2048;
}

/**
 * Generate an RSA key pair using Node.js crypto.
 * @param {number} keyLength - Key length in bits
 * @returns {{publicKey: string, privateKey: string}|null} Key pair or null on failure
 */
function generateKeyPair(keyLength) {
    try {
        return crypto.generateKeyPairSync('rsa', {
            modulusLength: keyLength,
            publicKeyEncoding: { type: 'pkcs1', format: 'pem' },
            privateKeyEncoding: { type: 'pkcs1', format: 'pem' }
        });
    } catch (e) {
        // Fallback to 1024 if requested length fails
        if (keyLength !== 1024) {
            return generateKeyPair(1024);
        }
        return null;
    }
}

/**
 * Generate or retrieve RSA keys for credential encryption.
 * The serviceName is used as a prefix for keychain storage: {serviceName}PublicKey and {serviceName}PrivateKey.
 *
 * @param {string} serviceName - Service name prefix for keychain storage (required)
 * @param {number} [keyLength] - RSA key length in bits (default: from RSA_KEY_LENGTH env var or 2048)
 * @returns {string|null} - The public key in PEM format, or null if generation fails
 */
function generateKeys(serviceName, keyLength) {
    if (typeof serviceName !== 'string') {
        throw new TypeError('serviceName must be a string');
    }

    // Check for existing keys first
    const existing = getPublicKey(serviceName);
    if (existing) return existing;

    return regenerateKeys(serviceName, keyLength);
}

/**
 * Get the stored public key from the system keychain without generating new keys.
 * The serviceName is used to construct the keychain service name: {serviceName}PublicKey.
 *
 * @param {string} serviceName - Service name prefix for keychain storage (required)
 * @returns {string|null} - The stored public key in PEM format, or null if not found
 */
function getPublicKey(serviceName) {
    if (typeof serviceName !== 'string') {
        throw new TypeError('serviceName must be a string');
    }

    try {
        return nativeKeyring.getPassword(`${serviceName}PublicKey`, ACCOUNT) || null;
    } catch (e) {
        return null;
    }
}

/**
 * Get the stored private key from the system keychain.
 * The serviceName is used to construct the keychain service name: {serviceName}PrivateKey.
 *
 * @param {string} serviceName - Service name prefix for keychain storage (required)
 * @returns {string|null} - The stored private key in PEM format, or null if not found
 */
function getPrivateKey(serviceName) {
    if (typeof serviceName !== 'string') {
        throw new TypeError('serviceName must be a string');
    }

    try {
        return nativeKeyring.getPassword(`${serviceName}PrivateKey`, ACCOUNT) || null;
    } catch (e) {
        return null;
    }
}

/**
 * Check if the system keychain is available.
 *
 * @returns {boolean} - True if keychain is available, false otherwise
 */
function isKeychainAvailable() {
    return nativeKeyring.isKeychainAvailable();
}

/**
 * Get the current platform name.
 *
 * @returns {string} - Platform name ("Windows", "Linux", "macOS", or "Unknown")
 */
function getPlatform() {
    return nativeKeyring.getPlatform();
}

/**
 * Force regeneration of keys with the specified length.
 * This will generate new keys and store them in the keychain.
 * The serviceName is used as a prefix for keychain storage: {serviceName}PublicKey and {serviceName}PrivateKey.
 *
 * @param {string} serviceName - Service name prefix for keychain storage (required)
 * @param {number} [keyLength] - RSA key length in bits (default: 2048)
 * @returns {string|null} - The new public key in PEM format, or null if generation fails
 */
function regenerateKeys(serviceName, keyLength) {
    if (typeof serviceName !== 'string') {
        throw new TypeError('serviceName must be a string');
    }

    const len = keyLength || getDefaultKeyLength();
    const keys = generateKeyPair(len);
    if (!keys) return null;

    try {
        const pubSuccess = nativeKeyring.setPassword(`${serviceName}PublicKey`, ACCOUNT, keys.publicKey);
        const privSuccess = nativeKeyring.setPassword(`${serviceName}PrivateKey`, ACCOUNT, keys.privateKey);

        if (pubSuccess && privSuccess) {
            return keys.publicKey;
        }
    } catch (e) {
        // Silent failure
    }

    return null;
}

/**
 * Clear stored keys from the system keychain.
 *
 * @param {string} serviceName - Service name prefix for keychain storage (required)
 * @returns {boolean} - True if keys were deleted, false otherwise
 */
function clearKeys(serviceName) {
    if (typeof serviceName !== 'string') {
        throw new TypeError('serviceName must be a string');
    }

    try {
        const pubDeleted = nativeKeyring.deletePassword(`${serviceName}PublicKey`, ACCOUNT);
        const privDeleted = nativeKeyring.deletePassword(`${serviceName}PrivateKey`, ACCOUNT);
        return pubDeleted || privDeleted;
    } catch (e) {
        return false;
    }
}

module.exports = {
    generateKeys,
    getPublicKey,
    getPrivateKey,
    isKeychainAvailable,
    getPlatform,
    regenerateKeys,
    clearKeys
};
