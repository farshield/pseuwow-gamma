#ifndef _WORLDSESSION_H
#define _WORLDSESSION_H

#include <deque>
#include <bitset>

#include "common.h"
#include "PseuWoW.h"
#include "Network/SocketHandler.h"
#include "Player.h"
#include "Auth/AuthCrypt.h"
#include "SharedDefines.h"
#include "ObjMgr.h"
#include "CacheHandler.h"
#include "Opcodes.h"

class WorldSocket;
class WorldPacket;
class Channel;
class RealmSession;
struct OpcodeHandler;
class World;

struct WhoListEntry
{
    std::string name;
    std::string gname;
    uint32 level;
    uint32 classId;
    uint32 raceId;
    uint32 zoneId;
};

struct DelayedWorldPacket
{
    DelayedWorldPacket() { pkt = NULL; when = clock(); }
    DelayedWorldPacket(WorldPacket *p, uint32 ms) { pkt = p; when = ms + clock(); }
    WorldPacket *pkt;
    clock_t when;
};

// helper used for GUI
struct CharacterListExt
{
    PlayerEnum p;
    std::string zone;
    std::string class_;
    std::string race;
    std::string map_;
};

enum PartyOperation
{
    PARTY_OP_INVITE = 0,
    PARTY_OP_LEAVE = 2,
    PARTY_OP_SWAP = 4
};

enum PartyResult
{
    ERR_PARTY_RESULT_OK                 = 0,
    ERR_BAD_PLAYER_NAME_S               = 1,
    ERR_TARGET_NOT_IN_GROUP_S           = 2,
    ERR_TARGET_NOT_IN_INSTANCE_S        = 3,
    ERR_GROUP_FULL                      = 4,
    ERR_ALREADY_IN_GROUP_S              = 5,
    ERR_NOT_IN_GROUP                    = 6,
    ERR_NOT_LEADER                      = 7,
    ERR_PLAYER_WRONG_FACTION            = 8,
    ERR_IGNORING_YOU_S                  = 9,
    ERR_LFG_PENDING                     = 12,
    ERR_INVITE_RESTRICTED               = 13,
    ERR_GROUP_SWAP_FAILED               = 14,               // if (PartyOperation == PARTY_OP_SWAP) ERR_GROUP_SWAP_FAILED else ERR_INVITE_IN_COMBAT
    ERR_INVITE_UNKNOWN_REALM            = 15,
    ERR_INVITE_NO_PARTY_SERVER          = 16,
    ERR_INVITE_PARTY_BUSY               = 17,
    ERR_PARTY_TARGET_AMBIGUOUS          = 18,
    ERR_PARTY_LFG_INVITE_RAID_LOCKED    = 19,
    ERR_PARTY_LFG_BOOT_LIMIT            = 20,
    ERR_PARTY_LFG_BOOT_COOLDOWN_S       = 21,
    ERR_PARTY_LFG_BOOT_IN_PROGRESS      = 22,
    ERR_PARTY_LFG_BOOT_TOO_FEW_PLAYERS  = 23,
    ERR_PARTY_LFG_BOOT_NOT_ELIGIBLE_S   = 24,
    ERR_RAID_DISALLOWED_BY_LEVEL        = 25,
    ERR_PARTY_LFG_BOOT_IN_COMBAT        = 26,
    ERR_VOTE_KICK_REASON_NEEDED         = 27,
    ERR_PARTY_LFG_BOOT_DUNGEON_COMPLETE = 28,
    ERR_PARTY_LFG_BOOT_LOOT_ROLLS       = 29,
    ERR_PARTY_LFG_TELEPORT_IN_COMBAT    = 30
};

typedef std::vector<WhoListEntry> WhoList;
typedef std::vector<CharacterListExt> CharList;
typedef std::deque<DelayedWorldPacket> DelayedPacketQueue;

class WorldSession
{
    friend class Channel;

public:
    WorldSession(PseuInstance *i);
    ~WorldSession();
    void Init(void);

