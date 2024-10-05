# Instant Messaging Program
Sending encrypted messages between clients using end-to-end encryption through a TCP server.
&nbsp;

## Technology Stack
- **Clients (C++)**: handles cryptographic operations and message encryption.
- **Server (Python)**: a TCP server that handles requests from clients in a stateless manner and interacts with the database.
- **SQLite Database**: a lightweight database that stores unpulled messages and client data.

## Cryptography
- **Crypto++**: Used by the clients for generating public/private key pairs and performing encryption/decryption operations with asymmetric (public/private key) and symmetric (shared key) encryption.
- **End-to-End Encryption**: Messages are encrypted using **public-key cryptography** for the initial key exchange and symmetric encryption for message transmission, ensuring end-to-end security. The server only relays encrypted messages without access to their content.

```mermaid 
sequenceDiagram
    participant Client A
    participant Server
    participant Client B

    Client A->>Client A: generatePrivateKey()
    Client A->>Client A: generatePublicKey()
    Client A->>Server: register(APubKey, Username)
    Client A->>Server: get clients list
    Server ->> Client A: <ClientsList>
    Client A->>Server: getPubKey(ClientB)
    Server->>Client A: <BPubKey>
    Client A->>Server: send to B: get symmetric key
    Note over Client A, Server: message encrypted by BPubKey
    loop
        Client B->>Server: pull messages
    end
    Server->>Client B: from A: get symmetric key
    Note over Server, Client B: message decrypted by BPriKey
    Client B->>Server: getPubKey(ClientA)
    Server->>Client B: <APubKey>
    Client B->>Server: send to A: <SymKey>
    Note over Server, Client B: message encrypted by APubKey
    loop
        Client A->>Server: pull messages
    end
    Server->>Client A: from B: <SymKey>
    Note over Server, Client A: message decrypted by APriKey
    Client A->>Server: send to B: message
    Note over Client A, Server: message encrypted by SymKey
    Server->>Client B: from A: message
    Note over Server, Client B: message decrypted by SymKey
