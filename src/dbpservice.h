// Copyright (c) 2017-2019 The Multiverse developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef MULTIVERSE_DBP_SERVICE_H
#define MULTIVERSE_DBP_SERVICE_H

#include "mvbase.h"
#include "dbpserver.h"
#include "event.h"
#include "virtualpeernetevent.h"
#include "mvpeernet.h"
#include "walleve/walleve.h"

#include <set>
#include <utility>
#include <unordered_map>


namespace multiverse
{

using namespace network;

class CDbpService : public walleve::IIOModule, virtual public CDBPEventListener, 
                    virtual public CMvDBPEventListener, virtual public CMvPeerEventListener, 
                    virtual public CWallevePeerEventListener 
{
public:
    CDbpService();
    virtual ~CDbpService() noexcept;

    void EnableForkNode(bool enable);
    void EnableSuperNode(bool enable);

    bool HandleEvent(CMvEventDbpConnect& event) override;
    bool HandleEvent(CMvEventDbpSub& event) override;
    bool HandleEvent(CMvEventDbpUnSub& event) override;
    bool HandleEvent(CMvEventDbpMethod& event) override;
    bool HandleEvent(CMvEventDbpPong& event) override;
    bool HandleEvent(CMvEventDbpBroken& event) override;
    bool HandleEvent(CMvEventDbpRemoveSession& event) override;
    
    // notify add msg(block tx ...) to event handler
    bool HandleEvent(CMvEventDbpUpdateNewBlock& event) override;
    bool HandleEvent(CMvEventDbpUpdateNewTx& event) override;

    // from virtualpeernet
    bool HandleEvent(CMvEventPeerActive& event) override;
    bool HandleEvent(CMvEventPeerDeactive& event) override;
    bool HandleEvent(CMvEventPeerSubscribe& event) override;
    bool HandleEvent(CMvEventPeerUnsubscribe& event) override;
    bool HandleEvent(CMvEventPeerInv& event) override;
    bool HandleEvent(CMvEventPeerBlock& event) override;
    bool HandleEvent(CMvEventPeerTx& event) override;
    bool HandleEvent(CMvEventPeerGetBlocks& event) override;
    bool HandleEvent(CMvEventPeerGetData& event) override;
    
    bool HandleEvent(CWalleveEventPeerNetReward& event) override;
    bool HandleEvent(CWalleveEventPeerNetClose& event) override;
    
    // from up node
    bool HandleEvent(CMvEventDbpVirtualPeerNet& event) override;

    //RPC Route
    bool HandleEvent(CMvEventRPCRouteStop& event) override;

protected:
    bool WalleveHandleInitialize() override;
    void WalleveHandleDeinitialize() override;

private:
    typedef std::set<std::string> ForksType;
    typedef std::set<std::string> IdsType;
    typedef std::tuple<int, std::string> ForkStates; // (lastHeight, lastBlockHash)
    typedef std::tuple<std::string, std::string, int> ParentForkInfo;  // (parentForkHash, parentJointHash, parentJointPointHeight)
    typedef std::vector<ParentForkInfo>  ForkTopology;
    
    bool CalcForkPoints(const uint256& forkHash);
    void TrySwitchFork(const uint256& blockHash, uint256& forkHash);
    bool GetLwsBlocks(const uint256& forkHash, const uint256& startHash, int32 n, std::vector<CMvDbpBlock>& blocks);
    bool IsEmpty(const uint256& hash);
    bool IsForkHash(const uint256& hash);

    bool IsMyFork(const uint256& hash);
    bool IsForkNodeOfSuperNode();
    bool IsRootNodeOfSuperNode();
    
    // from down node
    void HandleGetBlocks(CMvEventDbpMethod& event);
    void HandleGetTransaction(CMvEventDbpMethod& event);
    void HandleSendTransaction(CMvEventDbpMethod& event);
    void HandleSendEvent(CMvEventDbpMethod& event);

    bool IsTopicExist(const std::string& topic);

    void SubTopic(const std::string& id, const std::string& session, const std::string& topic);
    void UnSubTopic(const std::string& id);
    void RemoveSession(const std::string& session);

    void PushBlock(const std::string& forkid, const CMvDbpBlock& block);
    void PushTx(const std::string& forkid, const CMvDbpTransaction& dbptx);
    bool PushEvent(const CMvDbpVirtualPeerNetEvent& event);
   
    void SendEventToParentNode(CMvDbpVirtualPeerNetEvent& event);
    void UpdateGetDataEventRecord(const CMvEventPeerGetData& event);
    bool IsThisNodeData(const uint256& hashFork, uint64 nNonce, const uint256& dataHash);

    void FilterChildSubscribeFork(const CMvEventPeerSubscribe& in, CMvEventPeerSubscribe& out);
    void FilterChildUnsubscribeFork(const CMvEventPeerUnsubscribe& in, CMvEventPeerUnsubscribe& out);
    void FilterThisSubscribeFork(const CMvEventPeerSubscribe& in, CMvEventPeerSubscribe& out);
    void FilterThisUnsubscribeFork(const CMvEventPeerUnsubscribe& in, CMvEventPeerUnsubscribe& out);

    void RespondFailed(CMvEventDbpConnect& event);
    void RespondConnected(CMvEventDbpConnect& event);
    void RespondNoSub(CMvEventDbpSub& event);
    void RespondReady(CMvEventDbpSub& event);


protected:
    walleve::IIOProc* pDbpServer;
    walleve::IIOProc* pDbpClient;
    walleve::IIOProc* pVirtualPeerNet;
    IService* pService;
    ICoreProtocol* pCoreProtocol;
    IWallet* pWallet;
    IMvNetChannel* pNetChannel;
    walleve::IIOModule* pRPCMod;

private:
    std::map<std::string, ForksType> mapSessionChildNodeForks; // session => child node forks

    std::map<std::string, std::string> mapIdSubedSession;       // id => session
    std::unordered_map<std::string, IdsType> mapTopicIds;       // topic => ids

    std::unordered_map<std::string, std::pair<uint256,uint256>> mapForkPoint; // fork point hash => (fork hash, fork point hash)

    bool fEnableForkNode;
    bool fEnableSuperNode;

    
    std::map<uint64, CMvDbpVirtualPeerNetEvent> mapPeerEvent;

    /*Event router*/
    typedef std::pair<uint256, uint64> ForkNonceKeyType;
    std::map<ForkNonceKeyType, int> mapChildNodeForkCount;
    std::map<ForkNonceKeyType, int> mapThisNodeForkCount;
    std::map<ForkNonceKeyType, std::set<uint256>> mapThisNodeGetData; 
};

} // namespace multiverse

#endif //MULTIVERSE_DBP_SERVICE_H