    inline PseuInstance *GetInstance(void) { return _instance; }
    inline SCPDatabaseMgr& GetDBMgr(void) { return GetInstance()->dbmgr; }

    void AddToPktQueue(WorldPacket *pkt);
    void Update(void);
    void Start(void);
    inline bool MustDie(void) { return _mustdie; }
    void SetMustDie(void);
    void SendWorldPacket(WorldPacket&);
    void AddSendWorldPacket(WorldPacket *pkt);
    void AddSendWorldPacket(WorldPacket& pkt);
    inline bool InWorld(void) { return _logged; }
    inline uint32 GetLagMS(void) { return _lag_ms; }

    void SetTarget(uint64 guid);
    inline uint64 GetTarget(void) { return GetMyChar() ? GetMyChar()->GetTarget() : 0; }
    inline uint64 GetGuid(void) { return _myGUID; }
    inline Channel *GetChannels(void) { return _channels; }
    inline MyCharacter *GetMyChar(void) { ASSERT(_myGUID > 0); return (MyCharacter*)objmgr.GetObj(_myGUID); }
    inline World *GetWorld(void) { return _world; }

    std::string GetOrRequestPlayerName(uint64);
    std::string DumpPacket(WorldPacket& pkt, int errpos = -1, const char *errstr = NULL);

    inline uint32 GetCharsCount(void) { return _charList.size(); }
    inline CharacterListExt& GetCharFromList(uint32 id) { return _charList[id]; }
    void EnterWorldWithCharacter(std::string);
    void PreloadDataBeforeEnterWorld(PlayerEnum&);


    // CMSGConstructor
    void SendChatMessage(uint32 type, uint32 lang, std::string msg, std::string to="");
    void SendQueryPlayerName(uint64 guid);
    void SendPing(uint32);
    void SendEmote(uint32);
    void SendQueryItem(uint32 id, uint64 guid = 0);
    void SendSetSelection(uint64);
    void SendCastSpell(uint32 spellid, bool nocheck=false);
    void SendWhoListRequest(uint32 minlvl=0, uint32 maxlvl=100, uint32 racemask=-1, uint32 classmask=-1, std::string name="", std::string guildname="", std::vector<uint32> *zonelist=NULL, std::vector<std::string> *strlist=NULL);
    void SendQueryCreature(uint32 entry, uint64 guid = 0);
    void SendQueryGameobject(uint32 entry, uint64 guid = 0);
    void SendCharCreate(std::string name, uint8 race, uint8 class_, uint8 gender=0, uint8 skin=0, uint8 face=0, uint8 hairstyle=0, uint8 haircolor=0, uint8 facial=0, uint8 outfit=0);

    void SendGroupInvite(std::string playername);
    void SendGroupUninviteGuid(uint64 guid, std::string msg = "");
    void SendGroupUninvite(std::string playername);
    void SendGroupInviteResponse(std::string msg);
    void SendGroupAccept();
    void SendGroupDecline();
    void SendGroupSetLeader(uint64 guid);

    void HandleWorldPacket(WorldPacket*);

    inline void DisableOpcode(uint16 opcode) { _disabledOpcodes[opcode] = true; }
    inline void EnableOpcode(uint16 opcode) { _disabledOpcodes[opcode] = false; }
    inline bool IsOpcodeDisabled(uint16 opcode) { return _disabledOpcodes[opcode]; }

    PlayerNameCache plrNameCache;
    ObjMgr objmgr;


private:

    OpcodeHandler *_GetOpcodeHandlerTable(void) const;

    // Helpers
    void _OnEnterWorld(void); // = login
    void _OnLeaveWorld(void); // = logout
    void _DoTimedActions(void);
    void _DelayWorldPacket(WorldPacket&, uint32);
    void _HandleDelayedPackets(void);

