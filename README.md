# Node RSA Keys Generator

A cross-platform Node.js native module for generating RSA key pairs and securely storing them in the OS keychain.

## Overview

This module provides secure RSA key generation and management for credential encryption. Keys are automatically stored in the operating system's native keychain (Windows Credential Manager, GNOME Keyring, or macOS Keychain) for secure, persistent storage.

**Architecture:**
- RSA key generation: Node.js built-in `crypto` module
- Keychain storage: Native C++ module via N-API

This design eliminates external runtime dependencies while maintaining cross-platform compatibility.

## Features

- **Secure Key Storage**: Keys stored in OS-native keychains
- **Cross-Platform**: Full support for Windows, Linux, and macOS
- **No External Dependencies**: No OpenSSL or other runtime dependencies required
- **Python Compatible**: Interoperable with Python's keyring library
- **TypeScript Support**: Complete TypeScript definitions included
- **PKCS#1 Format**: PEM output compatible with Python pycryptodome

## Installation

```bash
npm install node-rsa-keys-generator
```

### System Requirements

**All Platforms:**
- Node.js >= 16.0.0

**For building from source:**

**Ubuntu/Debian:**
```bash
sudo apt-get update
sudo apt-get install build-essential libsecret-1-dev pkg-config
```

**CentOS/RHEL:**
```bash
sudo yum install gcc gcc-c++ libsecret-devel pkgconfig
```

**Fedora:**
```bash
sudo dnf install gcc gcc-c++ libsecret-devel pkgconfig
```

**Windows:**
- Visual Studio 2019 or later with C++ tools

**macOS:**
- Xcode Command Line Tools (no additional dependencies)

## Quick Start

### JavaScript

```javascript
const keysGenerator = require('node-rsa-keys-generator');

// serviceName is REQUIRED and used as prefix for keychain storage
// Keys stored as: {serviceName}PublicKey and {serviceName}PrivateKey

// Generate or retrieve RSA keys for your application
const publicKey = keysGenerator.generateKeys('MyApp');
console.log('Public Key:', publicKey);

// Check if keychain is available
if (keysGenerator.isKeychainAvailable()) {
    console.log('Keychain is available for secure storage');
}

// Get current platform
console.log('Platform:', keysGenerator.getPlatform());
```

### TypeScript

```typescript
import * as keysGenerator from 'node-rsa-keys-generator';

// serviceName is required as first parameter
const publicKey: string | null = keysGenerator.generateKeys('MyApp', 2048);

if (publicKey) {
    console.log('Successfully generated keys');
}
```

## API Reference

### `generateKeys(serviceName, keyLength?)`

Generates a new RSA key pair or retrieves existing keys from the keychain.

**Parameters:**
- `serviceName` (string, **required**): Service name prefix for keychain storage
- `keyLength` (number, optional): RSA key length in bits (default: 2048, or from `RSA_KEY_LENGTH` env var)

**Returns:** `string | null` - The public key in PEM format, or null if generation fails.

---

### `regenerateKeys(serviceName, keyLength?)`

Forces generation of new RSA keys, replacing any existing keys in the keychain.

**Parameters:**
- `serviceName` (string, **required**): Service name prefix for keychain storage
- `keyLength` (number, optional): RSA key length in bits (default: 2048)

**Returns:** `string | null` - The new public key in PEM format, or null if generation fails.

---

### `getPublicKey(serviceName)`

Retrieves the stored public key from the keychain without generating new keys.

**Parameters:**
- `serviceName` (string, **required**): Service name prefix for keychain storage

**Returns:** `string | null` - The stored public key in PEM format, or null if not found.

---

### `getPrivateKey(serviceName)`

Retrieves the stored private key from the keychain.

**Parameters:**
- `serviceName` (string, **required**): Service name prefix for keychain storage

**Returns:** `string | null` - The stored private key in PEM format, or null if not found.

---

### `clearKeys(serviceName)`

Deletes stored keys from the keychain.

**Parameters:**
- `serviceName` (string, **required**): Service name prefix for keychain storage

**Returns:** `boolean` - True if keys were deleted, false otherwise.

---

### `isKeychainAvailable()`

Checks if the system keychain is available for secure storage.

**Returns:** `boolean` - True if keychain is available, false otherwise.

---

### `getPlatform()`

Returns the current operating system platform.

**Returns:** `"Windows" | "Linux" | "macOS" | "Unknown"`

## Platform Support

| Platform | Keychain Backend | Status |
|----------|------------------|--------|
| Windows  | Credential Manager | Supported |
| Linux    | GNOME Keyring (libsecret) | Supported |
| macOS    | macOS Keychain | Supported |

### Python Keyring Compatibility

This module is fully compatible with Python's `keyring` library. Keys created with Python's keyring can be read by this module, and vice versa.

```javascript
// Python code that created the keys:
// import keyring
// keyring.set_password('MyAppPublicKey', 'key', public_key)
// keyring.set_password('MyAppPrivateKey', 'key', private_key)

// Node.js code to read the same keys:
const keysGenerator = require('node-rsa-keys-generator');
const publicKey = keysGenerator.getPublicKey('MyApp');
const privateKey = keysGenerator.getPrivateKey('MyApp');
```

## Examples

### Basic Usage

```javascript
const keysGenerator = require('node-rsa-keys-generator');

const publicKey = keysGenerator.generateKeys('MyApplication');
console.log('Public Key Generated:', publicKey ? 'Yes' : 'No');
```

### Multiple Applications

```javascript
const keysGenerator = require('node-rsa-keys-generator');

// Generate keys for different applications
const appAPublic = keysGenerator.generateKeys('ApplicationA', 2048);
const appBPublic = keysGenerator.generateKeys('ApplicationB', 4096);

// Retrieve specific application keys
const appAPrivate = keysGenerator.getPrivateKey('ApplicationA');
```

### Key Rotation

```javascript
const keysGenerator = require('node-rsa-keys-generator');

const serviceName = 'ProductionApp';

// Check if keys exist
let publicKey = keysGenerator.getPublicKey(serviceName);

if (publicKey) {
    console.log('Existing keys found - rotating');
    publicKey = keysGenerator.regenerateKeys(serviceName, 2048);
} else {
    publicKey = keysGenerator.generateKeys(serviceName, 2048);
    console.log('Initial keys generated');
}
```

### Cleanup

```javascript
const keysGenerator = require('node-rsa-keys-generator');

// Remove keys when no longer needed
const deleted = keysGenerator.clearKeys('MyApplication');
console.log('Keys deleted:', deleted);
```

## Environment Variables

- `RSA_KEY_LENGTH`: Default RSA key length in bits (default: 2048)

## Troubleshooting

### Linux: libsecret not found

```bash
sudo apt-get install libsecret-1-dev  # Debian/Ubuntu
sudo yum install libsecret-devel      # CentOS/RHEL
sudo dnf install libsecret-devel      # Fedora
```

### Keys not persisting

```javascript
if (!keysGenerator.isKeychainAvailable()) {
    console.error('Keychain not available - keys will not persist');
}
```

### Linux: No keyring daemon running

On headless Linux systems, you may need to start a keyring daemon or use a different secret service provider.

## License

MIT
