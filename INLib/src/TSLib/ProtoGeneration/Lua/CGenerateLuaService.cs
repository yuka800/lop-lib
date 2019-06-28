﻿using System.Collections.Generic;
using System.IO;

namespace TSLib.ProtoGeneration
{
    public class CGenerateLuaService
    {
        private CNameUnit m_nameUnit;
        private string m_writePath;

        private FileStream m_file;
        private StreamWriter m_write;
        private List<string> m_service_reigster_callback_list;
        private List<string> m_resMsgList;
        private string m_serviceBaseName;

        /// <summary>
        /// 输出模块系统消息处理文件. eg:MailServiceBase.lua
        /// </summary>
        public CGenerateLuaService(CNameUnit nameUnit, string writeRoot)
        {
            m_nameUnit = nameUnit;
            m_serviceBaseName = m_nameUnit.ModuleService;
            m_writePath = writeRoot + m_serviceBaseName + ".lua";
            m_service_reigster_callback_list = new List<string>();
            m_resMsgList = new List<string>();
        }

        public void StartWrite(List<SMsgID> systemList)
        {
            m_file = new FileStream(m_writePath, FileMode.OpenOrCreate);
            m_write = new StreamWriter(m_file);

            Write_Head();
            m_write.WriteLine();
            Write_Ctor();
            m_write.WriteLine();
            Write_OnRegister();
            m_write.WriteLine();
            Write_RegisterNetMsgHandler(systemList);
            m_write.WriteLine();
            Write_AddMsgExecFunc();
            m_write.WriteLine();
            Write_RegisterNetMsgFactory();
            m_write.WriteLine();
            Write_AddMsgObj();
            m_write.WriteLine();
            Write_SendMessage();
            m_write.WriteLine();
            Write_OnMessageCallbacks(systemList);
            m_write.WriteLine();
            Write_Finish();

            m_resMsgList.Clear();
            m_write.Close();
        }
        
        private void Write_Head()
        {
            m_write.WriteLine("-- ------------------------------------------------------------------------------");
            m_write.WriteLine("--  <autogenerated>");
            m_write.WriteLine("--      This code was generated by a tool.");
            m_write.WriteLine("--      Changes to this file may cause incorrect behavior and will be lost if ");
            m_write.WriteLine("--      the code is regenerated.");
            m_write.WriteLine("--      Below starts {0}", m_serviceBaseName);
            m_write.WriteLine("--      You need derive from this serivce class.", m_serviceBaseName);
            m_write.WriteLine("--  </autogenerated>");
            m_write.WriteLine("-- ------------------------------------------------------------------------------");
            m_write.WriteLine("");
            m_write.WriteLine("local {0} = class(\"{0}\", require(\"LULib.Framework.Service.BaseService\"))", m_serviceBaseName);
        }

        private void Write_Ctor()
        {
            m_write.WriteLine("function {0}:ctor()", m_serviceBaseName);
            m_write.WriteLine("    {0}.super.ctor(self)", m_serviceBaseName);
            m_write.WriteLine("end");
        }

        private void Write_OnRegister()
        {
            m_write.WriteLine("function {0}:onRegister()", m_serviceBaseName);
            m_write.WriteLine("    self._systemName = \"{0}\"", m_serviceBaseName);
            m_write.WriteLine();
            m_write.WriteLine("    NetworkManager:getInstance():registerNetworkSystem(self._systemName, CGatewayServerClt.Instance.TcpClient, message_pb.EFUNC_{0})", m_nameUnit.MoudleSysUpper);
            m_write.WriteLine();
            m_write.WriteLine("    self:registerNetMsgFactory()");
            m_write.WriteLine("    self:registerNetMsgHandler()");
            m_write.WriteLine("end");
        }

        private void Write_RegisterNetMsgHandler(List<SMsgID> systemList)
        {
            m_write.WriteLine("function {0}:registerNetMsgHandler()", m_serviceBaseName);
            foreach (SMsgID item in systemList)
            {
                MsgType msgType = CHelper.FindMsgType(item.enumName);
                if (msgType == MsgType.Recv)
                {
                    string msgTypeDetail = item.enumName.Replace(CConst.CLIENT_RECV_HEAD, string.Empty);
                    m_resMsgList.Add(item.enumName);
                    msgTypeDetail = msgTypeDetail.ToLower();
                    string transferServiceName = "onCT2GC" + CHelper.ConvertUnderlineStrToCamel(msgTypeDetail);
                    m_service_reigster_callback_list.Add(transferServiceName);
                    m_write.WriteLine("    self:addMsgExecFunc({0}_pb.{1}, function(reqLabel, data) " + "ErrorManager:processError({0}_pb, debug.getinfo(1), data) " + "self:{2}(reqLabel, data) end)",  m_nameUnit.MoudleSysName, item.enumName, transferServiceName);
                }
            }
            m_write.WriteLine("end");
        }

        private void Write_RegisterNetMsgFactory()
        {
            m_write.WriteLine("function {0}:registerNetMsgFactory()", m_serviceBaseName);
            foreach (string msgName in m_resMsgList)
            {
                string msgTypeDetail = msgName.Replace(CConst.CLIENT_RECV_HEAD, string.Empty);
                msgTypeDetail = msgTypeDetail.ToLower();
                string msgDetailFinal = CHelper.ConvertUnderlineStrToCamel(msgTypeDetail);
                m_write.WriteLine("    self:addMsgObject({0}_pb.{1}, {2}.MsgCT2GC{3}.New())", m_nameUnit.MoudleSysName, msgName, m_nameUnit.MoudleSysMsg, msgDetailFinal);
            }
            m_write.WriteLine("end");
        }

        private void Write_AddMsgExecFunc()
        {
            m_write.WriteLine("--");
            m_write.WriteLine("function {0}:addMsgExecFunc(msgID, msgExecObj)", m_serviceBaseName);
            m_write.WriteLine("    NetworkManager:getInstance():registerResponseHandler(self._systemName, NetworkConst.EServerType.ESERVER_GAMECLIENT, message_pb.EFUNC_" + m_nameUnit.MoudleSysUpper + ", msgID, msgExecObj)");
            m_write.WriteLine("end");
        }

        private void Write_AddMsgObj()
        {
            m_write.WriteLine("--");
            m_write.WriteLine("function {0}:addMsgObject(msgID, msgObject)", m_serviceBaseName);
            m_write.WriteLine("    NetMsgFactory:getInstance():AddMsgObj(NetworkConst.EServerType.ESERVER_GAMECLIENT, message_pb.EFUNC_" + m_nameUnit.MoudleSysUpper + ", msgID, msgObject)");
            m_write.WriteLine("end");
        }

        private void Write_SendMessage()
        {
            m_write.WriteLine("--");
            m_write.WriteLine("function {0}:sendMessage(msgData)", m_serviceBaseName);
            m_write.WriteLine("    NetworkManager:getInstance():sendMessage(self._systemName, msgData)");
            m_write.WriteLine("end");
        }

        private void Write_OnMessageCallbacks(List<SMsgID> systemList)
        {
            foreach (var callback_name in m_service_reigster_callback_list)
            {
                m_write.WriteLine("--");
                m_write.WriteLine("function {0}:{1}(reqLabel, data)", m_serviceBaseName, callback_name);
                m_write.WriteLine("    CommonUtil:throwNotImplementedException(\"[Lua-Net]{0}.{1}  not implemented.\")", m_serviceBaseName, callback_name);
                m_write.WriteLine("end");
            }
        }

        private void Write_Finish()
        {
            m_write.WriteLine("return {0}", m_serviceBaseName);
        }
    }
}