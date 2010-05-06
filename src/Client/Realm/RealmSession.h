#ifndef REALMSESSION_H
#define REALMSESSION_H

#include "common.h"
#include "Auth/MD5Hash.h"

enum RealmFlags
{
    REALM_FLAG_NONE         = 0x00,
    REALM_FLAG_INVALID      = 0x01,
    REALM_FLAG_OFFLINE      = 0x02,
    REALM_FLAG_SPECIFYBUILD = 0x04,                         // client will show realm version in RealmList screen in form "RealmName (major.minor.revision.build)"
    REALM_FLAG_UNK1         = 0x08,
    REALM_FLAG_UNK2         = 0x10,
    REALM_FLAG_NEW_PLAYERS  = 0x20,
    REALM_FLAG_RECOMMENDED  = 0x40,
    REALM_FLAG_FULL         = 0x80
};

struct SRealmInfo
{
    uint8       icon;           // icon near realm
    uint8       locked;         // added in 2.0.x
    uint8       realmFlags;     // see enum RealmFlags
    std::string name;           // Text zero terminated name of Realm
    std::string addr_port;      // Text zero terminated address of Realm ("ip:port")
    float       population;     // 1.6 -> population value. lower == lower population and vice versa
    uint8       chars_here;     // number of characters on this server
    uint8       timezone;       // timezone
    uint8       unknown;
    uint8       major_version;
    uint8       minor_version;
    uint8       bugfix_version;
    uint16      _build;
};

struct AuthHandler;
class RealmSocket;

class RealmSession
{
public:
    RealmSession(PseuInstance*);
    ~RealmSession();
    void AddToPktQueue(ByteBuffer*);
    void Connect(void);
    void Update(void);
    PseuInstance *GetInstance(void);
    void ClearSocket(void);
    void SetLogonData(void);
    void SendLogonChallenge(void);
    bool MustDie(void);
    void SetMustDie(void);
    bool SocketGood(void);
    void SetRealmAddr(std::string);
    inline uint32 GetRealmCount(void) { return _realms.size(); }
    inline SRealmInfo& GetRealm(uint32 i) { return _realms[i]; }


private:
    void _HandleRealmList(ByteBuffer&);
    void _HandleLogonProof(ByteBuffer&);
    void _HandleLogonChallenge(ByteBuffer&);
    void _HandleTransferInit(ByteBuffer&);
    void _HandleTransferData(ByteBuffer&);
    AuthHandler *_GetAuthHandlerTable(void) const;
    void SendRealmPacket(ByteBuffer&);
    void DumpInvalidPacket(ByteBuffer&);
    void DieOrReconnect(bool err = false);
    std::string _accname,_accpass;
    SocketHandler _sh;
    PseuInstance *_instance;
    ZThread::LockedQueue<ByteBuffer*,ZThread::FastMutex> pktQueue;
    RealmSocket *_socket;
    uint8 _m2[20];
    RealmSession *_session;
    BigNumber _key;
    bool _mustdie;
    bool _filetransfer;
    uint8 _file_md5[MD5_DIGEST_LENGTH];
    uint64 _file_done, _file_size;
    ByteBuffer _filebuf;
    ByteBuffer _transbuf; // stores parts of unfinished packets
    std::vector<SRealmInfo> _realms;
};


#endif
