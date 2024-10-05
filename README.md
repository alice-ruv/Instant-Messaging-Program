# Instant Messaging Program
Sending encrypted messages between clients (C++) using end-to-end encryption through a TCP stateless server (Python), which stores unpulled messages and client data in an SQLite database.

```mermaid 
sequenceDiagram
    participant Client A
    participant Server
    participant Client B

    rect rgba(100, 100, 100, .2)
    note right of Client A: Crypto++
    Client A->>Client A: generatePrivateKey()
    Client A->>Client A: generatePublicKey()
    end
    Client A->>Server: register(APubKey, Username)
    Client A->>Server: getPubKey(ClientB)
    Server->>Client A: BPubKey
    Client A->>Server: send to B: get symmetric key
    Note over Client A, Server: message encrypted by BPubKey
    loop
        Client B->>Server: pull messages
    end
    Server->>Client B: from A: get symmetric key
    Note over Server, Client B: message decrypted by BPriKey
    Client B->>Server: getPubKey(ClientA)
    Server->>Client B: APubKey
    Client B->>Server: send to A: SymKey
    Note over Server, Client B: message encrypted by APubKey
    loop
        Client A->>Server: pull messages
    end
    Server->>Client A: from B: SymKey
    Note over Server, Client A: message decrypted by APriKey
    Client A->>Server: send to B: message
    Note over Client A, Server: message encrypted by SymKey
    Server->>Client B: from A: message
    Note over Server, Client B: message decrypted by SymKey