    // Opcode Handlers
    void _HandleAuthChallengeOpcode(WorldPacket& recvPacket);
    void _HandleAuthResponseOpcode(WorldPacket& recvPacket);
    void _HandleCharEnumOpcode(WorldPacket& recvPacket);
    void _HandleSetProficiencyOpcode(WorldPacket& recvPacket);
    void _HandleAccountDataMD5Opcode(WorldPacket& recvPacket);
    void _HandleMessageChatOpcode(WorldPacket& recvPacket);
    void _HandleNameQueryResponseOpcode(WorldPacket& recvPacket);
    void _HandleMovementOpcode(WorldPacket& recvPacket);
    void _HandleSetSpeedOpcode(WorldPacket& recvPacket);
    void _HandleForceSetSpeedOpcode(WorldPacket& recvPacket);
    void _HandlePongOpcode(WorldPacket& recvPacket);
    void _HandleTradeStatusOpcode(WorldPacket& recvPacket);
    void _HandleGroupInviteOpcode(WorldPacket& recvPacket);
    void _HandleGroupUninviteOpcode(WorldPacket& recvPacket);
    void _HandleGroupDeclineOpcode(WorldPacket& recvPacket);
    void _HandlePartyCommandResultOpcode(WorldPacket& recvPacket);
	void _HandleTelePortAckOpcode(WorldPacket& recvPacket);
    void _HandleChannelNotifyOpcode(WorldPacket& recvPacket);
	void _HandleCastResultOpcode(WorldPacket& recvPacket);
    void _HandleCastSuccessOpcode(WorldPacket& recvPacket);
    void _HandleCompressedUpdateObjectOpcode(WorldPacket& recvPacket);
    void _HandleUpdateObjectOpcode(WorldPacket& recvPacket);
    void _HandleItemQuerySingleResponseOpcode(WorldPacket& recvPacket);
    void _HandleDestroyObjectOpcode(WorldPacket& recvPacket);
    void _HandleInitialSpellsOpcode(WorldPacket& recvPacket);
    void _HandleLearnedSpellOpcode(WorldPacket& recvPacket);	
    void _HandleRemovedSpellOpcode(WorldPacket& recvPacket);
	void _HandleChannelListOpcode(WorldPacket& recvPacket);
    void _HandleEmoteOpcode(WorldPacket& recvPacket);
    void _HandleTextEmoteOpcode(WorldPacket& recvPacket);
    void _HandleNewWorldOpcode(WorldPacket& recvPacket);
    void _HandleLoginVerifyWorldOpcode(WorldPacket& recvPacket);
    void _HandleMotdOpcode(WorldPacket& recvPacket);
    void _HandleNotificationOpcode(WorldPacket& recvPacket);
    void _HandleWhoOpcode(WorldPacket& recvPacket);
    void _HandleCreatureQueryResponseOpcode(WorldPacket& recvPacket);
    void _HandleGameobjectQueryResponseOpcode(WorldPacket& recvPacket);
    void _HandleCharCreateOpcode(WorldPacket& recvPacket);
    void _HandleCharDeleteOpcode(WorldPacket& recvPacket);
    void _HandleMonsterMoveOpcode(WorldPacket& recvPacket);

    // helper functions to keep SMSG_(COMPRESSED_)UPDATE_OBJECT easy to handle
	void _MovementUpdate(uint8 objtypeid, uint64 guid, WorldPacket& recvPacket); // Helper for _HandleUpdateObjectOpcode
    void _ValuesUpdate(uint64 uguid, WorldPacket& recvPacket); // ...
    void _QueryObjectInfo(uint64 guid);

    void _LoadCache(void);

    PseuInstance *_instance;
    WorldSocket *_socket;
    ZThread::LockedQueue<WorldPacket*,ZThread::FastMutex> pktQueue, sendPktQueue;
    DelayedPacketQueue delayedPktQueue;
    bool _logged,_mustdie; // world status
    SocketHandler _sh; // handles the WorldSocket
    Channel *_channels;
    uint64 _myGUID;
    World *_world;
    WhoList _whoList;
    CharList _charList;
    uint32 _lag_ms;
    std::bitset<MAX_OPCODE_ID> _disabledOpcodes;

    int32 _partyacceptexpire;
};

#endif
