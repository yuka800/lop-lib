// ------------------------------------------------------------------------------
//  <autogenerated>
//      This code was generated by a tool.
//      Changes to this file may cause incorrect behavior and will be lost if 
//      the code is regenerated.
//  </autogenerated>
// ------------------------------------------------------------------------------

#include "groupInfo.pe.h"
#include <BCLib/utility/convert.h>
#include <BCLib/utility/logFile.h>

namespace PTExt
{

CGroupInfo::CGroupInfo()
{
}

CGroupInfo::CGroupInfo(const PTBuf::CGroupInfo& ptBuf)
{
    m_ptBuf = ptBuf;
}

CGroupInfo::~CGroupInfo()
{
}

PTBuf::CGroupInfo& CGroupInfo::getBuf()
{
    return m_ptBuf;
}

void CGroupInfo::dumpInfo()
{
    BCLIB_LOG_TRACE(BCLib::ELOGMODULE_DEFAULT, "DumpInfo : groupid = %llu, groupname = %s, accountid = %llu, grouptype = %u, groupstate = %u, permission = %s, members = %s", m_ptBuf.groupid(), m_ptBuf.groupname().c_str(), m_ptBuf.accountid(), m_ptBuf.grouptype(), m_ptBuf.groupstate(), m_ptBuf.permission().c_str(), m_ptBuf.members().c_str());
}

bool CGroupInfo::isValidity(PTBuf::CGroupInfo& ptBuf)
{
    std::string strValue = "";

    // string
    strValue = "";
    BCLib::Utility::CConvert::utf8ToGB2312(strValue, ptBuf.groupname());
    if (strValue.size() > 64) return false;
    // ushort
    if (ptBuf.grouptype() > BCLIB_UINT16_MAX) return false;
    // byte
    if (ptBuf.groupstate() > BCLIB_UINT8_MAX) return false;
    // string
    strValue = "";
    BCLib::Utility::CConvert::utf8ToGB2312(strValue, ptBuf.permission());
    if (strValue.size() > 4096) return false;
    // string
    strValue = "";
    BCLib::Utility::CConvert::utf8ToGB2312(strValue, ptBuf.members());
    if (strValue.size() > 4096) return false;

    return true;
}

bool CGroupInfo::makeValidity(PTBuf::CGroupInfo& ptBuf)
{
    std::string strValue = "";

    // string
    strValue = "";
    BCLib::Utility::CConvert::utf8ToGB2312(strValue, ptBuf.groupname());
    if (strValue.size() > 64) strValue = strValue.substr(0, 64);
    BCLib::Utility::CConvert::gb2312ToUTF8(strValue, strValue);
    ptBuf.set_groupname(strValue);
    // ushort
    if (ptBuf.grouptype() > BCLIB_UINT16_MAX) ptBuf.set_grouptype(BCLIB_UINT16_MAX);
    // byte
    if (ptBuf.groupstate() > BCLIB_UINT8_MAX) ptBuf.set_groupstate(BCLIB_UINT8_MAX);
    // string
    strValue = "";
    BCLib::Utility::CConvert::utf8ToGB2312(strValue, ptBuf.permission());
    if (strValue.size() > 4096) strValue = strValue.substr(0, 4096);
    BCLib::Utility::CConvert::gb2312ToUTF8(strValue, strValue);
    ptBuf.set_permission(strValue);
    // string
    strValue = "";
    BCLib::Utility::CConvert::utf8ToGB2312(strValue, ptBuf.members());
    if (strValue.size() > 4096) strValue = strValue.substr(0, 4096);
    BCLib::Utility::CConvert::gb2312ToUTF8(strValue, strValue);
    ptBuf.set_members(strValue);

    return true;
}

bool CGroupInfo::canUpdate(PTBuf::CGroupInfo& oldBuf, PTBuf::CGroupInfo& newBuf)
{
    return true;
}

BCLIB_SINGLETON_DEFINE(CGroupInfoOwner)

CGroupInfoOwner::CGroupInfoOwner()
{
}

CGroupInfoOwner::~CGroupInfoOwner()
{
}

bool CGroupInfoOwner::addObject(BCLib::uint64 key, const PTBuf::CGroupInfo& ptBuf)
{
    if (m_hashMap.find(key) != m_hashMap.end()) return false;

    PTExt::CGroupInfo tmpBuf(ptBuf);
    m_hashMap.setValue(key, tmpBuf);
    return true;
}

bool CGroupInfoOwner::addObject(BCLib::uint64 key, const PTExt::CGroupInfo& ptBuf)
{
    if (m_hashMap.find(key) != m_hashMap.end()) return false;

    m_hashMap.setValue(key, ptBuf);
    return true;
}

PTExt::CGroupInfo* CGroupInfoOwner::getObject(BCLib::uint64 entityID)
{
    if (entityID == 0) return NULL;

    THashMap::iterator it = m_hashMap.find(entityID);
    if (it != m_hashMap.end())
    {
        return &it->second;
    }

    return NULL;
}

bool CGroupInfoOwner::delObject(BCLib::uint64 entityID)
{
    if (entityID == 0) return false;

    THashMap::iterator it = m_hashMap.find(entityID);
    if (it != m_hashMap.end())
    {
        m_hashMap.erase(it);
        return true;
    }

    return true;
}

void CGroupInfoOwner::dumpInfo(BCLib::uint64 entityID)
{
    if (entityID == 0) return;

    THashMap::iterator it = m_hashMap.find(entityID);
    if (it != m_hashMap.end())
    {
        it->second.dumpInfo();
    }
}


CGroupInfoList::CGroupInfoList()
{
}

CGroupInfoList::CGroupInfoList(const PTBuf::CGroupInfoList& ptList)
{
    m_List = ptList;
}

CGroupInfoList::~CGroupInfoList()
{
}

void CGroupInfoList::dumpInfo()
{
    for(int i = 0; i < m_List.objects_size(); i++)
    {
        CGroupInfo tmpBuf(m_List.objects(i));
        tmpBuf.dumpInfo();
    }
}

PTBuf::CGroupInfoList& CGroupInfoList::getList()
{
    return m_List;
}


BCLIB_SINGLETON_DEFINE(CGroupInfoListCtrl)

CGroupInfoListCtrl::CGroupInfoListCtrl()
{
}

CGroupInfoListCtrl::~CGroupInfoListCtrl()
{
}


BCLIB_SINGLETON_DEFINE(CGroupInfoListOwner)

CGroupInfoListOwner::CGroupInfoListOwner()
{
}

CGroupInfoListOwner::~CGroupInfoListOwner()
{
}

bool CGroupInfoListOwner::addObject(BCLib::uint64 entityID, const PTBuf::CGroupInfoList& ptList)
{
    if (m_hashMap.find(entityID) != m_hashMap.end()) return false;

    PTExt::CGroupInfoList tmpList(ptList);
    m_hashMap.setValue(entityID, tmpList);
    return true;
}

bool CGroupInfoListOwner::addObject(BCLib::uint64 entityID, const PTExt::CGroupInfoList& ptList)
{
    if (m_hashMap.find(entityID) != m_hashMap.end()) return false;

    m_hashMap.setValue(entityID, ptList);
    return true;
}

PTExt::CGroupInfoList* CGroupInfoListOwner::getObject(BCLib::uint64 entityID)
{
    if (entityID == 0) return NULL;

    THashMap::iterator it = m_hashMap.find(entityID);
    if (it != m_hashMap.end())
    {
        return &it->second;
    }

    return NULL;
}

bool CGroupInfoListOwner::delObject(BCLib::uint64 entityID)
{
    if (entityID == 0) return false;

    THashMap::iterator it = m_hashMap.find(entityID);
    if (it != m_hashMap.end())
    {
        m_hashMap.erase(it);
        return true;
    }

    return true;
}

void CGroupInfoListOwner::dumpInfo(BCLib::uint64 entityID)
{
    if (entityID == 0) return;

    THashMap::iterator it = m_hashMap.find(entityID);
    if (it != m_hashMap.end())
    {
        it->second.dumpInfo();
    }
}


} // namespace PTExt
