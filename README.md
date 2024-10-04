# Instant Messaging Program
Sending encrypted messages between clients (C++) using end-to-end encryption through a TCP stateless server (Python), which stores unpulled messages and client data in an SQLite DB.

```mermaid 
sequenceDiagram
    participant Client A
    participant Server
    participant Client B

    Client A->>Server: register(APubKey, Username)
    Server->>Client A: BPubKey
    Client A->>Client B: get symmetric key
    Client A->>Client B: message encrypted by BPubKey
    Client B->>Server: pull messages
    Server->>Client B: from A: GetSymKey
    Client B->>Client B: message decrypted by BPriKey
    Client B->>Server: getPubKey(ClientA)
    Server->>Client B: APubKey
    Client B->>Client A: send to A: SymKey
    Client B->>Client A: message encrypted by APubKey
    Client A->>Server: pull messages
    Server->>Client A: from B: SymKey
    Client A->>Client A: message decrypted by APriKey
    Client A->>Client B: send to B: message
    Client A->>Client B: message encrypted by SymKey
    Client B->>Server: pull messages
    Server->>Client B: from A: message
    Client B->>Client B: message decrypted by SymKey
