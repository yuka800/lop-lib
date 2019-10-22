//////////////////////////////////////////////////////////////////////
//  created:    2011/11/01
//  filename:   SFLib/commonServer/netClient/tcpClient.h
//  author:     League of Perfect
/// @brief
///
//////////////////////////////////////////////////////////////////////
#ifndef __SFLIB_COMMONSERVER_NETCLIENT_TCPCLIENT_H__
#define __SFLIB_COMMONSERVER_NETCLIENT_TCPCLIENT_H__

#include <BCLib/network/tcp/tcpClient.h>
#include <BCLib/framework/msgExecMgr.h>
#include <SFLib/message/gameFrame.h>
#include <SFLib/commonServer/msgLabel.h>

namespace SFLib
{
namespace CommonServer
{
class CTcpClient : public BCLib::Framework::CMsgExecMgr, public BCLib::Network::CTcpClient
{
public:
    CTcpClient();
    virtual ~CTcpClient();

    /// @brief ����peerID��Ҫ������Ϣ����ʧ�ܵ�ʱ�򴫸��߼���ʹ��
    /// @return �����ֽ��������ֵ
    int send(PeerID peerID, const SFLib::Message::CNetMessage* msg);
    /// @brief ����peerID��Ҫ������Ϣ����ʧ�ܵ�ʱ�򴫸��߼���ʹ��
    /// @return �����ֽ��������ֵ
    int send(PeerID peerID, const SFLib::Message::SNetMessage* msg, const BCLib::uint32 msgSize);

    void setServerType(EServerType serverType)
    {
        m_serverType = serverType;
    }
    EServerType getServerType()  const
    {
        return m_serverType;
    }

    void setServerID(ServerID serverID)
    {
        m_serverID = serverID;
    }
    ServerID getServerID() const
    {
        return m_serverID;
    }

    /// @brief ���������Ƿ��Ѿ���֤�Ϸ��ԣ�ֻ�����յ� SMsgMS2XSNtfVerifySuccess ��Ϣ�󣬲Ż�������֤ͨ��
    /// @return bool
    bool isVerified() const
    {
        return m_verified && m_serverType != ESERVER_UNKNOW && m_serverID != 0;
    }

    std::string getDebugPrompt();

protected:
    void _setVerified(bool verify)
    {
        m_verified = verify;
    }

    virtual bool _cbParseMsg(const void* msgBuff, BCLib::uint32 msgSize);
    virtual void _unhandledMsg(SSFMsgLabel& msgLabel, SFLib::Message::SNetMessage* msg, BCLib::uint32 msgSize);
    virtual bool _transformMsg(PeerID peerID, SFLib::Message::SNetMessage* msg, BCLib::uint32 msgSize);

    virtual bool _createMsgExecPtr(BCLib::uint16 type, BCLib::uint16 id, BCLib::Framework::CMsgExecPtr& msgExecPtr);
    virtual bool _createMsgExecPtr(BCLib::uint16 type, BCLib::Framework::CMsgExecPtr& msgExecPtr);
    virtual void _onXX2XSNtfServerType(BCLib::Framework::SThdMsgLabel* msgLabel, BCLib::Framework::SMessage* msg);

private:
    EServerType m_serverType;
    ServerID m_serverID;
    bool m_verified;
};
typedef BCLib::Utility::CSPointer<CTcpClient> CTcpClientPtr;
}//CommonServer
}//SFLib

#endif//__SFLIB_COMMONSERVER_NETCLIENT_TCPCLIENT_H__