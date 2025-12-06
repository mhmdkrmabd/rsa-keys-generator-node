/**
 * Node RSA Keys Generator - Native Node.js module for generating RSA keys
 * and securely storing them in OS keychain.
 */

/**
 * Generate or retrieve RSA keys for credential encryption.
 * The serviceName is used as a prefix for keychain storage: {serviceName}PublicKey and {serviceName}PrivateKey.
 *
 * @param serviceName - Service name prefix for keychain storage (required)
 * @param keyLength - RSA key length in bits (default: from RSA_KEY_LENGTH env var or 2048)
 * @returns The public key in PEM format, or null if generation fails
 */
export function generateKeys(serviceName: string, keyLength?: number): string | null;

/**
 * Get the stored public key from the system keychain without generating new keys.
 * The serviceName is used to construct the keychain service name: {serviceName}PublicKey.
 *
 * @param serviceName - Service name prefix for keychain storage (required)
 * @returns The stored public key in PEM format, or null if not found
 */
export function getPublicKey(serviceName: string): string | null;

/**
 * Get the stored private key from the system keychain.
 * The serviceName is used to construct the keychain service name: {serviceName}PrivateKey.
 *
 * @param serviceName - Service name prefix for keychain storage (required)
 * @returns The stored private key in PEM format, or null if not found
 */
export function getPrivateKey(serviceName: string): string | null;

/**
 * Check if the system keychain is available.
 *
 * @returns True if keychain is available, false otherwise
 */
export function isKeychainAvailable(): boolean;

/**
 * Get the current platform name.
 *
 * @returns Platform name ("Windows", "Linux", "macOS", or "Unknown")
 */
export function getPlatform(): "Windows" | "Linux" | "macOS" | "Unknown";

/**
 * Force regeneration of keys with the specified length.
 * This will generate new keys and store them in the keychain.
 * The serviceName is used as a prefix for keychain storage: {serviceName}PublicKey and {serviceName}PrivateKey.
 *
 * @param serviceName - Service name prefix for keychain storage (required)
 * @param keyLength - RSA key length in bits (default: 2048)
 * @returns The new public key in PEM format, or null if generation fails
 */
export function regenerateKeys(serviceName: string, keyLength?: number): string | null;

/**
 * Clear stored keys from the system keychain.
 *
 * @param serviceName - Service name prefix for keychain storage (required)
 * @returns True if keys were deleted, false otherwise
 */
export function clearKeys(serviceName: string): boolean;

/**
 * Default export of the module
 */
declare const keysGenerator: {
    generateKeys: typeof generateKeys;
    getPublicKey: typeof getPublicKey;
    getPrivateKey: typeof getPrivateKey;
    isKeychainAvailable: typeof isKeychainAvailable;
    getPlatform: typeof getPlatform;
    regenerateKeys: typeof regenerateKeys;
    clearKeys: typeof clearKeys;
};

export default keysGenerator;