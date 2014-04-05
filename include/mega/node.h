/**
 * @file mega/node.h
 * @brief Classes for accessing local and remote nodes
 *
 * (c) 2013-2014 by Mega Limited, Wellsford, New Zealand
 *
 * This file is part of the MEGA SDK - Client Access Engine.
 *
 * Applications using the MEGA API must present a valid application key
 * and comply with the the rules set forth in the Terms of Service.
 *
 * The MEGA SDK is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 *
 * @copyright Simplified (2-clause) BSD License.
 *
 * You should have received a copy of the license along with this
 * program.
 */

#ifndef MEGA_NODE_H
#define MEGA_NODE_H 1

#include "filefingerprint.h"
#include "file.h"
#include "attrmap.h"

namespace mega {
struct MEGA_API NodeCore
{
    // node's own handle
    handle nodehandle;

    // parent node handle (in a Node context, temporary placeholder until parent is set)
    handle parenthandle;

    // node type
    nodetype_t type;

    // full folder/file key, symmetrically or asymmetrically encrypted
    string nodekey;

    // new node's client-controlled timestamp (should be last modification)
    time_t clienttimestamp;

    // node attributes
    string attrstring;
};

// new node for putnodes()
struct MEGA_API NewNode : public NodeCore
{
    static const int UPLOADTOKENLEN = 27;

    newnodesource_t source;

    handle uploadhandle;
    byte uploadtoken[UPLOADTOKENLEN];

    handle syncid;
    LocalNode* localnode;

    NewNode()
    {
        syncid = UNDEF;
    }
};

// filesystem node
struct MEGA_API Node : public NodeCore, Cachable, FileFingerprint
{
    MegaClient* client;

    // node crypto keys
    string keystring;

    // change parent node association
    bool setparent(Node*);

    // copy JSON-delimited string
    static void copystring(string*, const char*);

    // try to resolve node key string
    bool applykey();

    // decrypt attribute string and set fileattrs
    void setattr();

    // display name (UTF-8)
    const char* displayname();

    // node-specific key
    SymmCipher key;

    // node attributes
    AttrMap attrs;

    // owner
    handle owner;

    // actual time this node was created (cannot be set by user)
    time_t ctime;

    // FILENODE nodes only: nonce, meta MAC, attribute string
    int64_t ctriv;
    int64_t metamac;
    string fileattrstring;

    // check presence of file attribute
    int hasfileattribute(fatype) const;

    // decrypt node attribute string
    static byte* decryptattr(SymmCipher*, const char*, int);

    // inbound share
    Share* inshare;

    // outbound shares by user
    share_map outshares;

    // incoming/outgoing share key
    SymmCipher* sharekey;

    // app-private pointer
    void* appdata;

    bool removed;

    struct
    {
        bool attrs : 1;
        bool owner : 1;
        bool ctime : 1;
        bool clienttimestamp : 1;
        bool fileattrstring : 1;
        bool inshare : 1;
        bool outshares : 1;
        bool parent : 1;
    } changed;
    
    void setkey(const byte* = NULL);

    void setfingerprint();

    void faspec(string*);

    // parent
    Node* parent;

    // children
    node_list children;

    // own position in parent's children
    node_list::iterator child_it;

    // own position in fingerprint set (only valid for file nodes)
    fingerprint_set::iterator fingerprint_it;

    // related synced item or NULL
    LocalNode* localnode;

    // active sync get
    struct SyncFileGet* syncget;

    // state of removal to //bin / SyncDebris
    syncdel_t syncdeleted;

    // location in the todebris node_set
    node_set::iterator todebris_it;

    // source tag
    int tag;

    // check if node is below this node
    bool isbelow(Node*) const;

    bool serialize(string*);
    static Node* unserialize(MegaClient*, string*, node_vector*);

    Node(MegaClient*, vector<Node*>*, handle, handle, nodetype_t, m_off_t, handle, const char*, time_t, time_t);
    ~Node();
};

struct MEGA_API LocalNode : public File, Cachable
{
    class Sync * sync;

    // parent linkage
    LocalNode* parent;

    // stored to rebuild tree after serialization => this must not be a pointer to parent->dbid
    int32_t parent_dbid;

    // children by name
    localnode_map children;

    // for filesystems with botched secondary ("short") names
    string slocalname;
    localnode_map schildren;

    // local filesystem node ID (inode...) for rename/move detection
    handle fsid;
    handlelocalnode_map::iterator fsid_it;

    // related cloud node, if any
    Node* node;

    // related pending node creation or NULL
    NewNode* newnode;

    // FILENODE or FOLDERNODE
    nodetype_t type;

    // detection of deleted filesystem records
    int scanseqno;

    // number of iterations since last seen
    int notseen;

    // global sync reference
    handle syncid;

    // was actively deleted
    bool deleted;

    // current subtree sync state: current and displayed
    treestate_t ts, dts;

    // update sync state all the way to the root node
    void treestate(treestate_t = TREESTATE_NONE);

    // timer to delay upload start
    dstime nagleds;
    void bumpnagleds();

    // if delage > 0, own iterator inside MegaClient::localsyncgone
    localnode_set::iterator notseen_it;

    // build full local path to this node
    void getlocalpath(string*, bool sdisable = false);
    void getlocalsubpath(string*);

    // return child node by name
    LocalNode* childbyname(string*);

    // node-specific DirNotify tag
    handle dirnotifytag;

    void prepare();
    void completed(Transfer*, LocalNode*);

    void setnode(Node*);

    void setnotseen(int);

    void setfsid(handle);

    void setnameparent(LocalNode*, string*);

    void init(Sync*, nodetype_t, LocalNode*, string*, string*);

    virtual bool serialize(string*);
    static LocalNode* unserialize( Sync* sync, string* sData, LocalNode* parent = NULL );

    ~LocalNode();
};
} // namespace

#endif
