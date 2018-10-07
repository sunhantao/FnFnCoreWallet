// Copyright (c) 2017-2018 The Multiverse developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef  MULTIVERSE_RPCCLIENT_H
#define  MULTIVERSE_RPCCLIENT_H

#include <string>
#include <vector>
#include <boost/asio.hpp>

#include "mvbase.h"
#include "walleve/walleve.h"
#include "json/json_spirit_value.h"
#include "rpc/rpc.h"

#ifdef WIN32
    typedef boost::asio::windows::stream_handle stream_desc;
#else
    typedef boost::asio::posix::stream_descriptor stream_desc;
#endif

namespace multiverse
{

class CRPCClient : public walleve::IIOModule, virtual public walleve::CWalleveHttpEventListener
{
public:
    CRPCClient(bool fConsole = true);
    ~CRPCClient();
    void DispatchLine(const std::string& strLine);

protected:
    bool WalleveHandleInitialize();
    void WalleveHandleDeinitialize();
    bool WalleveHandleInvoke();
    void WalleveHandleHalt();
    const CMvRPCClientConfig * WalleveConfig();

    bool HandleEvent(walleve::CWalleveEventHttpGetRsp& event);
    bool GetResponse(uint64 nNonce, const std::string& content);
    bool CallRPC(rpc::CRPCParamPtr spParam, int nReqId);
    bool CallConsoleCommand(const std::vector<std::string>& vCommand);
    void LaunchConsole();
    void LaunchCommand();
    void CancelCommand();

    void WaitForChars();
    void HandleRead(const boost::system::error_code& err, size_t nTransferred);
    void EnterLoop();
    void LeaveLoop();
    void ConsoleHandleLine(const std::string& strLine);;

protected:
    walleve::IIOProc *pHttpGet;
    walleve::CWalleveThread thrDispatch;
    std::vector<std::string> vArgs;
    uint64 nLastNonce;
    walleve::CIOCompletion ioComplt;
    boost::asio::io_service ioService;
    boost::asio::io_service::strand ioStrand;
    stream_desc inStream;
    boost::asio::mutable_buffer bufRead;
    bool fReading;
};

} // namespace multiverse
#endif //MULTIVERSE_RPCCLIENT_H

